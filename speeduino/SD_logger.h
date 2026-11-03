#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#ifdef SD_LOGGING

#ifdef __SD_H__
  #include <SD.h>
#else
  #include "SdFat.h"
#endif
#include "RingBuf.h"
#include <elapsedMillis.h>


#define SD_STATUS_OFF               0 /**< SD system is inactive. FS and file remain closed */
#define SD_STATUS_READY             1 /**< Card is present and ready, but a log session has not commenced */
#define SD_STATUS_ACTIVE            2 /**< Log session commenced */
#define SD_STATUS_ERROR_NO_CARD     3 /**< No SD card found when attempting to open file */
#define SD_STATUS_ERROR_NO_FS       4 /**< No filesystem found when attempting to open file */
#define SD_STATUS_ERROR_NO_WRITE    5 /**< Card and filesystem found, however file creation failed due to no write access */
#define SD_STATUS_ERROR_NO_SPACE    6 /**< File could not be preallocated as there is not enough space on card */
#define SD_STATUS_ERROR_WRITE_FAIL  7 /**< Log file created and opened, but a sector write failed during logging */
#define SD_STATUS_ERROR_FORMAT_FAIL 8 /**< Attempted formatting of SD card failed */

#define SD_SECTOR_SIZE              512 // Standard SD sector size

#if defined CORE_TEENSY
    #define SD_CS_PIN BUILTIN_SDCARD
#elif defined CORE_STM32
    #define SD_CS_PIN PD2  //CS pin can be pretty much anything, but PD2 is one of the ones left unused from SDIO pins.
#else
    #define SD_CS_PIN 10 //This is a made up value for now
#endif

#define SD_LOG_NUM_FIELDS   99 /**< The number of fields that are in the log. This is always smaller than the entry size due to some fields being 2 bytes */
#ifndef UNIT_TEST // Scope guard for unit testing
  #define SD_LOG_ENTRY_SIZE   140 /**< The size of the live data packet used by the SD card.*/
#else
  #define SD_LOG_ENTRY_SIZE   1 /**< The size of the live data packet used by the SD card.*/
#endif

#define SD_LOG_FILE_SIZE  10000000 //Default 10mb file size
#define MAX_LOG_FILES     9999
#define LOG_FILE_PREFIX "SPD_"
#define LOG_FILE_EXTENSION "csv"
#define SD_LOG_ENTRY_TOTAL_BYTES (SD_LOG_ENTRY_SIZE + SD_LOG_NUM_FIELDS + 1) //The total size of each SD log entry in bytes. This is the size of the data packet + 1 comma for each field + 1 for the newline character
#define RING_BUF_CAPACITY (SD_LOG_ENTRY_TOTAL_BYTES * 10) //Allow for 10 entries in the ringbuffer. Will need tuning
#define SD_SYNC_RPM_THRESHOLD 1700 //SD log sync can take up to 8ms on slow SD cards. To prevent potential issues we only perform this if the RPM is under a safe speed so that there will always be sufficient time for a main loop to run. 
#define SD_SYNC_MAX_TIME_PERIOD 20000 //The maximum time (in ms) that will be allowed before a sync will be forced. This is the longest amount of time that an SD log will potentially lose if the ECU is unexpectedly powered down. 

/*
Standard FAT16/32
SdFs sd; 
FsFile logFile;
RingBuf<ExFile, RING_BUF_CAPACITY> rb;
*/
//ExFat
extern SdExFat sd;
extern ExFile logFile;
extern RingBuf<ExFile, RING_BUF_CAPACITY> rb;

extern uint8_t SD_status;
extern uint16_t currentLogFileNumber;
extern bool manualLogActive;
extern elapsedMillis msSinceLastSDSync;

void initSD();
void writeSDLogEntry();
void writetSDLogHeader();
void beginSDLogging();
void endSDLogging();
bool syncSDLog();
void setTS_SD_status();
void formatExFat();
void deleteLogFile(char, char, char, char);
bool createLogFile();
void dateTime(uint16_t*, uint16_t*, uint8_t*); //Used for timestamping with RTC
uint16_t getNextSDLogFileNumber();
bool getSDLogFileDetails(uint8_t* , uint16_t);
void readSDSectors(uint8_t*, uint32_t, uint16_t);
uint32_t sectorCount();



#endif //SD_LOGGING
#endif //SD_LOGGER_H
