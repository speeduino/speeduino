#ifndef STORAGE_H
#define STORAGE_H

/** @file storage.h
 * @brief Functions for reading and writing user settings to/from EEPROM
 *
 * See the output from the print_eeprom_layout() unit test for the layout of the EEPROM.
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