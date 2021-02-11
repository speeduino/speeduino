#ifndef SD_H
#define SD_H

#ifdef SD_LOGGING

#include <SPI.h>
#include <SD.h>

#define SD_STATUS_OFF               0
#define SD_STATUS_READY             1
#define SD_STATUS_ERROR_NO_CARD     2
#define SD_STATUS_ERROR_NO_FS       3
#define SD_STATUS_ERROR_NO_WRITE    4

#ifdef CORE_TEENSY
    #define SD_CS_PIN BUILTIN_SDCARD
#else
    #define SD_CS_PIN 10 //This is a made up value for now
#endif

Sd2Card SD_card;
SdVolume SD_volume;
File logFile;

uint8_t SD_status = SD_STATUS_OFF;

void initSD();
void writeSDLogEntry();
void endSD();
void setTS_SD_status();

#endif //SD_LOGGING
#endif //SD_H
