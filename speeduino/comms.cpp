/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
/** @file
   * Process Incoming and outgoing serial communications.
 */
#include "globals.h"
#include "comms.h"
#include "comms_secondary.h"
#include "storage.h"
#include "maths.h"
#include "utilities.h"
#include "decoders.h"
#include "TS_CommandButtonHandler.h"
#include "pages.h"
#include "page_crc.h"
#include "logger.h"
#include "comms_legacy.h"
#include "src/FastCRC/FastCRC.h"
#include <avr/pgmspace.h>
#ifdef RTC_ENABLED
  #include "rtc_common.h"
  #include "comms_sd.h"
#endif
#ifdef SD_LOGGING
  #include "SD_logger.h"
#endif

/** @defgroup group-serial-comms-impl Serial comms implementation
 * @{
 */

// Forward declarations

/** @brief Processes a message once it has been fully received */
void processSerialCommand(void);

/** @brief Should be called when ::serialStatusFlag == SERIAL_TRANSMIT_TOOTH_INPROGRESS, */
void sendToothLog(void);

/** @brief Should be called when ::serialStatusFlag == LOG_SEND_COMPOSITE */
void sendCompositeLog(void);

/// @defgroup group-serial-return-codes Serial return codes sent to TS
/// @{
static constexpr byte SERIAL_RC_OK         = 0x00U; //!< Success
static constexpr byte SERIAL_RC_REALTIME   = 0x01U; //!< Unused
static constexpr byte SERIAL_RC_PAGE       = 0x02U; //!< Unused
static constexpr byte SERIAL_RC_BURN_OK    = 0x04U; //!< EEPROM write succeeded
static constexpr byte SERIAL_RC_TIMEOUT    = 0x80U; //!< Timeout error
static constexpr byte SERIAL_RC_CRC_ERR    = 0x82U; //!< CRC mismatch
static constexpr byte SERIAL_RC_UKWN_ERR   = 0x83U; //!< Unknown command
static constexpr byte SERIAL_RC_RANGE_ERR  = 0x84U; //!< Incorrect range. TS will not retry command
static constexpr byte SERIAL_RC_BUSY_ERR   = 0x85U; //!< TS will wait and retry
///@}

static constexpr uint8_t SEND_OUTPUT_CHANNELS = 48U; //!< Code for the "send output channels command"

#if defined(RTC_ENABLED) && defined(SD_LOGGING)
  #define COMMS_SD            
#endif

/// @defgroup group-serial-hard-coded-responses Hard coded response for some TS messages
/// @{
static constexpr byte serialVersion[] PROGMEM = {SERIAL_RC_OK, '0', '0', '2'};
static constexpr byte canId[] PROGMEM = {SERIAL_RC_OK, 0};
static constexpr byte codeVersion[] PROGMEM = { SERIAL_RC_OK, 's','p','e','e','d','u','i','n','o',' ','2','0','2','4','0','5','-','d','e','v'} ; //Note no null terminator in array and status variable at the start
static constexpr byte productString[] PROGMEM = { SERIAL_RC_OK, 'S', 'p', 'e', 'e', 'd', 'u', 'i', 'n', 'o', ' ', '2', '0', '2', '4', '.', '0', '5', '-', 'd', 'e', 'v'};
//static constexpr byte codeVersion[] PROGMEM = { SERIAL_RC_OK, 's','p','e','e','d','u','i','n','o',' ','2','0','2','4','0','2'} ; //Note no null terminator in array and status variable at the start
//static constexpr byte productString[] PROGMEM = { SERIAL_RC_OK, 'S', 'p', 'e', 'e', 'd', 'u', 'i', 'n', 'o', ' ', '2', '0', '2', '4', '.', '0', '2'};
static constexpr byte testCommsResponse[] PROGMEM = { SERIAL_RC_OK, 255 };
/// @}

/** 
 * @brief The number of bytes received or transmitted to date during nonblocking I/O.
 * @note We can share one variable between rx & tx because we only support simplex serial comms. 
 * I.e. we can only be receiving or transmitting at any one time.
 */
static uint16_t serialBytesRxTx = 0; 

static constexpr uint16_t SERIAL_TIMEOUT = 700; //!< Timeout threshold in milliseconds
static uint32_t serialReceiveStartTime = 0; //!< The time in milliseconds at which the serial receive started. Used for calculating whether a timeout has occurred

static FastCRC32 CRC32_serial; //!< Support accumulation of a CRC during non-blocking operations
static FastCRC32 CRC32_calibration; //!< Support accumulation of a CRC during calibration loads. Must be a separate instance to CRC32_serial due to calibration data being sent in multiple packets
using crc_t = uint32_t;

#ifdef COMMS_SD
#undef SERIAL_BUFFER_SIZE
/** @brief Serial payload buffer must be significantly larger for boards that support SD logging.
 * 
 * Large enough to contain 4 sectors + overhead 
 */
#define SERIAL_BUFFER_SIZE (2048 + 3)
static uint16_t SDcurrentDirChunk;
static uint32_t SDreadStartSector;
static uint32_t SDreadNumSectors;
static uint32_t SDreadCompletedSectors = 0;
#endif
static uint8_t serialPayload[SERIAL_BUFFER_SIZE]; //!< Serial payload buffer
static uint16_t serialPayloadLength = 0; //!< How many bytes in serialPayload were received or sent
Stream* pPrimarySerial;

#if defined(CORE_AVR)
#pragma GCC push_options
// This minimizes RAM usage at no performance cost
#pragma GCC optimize ("Os") 
#endif

/** @brief Has the current receive operation timed out? */
static inline bool isRxTimeout(void) {
  return (millis() - serialReceiveStartTime) > SERIAL_TIMEOUT;
}

// ====================================== Endianness Support =============================

/**
 * @brief Flush all remaining bytes from the rx serial buffer
 */
void flushRXbuffer(void)
{
  while (primarySerial.available() > 0) { primarySerial.read(); }
}

