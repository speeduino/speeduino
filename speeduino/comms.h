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


extern byte currentPage;//Not the same as the speeduino config page numbers
extern bool isMap; /**< Whether or not the currentPage contains only a 3D map that would require translation */
extern unsigned long requestCount; /**< The number of times the A command has been issued. This is used to track whether a reset has recently been performed on the controller */
extern byte currentCommand; /**< The serial command that is currently being processed. This is only useful when cmdPending=True */
extern bool cmdPending; /**< Whether or not a serial request has only been partially received. This occurs when a command character has been received in the serial buffer, but not all of its arguments have yet been received. If true, the active command will be stored in the currentCommand variable */
extern bool chunkPending; /**< Whether or not the current chucnk write is complete or not */
extern uint16_t chunkComplete; /**< The number of bytes in a chunk write that have been written so far */
extern uint16_t chunkSize; /**< The complete size of the requested chunk write */
extern int valueOffset; /**< THe memory offset within a given page for a value to be read from or written to. Note that we cannot use 'offset' as a variable name, it is a reserved word for several teensy libraries */
extern byte tsCanId;     // current tscanid requested
extern byte inProgressOffset;
extern byte inProgressLength;
extern uint32_t inProgressCompositeTime;
extern bool serialInProgress;
extern bool toothLogSendInProgress;
extern bool compositeLogSendInProgress;

void command();//This is the heart of the Command Line Interpeter.  All that needed to be done was to make it human readable.
void sendValues(uint16_t, uint16_t,byte, byte);
void sendValuesLegacy();
void saveConfig();
void sendPage();
void sendPageASCII();
void receiveCalibration(byte);
void sendToothLog(uint8_t);
void testComm();
void commandButtons(int16_t);
void sendCompositeLog(uint8_t);
byte getStatusEntry(uint16_t);

#endif // COMMS_H
