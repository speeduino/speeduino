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
#include "cancomms.h"
#include "storage.h"
#include "maths.h"
#include "utilities.h"
#include "decoders.h"
#include "TS_CommandButtonHandler.h"
#include "errors.h"
#include "pages.h"
#include "page_crc.h"
#include "logger.h"
#include "comms_legacy.h"
#include "src/FastCRC/FastCRC.h"
#include <avr/pgmspace.h>
#ifdef RTC_ENABLED
  #include "rtc_common.h"
#endif
#ifdef SD_LOGGING
  #include "SD_logger.h"
#endif

SerialStatus serialStatusFlag = SERIAL_INACTIVE;

constexpr byte serialVersion[] PROGMEM = {SERIAL_RC_OK, '0', '0', '2'};
constexpr byte canId[] PROGMEM = {SERIAL_RC_OK, 0};
constexpr byte codeVersion[] PROGMEM = { SERIAL_RC_OK, 's','p','e','e','d','u','i','n','o',' ','2','0','2','2','1','0','-','d','e','v'} ; //Note no null terminator in array and statu variable at the start
constexpr byte productString[] PROGMEM = { SERIAL_RC_OK, 'S', 'p', 'e', 'e', 'd', 'u', 'i', 'n', 'o', ' ', '2', '0', '2', '2', '.', '1', '0', '-', 'd', 'e', 'v'};
constexpr byte testCommsResponse[] PROGMEM = { SERIAL_RC_OK, 255 };

static uint16_t serialPayloadLength = 0;
/** The number of bytes received or transmitted to date during nonblocking I/O.
 * It's possible because we only support simpex serial comms. 
 * I.e. we can only be receiving or transmitting at any one time.
 */
static uint16_t serialBytesRxTx = 0; 
static uint32_t serialReceiveStartTime = 0; /**< The time at which the serial receive started. Used for calculating whether a timeout has occurred */
static FastCRC32 CRC32_serial; //This instance of CRC32 is exclusively used on the comms envelope CRC validations. It is separate to those used for page or calibration calculations to prevent update calls clashing with one another
#ifdef RTC_ENABLED
  static uint8_t serialPayload[SD_FILE_TRANSMIT_BUFFER_SIZE]; /**< Serial payload buffer must be significantly larger for boards that support SD logging. Large enough to contain 4 sectors + overhead */
  static uint16_t SDcurrentDirChunk;
  static uint32_t SDreadStartSector;
  static uint32_t SDreadNumSectors;
  static uint32_t SDreadCompletedSectors = 0;
#else
  static uint8_t serialPayload[SERIAL_BUFFER_SIZE]; /**< Serial payload buffer. */
#endif

#if defined(CORE_AVR)
#pragma GCC push_options
// These minimize RAM usage at no performance cost
#pragma GCC optimize ("Os") 
#endif

// ====================================== Internal Functions =============================

/**
 * @brief      Flush all remaining bytes from the rx serial buffer
 */
void flushRXbuffer(void)
{
  while (Serial.available() > 0) { Serial.read(); }
}

static uint32_t readSerial32()
{
  uint32_t crc1 = Serial.read();
  uint32_t crc2 = Serial.read();
  uint32_t crc3 = Serial.read();
  uint32_t crc4 = Serial.read();
  return  (crc1<<24) | (crc2<<16) | (crc3<<8) | crc4;
}

static uint32_t reverse_bytes(uint32_t i)
{
  union {uint32_t i; unsigned char b[sizeof(i)];} a,b;
  a.i=i;
  b.b[0]=a.b[3];
  b.b[1]=a.b[2];
  b.b[2]=a.b[1];
  b.b[3]=a.b[0];
  return b.i;
}

// Serial.write is blocking - it will wait for the buffer to clear
// We don't want that in some cases.
static uint16_t writeNonBlocking(const byte *buffer, size_t length)
{
  size_t capacity = min((size_t)Serial.availableForWrite(), length);
  return Serial.write(buffer, capacity);
}

static void serialWrite(uint32_t value)
{
  value = reverse_bytes(value);
  Serial.write((const byte*)&value, sizeof(value));
}

static uint32_t serialWriteUpdateCrc(uint32_t value)
{
  value = reverse_bytes(value);
  Serial.write((const byte*)&value, sizeof(value));
  return CRC32_serial.crc32_upd((const byte*)&value, sizeof(value), false);
}

static void serialWrite(uint16_t value)
{
  Serial.write((value >> 8) & 255);
  Serial.write(value & 255);
}

