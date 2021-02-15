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
| 1500  |192  | CANBUS config and data (Table 10_)  |
| 1692  |192  | Table 11 - General settings         |
| 2385  |2    | X and Y sizes for wmi table         |
| 2387  |64   | WMI Map (8x8)                       |
| 2451  |8    | WMI Table RPM bins                  |
| 2459  |8    | WMI Table TPS bins                  |
|                                                   |
| 2514  |44   | Table CRC32 values. Last table first|
| 2558  |1    | Last recorded Baro value            |
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
#define EEPROM_CONFIG4_END    837
#define EEPROM_CONFIG5_XSIZE  837
#define EEPROM_CONFIG5_YSIZE  838
#define EEPROM_CONFIG5_MAP    839
#define EEPROM_CONFIG5_XBINS  1095
#define EEPROM_CONFIG5_YBINS  1111
#define EEPROM_CONFIG6_START  1127
#define EEPROM_CONFIG6_END    1255
#define EEPROM_CONFIG7_XSIZE1 1255
#define EEPROM_CONFIG7_YSIZE1 1256
#define EEPROM_CONFIG7_MAP1   1257
#define EEPROM_CONFIG7_XBINS1 1321
#define EEPROM_CONFIG7_YBINS1 1329
#define EEPROM_CONFIG7_XSIZE2 1337
#define EEPROM_CONFIG7_YSIZE2 1338
#define EEPROM_CONFIG7_MAP2   1339
#define EEPROM_CONFIG7_XBINS2 1403
#define EEPROM_CONFIG7_YBINS2 1411
#define EEPROM_CONFIG7_XSIZE3 1419
#define EEPROM_CONFIG7_YSIZE3 1420
#define EEPROM_CONFIG7_MAP3   1421
#define EEPROM_CONFIG7_XBINS3 1485
#define EEPROM_CONFIG7_YBINS3 1493
#define EEPROM_CONFIG7_END    1501
#define EEPROM_CONFIG8_XSIZE1 1501
#define EEPROM_CONFIG8_YSIZE1 1502
#define EEPROM_CONFIG8_MAP1   1503
#define EEPROM_CONFIG8_XBINS1 1539
#define EEPROM_CONFIG8_YBINS1 1545
#define EEPROM_CONFIG8_XSIZE2 1551
#define EEPROM_CONFIG8_YSIZE2 1552
#define EEPROM_CONFIG8_MAP2   1553
#define EEPROM_CONFIG8_XBINS2 1589
#define EEPROM_CONFIG8_YBINS2 1595
#define EEPROM_CONFIG8_XSIZE3 1601
#define EEPROM_CONFIG8_YSIZE3 1602
#define EEPROM_CONFIG8_MAP3   1603
#define EEPROM_CONFIG8_XBINS3 1639
#define EEPROM_CONFIG8_YBINS3 1645
#define EEPROM_CONFIG8_XSIZE4 1651
#define EEPROM_CONFIG8_YSIZE4 1652
#define EEPROM_CONFIG8_MAP4   1653
#define EEPROM_CONFIG8_XBINS4 1689
#define EEPROM_CONFIG8_YBINS4 1695
#define EEPROM_CONFIG9_START  1710
#define EEPROM_CONFIG9_END    1902
#define EEPROM_CONFIG10_START 1902
#define EEPROM_CONFIG10_END   2094
#define EEPROM_CONFIG11_XSIZE 2094
#define EEPROM_CONFIG11_YSIZE 2095
#define EEPROM_CONFIG11_MAP   2096
#define EEPROM_CONFIG11_XBINS 2352
#define EEPROM_CONFIG11_YBINS 2369
#define EEPROM_CONFIG11_END   2385
#define EEPROM_CONFIG12_XSIZE 2385
#define EEPROM_CONFIG12_YSIZE 2386
#define EEPROM_CONFIG12_MAP   2387
#define EEPROM_CONFIG12_XBINS 2451
#define EEPROM_CONFIG12_YBINS 2459
#define EEPROM_CONFIG12_XSIZE2 2467
//#define EEPROM_CONFIG12_YSIZE2 2468
//#define EEPROM_CONFIG12_MAP2   2469
//#define EEPROM_CONFIG12_XBINS2 2533
//#define EEPROM_CONFIG12_YBINS2 2541
#define EEPROM_CONFIG12_XSIZE3 2549
#define EEPROM_CONFIG12_YSIZE3 2550
#define EEPROM_CONFIG12_MAP3   2551
#define EEPROM_CONFIG12_XBINS3 2567
#define EEPROM_CONFIG12_YBINS3 2571
#define EEPROM_CONFIG12_END   2575
#define EEPROM_CONFIG13_START 2580
#define EEPROM_CONFIG13_END   2708
#define EEPROM_CONFIG14_XSIZE 2708
#define EEPROM_CONFIG14_YSIZE 2709
#define EEPROM_CONFIG14_MAP   2710
#define EEPROM_CONFIG14_XBINS 2966
#define EEPROM_CONFIG14_YBINS 2982
#define EEPROM_CONFIG14_END   2998
//This is OUT OF ORDER as Table 8 was expanded to add fuel trim 5-8. The EEPROM for them is simply added here so as not to impact existing tunes
#define EEPROM_CONFIG8_XSIZE5 2999
#define EEPROM_CONFIG8_YSIZE5 3000
#define EEPROM_CONFIG8_MAP5   3001
#define EEPROM_CONFIG8_XBINS5 3037
#define EEPROM_CONFIG8_YBINS5 3043
#define EEPROM_CONFIG8_XSIZE6 3049
#define EEPROM_CONFIG8_YSIZE6 3050
#define EEPROM_CONFIG8_MAP6   3051
#define EEPROM_CONFIG8_XBINS6 3087
#define EEPROM_CONFIG8_YBINS6 3093
#define EEPROM_CONFIG8_XSIZE7 3099
#define EEPROM_CONFIG8_YSIZE7 3100
#define EEPROM_CONFIG8_MAP7   3101
#define EEPROM_CONFIG8_XBINS7 3137
#define EEPROM_CONFIG8_YBINS7 3143
#define EEPROM_CONFIG8_XSIZE8 3149
#define EEPROM_CONFIG8_YSIZE8 3150
#define EEPROM_CONFIG8_MAP8   3151
#define EEPROM_CONFIG8_XBINS8 3187
#define EEPROM_CONFIG8_YBINS8 3193


//Calibration data is stored at the end of the EEPROM (This is in case any further calibration tables are needed as they are large blocks)
#define EEPROM_PAGE_CRC32     3686 //Size of this is 4 * <number of pages> (CRC32 = 32 bits): 3742 - (14 * 4) = 3686
#define EEPROM_LAST_BARO      3742 // 3743 - 1
//New values using 2D tables
#define EEPROM_CALIBRATION_O2   3743 //3839-96 +64
#define EEPROM_CALIBRATION_IAT  3839 //3967-128
#define EEPROM_CALIBRATION_CLT  3967 //4095-128
//These were the values used previously when all calibration tables were 512 long. They need to be retained for the update process (202005 -> 202008) can work. 
#define EEPROM_CALIBRATION_O2_OLD   2559
#define EEPROM_CALIBRATION_IAT_OLD  3071
#define EEPROM_CALIBRATION_CLT_OLD  3583

#endif // STORAGE_H
