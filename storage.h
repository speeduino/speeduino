#include <EEPROM.h>

void writeConfig();
void writeTables();

void loadConfig();
void loadTables();

/*
Current layout of data (Version 1) is as follows (All sizes are in bytes):
|---------------------------------------------------|
|Byte # |Size | Description                         |
|---------------------------------------------------|
| 0     |1    | Data structure version              |
| 1     |2    | X and Y sizes for VE table          |
| 3     |125  | Config Page 1 (VE Table)            |
| 128   |2    | X and Y sizes for Ign table         |
| 130   |125  | Config Page 2 (Ign Table)           |
-----------------------------------------------------
*/

#define EEPROM_CONFIG1 3
#define EEPROM_CONFIG2 130