static uint16_t sendBufferAndCrcNonBlocking(const byte *buffer, size_t start, size_t length)
{
  start = start + writeNonBlocking(buffer+start, length-start);
  
  if (start==length)
  {
    serialWrite(CRC32_serial.crc32(buffer, length));
  }

  return start;
}

static uint16_t sendBufferAndCrcNonBlocking(const byte *buffer, uint16_t length)
{
  serialWrite(length);
  return sendBufferAndCrcNonBlocking(buffer, 0, length);
}

using pCrcCalc = uint32_t (FastCRC32::*)(const uint8_t *, const uint16_t, bool);

static bool writePage(uint8_t pageNum, uint16_t offset, const byte *buffer, uint16_t length)
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

static void loadPageToBuffer(uint8_t pageNum, uint16_t offset, byte *buffer, uint16_t length)
{
  for(uint16_t i = 0; i < length; i++)
  {
    buffer[i] = getPageValue(pageNum, offset + i);
  }
}

/** Send a status record back to tuning/logging SW.
 * This will "live" information from @ref currentStatus struct.
 * @param offset - Start field number
 * @param packetLength - Length of actual message (after possible ack/confirm headers)
 * E.g. tuning sw command 'A' (Send all values) will send data from field number 0, LOG_ENTRY_SIZE fields.
 */
//void sendValues(int packetlength, byte portNum)
static void generateLiveValues(uint16_t offset, uint16_t packetLength)
{  
  if(firstCommsRequest) 
  { 
    firstCommsRequest = false;
    currentStatus.secl = 0; 
  }

  currentStatus.spark ^= (-currentStatus.hasSync ^ currentStatus.spark) & (1U << BIT_SPARK_SYNC); //Set the sync bit of the Spark variable to match the hasSync variable

  serialPayload[0] = SERIAL_RC_OK;
  for(byte x=0; x<packetLength; x++)
  {
    serialPayload[x+1] = getTSLogEntry(offset+x); 
  }
  // Reset any flags that are being used to trigger page refreshes
  BIT_CLEAR(currentStatus.status3, BIT_STATUS3_VSS_REFRESH);

}

static void sendSerialReturnCode(byte returnCode)
{
  serialWrite((uint16_t)sizeof(returnCode));
  Serial.write(returnCode);
  serialWrite(CRC32_serial.crc32(&returnCode, sizeof(returnCode)));
}

static void sendSerialPayloadNonBlocking(uint16_t payloadLength)
{
  //Start new transmission session
  serialPayloadLength = payloadLength;
  serialBytesRxTx = sendBufferAndCrcNonBlocking(serialPayload, payloadLength);
  serialStatusFlag = serialBytesRxTx==payloadLength ? SERIAL_INACTIVE : SERIAL_WRITE_INPROGRESS;
}

static void loadO2Calibration(uint16_t calibrationLength, uint16_t offset)
{
  // First pass through the loop, we need to INITIALIZE the CRC
  pCrcCalc pCrcFun = offset==0 ? &FastCRC32::crc32 : &FastCRC32::crc32_upd;
  uint32_t calibrationCRC = 0;
//Check if this is the final chunk of calibration data
#ifdef CORE_STM32
  //STM32 requires TS to send 16 x 64 bytes chunk rather than 4 x 256 bytes. 
  bool finalBlock = offset == (64*15);
#else
  bool finalBlock = offset == (256*3);
#endif

  //Read through the current chunk (Should be 256 bytes long)
  // Note there are 2 loops here: 
  //    [x, calibrationLength)
  //    [offset, offset+calibrationLength)
  for(uint16_t x = 0; x < calibrationLength; ++x, ++offset)
  {
    //TS sends a total of 1024 bytes of calibration data, broken up into 256 byte chunks
    //As we're using an interpolated 2D table, we only need to store 32 values out of this 1024
    if( (x % 32) == 0 )
    {
      o2Calibration_values[offset/32] = serialPayload[x+7]; //O2 table stores 8 bit values
      o2Calibration_bins[offset/32]   = offset;
    }

    //Update the CRC
    calibrationCRC = (CRC32_serial.*pCrcFun)(&serialPayload[x+7], 1, false);
    // Subsequent passes through the loop, we need to UPDATE the CRC
    pCrcFun = &FastCRC32::crc32_upd;
  }

  if(finalBlock) 
  {
    storeCalibrationCRC32(O2_CALIBRATION_PAGE, ~calibrationCRC);
    writeCalibrationPage(O2_CALIBRATION_PAGE);
  }
}

