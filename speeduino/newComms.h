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
#define SD_READFILE_PAGE    0x14
#define SD_RTC_PAGE         0x07

#define SD_READ_STAT_ARG1   0x0000
#define SD_READ_STAT_ARG2   0x0010
#define SD_READ_DIR_ARG1    0x0000
#define SD_READ_DIR_ARG2    0x0202
#define SD_READ_SEC_ARG1    0x0002
#define SD_READ_SEC_ARG2    0x0004
#define SD_READ_STRM_ARG1   0x0004
#define SD_READ_STRM_ARG2   0x0001
#define SD_READ_COMP_ARG1   0x0000 //Not used for anything
#define SD_READ_COMP_ARG2   0x0800
#define SD_RTC_READ_ARG1    0x024D
#define SD_RTC_READ_ARG2    0x0008

#define SD_WRITE_DO_ARG1    0x0000
#define SD_WRITE_DO_ARG2    0x0001
#define SD_WRITE_DIR_ARG1   0x0001
#define SD_WRITE_DIR_ARG2   0x0002
#define SD_WRITE_SEC_ARG1   0x0003
#define SD_WRITE_SEC_ARG2   0x0204
#define SD_WRITE_COMP_ARG1  0x0005
#define SD_WRITE_COMP_ARG2  0x0008
#define SD_ERASEFILE_ARG1   0x0006
#define SD_ERASEFILE_ARG2   0x0006
#define SD_SPD_TEST_ARG1    0x0007
#define SD_SPD_TEST_ARG2    0x0004
#define SD_RTC_WRITE_ARG1   0x027E
#define SD_RTC_WRITE_ARG2   0x0009


#define SERIAL_CRC_LENGTH   4
#define SERIAL_LEN_SIZE     2
#define SERIAL_OVERHEAD_SIZE (SERIAL_LEN_SIZE + SERIAL_CRC_LENGTH) //The overhead for each serial command is 6 bytes. 2 bytes for the length and 4 bytes for the CRC

#ifdef RTC_ENABLED
  #define SD_FILE_TRANSMIT_BUFFER_SIZE 2048 + 3
  extern uint8_t serialSDTransmitPayload[SD_FILE_TRANSMIT_BUFFER_SIZE];
  extern uint16_t SDcurrentDirChunk;
  extern uint32_t SDreadStartSector;
  extern uint32_t SDreadNumSectors; //Number of sectors to read
  extern uint32_t SDreadCompletedSectors; //Number of sectors that have been read
#endif

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
extern uint8_t serialPayload[SERIAL_BUFFER_SIZE]; /**< Pointer to the serial payload buffer. */
extern uint16_t serialBytesReceived; /**< The number of bytes received in the serial buffer during the current command. */
extern bool serialWriteInProgress;
extern uint16_t serialBytesTransmitted;

void parseSerial();//This is the heart of the Command Line Interpeter.  All that needed to be done was to make it human readable.
void processSerialCommand();
void sendSerialReturnCode(byte returnCode);
void sendSerialPayload(void*, uint16_t payloadLength);

void generateLiveValues(uint16_t, uint16_t);
void saveConfig();
void receiveCalibrationNew(byte);
void generateToothLog(uint8_t);
void commandButtons(int16_t);
void generateCompositeLog(uint8_t);
void continueSerialTransmission();

#endif // COMMS_H