/** @brief Reverse the byte order of a uint32_t
 * 
 * @attention noinline is needed to prevent enlarging callers stack frame, which in turn throws
 * off free ram reporting.
 * */
static __attribute__((noinline)) uint32_t reverse_bytes(uint32_t i)
{
  return  ((i>>24) & 0xffU) | // move byte 3 to byte 0
          ((i<<8 ) & 0xff0000U) | // move byte 1 to byte 2
          ((i>>8 ) & 0xff00U) | // move byte 2 to byte 1
          ((i<<24) & 0xff000000U);
}

// ====================================== Blocking IO Support ================================

void writeByteReliableBlocking(byte value) {
  // Some platforms (I'm looking at you Teensy 3.5) do not mimic the Arduino 1.0
  // contract which synchronously blocks. 
  // https://github.com/PaulStoffregen/cores/blob/master/teensy3/usb_primarySerial.c#L215
  while (!primarySerial.availableForWrite()) { /* Wait for the buffer to free up space */ }
  primarySerial.write(value);
}

// ====================================== Multibyte Primitive Type IO Support =============================

/** @brief Read from the serial port into the supplied buffer 
 * @attention The buffer is filled in reverse, since TS comms is little-endian.
*/
static void readSerialTimeout(char *buffer, size_t length) {
  // Teensy 3.5: Serial.available() should only be used as a boolean test
  // See https://www.pjrc.com/teensy/td_serial.html#singlebytepackets
  while (length>0U) {
    if (primarySerial.available()!=0) {
      buffer[--length] =(byte)primarySerial.read();
    } else if(isRxTimeout()) {
      return;
    } else { /* MISRA - no-op */ }
  }
}

/**
 * @brief Reads an integral type, timing out if necessary
 * 
 * @tparam TIntegral The integral type. E.g. uint16_t 
 */
template <typename TIntegral>
static __attribute__((noinline)) TIntegral readSerialIntegralTimeout(void) {
  // We use type punning to read into a buffer and convert to the appropriate type
  union {
    char raw[sizeof(TIntegral)];
    TIntegral value;
  } buffer;
  readSerialTimeout(buffer.raw, sizeof(buffer.raw));

  if(isRxTimeout()) {
    return TIntegral();
  }
  return buffer.value;
}

/** @brief Write a uint32_t to Serial 
 * @returns The value as transmitted on the wire
*/
static uint32_t serialWrite(uint32_t value)
{
  value = reverse_bytes(value);
  const byte *pBuffer = (const byte*)&value;
  writeByteReliableBlocking(pBuffer[0]);
  writeByteReliableBlocking(pBuffer[1]);
  writeByteReliableBlocking(pBuffer[2]);
  writeByteReliableBlocking(pBuffer[3]);
  return value;
}

/** @brief Write a uint16_t to Serial */
static void serialWrite(uint16_t value)
{
  writeByteReliableBlocking((value >> 8U) & 255U);
  writeByteReliableBlocking(value & 255U);
}

// ====================================== Non-blocking IO Support =============================

/** @brief Send as much data as possible without blocking the caller
 * @return Number of bytes sent
 */
static uint16_t writeNonBlocking(const byte *buffer, size_t length)
{
  uint16_t bytesTransmitted = 0;

  while (bytesTransmitted<length 
        && primarySerial.availableForWrite() != 0 
        // Just in case
        && primarySerial.write(buffer[bytesTransmitted]) == 1)
  {
    bytesTransmitted++;
  }

  return bytesTransmitted;

  // This doesn't work on Teensy.
  // See https://github.com/PaulStoffregen/cores/issues/10#issuecomment-61514955
  // size_t capacity = min((size_t)Serial.availableForWrite(), length);
  // return Serial.write(buffer, capacity);
}

/** @brief Write a uint32_t to Serial without blocking the caller
 * @return Number of bytes sent
 */
static size_t writeNonBlocking(size_t start, uint32_t value)
{
  value = reverse_bytes(value);
  const byte *pBuffer = (const byte*)&value;
  return writeNonBlocking(pBuffer+start, sizeof(value)-start); //cppcheck-suppress misra-c2012-17.2
}


/** @brief Send the buffer, followed by it's CRC
 * 
 * This is supposed to be called multiple times for the same buffer until
 * it's all sent.
 * 
 * @param buffer The buffer
 * @param start Index into the buffer to start sending at. [0, length)
 * @param length Total size of the buffer
 * @return Cumulative total number of bytes written . I.e. the next start value
 */
static uint16_t sendBufferAndCrcNonBlocking(const byte *buffer, size_t start, size_t length)
{
  if (start<length)
  {
    start = start + writeNonBlocking(buffer+start, length-start);
  }
  
  if (start>=length && start<length+sizeof(crc_t))
  {
    start = start + writeNonBlocking(start-length, CRC32_serial.crc32(buffer, length));
  }

  return start;
}

/** @brief Start sending the shared serialPayload buffer.
 * 
 * ::serialStatusFlag will be signal the result of the send:<br>
 * ::serialStatusFlag == SERIAL_INACTIVE: send is complete <br>
 * ::serialStatusFlag == SERIAL_TRANSMIT_INPROGRESS: partial send, subsequent calls to continueSerialTransmission
 * will finish sending serialPayload
 * 
 * @param payloadLength How many bytes to send [0, sizeof(serialPayload))
*/
static void sendSerialPayloadNonBlocking(uint16_t payloadLength)
{
  //Start new transmission session
  serialStatusFlag = SERIAL_TRANSMIT_INPROGRESS;
  serialWrite(payloadLength);
  serialPayloadLength = payloadLength;
  serialBytesRxTx = sendBufferAndCrcNonBlocking(serialPayload, 0, payloadLength);
  serialStatusFlag = serialBytesRxTx==payloadLength+sizeof(crc_t) ? SERIAL_INACTIVE : SERIAL_TRANSMIT_INPROGRESS;
}

// ====================================== TS Message Support =============================

