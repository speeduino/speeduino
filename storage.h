#include <EEPROM.h>

void writeConfig();

void loadConfig();

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
| 213   |45   | Remaining Page 2 settings           |
| 258   |12   | Calibration data (CLT)              |
| 260   |12   | Calibration data (IAT)              |
| 272   |12   | Calibration data (O2)               |
| 284   |28   | Calibration data (Reserved)         |
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

#define EEPROM_CONFIG_END 258

#define EEPROM_CALIBRATION_CLT 258
#define EEPROM_CALIBRATION_IAT 260
#define EEPROM_CALIBRATION_O2 272

