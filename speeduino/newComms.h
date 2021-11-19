/** \file comms.h
 * @brief File for handling all serial requests 
 * @author Josh Stewart
 * 
 * This file contains all the functions associated with serial comms.
 * This includes sending of live data, sending/receiving current page data, sending CRC values of pages, receiving sensor calibration data etc
 * 
 */

#ifndef NEW_COMMS_H
#define NEW_COMMS_H

//Hardcoded TunerStudio addresses/commands for various SD/RTC commands
#define SD_READWRITE_PAGE   0x11
#define SD_RTC_PAGE         0x07
#define SD_READ_STAT_OFFSET 0x0000
#define SD_READ_STAT_LENGTH 0x1000
#define SD_READ_DIR_OFFSET  0x0100
#define SD_READ_DIR_LENGTH  0x0200
#define SD_READ_SEC_OFFSET  0x0200
#define SD_READ_SEC_LENGTH  0x0400
#define SD_READ_STRM_OFFSET 0x0400
#define SD_READ_STRM_LENGTH 0x0100
#define SD_WRITE_DO_OFFSET  0x0000
#define SD_WRITE_DO_LENGTH  0x0001
#define SD_WRITE_SEC_OFFSET 0x0300
#define SD_WRITE_SEC_LENGTH 0x0402
#define SD_ERASEFILE_OFFSET 0x0600
#define SD_ERASEFILE_LENGTH 0x0600
#define SD_SPD_TEST_OFFSET  0x0700
#define SD_SPD_TEST_LENGTH  0x0400
#define SD_RTC_WRITE_OFFSET 0x7E02
#define SD_RTC_WRITE_LENGTH 0x0900
#define SD_RTC_READ_OFFSET  0x4D02
#define SD_RTC_READ_LENGTH  0x0800

#define SERIAL_CRC_LENGTH   4
#define SERIAL_LEN_SIZE     2
#define SERIAL_OVERHEAD_SIZE (SERIAL_LEN_SIZE + SERIAL_CRC_LENGTH) //The overhead for each serial command is 6 bytes. 2 bytes for the length and 4 bytes for the CRC

//Serial return codes
#define SERIAL_RC_OK        0x00
#define SERIAL_RC_REALTIME  0x01
#define SERIAL_RC_PAGE      0x02

#define SERIAL_RC_BURN_OK   0x04

#define SERIAL_RC_CRC_ERROR 0x82

extern uint16_t serialPayloadLength;
extern uint32_t serialCRC;
extern bool serialReceivePending; /**< Whether or not a serial request has only been partially received. This occurs when a the length has been received in the serial buffer, but not all of the payload or CRC has yet been received. */
//extern uint8_t *serialPayload; /**< Pointer to the serial payload buffer. */
extern uint8_t serialPayload[257]; /**< Pointer to the serial payload buffer. */
extern uint16_t serialBytesReceived; /**< The number of bytes received in the serial buffer during the current command. */
extern bool serialWriteInProgress;

void parseSerial();//This is the heart of the Command Line Interpeter.  All that needed to be done was to make it human readable.
void processSerialCommand();
void sendSerialReturnCode(byte returnCode);
void sendSerialPayload(void*, byte payloadLength);

void generateLiveValues(uint16_t, uint16_t);
void saveConfig();
void receiveCalibrationNew(byte);
void generateToothLog(uint8_t);
void commandButtons(int16_t);
void generateCompositeLog(uint8_t);

#endif // COMMS_H