/** @brief Send a message to TS containing only a return code.
 * 
 * This is used when TS asks for an action to happen (E.g. start a logger) or
 * to signal an error condition to TS
 * 
 * @attention This is a blocking operation
 */
static void sendReturnCodeMsg(byte returnCode)
{
  serialWrite((uint16_t)sizeof(returnCode));
  writeByteReliableBlocking(returnCode);
  (void)serialWrite(CRC32_serial.crc32(&returnCode, sizeof(returnCode)));
}

// ====================================== Command/Action Support =============================

// The functions in this section are abstracted out to prevent enlarging callers stack frame, 
// which in turn throws off free ram reporting.

/**
 * @brief Update a pages contents from a buffer
 * 
 * @param pageNum The index of the page to update
 * @param offset Offset into the page
 * @param buffer The buffer to read from
 * @param length The buffer length
 * @return true if page updated successfully
 * @return false if page cannot be updated
 */
static bool updatePageValues(uint8_t pageNum, uint16_t offset, const byte *buffer, uint16_t length)
{
  if ( (offset + length) <= getPageSize(pageNum) )
  {
    for(uint16_t i = 0; i < length; i++)
    {
      setPageValue(pageNum, (offset + i), buffer[i]);
    }
    deferEEPROMWritesUntil = micros() + EEPROM_DEFER_DELAY;
    return true;
  }

  return false;
}

/**
 * @brief Loads a pages contents into a buffer
 * 
 * @param pageNum The index of the page to update
 * @param offset Offset into the page
 * @param buffer The buffer to read from
 * @param length The buffer length
 */
static void loadPageValuesToBuffer(uint8_t pageNum, uint16_t offset, byte *buffer, uint16_t length)
{
  for(uint16_t i = 0; i < length; i++)
  {
    buffer[i] = getPageValue(pageNum, offset + i);
  }
}

/** @brief Send a status record back to tuning/logging SW.
 * This will "live" information from @ref currentStatus struct.
 * @param offset - Start field number
 * @param packetLength - Length of actual message (after possible ack/confirm headers)
 * E.g. tuning sw command 'A' (Send all values) will send data from field number 0, LOG_ENTRY_SIZE fields.
 */
static void generateLiveValues(uint16_t offset, uint16_t packetLength)
{  
  if(firstCommsRequest) 
  { 
    firstCommsRequest = false;
    currentStatus.secl = 0; 
  }

  currentStatus.status2 ^= (-currentStatus.hasSync ^ currentStatus.status2) & (1U << BIT_STATUS2_SYNC); //Set the sync bit of the Spark variable to match the hasSync variable

  serialPayload[0] = SERIAL_RC_OK;
  for(uint16_t x=0; x<packetLength; x++)
  {
    serialPayload[x+1U] = getTSLogEntry(offset+x); 
  }
  // Reset any flags that are being used to trigger page refreshes
  BIT_CLEAR(currentStatus.status3, BIT_STATUS3_VSS_REFRESH);
}

/**
 * @brief Update the oxygen sensor table from serialPayload
 * 
 * @param offset Offset into serialPayload and the table
 * @param chunkSize Number of bytes available in serialPayload
 */
static void loadO2CalibrationChunk(uint16_t offset, uint16_t chunkSize)
{
  using pCrcCalc = uint32_t (FastCRC32::*)(const uint8_t *, const uint16_t, bool);
  // First pass through the loop, we need to INITIALIZE the CRC
  pCrcCalc pCrcFun = offset==0U ? &FastCRC32::crc32 : &FastCRC32::crc32_upd;
  uint32_t calibrationCRC = 0U;

  //Read through the current chunk (Should be 256 bytes long)
  // Note there are 2 loops here: 
  //    [x, chunkSize)
  //    [offset, offset+chunkSize)
  for(uint16_t x = 0; x < chunkSize; ++x, ++offset)
  {
    //TS sends a total of 1024 bytes of calibration data, broken up into 256 byte chunks
    //As we're using an interpolated 2D table, we only need to store 32 values out of this 1024
    if( (x % 32U) == 0U )
    {
      o2Calibration_values[offset/32U] = serialPayload[x+7U]; //O2 table stores 8 bit values
      o2Calibration_bins[offset/32U]   = offset;
    }

    //Update the CRC
    calibrationCRC = (CRC32_calibration.*pCrcFun)(&serialPayload[x+7U], 1, false);
    // Subsequent passes through the loop, we need to UPDATE the CRC
    pCrcFun = &FastCRC32::crc32_upd;
  }
 
  if( offset >= 1023U ) 
  {
    //All chunks have been received (1024 values). Finalise the CRC and burn to EEPROM
    storeCalibrationCRC32(O2_CALIBRATION_PAGE, ~calibrationCRC);
    writeCalibrationPage(O2_CALIBRATION_PAGE);
  }
}

/**
 * @brief Convert 2 bytes into an offset temperature in degrees Celsius
 * @attention Returned value will be offset CALIBRATION_TEMPERATURE_OFFSET
 */
static uint16_t toTemperature(byte lo, byte hi)
{
  int16_t tempValue = (int16_t)(word(hi, lo)); //Combine the 2 bytes into a single, signed 16-bit value
  tempValue = tempValue / 10; //TS sends values multiplied by 10 so divide back to whole degrees. 
  tempValue = ((tempValue - 32) * 5) / 9; //Convert from F to C
  //Apply the temp offset and check that it results in all values being positive
  return max( tempValue + CALIBRATION_TEMPERATURE_OFFSET, 0 );
}

/**
 * @brief Update a temperature calibration table from serialPayload
  * 
 * @param calibrationLength The chunk size received from TS
 * @param calibrationPage Index of the table
 * @param values The table values
 * @param bins The table bin values
 */
