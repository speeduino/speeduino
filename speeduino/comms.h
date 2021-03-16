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


byte currentPage = 1;//Not the same as the speeduino config page numbers
bool isMap = true; /**< Whether or not the currentPage contains only a 3D map that would require translation */
unsigned long requestCount = 0; /**< The number of times the A command has been issued. This is used to track whether a reset has recently been performed on the controller */
byte currentCommand; /**< The serial command that is currently being processed. This is only useful when cmdPending=True */
bool cmdPending = false; /**< Whether or not a serial request has only been partially received. This occurs when a command character has been received in the serial buffer, but not all of its arguments have yet been received. If true, the active command will be stored in the currentCommand variable */
bool chunkPending = false; /**< Whether or not the current chucnk write is complete or not */
uint16_t chunkComplete = 0; /**< The number of bytes in a chunk write that have been written so far */
uint16_t chunkSize = 0; /**< The complete size of the requested chunk write */
int valueOffset; /**< THe memory offset within a given page for a value to be read from or written to. Note that we cannot use 'offset' as a variable name, it is a reserved word for several teensy libraries */
byte tsCanId = 0;     // current tscanid requested
byte inProgressOffset;
byte inProgressLength;
uint32_t inProgressCompositeTime;
bool serialInProgress = false;
bool toothLogSendInProgress = false;
bool compositeLogSendInProgress = false;

const char pageTitles[] PROGMEM //This is being stored in the avr flash instead of SRAM which there is not very much of
  {
   "\nVE Map\0"//This is an alternative to using a 2D array which would waste space because of the different lengths of the strings
   "\nPg 1 Config\0"// 21-The configuration page titles' indexes are found by counting the chars
   "\nIgnition Map\0"//35-The map page titles' indexes are put into a var called currentTitleIndex. That represents the first char of each string.
   "\nPg 2 Config\0" //48
   "\nAFR Map\0" //56
   "\nPg 3 Config\0" //69
   "\nPg 4 Config\0" //82
   "\nBoost Map\0" //93
   "\nVVT Map\0"//102-No need to put a trailing null because it's the last string and the compliler does it for you.
   "\nPg 10 Config\0"//116
   "\n2nd Fuel Map\0"//130
   "\nWMI Map\0"//139
   "\nPrgm IO\0"//148
   "\n2nd Ignition Map"
  };

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
