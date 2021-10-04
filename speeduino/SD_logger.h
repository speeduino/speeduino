#ifndef SD_H
#define SD_H

#ifdef SD_LOGGING

#include <SD.h>
#include "RingBuf.h"

#define SD_STATUS_OFF               0
#define SD_STATUS_READY             1
#define SD_STATUS_ERROR_NO_CARD     2
#define SD_STATUS_ERROR_NO_FS       3
#define SD_STATUS_ERROR_NO_WRITE    4
#define SD_STATUS_ERROR_NO_SPACE    5
#define SD_STATUS_ERROR_WRITE_FAIL  6

#define SD_SECTOR_SIZE              512 // Standard SD sector size

#ifdef CORE_TEENSY
    #define SD_CS_PIN BUILTIN_SDCARD
#else
    #define SD_CS_PIN 10 //This is a made up value for now
#endif

SdFs sd;
FsFile logFile;

uint8_t SD_status = SD_STATUS_OFF;

void initSD();
void writeSDLogEntry();
void endSD();
void setTS_SD_status();

//Test values only
#define SD_LOG_FILE_SIZE  10000000 //Defuault 10mb file size
#define LOG_FILENAME "SdioLogger.csv"
#define RING_BUF_CAPACITY SD_LOG_ENTRY_SIZE * 10 //Allow for 10 entries in the ringbuffer. Will need tuning
RingBuf<FsFile, RING_BUF_CAPACITY> rb;

#endif //SD_LOGGING
#endif //SD_H