static void processTemperatureCalibrationTableUpdate(uint16_t calibrationLength, uint8_t calibrationPage, uint16_t *values, uint16_t *bins)
{
  //Temperature calibrations are sent as 32 16-bit values
  if(calibrationLength == 64U)
  {
    for (uint16_t x = 0; x < 32U; x++)
    {
      values[x] = toTemperature(serialPayload[(2U * x) + 7U], serialPayload[(2U * x) + 8U]);
      bins[x] = (x * 33U); // 0*33=0 to 31*33=1023
    }
    storeCalibrationCRC32(calibrationPage, CRC32_calibration.crc32(&serialPayload[7], 64));
    writeCalibrationPage(calibrationPage);
    sendReturnCodeMsg(SERIAL_RC_OK);
  }
  else 
  { 
    sendReturnCodeMsg(SERIAL_RC_RANGE_ERR); 
  }
}

// ====================================== End Internal Functions =============================


/** Processes the incoming data on the serial buffer based on the command sent.
Can be either data for a new command or a continuation of data for command that is already in progress:

Commands are single byte (letter symbol) commands.
*/
void serialReceive(void)
{
  //Check for an existing legacy command in progress
  if(serialStatusFlag==SERIAL_COMMAND_INPROGRESS_LEGACY)
  {
    legacySerialCommand();
    return;
  }

  if (primarySerial.available()!=0 && serialStatusFlag == SERIAL_INACTIVE)
  { 
    //New command received
    //Need at least 2 bytes to read the length of the command
    char highByte = (char)primarySerial.peek();

    //Check if the command is legacy using the call/response mechanism
    if(highByte == 'F')
    {
      //F command is always allowed as it provides the initial serial protocol version. 
      legacySerialCommand();
      return;
    }
    else if( (((highByte >= 'A') && (highByte <= 'z')) || (highByte == '?')) && (BIT_CHECK(currentStatus.status4, BIT_STATUS4_ALLOW_LEGACY_COMMS)) )
    {
      //Handle legacy cases here
      legacySerialCommand();
      return;
    }
    else
    {
      serialReceiveStartTime = millis();
      serialPayloadLength = readSerialIntegralTimeout<uint16_t>();
      if (!isRxTimeout()) {
        serialBytesRxTx = 0U;
        serialStatusFlag = SERIAL_RECEIVE_INPROGRESS; //Flag the serial receive as being in progress
      }
    }
  }

  //If there is a serial receive in progress, read as much from the buffer as possible or until we receive all bytes
  while( (primarySerial.available() > 0) && (serialStatusFlag == SERIAL_RECEIVE_INPROGRESS) )
  {
    if (serialBytesRxTx < serialPayloadLength )
    {
      serialPayload[serialBytesRxTx] = (byte)primarySerial.read();
      ++serialBytesRxTx;
    }
    else
    {
      uint32_t incomingCrc = readSerialIntegralTimeout<uint32_t>();
      serialStatusFlag = SERIAL_INACTIVE; //The serial receive is now complete

      if (!isRxTimeout()) // CRC read can timeout also!
      {
        if (incomingCrc == CRC32_serial.crc32(serialPayload, serialPayloadLength))
        {
          //CRC is correct. Process the command
          processSerialCommand();
          BIT_CLEAR(currentStatus.status4, BIT_STATUS4_ALLOW_LEGACY_COMMS); //Lock out legacy commands until next power cycle
        }
        else {
          //CRC Error. Need to send an error message
          sendReturnCodeMsg(SERIAL_RC_CRC_ERR);
          flushRXbuffer();
        }
      }
      // else timeout - code below will kick in.
    }
  } //Data in serial buffer and serial receive in progress

  //Check for a timeout
  if( isRxTimeout() )
  {
    serialStatusFlag = SERIAL_INACTIVE; //Reset the serial receive

    flushRXbuffer();
    sendReturnCodeMsg(SERIAL_RC_TIMEOUT);
  } //Timeout
}

void serialTransmit(void)
{
  switch (serialStatusFlag)
  {
    case SERIAL_TRANSMIT_INPROGRESS_LEGACY:
      sendValues(logItemsTransmitted, inProgressLength, SEND_OUTPUT_CHANNELS, Serial, serialStatusFlag);
      break;

    case SERIAL_TRANSMIT_TOOTH_INPROGRESS:
      sendToothLog();
      break;

    case SERIAL_TRANSMIT_TOOTH_INPROGRESS_LEGACY:
      sendToothLog_legacy(logItemsTransmitted);
      break;

    case SERIAL_TRANSMIT_COMPOSITE_INPROGRESS:
      sendCompositeLog();
      break;

    case SERIAL_TRANSMIT_INPROGRESS:
      serialBytesRxTx = sendBufferAndCrcNonBlocking(serialPayload, serialBytesRxTx, serialPayloadLength);
      serialStatusFlag = serialBytesRxTx==serialPayloadLength+sizeof(crc_t) ? SERIAL_INACTIVE : SERIAL_TRANSMIT_INPROGRESS;
      break;

    default: // Nothing to do
      break;
  }
}

