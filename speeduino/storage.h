#ifndef STORAGE_H
#define STORAGE_H

/** @file storage.h
 * @brief Functions for reading and writing user settings to/from EEPROM
 *
 * Current layout of EEPROM is as follows (Version 18):
 *
 * |Offset (Dec)|Size (Bytes)| Description                          | Reference                          |
 * | ---------: | :--------: | :----------------------------------: | :--------------------------------- |
 * | 0          |1           | EEPROM version                       | @ref EEPROM_DATA_VERSION           |
 * | 1          |2           | HOLE ??                              |                                    |
 * | 3          |256         | Fuel table (16x16)                   | @ref EEPROM_CONFIG1_MAP            |
 * | 259        |16          | Fuel table (X axis) (RPM)            |                                    |
 * | 275        |16          | Fuel table (Y axis) (MAP/TPS)        |                                    |
 * | 291        |128         | Page 2 settings                      | @ref EEPROM_CONFIG2_START          |
 * | 419        |2           | HOLE ??                              |                                    |
 * | 421        |256         | Ignition table (16x16)               | @ref EEPROM_CONFIG3_MAP            |
 * | 677        |16          | Ignition table (X axis) (RPM)        |                                    |
 * | 693        |16          | Ignition table (Y axis) (MAP/TPS)    |                                    |
 * | 709        |128         | Page 4 settings                      | @ref EEPROM_CONFIG4_START          |
 * | 837        |2           | HOLE ??                              |                                    |
 * | 839        |256         | AFR target table (16x16)             | @ref EEPROM_CONFIG5_MAP            |
 * | 1095       |16          | AFR target table (X axis) (RPM)      |                                    |
 * | 1111       |16          | AFR target table (Y axis) (MAP/TPS)  |                                    |
 * | 1127       |128         | Page 6 settings                      | @ref EEPROM_CONFIG6_START          |
 * | 1255       |2           | HOLE ??                              |                                    |
 * | 1257       |64          | Boost table (8x8)                    | @ref EEPROM_CONFIG7_MAP1           |
 * | 1321       |8           | Boost table (X axis) (RPM)           |                                    |
 * | 1329       |8           | Boost table (Y axis) (TPS)           |                                    |
 * | 1337       |2           | HOLE ??                              |                                    |
 * | 1339       |64          | VVT table (8x8)                      | @ref EEPROM_CONFIG7_MAP2           |
 * | 1403       |8           | VVT table (X axis) (RPM)             |                                    |
 * | 1411       |8           | VVT table (Y axis) (MAP)             |                                    |
 * | 1419       |2           | HOLE ??                              |                                    |
 * | 1421       |64          | Staging table (8x8)                  | @ref EEPROM_CONFIG7_MAP3           |
 * | 1485       |8           | Staging table (X axis) (RPM)         |                                    |
 * | 1493       |8           | Staging table (Y axis) (MAP)         |                                    |
 * | 1501       |2           | HOLE ??                              |                                    |
 * | 1503       |36          | Trim1 table (6x6)                    | @ref EEPROM_CONFIG8_MAP1           |
 * | 1539       |6           | Trim1 table (X axis) (RPM)           |                                    |
 * | 1545       |6           | Trim1 table (Y axis) (MAP)           |                                    |
 * | 1551       |2           | HOLE ??                              |                                    |
 * | 1553       |36          | Trim2 table (6x6)                    | @ref EEPROM_CONFIG8_MAP2           |
 * | 1589       |6           | Trim2 table (X axis) (RPM)           |                                    |
 * | 1595       |6           | Trim2 table (Y axis) (MAP)           |                                    |
 * | 1601       |2           | HOLE ??                              |                                    |
 * | 1603       |36          | Trim3 table (6x6)                    | @ref EEPROM_CONFIG8_MAP3           |
 * | 1639       |6           | Trim3 table (X axis) (RPM)           |                                    |
 * | 1545       |6           | Trim3 table (Y axis) (MAP)           |                                    |
 * | 1651       |2           | HOLE ??                              |                                    |
 * | 1653       |36          | Trim4 table (6x6)                    | @ref EEPROM_CONFIG8_MAP4           |
 * | 1689       |6           | Trim4 table (X axis) (RPM)           |                                    |
 * | 1595       |6           | Trim4 table (Y axis) (MAP)           |                                    |
 * | 1701       |9           | HOLE ??                              |                                    |
 * | 1710       |192         | Page 9 settings                      | @ref EEPROM_CONFIG9_START          |
 * | 1902       |192         | Page 10 settings                     | @ref EEPROM_CONFIG10_START         |
 * | 2094       |2           | HOLE ??                              |                                    |
 * | 2096       |256         | Fuel2 table (16x16)                  | @ref EEPROM_CONFIG11_MAP           |
 * | 2352       |16          | Fuel2 table (X axis) (RPM)           |                                    |
 * | 2368       |16          | Fuel2 table (Y axis) (MAP/TPS)       |                                    |
 * | 2384       |3           | HOLE ??                              |                                    |
 * | 2387       |64          | WMI table (8x8)                      | @ref EEPROM_CONFIG12_MAP           |
 * | 2451       |8           | WMI table (X axis) (RPM)             |                                    |
 * | 2459       |8           | WMI table (Y axis) (MAP)             |                                    |
 * | 2467       |2           | HOLE ??                              |                                    |
 * | 2469       |64          | VVT2 table (8x8)                     | @ref EEPROM_CONFIG12_MAP2          |
 * | 2553       |8           | VVT2 table (X axis) (RPM)            |                                    |
 * | 2541       |8           | VVT2 table (Y axis) (MAP)            |                                    |
 * | 2549       |2           | HOLE ??                              |                                    |
 * | 2551       |16          | Dwell table (4x4)                    | @ref EEPROM_CONFIG12_MAP3          |
 * | 2567       |4           | Dwell table (X axis) (RPM)           |                                    |
 * | 2571       |4           | Dwell table (Y axis) (MAP)           |                                    |
 * | 2575       |5           | HOLE ??                              |                                    |
 * | 2580       |128         | Page 13 settings                     | @ref EEPROM_CONFIG13_START         |
 * | 2708       |2           | HOLE ??                              |                                    |
 * | 2710       |256         | Ignition2 table (16x16)              | @ref EEPROM_CONFIG14_MAP           |
 * | 2966       |16          | Ignition2 table (X axis) (RPM)       |                                    |
 * | 2982       |16          | Ignition2 table (Y axis) (MAP/TPS)   |                                    |
 * | 2998       |3           | HOLE ??                              |                                    |
 * | 3001       |36          | Trim5 table (6x6)                    | @ref EEPROM_CONFIG8_MAP5           |
 * | 3037       |6           | Trim5 table (X axis) (RPM)           |                                    |
 * | 3043       |6           | Trim5 table (Y axis) (MAP)           |                                    |
 * | 3049       |2           | HOLE ??                              |                                    |
 * | 3051       |36          | Trim6 table (6x6)                    | @ref EEPROM_CONFIG8_MAP6           |
 * | 3087       |6           | Trim6 table (X axis) (RPM)           |                                    |
 * | 3093       |6           | Trim6 table (Y axis) (MAP)           |                                    |
 * | 3099       |2           | HOLE ??                              |                                    |
 * | 3101       |36          | Trim7 table (6x6)                    | @ref EEPROM_CONFIG8_MAP7           |
 * | 3137       |6           | Trim7 table (X axis) (RPM)           |                                    |
 * | 3143       |6           | Trim7 table (Y axis) (MAP)           |                                    |
 * | 3149       |2           | HOLE ??                              |                                    |
 * | 3151       |36          | Trim8 table (6x6)                    | @ref EEPROM_CONFIG8_MAP8           |
 * | 3187       |6           | Trim8 table (X axis) (RPM)           |                                    |
 * | 3193       |6           | Trim8 table (Y axis) (MAP)           |                                    |
 * | 3199       |2           | HOLE ??                              |                                    |
 * | 3201       |64          | boostLUT table (8x8)                 | @ref EEPROM_CONFIG15_MAP           |
 * | 3265       |8           | boostLUT table (X axis) (RPM)        |                                    |
 * | 3273       |8           | boostLUT table (Y axis) (targetBoost)|                                    |
 * | 3281       |1           | boostLUT enable                      | @ref EEPROM_CONFIG15_START         |
 * | 3282       |1           | boostDCWhenDisabled                  |                                    |
 * | 3283       |1           | boostControlEnableThreshold          |                                    |
 * | 3284       |14          | A/C Control Settings                 |                                    |
 * | 3298       |159         | Page 15 spare                        |                                    |
 * | 3457       |217         | EMPTY                                |                                    |
 * | 3674       |4           | CLT Calibration CRC32                |                                    |
 * | 3678       |4           | IAT Calibration CRC32                |                                    |
 * | 3682       |4           | O2 Calibration CRC32                 |                                    |
 * | 3686       |56          | HOLE ??                              |                                    |
 * | 3742       |1           | Baro value saved at init             | @ref EEPROM_LAST_BARO              |
 * | 3743       |64          | O2 Calibration Bins                  | @ref EEPROM_CALIBRATION_O2_BINS    |
 * | 3807       |32          | O2 Calibration Values                | @ref EEPROM_CALIBRATION_O2_VALUES  |
 * | 3839       |64          | IAT Calibration Bins                 | @ref EEPROM_CALIBRATION_IAT_BINS   |
 * | 3903       |64          | IAT Calibration Values               | @ref EEPROM_CALIBRATION_IAT_VALUES |
 * | 3967       |64          | CLT Calibration Bins                 | @ref EEPROM_CALIBRATION_CLT_BINS   |
 * | 4031       |64          | CLT Calibration Values               | @ref EEPROM_CALIBRATION_CLT_VALUES |
 * | 4095       |            | END                                  |                                    |
 *
 */

