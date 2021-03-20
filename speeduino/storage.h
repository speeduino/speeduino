#ifndef STORAGE_H
#define STORAGE_H

#include "globals.h"

void writeAllConfig();
void writeConfig(byte);
void loadConfig();
void loadCalibration();
void writeCalibration();
void loadCalibration_new();
void writeCalibration_new();
void resetConfigPages();

//These are utility functions that prevent other files from having to use EEPROM.h directly
byte readLastBaro();
void storeLastBaro(byte);
void storeCalibrationValue(uint16_t, byte);
byte readEEPROMVersion();
void storeEEPROMVersion(byte);
void storePageCRC32(byte, uint32_t);
uint32_t readPageCRC32(byte);

#if defined(CORE_STM32) || defined(CORE_TEENSY) & !defined(USE_SPI_EEPROM)
#define EEPROM_MAX_WRITE_BLOCK 64 //The maximum number of write operations that will be performed in one go. If we try to write to the EEPROM too fast (Each write takes ~3ms) then the rest of the system can hang)
#else
#define EEPROM_MAX_WRITE_BLOCK 30 //The maximum number of write operations that will be performed in one go. If we try to write to the EEPROM too fast (Each write takes ~3ms) then the rest of the system can hang)
#endif
extern bool eepromWritesPending;

/*
Current layout of EEPROM data is as follows (All sizes are in bytes):
|------------------------------------------------------------------------|
|Byte #              |Size         | Description                         |
|------------------------------------------------------------------------|
| 0                  |1            | Data structure version              |
| 1                  |sizeof(p1)   | page 1                              | // Pages are 
| ....               |....         | .....                               | // packed end-
| sizeof(page n-1)+1 |sizeof(pN)   | page n                              | // to-end going
| ....               |....         | .....                               | // "up"
| sizeof(page n-1)+1 |sizeof(pMax) | page Max                            | // |
|                                                                        | // V
|                                                                        | 
| Baro-sizeof(CRCs)  | sizeof(CRCs)| Table CRC32 values. Last table first| // ^
| O2-1               | 1           | Last recorded Baro value            | // |
| IAT-sizeof(O2)     | sizeof(O2)  | O2 calibration data                 | // Calibration data
| (CLT-sizeof(IAT)   | sizeof(IAT) | IAT calibration data                | // is packed end-
| (end-sizeof(CLT))  | sizeof(CLT) | Coolant calibration data            | // -to-end going
| 4095               | 0           | Storage End                         | // "down"
|------------------------------------------------------------------------|
*/

#define EEPROM_DATA_VERSION   0

//Calibration data is stored at the end of the EEPROM (This is in case any further calibration tables are needed as they are large blocks)
#define EEPROM_PAGE_CRC32     3686 //Size of this is 4 * <number of pages> (CRC32 = 32 bits): 3742 - (14 * 4) = 3686
#define EEPROM_LAST_BARO      3742 // 3743 - 1
//New values using 2D tables
#define EEPROM_CALIBRATION_O2   3743 //3839-96 +64
#define EEPROM_CALIBRATION_IAT  3839 //3967-128
#define EEPROM_CALIBRATION_CLT  3967 //4095-128

#endif // STORAGE_H