void processSerialCommand(void)
{
  switch (serialPayload[0])
  {

    case 'A': // send x bytes of realtime values in legacy support format
      generateLiveValues(0, LOG_ENTRY_SIZE); 
      break;

    case 'b': // New EEPROM burn command to only burn a single page at a time 
      if( (micros() > deferEEPROMWritesUntil)) { writeConfig(serialPayload[2]); } //Read the table number and perform burn. Note that byte 1 in the array is unused
      else { BIT_SET(currentStatus.status4, BIT_STATUS4_BURNPENDING); }
      
      sendReturnCodeMsg(SERIAL_RC_BURN_OK);
      break;

    case 'B': // Same as above, but for the comms compat mode. Slows down the burn rate and increases the defer time
      BIT_SET(currentStatus.status4, BIT_STATUS4_COMMS_COMPAT); //Force the compat mode
      deferEEPROMWritesUntil += (EEPROM_DEFER_DELAY/4); //Add 25% more to the EEPROM defer time
      if( (micros() > deferEEPROMWritesUntil)) { writeConfig(serialPayload[2]); } //Read the table number and perform burn. Note that byte 1 in the array is unused
      else { BIT_SET(currentStatus.status4, BIT_STATUS4_BURNPENDING); }
      
      sendReturnCodeMsg(SERIAL_RC_BURN_OK);
      break;

    case 'C': // test communications. This is used by Tunerstudio to see whether there is an ECU on a given serial port
      (void)memcpy_P(serialPayload, testCommsResponse, sizeof(testCommsResponse) );
      sendSerialPayloadNonBlocking(sizeof(testCommsResponse));
      break;

    case 'd': // Send a CRC32 hash of a given page
    {
      uint32_t CRC32_val = reverse_bytes(calculatePageCRC32( serialPayload[2] ));

      serialPayload[0] = SERIAL_RC_OK;
      (void)memcpy(&serialPayload[1], (byte*)&CRC32_val, sizeof(CRC32_val));
      sendSerialPayloadNonBlocking(5);      
      break;
    }

    case 'E': // receive command button commands
      (void)TS_CommandButtonsHandler(word(serialPayload[1], serialPayload[2]));
      sendReturnCodeMsg(SERIAL_RC_OK);
      break;

    case 'f': //Send serial capability details
      serialPayload[0] = SERIAL_RC_OK;
      serialPayload[1] = 2; //Serial protocol version
      serialPayload[2] = highByte(BLOCKING_FACTOR);
      serialPayload[3] = lowByte(BLOCKING_FACTOR);
      serialPayload[4] = highByte(TABLE_BLOCKING_FACTOR);
      serialPayload[5] = lowByte(TABLE_BLOCKING_FACTOR);
      
      sendSerialPayloadNonBlocking(6);
      break;

    case 'F': // send serial protocol version
      (void)memcpy_P(serialPayload, serialVersion, sizeof(serialVersion) );
      sendSerialPayloadNonBlocking(sizeof(serialVersion));
      break;

    case 'H': //Start the tooth logger
      startToothLogger();
      sendReturnCodeMsg(SERIAL_RC_OK);
      break;

    case 'h': //Stop the tooth logger
      stopToothLogger();
      sendReturnCodeMsg(SERIAL_RC_OK);
      break;

    case 'I': // send CAN ID
      (void)memcpy_P(serialPayload, canId, sizeof(canId) );
      sendSerialPayloadNonBlocking(sizeof(serialVersion));
      break;

    case 'J': //Start the composite logger
      startCompositeLogger();
      sendReturnCodeMsg(SERIAL_RC_OK);
      break;

    case 'j': //Stop the composite logger
      stopCompositeLogger();
      sendReturnCodeMsg(SERIAL_RC_OK);
      break;

    case 'k': //Send CRC values for the calibration pages
    {
      uint32_t CRC32_val = reverse_bytes(readCalibrationCRC32(serialPayload[2])); //Get the CRC for the requested page

      serialPayload[0] = SERIAL_RC_OK;
      (void)memcpy(&serialPayload[1], (byte*)&CRC32_val, sizeof(CRC32_val));
      sendSerialPayloadNonBlocking(5);
      break;
    }

    case 'M':
    {
      //New write command
      //7 bytes required:
      //2 - Page identifier
      //2 - offset
      //2 - Length
      //1 - 1st New value
      if (updatePageValues(serialPayload[2], word(serialPayload[4], serialPayload[3]), &serialPayload[7], word(serialPayload[6], serialPayload[5])))
      {
        sendReturnCodeMsg(SERIAL_RC_OK);    
      }
      else
      {
        //This should never happen, but just in case
        sendReturnCodeMsg(SERIAL_RC_RANGE_ERR);
      }
      break;
    }  

    case 'O': //Start the composite logger 2nd cam (teritary)
      startCompositeLoggerTertiary();
      sendReturnCodeMsg(SERIAL_RC_OK);
      break;

    case 'o': //Stop the composite logger 2nd cam (tertiary)
      stopCompositeLoggerTertiary();
      sendReturnCodeMsg(SERIAL_RC_OK);
      break;

    case 'X': //Start the composite logger 2nd cam (teritary)
      startCompositeLoggerCams();
      sendReturnCodeMsg(SERIAL_RC_OK);
      break;

    case 'x': //Stop the composite logger 2nd cam (tertiary)
      stopCompositeLoggerCams();
      sendReturnCodeMsg(SERIAL_RC_OK);
      break;

    /*
    * New method for sending page values (MS command equivalent is 'r')
    */
    case 'p':
    {
      //6 bytes required:
      //2 - Page identifier
      //2 - offset
      //2 - Length
      uint16_t length = word(serialPayload[6], serialPayload[5]);

      //Setup the transmit buffer
      serialPayload[0] = SERIAL_RC_OK;
      loadPageValuesToBuffer(serialPayload[2], word(serialPayload[4], serialPayload[3]), &serialPayload[1], length);
      sendSerialPayloadNonBlocking(length + 1U);
      break;
    }

    case 'Q': // send code version
      (void)memcpy_P(serialPayload, codeVersion, sizeof(codeVersion) );
      sendSerialPayloadNonBlocking(sizeof(codeVersion));
      break;

    case 'r': //New format for the optimised OutputChannels
    {
      uint8_t cmd = serialPayload[2];
      uint16_t offset = word(serialPayload[4], serialPayload[3]);
      uint16_t length = word(serialPayload[6], serialPayload[5]);
#ifdef COMMS_SD     
      uint16_t SD_arg1 = word(serialPayload[3], serialPayload[4]);
      uint16_t SD_arg2 = word(serialPayload[5], serialPayload[6]);
#endif

      if(cmd == SEND_OUTPUT_CHANNELS) //Send output channels command 0x30 is 48dec
      {
        generateLiveValues(offset, length);
        sendSerialPayloadNonBlocking(length + 1U);
      }
      else if(cmd == 0x0fU)
      {
        //Request for signature
        (void)memcpy_P(serialPayload, codeVersion, sizeof(codeVersion) );
        sendSerialPayloadNonBlocking(sizeof(codeVersion));
      }
#ifdef COMMS_SD
      else if(cmd == SD_RTC_PAGE) //Request to read SD card RTC
      {
        serialPayload[0] = SERIAL_RC_OK;
        serialPayload[1] = rtc_getSecond(); //Seconds
        serialPayload[2] = rtc_getMinute(); //Minutes
        serialPayload[3] = rtc_getHour(); //Hours
        serialPayload[4] = rtc_getDOW(); //Day of week
        serialPayload[5] = rtc_getDay(); //Day of month
        serialPayload[6] = rtc_getMonth(); //Month
        serialPayload[7] = highByte(rtc_getYear()); //Year
        serialPayload[8] = lowByte(rtc_getYear()); //Year
        sendSerialPayloadNonBlocking(9);
      }
      else if(cmd == SD_READWRITE_PAGE) //Request SD card extended parameters
      {
        //SD read commands use the offset and length fields to indicate the request type
        if((SD_arg1 == SD_READ_STAT_ARG1) && (SD_arg2 == SD_READ_STAT_ARG2))
        {
          //Read the status of the SD card
          
          serialPayload[0] = SERIAL_RC_OK;

          serialPayload[1] = currentStatus.TS_SD_Status;
          serialPayload[2] = 0; //Error code
 
          //Sector size = 512
          serialPayload[3] = 2;
          serialPayload[4] = 0;

          //Max blocks (4 bytes)
          uint32_t sectors = sectorCount();
          serialPayload[5] = ((sectors >> 24) & 255);
          serialPayload[6] = ((sectors >> 16) & 255);
          serialPayload[7] = ((sectors >> 8) & 255);
          serialPayload[8] = (sectors & 255);

          //Max roots (Number of files)
          uint16_t numLogFiles = getNextSDLogFileNumber() - 2; // -1 because this returns the NEXT file name not the current one and -1 because TS expects a 0 based index
          serialPayload[9] = highByte(numLogFiles);
          serialPayload[10] = lowByte(numLogFiles);

          //Dir Start (4 bytes)
          serialPayload[11] = 0;
          serialPayload[12] = 0;
          serialPayload[13] = 0;
          serialPayload[14] = 0;

          //Unknown purpose for last 2 bytes
          serialPayload[15] = 0;
          serialPayload[16] = 0;

          sendSerialPayloadNonBlocking(17);

        }
        else if((SD_arg1 == SD_READ_DIR_ARG1) && (SD_arg2 == SD_READ_DIR_ARG2))
        {
          //Send file details
          serialPayload[0] = SERIAL_RC_OK;

          uint16_t logFileNumber = (SDcurrentDirChunk * 16) + 1;
          uint8_t filesInCurrentChunk = 0;
          uint16_t payloadIndex = 1;
          while((filesInCurrentChunk < 16) && (getSDLogFileDetails(&serialPayload[payloadIndex], logFileNumber) == true))
          {
            logFileNumber++;
            filesInCurrentChunk++;
            payloadIndex += 32;
          }
          serialPayload[payloadIndex] = lowByte(SDcurrentDirChunk);
          serialPayload[payloadIndex + 1] = highByte(SDcurrentDirChunk);

          sendSerialPayloadNonBlocking(payloadIndex + 2);
        }
      }
      else if(cmd == SD_READFILE_PAGE)
      {
        //Fetch data from file
        if(SD_arg2 == SD_READ_COMP_ARG2)
        {
          //arg1 is the block number to return
          serialPayload[0] = SERIAL_RC_OK;
          serialPayload[1] = highByte(SD_arg1);
          serialPayload[2] = lowByte(SD_arg1);

          uint32_t currentSector = SDreadStartSector + (SD_arg1 * 4);
          
          int32_t numSectorsToSend = 0;
          if(SDreadNumSectors > SDreadCompletedSectors)
          {
            numSectorsToSend = SDreadNumSectors - SDreadCompletedSectors;
            if(numSectorsToSend > 4) //Maximum of 4 sectors at a time
            {
              numSectorsToSend = 4;
            }
          }
          SDreadCompletedSectors += numSectorsToSend;
          
          if(numSectorsToSend <= 0) { sendReturnCodeMsg(SERIAL_RC_OK); }
          else
          {
            readSDSectors(&serialPayload[3], currentSector, numSectorsToSend); 
            sendSerialPayloadNonBlocking(numSectorsToSend * SD_SECTOR_SIZE + 3);
          }
        }
      }
#endif
      else
      {
        //No other r/ commands should be called
      }
      break;
    }

    case 'S': // send code version
      (void)memcpy_P(serialPayload, productString, sizeof(productString) );
      sendSerialPayloadNonBlocking(sizeof(productString));
      currentStatus.secl = 0; //This is required in TS3 due to its stricter timings
      break;

    case 'T': //Send 256 tooth log entries to Tuner Studios tooth logger
      logItemsTransmitted = 0;
      if(currentStatus.toothLogEnabled == true) { sendToothLog(); } //Sends tooth log values as ints
      else if (currentStatus.compositeTriggerUsed > 0U) { sendCompositeLog(); }
      else { /* MISRA no-op */ }
      break;

    case 't': // receive new Calibration info. Command structure: "t", <tble_idx> <data array>.
    {
      uint8_t cmd = serialPayload[2];
      uint16_t offset = word(serialPayload[3], serialPayload[4]);
      uint16_t calibrationLength = word(serialPayload[5], serialPayload[6]); // Should be 256

      if(cmd == O2_CALIBRATION_PAGE)
      {
        loadO2CalibrationChunk(offset, calibrationLength);
        sendReturnCodeMsg(SERIAL_RC_OK);
        primarySerial.flush(); //This is safe because engine is assumed to not be running during calibration
      }
      else if(cmd == IAT_CALIBRATION_PAGE)
      {
        processTemperatureCalibrationTableUpdate(calibrationLength, IAT_CALIBRATION_PAGE, iatCalibration_values, iatCalibration_bins);
      }
      else if(cmd == CLT_CALIBRATION_PAGE)
      {
        processTemperatureCalibrationTableUpdate(calibrationLength, CLT_CALIBRATION_PAGE, cltCalibration_values, cltCalibration_bins);
      }
      else
      {
        sendReturnCodeMsg(SERIAL_RC_RANGE_ERR);
      }
      break;
    }

    case 'U': //User wants to reset the Arduino (probably for FW update)
      if (resetControl != RESET_CONTROL_DISABLED)
      {
      #ifndef SMALL_FLASH_MODE
        if (serialStatusFlag == SERIAL_INACTIVE) { primarySerial.println(F("Comms halted. Next byte will reset the Arduino.")); }
      #endif

        while (primarySerial.available() == 0) { }
        digitalWrite(pinResetControl, LOW);
      }
      else
      {
      #ifndef SMALL_FLASH_MODE
        if (serialStatusFlag == SERIAL_INACTIVE) { primarySerial.println(F("Reset control is currently disabled.")); }
      #endif
      }
      break;

    case 'w':
    {
#ifdef COMMS_SD
      uint8_t cmd = serialPayload[2];
      uint16_t SD_arg1 = word(serialPayload[3], serialPayload[4]);
      uint16_t SD_arg2 = word(serialPayload[5], serialPayload[6]);
      if(cmd == SD_READWRITE_PAGE)
        { 
          if((SD_arg1 == SD_WRITE_DO_ARG1) && (SD_arg2 == SD_WRITE_DO_ARG2))
          {
            /*
            SD DO command. Single byte of data where the commands are:
            0 Reset
            1 Reset
            2 Stop logging
            3 Start logging
            4 Load status variable
            5 Init SD card
            */
            uint8_t command = serialPayload[7];
            if(command == 2) { endSDLogging(); manualLogActive = false; }
            else if(command == 3) { beginSDLogging(); manualLogActive = true; }
            else if(command == 4) { setTS_SD_status(); }
            //else if(command == 5) { initSD(); }
            
            sendReturnCodeMsg(SERIAL_RC_OK);
          }
          else if((SD_arg1 == SD_WRITE_DIR_ARG1) && (SD_arg2 == SD_WRITE_DIR_ARG2))
          {
            //Begin SD directory read. Value in payload represents the directory chunk to read
            //Directory chunks are each 16 files long
            SDcurrentDirChunk = word(serialPayload[7], serialPayload[8]);
            sendReturnCodeMsg(SERIAL_RC_OK);
          }
          else if((SD_arg1 == SD_WRITE_READ_SEC_ARG1) && (SD_arg2 == SD_WRITE_READ_SEC_ARG2))
          {
            //Read sector Init? Unsure what this is meant to do however it is sent at the beginning of a Card Format request and requires an OK response
            //Provided the sector being requested is x0 x0 x0 x0, we treat this as a SD Card format request
            if( (serialPayload[7] == 0) && (serialPayload[8] == 0) && (serialPayload[9] == 0) && (serialPayload[10] == 0) )
            {
              //SD Card format request
              formatExFat();
            }
            sendReturnCodeMsg(SERIAL_RC_OK);
          }
          else if((SD_arg1 == SD_WRITE_WRITE_SEC_ARG1) && (SD_arg2 == SD_WRITE_WRITE_SEC_ARG2))
          {
            //SD write sector command
          }
          else if((SD_arg1 == SD_ERASEFILE_ARG1) && (SD_arg2 == SD_ERASEFILE_ARG2))
          {
            //Erase file command
            //We just need the 4 ASCII characters of the file name
            char log1 = serialPayload[7];
            char log2 = serialPayload[8];
            char log3 = serialPayload[9];
            char log4 = serialPayload[10];

            deleteLogFile(log1, log2, log3, log4);
            sendReturnCodeMsg(SERIAL_RC_OK);
          }
          else if((SD_arg1 == SD_SPD_TEST_ARG1) && (SD_arg2 == SD_SPD_TEST_ARG2))
          {
            //Perform a speed test on the SD card
            //First 4 bytes are the sector number to write to
            #if 0
            TODO: Need to write test routine
            uint32_t sector;
            uint8_t sector1 = serialPayload[7];
            uint8_t sector2 = serialPayload[8];
            uint8_t sector3 = serialPayload[9];
            uint8_t sector4 = serialPayload[10];
            sector = (sector1 << 24) | (sector2 << 16) | (sector3 << 8) | sector4;


            //Last 4 bytes are the number of sectors to test
            uint32_t testSize;
            uint8_t testSize1 = serialPayload[11];
            uint8_t testSize2 = serialPayload[12];
            uint8_t testSize3 = serialPayload[13];
            uint8_t testSize4 = serialPayload[14];
            testSize = (testSize1 << 24) | (testSize2 << 16) | (testSize3 << 8) | testSize4; 
            #endif

            sendReturnCodeMsg(SERIAL_RC_OK);

          }
          else if((SD_arg1 == SD_WRITE_COMP_ARG1) && (SD_arg2 == SD_WRITE_COMP_ARG2))
          {
            //Prepare to read a 2024 byte chunk of data from the SD card
            uint8_t sector1 = serialPayload[7];
            uint8_t sector2 = serialPayload[8];
            uint8_t sector3 = serialPayload[9];
            uint8_t sector4 = serialPayload[10];
            //SDreadStartSector = (sector1 << 24) | (sector2 << 16) | (sector3 << 8) | sector4;
            SDreadStartSector = (sector4 << 24) | (sector3 << 16) | (sector2 << 8) | sector1;
            //SDreadStartSector = sector4 | (sector3 << 8) | (sector2 << 16) | (sector1 << 24);

            //Next 4 bytes are the number of sectors to write
            uint8_t sectorCount1 = serialPayload[11];
            uint8_t sectorCount2 = serialPayload[12];
            uint8_t sectorCount3 = serialPayload[13];
            uint8_t sectorCount4 = serialPayload[14];
            SDreadNumSectors = (sectorCount1 << 24) | (sectorCount2 << 16) | (sectorCount3 << 8) | sectorCount4;

            //Reset the sector counter
            SDreadCompletedSectors = 0;

            sendReturnCodeMsg(SERIAL_RC_OK);
          }
        }
        else if(cmd == SD_RTC_PAGE)
        {
          //Used for setting RTC settings
          if((SD_arg1 == SD_RTC_WRITE_ARG1) && (SD_arg2 == SD_RTC_WRITE_ARG2))
          {
            //Set the RTC date/time
            byte second = serialPayload[7];
            byte minute = serialPayload[8];
            byte hour = serialPayload[9];
            //byte dow = serialPayload[10]; //Not used
            byte day = serialPayload[11];
            byte month = serialPayload[12];
            uint16_t year = word(serialPayload[13], serialPayload[14]);
            rtc_setTime(second, minute, hour, day, month, year);
            sendReturnCodeMsg(SERIAL_RC_OK);
          }
        }
#endif
      break;
    }

    default:
      //Unknown command
      sendReturnCodeMsg(SERIAL_RC_UKWN_ERR);
      break;
  }
}