static uint16_t toTemperature(byte lo, byte hi)
{
  int16_t tempValue = (int16_t)(word(hi, lo)); //Combine the 2 bytes into a single, signed 16-bit value
  tempValue = tempValue / 10; //TS sends values multiplied by 10 so divide back to whole degrees. 
  tempValue = ((tempValue - 32) * 5) / 9; //Convert from F to C
  //Apply the temp offset and check that it results in all values being positive
  return max( tempValue + CALIBRATION_TEMPERATURE_OFFSET, 0 );
}

static void updateTmpCalibration(uint8_t calibrationPage, uint16_t *values, uint16_t *bins)
{
  // Temperature calibrations are sent as 32 16-bit values (ie 64 bytes total)
  for (uint16_t x = 0; x < 32; x++)
  {
    values[x] = toTemperature(serialPayload[(2 * x) + 7], serialPayload[(2 * x) + 8]);
    bins[x] = (x * 33U); // 0*33=0 to 31*33=1023
  }
  storeCalibrationCRC32(calibrationPage, CRC32_serial.crc32(&serialPayload[7], 64));
  writeCalibrationPage(calibrationPage);
}

void processTemperatureCalibrationTableUpdate(uint16_t calibrationLength, uint8_t calibrationPage, uint16_t *values, uint16_t *bins)
{
  //Temperature calibrations are sent as 32 16-bit values
  if(calibrationLength == 64)
  {
    updateTmpCalibration(calibrationPage, values, bins);
    sendSerialReturnCode(SERIAL_RC_OK);
  }
  else 
  { 
    sendSerialReturnCode(SERIAL_RC_RANGE_ERR); 
  }
}

// ====================================== End Internal Functions =============================


/** Processes the incoming data on the serial buffer based on the command sent.
Can be either data for a new command or a continuation of data for command that is already in progress:
- cmdPending = If a command has started but is waiting on further data to complete
- chunkPending = Specifically for the new receive value method where TS will send a known number of contiguous bytes to be written to a table

Comands are single byte (letter symbol) commands.
*/
void parseSerial(void)
{
  //Check for an existing legacy command in progress
  if(cmdPending == true)
  {
    legacySerialCommand();
    return;
  }

  if (serialStatusFlag == SERIAL_INACTIVE)
  { 
    //New command received
    //Need at least 2 bytes to read the length of the command
    byte highByte = (byte)Serial.read();

    //Check if the command is legacy using the call/response mechanism
    if( ((highByte >= 'A') && (highByte <= 'z')) || (highByte == '?') )
    {
      //Handle legacy cases here
      serialStatusFlag = SERIAL_INACTIVE; //Make sure new serial handling does not interfere with legacy handling
      legacySerial = true;
      currentCommand = highByte;
      legacySerialCommand();
      return;
    }
    else
    {
      while(Serial.available() == 0) { /* Wait for the 2nd byte to be received (This will almost never happen) */ }

      serialPayloadLength = word(highByte, Serial.read());
      serialBytesRxTx = 2;
      serialStatusFlag = SERIAL_RECEIVE_PENDING; //Flag the serial receive as being in progress
      cmdPending = false; // Make sure legacy handling does not interfere with new serial handling
      serialReceiveStartTime = millis();
    }
  }

  //If there is a serial receive in progress, read as much from the buffer as possible or until we receive all bytes
  while( (Serial.available() > 0) && (serialStatusFlag == SERIAL_RECEIVE_PENDING) )
  {
    if (serialBytesRxTx < (serialPayloadLength + SERIAL_LEN_SIZE) )
    {
      serialPayload[serialBytesRxTx - SERIAL_LEN_SIZE] = (byte)Serial.read();
      serialBytesRxTx++;
    }
    else if (Serial.available() >= SERIAL_CRC_LENGTH)
    {
      serialStatusFlag = SERIAL_INACTIVE; //The serial receive is now complete
      if(readSerial32() != CRC32_serial.crc32(serialPayload, serialPayloadLength))
      {
        //CRC Error. Need to send an error message
        sendSerialReturnCode(SERIAL_RC_CRC_ERR);
        flushRXbuffer();
      }
      else
      {
        //CRC is correct. Process the command
        processSerialCommand();
      } //CRC match
    } //CRC received in full

    //Check for a timeout
    if( (millis() - serialReceiveStartTime) > SERIAL_TIMEOUT)
    {
      //Timeout occurred
      serialStatusFlag = SERIAL_INACTIVE; //Reset the serial receive

      flushRXbuffer();
      sendSerialReturnCode(SERIAL_RC_TIMEOUT);
    } //Timeout
  } //Data in serial buffer and serial receive in progress
}