#include <stdint.h>

/**
 * @defgroup storage-pages Page save & load
 * 
 * @brief Functions to save & load "pages" to/from durable storage
 *
 * A "page" is a TunerStudio concept: they are defined in the project ini file.
 * It's basically the tune, minus calibration tables. 
 * @{
 */

/** 
 * @brief Write all pages from RAM to durable storage 
 * 
 * Note that this might not save everything due to write throttling.
 * Callers can keep calling this function until isEepromWritePending returns false.
 */
void saveAllPages(void);

/** @brief Write one page from RAM to durable storage.
 * 
 * Note that this might not save everything due to write throttling.
 * Callers can keep calling this function until isEepromWritePending returns false.
 */
void savePage(uint8_t pageNum);

/** @brief Load all pages from durable storage. I.e. load the tune */
void loadAllPages(void);

/**
 * @brief Do we have page data that needs to be written to durable storage?
 * 
 * @return true Page data needs to be written
 * @return false Nothing needs to be written
 */
bool isEepromWritePending(void);

/**
 * @brief Set or clear the Write Pending flag
 * 
 * @param isPending True if data needs to be written, false otherwise.
 */
void setEepromWritePending(bool isPending);

///@}

/**
 * @defgroup storage-calibration-tables Calibration table save & load
 * 
 * @brief Functions to save & load sensor calibration tables to/from durable storage
 *
 * This is separate from the page load as the calibrations do not exist as pages within 
 * the ini file for Tuner Studio.
 * @{
 */