/** 
 * 
*/
void sendToothLog(void)
{
  //We need TOOTH_LOG_SIZE number of records to send to TunerStudio. If there aren't that many in the buffer then we just return and wait for the next call
  if (BIT_CHECK(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY) == false) 
  {
    //If the buffer is not yet full but TS has timed out, pad the rest of the buffer with 0s
    while(toothHistoryIndex < TOOTH_LOG_SIZE)
    {
      toothHistory[toothHistoryIndex] = 0;
      toothHistoryIndex++;
    }
  }

  uint32_t CRC32_val = 0U;
  if(logItemsTransmitted == 0U)
  {
    //Transmit the size of the packet
    (void)serialWrite((uint16_t)(sizeof(toothHistory) + 1U)); //Size of the tooth log (uint32_t values) plus the return code
    //Begin new CRC hash
    const uint8_t returnCode = SERIAL_RC_OK;
    CRC32_val = CRC32_serial.crc32(&returnCode, 1, false);

    //Send the return code
    writeByteReliableBlocking(returnCode);
  }
  
  for (; logItemsTransmitted < TOOTH_LOG_SIZE; logItemsTransmitted++)
  {
    //Check whether the tx buffer still has space
    if(primarySerial.availableForWrite() < 4) 
    { 
      //tx buffer is full. Store the current state so it can be resumed later
      serialStatusFlag = SERIAL_TRANSMIT_TOOTH_INPROGRESS;
      return;
    }

    //Transmit the tooth time
    uint32_t transmitted = serialWrite(toothHistory[logItemsTransmitted]);
    CRC32_val = CRC32_serial.crc32_upd((const byte*)&transmitted, sizeof(transmitted), false);
  }
  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
  serialStatusFlag = SERIAL_INACTIVE;
  toothHistoryIndex = 0;
  logItemsTransmitted = 0;

  //Apply the CRC reflection
  CRC32_val = ~CRC32_val;

  //Send the CRC
  (void)serialWrite(CRC32_val);
}