void continueSerialTransmission(void)
{
  if(serialStatusFlag == SERIAL_WRITE_INPROGRESS)
  {
    serialBytesRxTx = sendBufferAndCrcNonBlocking(serialPayload, serialBytesRxTx, serialPayloadLength);
    serialStatusFlag = serialBytesRxTx==serialPayloadLength ? SERIAL_INACTIVE : SERIAL_WRITE_INPROGRESS;
  }
}

void processSerialCommand(void)
{
  currentCommand = serialPayload[0];

  switch (currentCommand)
  {

    case 'A': // send x bytes of realtime values
      generateLiveValues(0, LOG_ENTRY_SIZE); 
      break;

    case 'b': // New EEPROM burn command to only burn a single page at a time 
      if( (micros() > deferEEPROMWritesUntil)) { writeConfig(serialPayload[2]); } //Read the table number and perform burn. Note that byte 1 in the array is unused
      else { BIT_SET(currentStatus.status4, BIT_STATUS4_BURNPENDING); }
      
      sendSerialReturnCode(SERIAL_RC_BURN_OK);
      break;

    case 'C': // test communications. This is used by Tunerstudio to see whether there is an ECU on a given serial port
      memcpy_P(serialPayload, testCommsResponse, sizeof(testCommsResponse) );
      sendSerialPayloadNonBlocking(sizeof(testCommsResponse));
      break;

    case 'd': // Send a CRC32 hash of a given page
    {
      uint32_t CRC32_val = calculatePageCRC32( serialPayload[2] );

      serialPayload[0] = SERIAL_RC_OK;
      serialPayload[1] = ((CRC32_val >> 24) & 255);
      serialPayload[2] = ((CRC32_val >> 16) & 255);
      serialPayload[3] = ((CRC32_val >> 8) & 255);
      serialPayload[4] = (CRC32_val & 255);
      sendSerialPayloadNonBlocking(5);      
      break;
    }

    case 'E': // receive command button commands
      TS_CommandButtonsHandler(word(serialPayload[1], serialPayload[2]));
      sendSerialReturnCode(SERIAL_RC_OK);
      break;

    case 'F': // send serial protocol version
      memcpy_P(serialPayload, serialVersion, sizeof(serialVersion) );
      sendSerialPayloadNonBlocking(sizeof(serialVersion));
      break;

    case 'H': //Start the tooth logger
      startToothLogger();
      sendSerialReturnCode(SERIAL_RC_OK);
      break;

    case 'h': //Stop the tooth logger
      stopToothLogger();
      sendSerialReturnCode(SERIAL_RC_OK);
      break;

    case 'I': // send CAN ID
      memcpy_P(serialPayload, canId, sizeof(canId) );
      sendSerialPayloadNonBlocking(sizeof(serialVersion));
      break;

    case 'J': //Start the composite logger
      startCompositeLogger();
      sendSerialReturnCode(SERIAL_RC_OK);
      break;

    case 'j': //Stop the composite logger
      stopCompositeLogger();
      sendSerialReturnCode(SERIAL_RC_OK);
      break;

    case 'k': //Send CRC values for the calibration pages
    {
      uint32_t CRC32_val = readCalibrationCRC32(serialPayload[2]); //Get the CRC for the requested page

      serialPayload[0] = SERIAL_RC_OK;
      serialPayload[1] = ((CRC32_val >> 24) & 255);
      serialPayload[2] = ((CRC32_val >> 16) & 255);
      serialPayload[3] = ((CRC32_val >> 8) & 255);
      serialPayload[4] = (CRC32_val & 255);
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
      if (writePage(serialPayload[2], word(serialPayload[4], serialPayload[3]), &serialPayload[7], word(serialPayload[6], serialPayload[5])))
      {
        sendSerialReturnCode(SERIAL_RC_OK);    
      }
      else
      {
        //This should never happen, but just in case
        sendSerialReturnCode(SERIAL_RC_RANGE_ERR);
      }
      break;
    }  

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
      loadPageToBuffer(serialPayload[2], word(serialPayload[4], serialPayload[3]), &serialPayload[1], length);
      sendSerialPayloadNonBlocking(length + 1);
      break;
    }

    case 'Q': // send code version
      memcpy_P(serialPayload, codeVersion, sizeof(codeVersion) );
      sendSerialPayloadNonBlocking(sizeof(codeVersion));
      break;

    case 'r': //New format for the optimised OutputChannels
    {
      uint8_t cmd = serialPayload[2];
      uint16_t offset = word(serialPayload[4], serialPayload[3]);
      uint16_t length = word(serialPayload[6], serialPayload[5]);
#ifdef RTC_ENABLED      
      uint16_t SD_arg1 = word(serialPayload[3], serialPayload[4]);
      uint16_t SD_arg2 = word(serialPayload[5], serialPayload[6]);
#endif

      if(cmd == 0x30) //Send output channels command 0x30 is 48dec
      {
        generateLiveValues(offset, length);
        sendSerialPayloadNonBlocking(length + 1);
      }
#ifdef RTC_ENABLED
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
          /*
          serialPayload[5] = 0;
          serialPayload[6] = 0x20; //1gb dummy card
          serialPayload[7] = 0;
          serialPayload[8] = 0;
          */

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
          //Serial.print("Index:");
          //Serial.print(payloadIndex);

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
          
          if(numSectorsToSend <= 0) { sendSerialReturnCode(SERIAL_RC_OK); }
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
      cmdPending = false;    
      break;
    }

    case 'S': // send code version
      memcpy_P(serialPayload, productString, sizeof(productString) );
      sendSerialPayloadNonBlocking(sizeof(productString));
      currentStatus.secl = 0; //This is required in TS3 due to its stricter timings
      break;

    case 'T': //Send 256 tooth log entries to Tuner Studios tooth logger
      logItemsTransmitted = 0;
      if(currentStatus.toothLogEnabled == true) { sendToothLog(); } //Sends tooth log values as ints
      else if (currentStatus.compositeLogEnabled == true) { sendCompositeLog(); }
      break;

    case 't': // receive new Calibration info. Command structure: "t", <tble_idx> <data array>.
    {
      uint8_t cmd = serialPayload[2];
      uint16_t offset = word(serialPayload[3], serialPayload[4]);
      uint16_t calibrationLength = word(serialPayload[5], serialPayload[6]); // Should be 256

      if(cmd == O2_CALIBRATION_PAGE)
      {
        loadO2Calibration(calibrationLength, offset);
        sendSerialReturnCode(SERIAL_RC_OK);
        Serial.flush(); //This is safe because engine is assumed to not be running during calibration
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
        sendSerialReturnCode(SERIAL_RC_RANGE_ERR);
      }
      break;
    }

    case 'U': //User wants to reset the Arduino (probably for FW update)
      if (resetControl != RESET_CONTROL_DISABLED)
      {
      #ifndef SMALL_FLASH_MODE
        if (!cmdPending) { Serial.println(F("Comms halted. Next byte will reset the Arduino.")); }
      #endif

        while (Serial.available() == 0) { }
        digitalWrite(pinResetControl, LOW);
      }
      else
      {
      #ifndef SMALL_FLASH_MODE
        if (!cmdPending) { Serial.println(F("Reset control is currently disabled.")); }
      #endif
      }
      break;

    case 'w':
    {
#ifdef RTC_ENABLED
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
            
            sendSerialReturnCode(SERIAL_RC_OK);
          }
          else if((SD_arg1 == SD_WRITE_DIR_ARG1) && (SD_arg2 == SD_WRITE_DIR_ARG2))
          {
            //Begin SD directory read. Value in payload represents the directory chunk to read
            //Directory chunks are each 16 files long
            SDcurrentDirChunk = word(serialPayload[7], serialPayload[8]);
            sendSerialReturnCode(SERIAL_RC_OK);
          }
          else if((SD_arg1 == SD_WRITE_SEC_ARG1) && (SD_arg2 == SD_WRITE_SEC_ARG2))
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
            sendSerialReturnCode(SERIAL_RC_OK);
          }
          else if((SD_arg1 == SD_SPD_TEST_ARG1) && (SD_arg2 == SD_SPD_TEST_ARG2))
          {
            //Perform a speed test on the SD card
            //First 4 bytes are the sector number to write to
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

            sendSerialReturnCode(SERIAL_RC_OK);

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

            sendSerialReturnCode(SERIAL_RC_OK);
          }
        }
        else if(cmd == SD_RTC_PAGE)
        {
          cmdPending = false;
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
            sendSerialReturnCode(SERIAL_RC_OK);
          }
        }
