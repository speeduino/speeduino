#ifndef STORAGE_H
#define STORAGE_H

void writeAllConfig();
void writeConfig();
void loadConfig();
void loadCalibration();
void writeCalibration();

#define EEPROM_MAX_WRITE_BLOCK 50 //The maximum number of write operations that will be performed in one go. If we try to write to the EEPROM too fast (Each write takes ~3ms) then the rest of the system can hang)
bool eepromWritesPending = false;

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
| 1500  |128  | CANBUS config and data (Table 10_)  |
| 1628  |192  | Table 11 - General settings         |
|                                                   |
| 2559  |512  | Calibration data (O2)               |
| 3071  |512  | Calibration data (IAT)              |
| 3583  |512  | Calibration data (CLT)              |
-----------------------------------------------------
*/

#define EEPROM_DATA_VERSION   0

#define EEPROM_CONFIG1_XSIZE  1
#define EEPROM_CONFIG1_YSIZE  2
#define EEPROM_CONFIG1_MAP    3
#define EEPROM_CONFIG1_XBINS  259
#define EEPROM_CONFIG1_YBINS  275
#define EEPROM_CONFIG2_START  291
#define EEPROM_CONFIG2_END    419
#define EEPROM_CONFIG3_XSIZE  419
#define EEPROM_CONFIG3_YSIZE  420
#define EEPROM_CONFIG3_MAP    421
#define EEPROM_CONFIG3_XBINS  677
#define EEPROM_CONFIG3_YBINS  693
#define EEPROM_CONFIG4_START  709
#define EEPROM_CONFIG4_END    773
#define EEPROM_CONFIG5_XSIZE  773
#define EEPROM_CONFIG5_YSIZE  774
#define EEPROM_CONFIG5_MAP    775
#define EEPROM_CONFIG5_XBINS  1031
#define EEPROM_CONFIG5_YBINS  1047
#define EEPROM_CONFIG6_START  1063
#define EEPROM_CONFIG6_END    1191
#define EEPROM_CONFIG8_XSIZE1 1191
#define EEPROM_CONFIG8_YSIZE1 1192
#define EEPROM_CONFIG8_MAP1   1193
#define EEPROM_CONFIG8_XBINS1 1257
#define EEPROM_CONFIG8_YBINS1 1265
#define EEPROM_CONFIG8_XSIZE2 1273
#define EEPROM_CONFIG8_YSIZE2 1274
#define EEPROM_CONFIG8_MAP2   1275
#define EEPROM_CONFIG8_XBINS2 1339
#define EEPROM_CONFIG8_YBINS2 1347
#define EEPROM_CONFIG8_END    1355

#define EEPROM_CONFIG9_XSIZE1 1355
#define EEPROM_CONFIG9_YSIZE1 1356
#define EEPROM_CONFIG9_MAP1   1357
#define EEPROM_CONFIG9_XBINS1 1393
#define EEPROM_CONFIG9_YBINS1 1399
#define EEPROM_CONFIG9_XSIZE2 1405
#define EEPROM_CONFIG9_YSIZE2 1406
#define EEPROM_CONFIG9_MAP2   1407
#define EEPROM_CONFIG9_XBINS2 1443
#define EEPROM_CONFIG9_YBINS2 1449
#define EEPROM_CONFIG9_XSIZE3 1455
#define EEPROM_CONFIG9_YSIZE3 1456
#define EEPROM_CONFIG9_MAP3   1457
#define EEPROM_CONFIG9_XBINS3 1493
#define EEPROM_CONFIG9_YBINS3 1499
#define EEPROM_CONFIG9_XSIZE4 1505
#define EEPROM_CONFIG9_YSIZE4 1506
#define EEPROM_CONFIG9_MAP4   1507
#define EEPROM_CONFIG9_XBINS4 1543
#define EEPROM_CONFIG9_YBINS4 1549
#define EEPROM_CONFIG10_START 1564
#define EEPROM_CONFIG10_END   1692
#define EEPROM_CONFIG11_START 1692
#define EEPROM_CONFIG11_END   1884

//Calibration data is stored at the end of the EEPROM (This is in case any further calibration tables are needed as they are large blocks)
#define EEPROM_LAST_BARO      2558
#define EEPROM_CALIBRATION_O2 2559
#define EEPROM_CALIBRATION_IAT 3071
#define EEPROM_CALIBRATION_CLT 3583

#endif // STORAGE_H
