#ifndef STORAGE_H
#define STORAGE_H

void writeConfig();
void loadConfig();
void loadCalibration();
void writeCalibration();

/*
Current layout of EEPROM data (Version 3) is as follows (All sizes are in bytes):
|---------------------------------------------------|
|Byte # |Size | Description                         |
|---------------------------------------------------|
| 0     |1    | Data structure version              |
| 1     |2    | X and Y sizes for VE table          |
| 3     |256  | VE Map (16x16)                      |
| 259   |16   | VE Table RPM bins                   |
| 275   |16   | VE Table MAP/TPS bins               |
| 291   |64   | Page 2 settings (Non-Map page)      |
| 355   |2    | X and Y sizes for Ign table         |
| 357   |256  | Ignition Map (16x16)                |
| 613   |16   | Ign Table RPM bins                  |
| 629   |16   | Ign Table MAP/TPS bins              |
| 645   |64   | Page 4 settings (Non-Map page)      |
| 709   |2    | X and Y sizes for AFR table         |
| 711   |256  | AFR Target Map (16x16)              |
| 967   |16   | AFR Table RPM bins                  |
| 983   |16   | AFR Table MAP/TPS bins              |
| 999   |64   | Remaining Page 3 settings           |
| 1063  |64   | Page 4 settings                     |
| 1127  |2    | X and Y sizes for boost table       |
| 1129  |64   | Boost Map (8x8)                     |
| 1193  |8    | Boost Table RPM bins                |
| 1201  |8    | Boost Table TPS bins                |
| 1209  |2    | X and Y sizes                       |
| 1211  |64   | PAGE 8 MAP2                         |
| 1275  |8    | Xbins Map2                          |
| 1283  |8    | Ybins Map2                          |
| 1291  |2    | X and Y sizes1                      |
| 1293``|36   | PAGE 9 MAP1                         |
| 1329  |12   | X and Y Bins1                       |
| 1341  |2    | X and Y size2                       |
| 1343  |36   | PAGE 9 MAP2                         |
| 1379  |6    | X and Y Bins2                       |
| 1391  |2    | X and Y sizes3                      |
| 1393  |36   | PAGE 9 MAP3                         |
| 1429  |6    | X and Y Bins3                       |
| 1441  |2    | X and Y size4                       |
| 1443  |36   | PAGE 9 MAP4                         |
| 1479  |6    | X and Y Bins4                       |
| 1500  |128  | CANBUS config and data              |
|                                                   |
| 2559  |512  | Calibration data (O2)               |
| 3071  |512  | Calibration data (IAT)              |
| 3583  |512  | Calibration data (CLT)              |
-----------------------------------------------------
*/

#define EEPROM_CONFIG1_XSIZE  1
#define EEPROM_CONFIG1_YSIZE  2
#define EEPROM_CONFIG1_MAP    3
#define EEPROM_CONFIG1_XBINS  259
#define EEPROM_CONFIG1_YBINS  275
#define EEPROM_CONFIG2_START  291
#define EEPROM_CONFIG2_END    355 // +64   131
#define EEPROM_CONFIG3_XSIZE  355
#define EEPROM_CONFIG3_YSIZE  356
#define EEPROM_CONFIG3_MAP    357
#define EEPROM_CONFIG3_XBINS  613
#define EEPROM_CONFIG3_YBINS  629
#define EEPROM_CONFIG4_START  645
#define EEPROM_CONFIG4_END    709
#define EEPROM_CONFIG5_XSIZE  709
#define EEPROM_CONFIG5_YSIZE  710
#define EEPROM_CONFIG5_MAP    711
#define EEPROM_CONFIG5_XBINS  967
#define EEPROM_CONFIG5_YBINS  983
#define EEPROM_CONFIG6_START  999
#define EEPROM_CONFIG6_END    1063
#define EEPROM_CONFIG7_START  1063
#define EEPROM_CONFIG7_END    1127   
#define EEPROM_CONFIG8_XSIZE1 1127   
#define EEPROM_CONFIG8_YSIZE1 1128
#define EEPROM_CONFIG8_MAP1   1129
#define EEPROM_CONFIG8_XBINS1 1193
#define EEPROM_CONFIG8_YBINS1 1201
#define EEPROM_CONFIG8_XSIZE2 1209  
#define EEPROM_CONFIG8_YSIZE2 1210
#define EEPROM_CONFIG8_MAP2   1211
#define EEPROM_CONFIG8_XBINS2 1275
#define EEPROM_CONFIG8_YBINS2 1283
#define EEPROM_CONFIG8_END    1291

#define EEPROM_CONFIG9_XSIZE1 1291
#define EEPROM_CONFIG9_YSIZE1 1292
#define EEPROM_CONFIG9_MAP1   1293
#define EEPROM_CONFIG9_XBINS1 1329
#define EEPROM_CONFIG9_YBINS1 1335
#define EEPROM_CONFIG9_XSIZE2 1341
#define EEPROM_CONFIG9_YSIZE2 1342
#define EEPROM_CONFIG9_MAP2   1343
#define EEPROM_CONFIG9_XBINS2 1379
#define EEPROM_CONFIG9_YBINS2 1385
#define EEPROM_CONFIG9_XSIZE3 1391
#define EEPROM_CONFIG9_YSIZE3 1392
#define EEPROM_CONFIG9_MAP3   1393
#define EEPROM_CONFIG9_XBINS3 1429
#define EEPROM_CONFIG9_YBINS3 1435
#define EEPROM_CONFIG9_XSIZE4 1441
#define EEPROM_CONFIG9_YSIZE4 1442
#define EEPROM_CONFIG9_MAP4   1443
#define EEPROM_CONFIG9_XBINS4 1479
#define EEPROM_CONFIG9_YBINS4 1485
#define EEPROM_CONFIG10_START 1500
#define EEPROM_CONFIG10_END   1628

//Calibration data is stored at the end of the EEPROM (This is in case any further calibration tables are needed as they are large blocks)
#define EEPROM_LAST_BARO      2558
#define EEPROM_CALIBRATION_O2 2559
#define EEPROM_CALIBRATION_IAT 3071
#define EEPROM_CALIBRATION_CLT 3583

#endif // STORAGE_H
