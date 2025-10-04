/** \file comms.h
 * @brief File for handling all serial requests 
 * @author Josh Stewart
 * 
 * This file contains all the functions associated with serial comms.
 * This includes sending of live data, sending/receiving current page data, sending CRC values of pages, receiving sensor calibration data etc
 * 
 */

#ifndef COMMS_H
#define COMMS_H

#include <memory>
#include "globals.h"
#include BOARD_H

static constexpr uint16_t SERIAL_TIMEOUT = 400; //!< Timeout threshold in milliseconds

#if defined(CORE_TEENSY)
  #define BLOCKING_FACTOR       251
  #define TABLE_BLOCKING_FACTOR 256
#elif defined(CORE_STM32)
  #define BLOCKING_FACTOR       121
  #define TABLE_BLOCKING_FACTOR 64
#elif defined(CORE_AVR)
  #define BLOCKING_FACTOR       121
  #define TABLE_BLOCKING_FACTOR 64
#endif
#if defined(RTC_ENABLED) && defined(SD_LOGGING)
  #define COMMS_SD            
#endif

/** \enum SerialStatus
 * @brief The current state of serial communication
 * */
enum SerialStatus {
  /** No serial comms is in progress */
  SERIAL_INACTIVE, 
  /** A partial write is in progress. */
  SERIAL_TRANSMIT_INPROGRESS, 
  /** A partial write is in progress (legacy send). */
  SERIAL_TRANSMIT_INPROGRESS_LEGACY, 
  /** We are part way through transmitting the tooth log */
  SERIAL_TRANSMIT_TOOTH_INPROGRESS, 
  /** We are part way through transmitting the tooth log (legacy send) */
  SERIAL_TRANSMIT_TOOTH_INPROGRESS_LEGACY, 
  /** We are part way through transmitting the composite log */
  SERIAL_TRANSMIT_COMPOSITE_INPROGRESS,
  /** We are part way through transmitting the composite log (legacy send) */
  SERIAL_TRANSMIT_COMPOSITE_INPROGRESS_LEGACY,
  /** Whether or not a serial request has only been partially received.
   * This occurs when a the length has been received in the serial buffer,
   * but not all of the payload or CRC has yet been received. 
   * 
   * Expectation is that ::serialReceive is called  until the status reverts 
   * to SERIAL_INACTIVE
  */
  SERIAL_RECEIVE_INPROGRESS,
  /** We are part way through processing a legacy serial commang: call ::serialReceive */
  SERIAL_COMMAND_INPROGRESS_LEGACY,
};

//#define primarySerial (*pPrimarySerial)
#define primarySerial (*(primaryComms.pSerial))

struct commsInterface
{
  Stream *pSerial;
  uint32_t serialReceiveStartTime; //!< The time in milliseconds at which the serial receive started. Used for calculating whether a timeout has occurred
  #ifdef COMMS_SD
    /** @brief Serial payload buffer must be significantly larger for boards that support SD logging.
    * 
    * Large enough to contain 4 sectors + overhead 
    */
    uint16_t SDcurrentDirChunk;
    uint32_t SDreadStartSector;
    uint32_t SDreadNumSectors;
    uint32_t SDreadCompletedSectors;
  #endif
  std::unique_ptr<uint8_t[]> serialPayload;  // Dynamically sized payload //!< Serial payload buffer. Note that the size is set below at creation time
  size_t payloadSize{};                // size of the payload buffer
  uint16_t serialPayloadLength; //!< How many bytes in serialPayload were received or sent
  SerialStatus serialStatusFlag;

  // Constructor
  commsInterface(Stream* serial, size_t payloadLen)
      : pSerial(serial),
        serialPayload(std::make_unique<uint8_t[]>(payloadLen)),
        payloadSize(payloadLen)
  {}

  // Array-style access to payload
  uint8_t& operator[](size_t i) { return serialPayload[i]; }
  const uint8_t& operator[](size_t i) const { return serialPayload[i]; }
  uint8_t* buffer() { return serialPayload.get(); }
  const uint8_t* buffer() const { return serialPayload.get(); }

  void flushRXbuffer(void)
  {
    while (pSerial->available() > 0) { pSerial->read(); }
  }

  /** @brief Has the current receive operation timed out? */
  bool isRxTimeout(void) 
  {
    return (millis() - serialReceiveStartTime) > SERIAL_TIMEOUT;
  }

  /**
   * @brief Is a serial write in progress?
   * 
   * Expectation is that ::serialTransmit is called until this
   * returns false
   */
  inline bool serialTransmitInProgress(void) 
  {
    return serialStatusFlag == SERIAL_TRANSMIT_INPROGRESS
        || serialStatusFlag == SERIAL_TRANSMIT_INPROGRESS_LEGACY
        || serialStatusFlag == SERIAL_TRANSMIT_TOOTH_INPROGRESS
        || serialStatusFlag == SERIAL_TRANSMIT_TOOTH_INPROGRESS_LEGACY
        || serialStatusFlag == SERIAL_TRANSMIT_COMPOSITE_INPROGRESS
        || serialStatusFlag == SERIAL_TRANSMIT_COMPOSITE_INPROGRESS_LEGACY;
  }

  /**
   * @brief Is a non-blocking serial receive operation in progress?
   * 
   * Expectation is the ::serialReceive is called until this
   * returns false.
   */
  inline bool serialRecieveInProgress(void) 
  {
    return serialStatusFlag == SERIAL_RECEIVE_INPROGRESS
        || serialStatusFlag == SERIAL_COMMAND_INPROGRESS_LEGACY;
  }
};
extern commsInterface primaryComms;


/**
 * @brief The serial receive pump. Should be called whenever the serial port
 * has data available to read.
 */
void serialReceive(commsInterface *);

/** @brief The serial transmit pump. Should be called when ::serialStatusFlag indicates a transmit
 * operation is in progress */
void serialTransmit(commsInterface *comms);


#endif // COMMS_H
