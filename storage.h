#include <EEPROM.h>

void writeConfig();
void writeTables();

void loadConfig();
void loadTables();

/*
Current layout of EEPROM data (Version 1) is as follows (All sizes are in bytes):
|---------------------------------------------------|
|Byte # |Size | Description                         |
|---------------------------------------------------|
| 0     |1    | Data structure version              |
| 1     |2    | X and Y sizes for VE table          |
| 3     |64   | VE Map (8x8)                        |
| 67    |8    | VE Table RPM bins                   |
| 75    |8    | VE Table MAP/TPS bins               |
| 83    |45   | Remaining Page 1 settings           |
| 128   |2    | X and Y sizes for Ign table         |
| 130   |64   | Ignition Map (8x8)                  |
| 194   |8    | Ign Table RPM bins                  |
| 202   |8    | Ign Table MAP/TPS bins              |
| 210   |45   | Remaining Page 2 settings           |
-----------------------------------------------------
*/

#define EEPROM_SIZE 255

#define EEPROM_CONFIG1_XSIZE 1
#define EEPROM_CONFIG1_YSIZE 2
#define EEPROM_CONFIG1_MAP 3
#define EEPROM_CONFIG1_XBINS 67
#define EEPROM_CONFIG1_YBINS 75
#define EEPROM_CONFIG1_SETTINGS 85
#define EEPROM_CONFIG2_XSIZE 128
#define EEPROM_CONFIG2_YSIZE 129
#define EEPROM_CONFIG2_MAP 130
#define EEPROM_CONFIG2_XBINS 194
#define EEPROM_CONFIG2_YBINS 202
#define EEPROM_CONFIG2_SETTINGS 210