/** @brief Load the curves for all sensors from durable storage */
void loadAllCalibrationTables(void);

/** @brief Store the curves for all sensors in durable storage */
void saveAllCalibrationTables(void);

/** @brief Enum to identify sensor calibration tables */
enum class SensorCalibrationTable : uint8_t {
    /** @brief The coolant sensor calibration curve */
    CoolantSensor = 0U,
    /** @brief Intake Air Temperature (IAT) sensor calibration curve */
    IntakeAirTempSensor = 1U,
    /** @brief The Oxygen (O2) sensor calibration curve */
    O2Sensor = 2U,
};

/** @brief Store one sensor curve in durable storage */
void saveCalibrationTable(SensorCalibrationTable sensor);

/** @brief Store the CRC for one sensor curve in durable storage */
void saveCalibrationCrc(SensorCalibrationTable sensor, uint32_t crc);

/** @brief Retrieve the CRC for one sensor curve from durable storage */
uint32_t loadCalibrationCrc(SensorCalibrationTable sensor);

///@}

/**
 * @defgroup storage-utility Storage utility functions
 * 
 * @brief Wrappers to prevent other files from having to use EEPROM.h directly
 * @{
 */

/** @brief Last barometer reading cache */
uint8_t loadLastBaro(void);
/** @copydoc loadLastBaro */
void saveLastBaro(uint8_t newValue);

/** @brief Save/load the version number of the storage layout */
uint8_t loadEEPROMVersion(void);
/** @copydoc loadEEPROMVersion */
void saveEEPROMVersion(uint8_t newVersion);

/** @brief Clears all of the durable store */
void clearStorage(void);

///@}

/**
 * @defgroup storage-raw Raw access to the EEPROM
 * 
 * @brief Wrappers to prevent other files from having to use EEPROM.h directly
 * @{
 */

/** @brief Write a single byte to durable storage. 
 * 
 * @warning Use carefully - this could do a lot of damage to the stored tune & tables.
 */
void EEPROMWriteRaw(uint16_t address, byte data);

/** @brief Read a single byte from durable storage */
byte EEPROMReadRaw(uint16_t address);

/** @brief Total number of bytes in durable storage */
uint16_t getEEPROMSize(void);

///@}

#endif // STORAGE_H