void sendCompositeLog(void)
{
  if ( BIT_CHECK(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY) == false )
  {
    //If the buffer is not yet full but TS has timed out, pad the rest of the buffer with 0s
    while(toothHistoryIndex < TOOTH_LOG_SIZE)
    {
      toothHistory[toothHistoryIndex] = toothHistory[toothHistoryIndex-1U]; //Composite logger needs a realistic time value to display correctly. Copy the last value
      compositeLogHistory[toothHistoryIndex] = 0U;
      toothHistoryIndex++;
    }
  }

  uint32_t CRC32_val = 0;
  if(logItemsTransmitted == 0U)
  { 
    //Transmit the size of the packet
    (void)serialWrite((uint16_t)(sizeof(toothHistory) + sizeof(compositeLogHistory) + 1U)); //Size of the tooth log (uint32_t values) plus the return code
    
    //Begin new CRC hash
    const uint8_t returnCode = SERIAL_RC_OK;
    CRC32_val = CRC32_serial.crc32(&returnCode, 1, false);

    //Send the return code
    writeByteReliableBlocking(returnCode);
  }

  for (; logItemsTransmitted < TOOTH_LOG_SIZE; logItemsTransmitted++)
  {
    //Check whether the tx buffer still has space
    if((uint16_t)primarySerial.availableForWrite() < sizeof(toothHistory[logItemsTransmitted])+sizeof(compositeLogHistory[logItemsTransmitted])) 
    { 
      //tx buffer is full. Store the current state so it can be resumed later
      serialStatusFlag = SERIAL_TRANSMIT_COMPOSITE_INPROGRESS;
      return;
    }

    uint32_t transmitted = serialWrite(toothHistory[logItemsTransmitted]); //This combined runtime (in us) that the log was going for by this record
    (void)CRC32_serial.crc32_upd((const byte*)&transmitted, sizeof(transmitted), false);

    //The status byte (Indicates the trigger edge, whether it was a pri/sec pulse, the sync status)
    writeByteReliableBlocking(compositeLogHistory[logItemsTransmitted]);
    CRC32_val = CRC32_serial.crc32_upd((const byte*)&compositeLogHistory[logItemsTransmitted], sizeof(compositeLogHistory[logItemsTransmitted]), false);
  }
  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
  toothHistoryIndex = 0;
  serialStatusFlag = SERIAL_INACTIVE;
  logItemsTransmitted = 0;

  //Apply the CRC reflection
  CRC32_val = ~CRC32_val;

  //Send the CRC
  (void)serialWrite(CRC32_val);
}

#if defined(CORE_AVR)
#pragma GCC pop_options
#endif

///@}
