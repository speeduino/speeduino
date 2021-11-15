#ifndef SD_H
#define SD_H

#ifdef SD_LOGGING

#include <SD.h>
#include "RingBuf.h"

#define SD_STATUS_OFF               0 /**< SD system is inactive. FS and file remain closed */
#define SD_STATUS_READY             1 /**< File has been openeed and preallocated, but a log session has not commenced */
#define SD_STATUS_ACTIVE            2 /**< Log session commenced */
#define SD_STATUS_ERROR_NO_CARD     3 /**< No SD card found when attempting to open file */
#define SD_STATUS_ERROR_NO_FS       4 /**< No filesystem found when attempting to open file */
#define SD_STATUS_ERROR_NO_WRITE    5 /**< Card and filesystem found, however file creation failed due to no write access */
#define SD_STATUS_ERROR_NO_SPACE    6 /**< File could not be preallocated as there is not enough space on card */
#define SD_STATUS_ERROR_WRITE_FAIL  7 /**< Log file created and opened, but a sector write failed during logging */
#define SD_STATUS_ERROR_FORMAT_FAIL 8 /**< Attempted formatting of SD card failed */

#define SD_SECTOR_SIZE              512 // Standard SD sector size

#ifdef CORE_TEENSY
    #define SD_CS_PIN BUILTIN_SDCARD
#else
    #define SD_CS_PIN 10 //This is a made up value for now
#endif

//Test values only
#define SD_LOG_FILE_SIZE  10000000 //Defuault 10mb file size
#define LOG_FILENAME "SdioLogger.csv"
#define RING_BUF_CAPACITY SD_LOG_ENTRY_SIZE * 10 //Allow for 10 entries in the ringbuffer. Will need tuning

/*
Standard FAT16/32
SdFs sd; 
FsFile logFile;
RingBuf<FsFile, RING_BUF_CAPACITY> rb;
*/
//ExFat
SdExFat sd;
ExFile logFile;
RingBuf<ExFile, RING_BUF_CAPACITY> rb;

uint8_t SD_status = SD_STATUS_OFF;

void initSD();
void writeSDLogEntry();
void writetSDLogHeader();
void beginSDLogging();
void endSDLogging();
void setTS_SD_status();
void formatExFat();
bool createLogFile();
void dateTime(uint16_t*, uint16_t*, uint8_t*); //Used for timestamping with RTC




#endif //SD_LOGGING
#endif //SD_H