#endif
      break;
    }

    default:
      //Unknown command
      sendSerialReturnCode(SERIAL_RC_UKWN_ERR);
      break;
  }
}

/** 
 * 
*/
void sendToothLog(void)
{
  //We need TOOTH_LOG_SIZE number of records to send to TunerStudio. If there aren't that many in the buffer then we just return and wait for the next call
  if (BIT_CHECK(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY)) //Sanity check. Flagging system means this should always be true
  {
    uint32_t CRC32_val = 0;
    if(logItemsTransmitted == 0)
    {
      //Transmit the size of the packet
      serialWrite((uint16_t)(sizeof(toothHistory) + 1)); //Size of the tooth log (uint32_t values) plus the return code
      //Begin new CRC hash
      const uint8_t returnCode = SERIAL_RC_OK;
      CRC32_val = CRC32_serial.crc32(&returnCode, 1, false);

      //Send the return code
      Serial.write(returnCode);
    }
    
    for (; logItemsTransmitted < TOOTH_LOG_SIZE; logItemsTransmitted++)
    {
      //Check whether the tx buffer still has space
      if(Serial.availableForWrite() < 4) 
      { 
        //tx buffer is full. Store the current state so it can be resumed later
        logSendStatusFlag = LOG_SEND_TOOTH;
        legacySerial = false;
        return;
      }

      //Transmit the tooth time
      CRC32_val = serialWriteUpdateCrc(toothHistory[logItemsTransmitted]);
    }
    BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
    cmdPending = false;
    logSendStatusFlag = LOG_SEND_NONE;
    toothHistoryIndex = 0;
    logItemsTransmitted = 0;

    //Apply the CRC reflection
    CRC32_val = ~CRC32_val;

    //Send the CRC
    serialWrite(CRC32_val);
  }
  else 
  { 
    sendSerialReturnCode(SERIAL_RC_BUSY_ERR);
    cmdPending = false; 
    logSendStatusFlag = LOG_SEND_NONE;
  } 
}

