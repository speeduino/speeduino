#ifndef SD_H
#define SD_H
#include "globals.h"
#ifdef SD_LOGGER_ENABLED
#include SD_LIB_H

#define SD_STATUS_OFF               0
#define SD_STATUS_FS_READY          1
#define SD_STATUS_CARD_READY        2
#define SD_STATUS_LOGGING           4
#define SD_STATUS_ERROR_NO_WRITE    8

#define SD_LOGGER_BUFFER_SIZE       16384U
#define SD_LOGGER_WRITE_TRIG        512U    //when to write to sdcard. Minmum is 512 bytes, and always must be integer multiple of 512 bytes for efficiency 
#define SD_LOGGER_CLOSE_FILE_TOUT   300     //Timeout on closing file of 300 milliseconds 
#define SD_LOGGER_FLUSH_FILE_TRIG   30      //After how many chuncks of data a flush of the file is executed. 

#ifdef CORE_TEENSY
    #define SD_CS_PIN BUILTIN_SDCARD
#elif STM32F4
    #define SD_CS_PIN SD_DETECT_NONE
#else
    #define SD_CS_PIN 10 //This is a made up value for now
#endif

extern "C" {uint32_t get_fattime (void);}
void sd_logger_init();
void sd_logger_openLogFile();
void sd_logger_closeLogFile();
void sd_logger_writeLogEntry();

#endif //SD_LOGGING
#endif //SD_H
