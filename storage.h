#ifndef STORAGE_H
#define STORAGE_H
#include <EEPROM.h>

void writeConfig();
void loadConfig();
void loadCalibration();
void writeCalibration();

/*
Current layout of EEPROM data (Version 2) is as follows (All sizes are in bytes):
|---------------------------------------------------|
|Byte # |Size | Description                         |
|---------------------------------------------------|
| 0     |1    | Data structure version              |
| 1     |2    | X and Y sizes for VE table          |
| 3     |64   | VE Map (8x8)                        |
| 67    |8    | VE Table RPM bins                   |
| 75    |8    | VE Table MAP/TPS bins               |
| 83    |48   | Remaining Page 1 settings           |
| 131   |2    | X and Y sizes for Ign table         |
| 133   |64   | Ignition Map (8x8)                  |
| 197   |8    | Ign Table RPM bins                  |
| 205   |8    | Ign Table MAP/TPS bins              |
| 213   |48   | Remaining Page 2 settings           |
| 261   |2    | X and Y sizes for AFR table         |
| 263   |64   | AFR Target Map (8x8)                |
| 327   |8    | AFR Table RPM bins                  |
| 335   |8    | AFR Table MAP/TPS bins              |
| 343   |48   | Remaining Page 3 settings           |
| 2559  |512  | Calibration data (O2)              |
| 3071  |512  | Calibration data (IAT)             |
| 3583  |512  | Calibration data (CLT)             |
-----------------------------------------------------
*/

#define EEPROM_CONFIG1_XSIZE 1
#define EEPROM_CONFIG1_YSIZE 2
#define EEPROM_CONFIG1_MAP 3
#define EEPROM_CONFIG1_XBINS 67
#define EEPROM_CONFIG1_YBINS 75
#define EEPROM_CONFIG1_SETTINGS 83
#define EEPROM_CONFIG1_END 131
#define EEPROM_CONFIG2_XSIZE 131
#define EEPROM_CONFIG2_YSIZE 132
#define EEPROM_CONFIG2_MAP 133
#define EEPROM_CONFIG2_XBINS 197
#define EEPROM_CONFIG2_YBINS 205
#define EEPROM_CONFIG2_SETTINGS 213
#define EEPROM_CONFIG2_END 261
#define EEPROM_CONFIG3_XSIZE 261
#define EEPROM_CONFIG3_YSIZE 262
#define EEPROM_CONFIG3_MAP 263
#define EEPROM_CONFIG3_XBINS 327
#define EEPROM_CONFIG3_YBINS 335
#define EEPROM_CONFIG3_SETTINGS 343
#define EEPROM_CONFIG3_END 391

//Calibration data is stored at the end of the EEPROM (This is in case any further calibration tables are needed as they are large blocks)
#define EEPROM_CALIBRATION_O2 2559
#define EEPROM_CALIBRATION_IAT 3071
#define EEPROM_CALIBRATION_CLT 3583

#endif // STORAGE_H