void sendCompositeLog(void)
{
  if ( (BIT_CHECK(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY)) || (logSendStatusFlag == LOG_SEND_COMPOSITE) ) //Sanity check. Flagging system means this should always be true
  {
    uint32_t CRC32_val = 0;
    if(logItemsTransmitted == 0)
    { 
      //Transmit the size of the packet
      serialWrite((uint16_t)(sizeof(toothHistory) + sizeof(compositeLogHistory) + 1)); //Size of the tooth log (uint32_t values) plus the return code
      
      //Begin new CRC hash
      const uint8_t returnCode = SERIAL_RC_OK;
      CRC32_val = CRC32_serial.crc32(&returnCode, 1, false);

      //Send the return code
      Serial.write(returnCode);
    }

    for (; logItemsTransmitted < TOOTH_LOG_SIZE; logItemsTransmitted++)
    {
      //Check whether the tx buffer still has space
      if((uint16_t)Serial.availableForWrite() < sizeof(toothHistory[logItemsTransmitted])+sizeof(compositeLogHistory[logItemsTransmitted])) 
      { 
        //tx buffer is full. Store the current state so it can be resumed later
        logSendStatusFlag = LOG_SEND_COMPOSITE;
        legacySerial = false;
        return;
      }

      serialWriteUpdateCrc(toothHistory[logItemsTransmitted]); //This combined runtime (in us) that the log was going for by this record

      //The status byte (Indicates the trigger edge, whether it was a pri/sec pulse, the sync status)
      Serial.write(compositeLogHistory[logItemsTransmitted]);
      CRC32_val = CRC32_serial.crc32_upd((const byte*)&compositeLogHistory[logItemsTransmitted], sizeof(compositeLogHistory[logItemsTransmitted]), false);
    }
    BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
    toothHistoryIndex = 0;
    cmdPending = false;
    logSendStatusFlag = LOG_SEND_NONE;
    logItemsTransmitted = 0;

    //Apply the CRC reflection
    CRC32_val = ~CRC32_val;

    //Send the CRC
    serialWrite(CRC32_val);
  }
  else 
  { 
    sendSerialReturnCode(SERIAL_RC_BUSY_ERR);
    cmdPending = false; 
    logSendStatusFlag = LOG_SEND_NONE;
  } 
}

#if defined(CORE_AVR)
#pragma GCC pop_options
#endif