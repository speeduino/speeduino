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

const char *ptr_fields[] = {"hasSync","RPM","MAP", "TPS", "tpsDOT","mapDOT","rpmDOT","VE1", "VE2","O2","O2_2"\
                            ,"coolant","IAT","dwell","battery10","advance","advance1","advance2","corrections",\
                            "AEamount","egoCorrection","wueCorrection","batCorrection","iatCorrection","baroCorrection",\
                            "launchCorrection","flexCorrection","fuelTempCorrection","flexIgnCorrection","afrTarget",\
                            "idleDuty","CLIdleTarget","idleUpActive","CTPSActive","fanOn","ethanolPct","fuelTemp",\
                            "AEEndTime","status1","spark","spark2","engine","PW1","PW2","PW3","PW4","PW5","PW6","PW7"\
                            ,"PW8","runSecs","secl","loopsPerSecond","launchingSoft","launchingHard","freeRAM",\
                            "startRevolutions","boostTarget","testOutputs","testActive","boostDuty","idleLoad",\
                            "status3","flexBoostCorrection","nitrous_status","fuelLoad","fuelLoad2","ignLoad",\
                            "fuelPumpOn","syncLossCounter","knockRetard","knockActive","toothLogEnabled",\
                            "compositeLogEnabled","vvt1Angle","vvt1Angle","vvt1TargetAngle","vvt1Duty",\
                            "injAngle","ASEValue","vss","idleUpOutputActive","gear","fuelPressure","oilPressure"\
                            ,"engineProtectStatus","wmiPW", NULL};

extern "C" {uint32_t get_fattime (void);}
void sd_logger_init();
void sd_logger_openLogFile();
void sd_logger_closeLogFile();
void sd_logger_writeLogEntry();

#endif //SD_LOGGING
#endif //SD_H
