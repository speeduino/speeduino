/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
/** @file
 * Lower level ConfigPage*, Table2D, Table3D and EEPROM storage operations.
 */

#include "globals.h"
#include "storage.h"
#include "pages.h"
#include "sensors.h"
#include "utilities.h"
#include "preprocessor.h"
#include "unit_testing.h"

#if defined(CORE_AVR)
#pragma GCC push_options
// This minimizes RAM usage at no performance cost
#pragma GCC optimize ("Os") 
#endif

static byte default_read(uint16_t) {
  return 0U;
}
static void default_write(uint16_t, byte) {
  ;
}
static uint16_t default_length(void) {
  return 0U;
}
static storage_api_t externalApi = {
    .read = default_read,
    .write = default_write,
    .length = default_length,
  };

void setStorageAPI(const storage_api_t &api) {
  externalApi = api;
}

// Calibration data is stored at the end of the EEPROM (This is in case any further calibration tables are needed as they are large blocks)
constexpr uint16_t EEPROM_DATA_VERSION = 0;
constexpr uint16_t STORAGE_END = 0xFFF;
constexpr uint16_t EEPROM_CALIBRATION_CLT_VALUES = STORAGE_END-(uint16_t)sizeof(decltype(cltCalibrationTable)::values);
constexpr uint16_t EEPROM_CALIBRATION_CLT_BINS =  EEPROM_CALIBRATION_CLT_VALUES-(uint16_t)sizeof(decltype(cltCalibrationTable)::axis);
constexpr uint16_t EEPROM_CALIBRATION_IAT_VALUES = EEPROM_CALIBRATION_CLT_BINS-(uint16_t)sizeof(decltype(iatCalibrationTable)::values);
constexpr uint16_t EEPROM_CALIBRATION_IAT_BINS = EEPROM_CALIBRATION_IAT_VALUES-(uint16_t)sizeof(decltype(iatCalibrationTable)::axis);
constexpr uint16_t EEPROM_CALIBRATION_O2_VALUES = EEPROM_CALIBRATION_IAT_BINS-(uint16_t)sizeof(decltype(o2CalibrationTable)::values);
constexpr uint16_t EEPROM_CALIBRATION_O2_BINS =   EEPROM_CALIBRATION_O2_VALUES-(uint16_t)sizeof(decltype(o2CalibrationTable)::axis);
constexpr uint16_t EEPROM_LAST_BARO = (EEPROM_CALIBRATION_O2_BINS-(uint16_t)1);

constexpr uint16_t EEPROM_CONFIG1_MAP    = 3;
constexpr uint16_t EEPROM_CONFIG2_START  = 291;
constexpr uint16_t EEPROM_CONFIG3_MAP    = 421;
constexpr uint16_t EEPROM_CONFIG4_START  = 709;
constexpr uint16_t EEPROM_CONFIG5_MAP    = 839;
constexpr uint16_t EEPROM_CONFIG6_START  = 1127;
constexpr uint16_t EEPROM_CONFIG7_MAP1   = 1257;
constexpr uint16_t EEPROM_CONFIG7_MAP2   = 1339;
constexpr uint16_t EEPROM_CONFIG7_MAP3   = 1421;
constexpr uint16_t EEPROM_CONFIG8_MAP1   = 1503;
constexpr uint16_t EEPROM_CONFIG8_MAP2   = 1553;
constexpr uint16_t EEPROM_CONFIG8_MAP3   = 1603;
constexpr uint16_t EEPROM_CONFIG8_MAP4   = 1653;
constexpr uint16_t EEPROM_CONFIG9_START  = 1710;
constexpr uint16_t EEPROM_CONFIG10_START = 1902;
constexpr uint16_t EEPROM_CONFIG11_MAP   = 2096;
constexpr uint16_t EEPROM_CONFIG12_MAP   = 2387;
constexpr uint16_t EEPROM_CONFIG12_MAP2  = 2469;
constexpr uint16_t EEPROM_CONFIG12_MAP3  = 2551;
constexpr uint16_t EEPROM_CONFIG13_START = 2580;
constexpr uint16_t EEPROM_CONFIG14_MAP   = 2710;
//This is OUT OF ORDER as Page 8 was expanded to add fuel trim tables 5-8. The EEPROM for them is simply added here so as not to impact existing tunes
constexpr uint16_t EEPROM_CONFIG8_MAP5   = 3001;
constexpr uint16_t EEPROM_CONFIG8_MAP6   = 3051;
constexpr uint16_t EEPROM_CONFIG8_MAP7   = 3101;
constexpr uint16_t EEPROM_CONFIG8_MAP8   = 3151;
//Page 15 added after OUT OF ORDER page 8
constexpr uint16_t EEPROM_CONFIG15_MAP   = 3199;
constexpr uint16_t EEPROM_CONFIG15_START = 3281;

#if defined(UNIT_TEST)
uint16_t MAX_PAGE_ADDRESS = EEPROM_LAST_BARO-sizeof(uint8_t);
uint16_t STORAGE_SIZE = STORAGE_END;

// Maps an entity to it's storage start address on the EEPROM.
//
// This is *THE* single source of truth for mapping the tune
// (I.e page entities) to EEPROM locations.
TESTABLE_STATIC uint16_t getEntityStartAddress(page_iterator_t entity) {
  struct entity_storage_map_t {
      void *pEntity;
      uint16_t eepromStartAddress;
  };
  // Store a map of entity to EEPROM address in FLASH memory.
  static const entity_storage_map_t entityMap[] PROGMEM = {
    { &fuelTable, EEPROM_CONFIG1_MAP },
    { &configPage2, EEPROM_CONFIG2_START },
    { &ignitionTable, EEPROM_CONFIG3_MAP },
    { &configPage4, EEPROM_CONFIG4_START },
    { &afrTable, EEPROM_CONFIG5_MAP },
    { &configPage6, EEPROM_CONFIG6_START },
    { &boostTable, EEPROM_CONFIG7_MAP1 }, 
    { &vvtTable, EEPROM_CONFIG7_MAP2 }, 
    { &stagingTable, EEPROM_CONFIG7_MAP3 },
    { &trim1Table, EEPROM_CONFIG8_MAP1 },
    { &trim2Table, EEPROM_CONFIG8_MAP2 },
    { &trim3Table, EEPROM_CONFIG8_MAP3 },
    { &trim4Table, EEPROM_CONFIG8_MAP4 },
    { &trim5Table, EEPROM_CONFIG8_MAP5 },
    { &trim6Table, EEPROM_CONFIG8_MAP6 },
    { &trim7Table, EEPROM_CONFIG8_MAP7 },
    { &trim8Table, EEPROM_CONFIG8_MAP8 },
    { &configPage9, EEPROM_CONFIG9_START },
    { &configPage10, EEPROM_CONFIG10_START },
    { &fuelTable2, EEPROM_CONFIG11_MAP },
    { &wmiTable, EEPROM_CONFIG12_MAP },
    { &vvt2Table, EEPROM_CONFIG12_MAP2 },
    { &dwellTable, EEPROM_CONFIG12_MAP3 },
    { &configPage13, EEPROM_CONFIG13_START },
    { &ignitionTable2, EEPROM_CONFIG14_MAP },
    { &boostTableLookupDuty, EEPROM_CONFIG15_MAP },
    { &configPage15, EEPROM_CONFIG15_START },
  };
  static const constexpr entity_storage_map_t* entityMapEnd = entityMap + _countof(entityMap);

  // Linear search of the address map.
  const entity_storage_map_t *pMapEntry = entityMap;
  while ((pMapEntry!=entityMapEnd) && (entity.pData!=pgm_read_ptr(&pMapEntry->pEntity))) {
    ++pMapEntry;
  }
  uint16_t address = 0U;
  if (pMapEntry!=entityMapEnd) {
    address = pgm_read_word(&(pMapEntry->eepromStartAddress));
  }
  return address;
}

#endif

bool isEepromWritePending(void)
{
  return currentStatus.burnPending;
}

void setEepromWritePending(bool isPending) {
  currentStatus.burnPending = isPending;
}

void saveAllPages(void)
{
  setEepromWritePending(false);

  uint8_t pageCount = getPageCount();
  uint8_t page = 1U;
  while (page<pageCount && !isEepromWritePending())
  {
    savePage(page);
    ++page;
  }
}

//  ================================= Internal write support ===============================
struct write_location{
  uint16_t address;
  uint16_t writesRemaining;
};

static inline uint16_t write_range(const byte *pStart, const byte *pEnd, uint16_t address, uint16_t writesRemaining)
{
  return updateBlockLimitWriteOps(externalApi, address, pStart, pEnd, writesRemaining);
}

static inline write_location write(const table_row_iterator &row, const write_location &location)
{
  return { (uint16_t)(location.address+row.size()), updateBlockLimitWriteOps(externalApi, location.address, &*row, row.end(), location.writesRemaining) };
}

static inline write_location write(table_value_iterator it, write_location location)
{
  while (location.writesRemaining>0U && !it.at_end())
  {
    location = write(it.operator*(), location);
    ++it;
  }
  return location;
}

static inline write_location write(table_axis_iterator it, write_location location)
{
  while (location.writesRemaining>0U && !it.at_end())
  {
    if (update(externalApi, location.address, it.operator*())) {
      --location.writesRemaining;
    }
    ++location.address;
    ++it;
  }
  return location;
}

static inline uint16_t writeTable(void *pTable, table_type_t key, uint16_t address, uint16_t writesRemaining)
{
  return write(y_rbegin(pTable, key), 
                write(x_begin(pTable, key), 
                  write(rows_begin(pTable, key), { address, writesRemaining }))).writesRemaining;
}

//  ================================= End write support ===============================

void savePage(uint8_t pageNum)
{
  uint16_t writesRemaining = getEepromWriteBlockSize(currentStatus);

  switch(pageNum)
  {
    case veMapPage:
      /*---------------------------------------------------
      | Fuel table (See storage.h for data layout) - Page 1
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      writesRemaining = writeTable(&fuelTable, decltype(fuelTable)::type_key, EEPROM_CONFIG1_MAP, writesRemaining);
      break;

    case veSetPage:
      /*---------------------------------------------------
      | Config page 2 (See storage.h for data layout)
      | 64 byte long config table
      -----------------------------------------------------*/
      writesRemaining = write_range((byte *)&configPage2, (byte *)&configPage2+sizeof(configPage2), EEPROM_CONFIG2_START, writesRemaining);
      break;

    case ignMapPage:
      /*---------------------------------------------------
      | Ignition table (See storage.h for data layout) - Page 1
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      writesRemaining = writeTable(&ignitionTable, decltype(ignitionTable)::type_key, EEPROM_CONFIG3_MAP, writesRemaining);
      break;

    case ignSetPage:
      /*---------------------------------------------------
      | Config page 2 (See storage.h for data layout)
      | 64 byte long config table
      -----------------------------------------------------*/
      writesRemaining = write_range((byte *)&configPage4, (byte *)&configPage4+sizeof(configPage4), EEPROM_CONFIG4_START, writesRemaining);
      break;

    case afrMapPage:
      /*---------------------------------------------------
      | AFR table (See storage.h for data layout) - Page 5
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      writesRemaining = writeTable(&afrTable, decltype(afrTable)::type_key, EEPROM_CONFIG5_MAP, writesRemaining);
      break;

    case afrSetPage:
      /*---------------------------------------------------
      | Config page 3 (See storage.h for data layout)
      | 64 byte long config table
      -----------------------------------------------------*/
      writesRemaining = write_range((byte *)&configPage6, (byte *)&configPage6+sizeof(configPage6), EEPROM_CONFIG6_START, writesRemaining);
      break;

    case boostvvtPage:
      /*---------------------------------------------------
      | Boost and vvt tables (See storage.h for data layout) - Page 8
      | 8x8 table itself + the 8 values along each of the axis
      -----------------------------------------------------*/
      writesRemaining = writeTable(&boostTable, decltype(boostTable)::type_key, EEPROM_CONFIG7_MAP1, writesRemaining);
      writesRemaining = writeTable(&vvtTable, decltype(vvtTable)::type_key, EEPROM_CONFIG7_MAP2, writesRemaining);
      writesRemaining = writeTable(&stagingTable, decltype(stagingTable)::type_key, EEPROM_CONFIG7_MAP3, writesRemaining);
      break;

    case seqFuelPage:
      /*---------------------------------------------------
      | Fuel trim tables (See storage.h for data layout) - Page 9
      | 6x6 tables itself + the 6 values along each of the axis
      -----------------------------------------------------*/
      writesRemaining = writeTable(&trim1Table, decltype(trim1Table)::type_key, EEPROM_CONFIG8_MAP1, writesRemaining);
      writesRemaining = writeTable(&trim2Table, decltype(trim2Table)::type_key, EEPROM_CONFIG8_MAP2, writesRemaining);
      writesRemaining = writeTable(&trim3Table, decltype(trim3Table)::type_key, EEPROM_CONFIG8_MAP3, writesRemaining);
      writesRemaining = writeTable(&trim4Table, decltype(trim4Table)::type_key, EEPROM_CONFIG8_MAP4, writesRemaining);
      writesRemaining = writeTable(&trim5Table, decltype(trim5Table)::type_key, EEPROM_CONFIG8_MAP5, writesRemaining);
      writesRemaining = writeTable(&trim6Table, decltype(trim6Table)::type_key, EEPROM_CONFIG8_MAP6, writesRemaining);
      writesRemaining = writeTable(&trim7Table, decltype(trim7Table)::type_key, EEPROM_CONFIG8_MAP7, writesRemaining);
      writesRemaining = writeTable(&trim8Table, decltype(trim8Table)::type_key, EEPROM_CONFIG8_MAP8, writesRemaining);
      break;

    case canbusPage:
      /*---------------------------------------------------
      | Config page 10 (See storage.h for data layout)
      | 192 byte long config table
      -----------------------------------------------------*/
      writesRemaining = write_range((byte *)&configPage9, (byte *)&configPage9+sizeof(configPage9), EEPROM_CONFIG9_START, writesRemaining);
      break;

    case warmupPage:
      /*---------------------------------------------------
      | Config page 11 (See storage.h for data layout)
      | 192 byte long config table
      -----------------------------------------------------*/
      writesRemaining = write_range((byte *)&configPage10, (byte *)&configPage10+sizeof(configPage10), EEPROM_CONFIG10_START, writesRemaining);
      break;

    case fuelMap2Page:
      /*---------------------------------------------------
      | Fuel table 2 (See storage.h for data layout)
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      writesRemaining = writeTable(&fuelTable2, decltype(fuelTable2)::type_key, EEPROM_CONFIG11_MAP, writesRemaining);
      break;

    case wmiMapPage:
      /*---------------------------------------------------
      | WMI and Dwell tables (See storage.h for data layout) - Page 12
      | 8x8 WMI table itself + the 8 values along each of the axis
      | 8x8 VVT2 table + the 8 values along each of the axis
      | 4x4 Dwell table itself + the 4 values along each of the axis
      -----------------------------------------------------*/
      writesRemaining = writeTable(&wmiTable, decltype(wmiTable)::type_key, EEPROM_CONFIG12_MAP, writesRemaining);
      writesRemaining = writeTable(&vvt2Table, decltype(vvt2Table)::type_key, EEPROM_CONFIG12_MAP2, writesRemaining);
      writesRemaining = writeTable(&dwellTable, decltype(dwellTable)::type_key, EEPROM_CONFIG12_MAP3, writesRemaining);
      break;
      
    case progOutsPage:
      /*---------------------------------------------------
      | Config page 13 (See storage.h for data layout)
      -----------------------------------------------------*/
      writesRemaining = write_range((byte *)&configPage13, (byte *)&configPage13+sizeof(configPage13), EEPROM_CONFIG13_START, writesRemaining);
      break;
    
    case ignMap2Page:
      /*---------------------------------------------------
      | Ignition table (See storage.h for data layout) - Page 1
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      writesRemaining = writeTable(&ignitionTable2, decltype(ignitionTable2)::type_key, EEPROM_CONFIG14_MAP, writesRemaining);
      break;

    case boostvvtPage2:
      /*---------------------------------------------------
      | Boost duty cycle lookuptable (See storage.h for data layout) - Page 15
      | 8x8 table itself + the 8 values along each of the axis
      -----------------------------------------------------*/
      writesRemaining = writeTable(&boostTableLookupDuty, decltype(boostTableLookupDuty)::type_key, EEPROM_CONFIG15_MAP, writesRemaining);

      /*---------------------------------------------------
      | Config page 15 (See storage.h for data layout)
      -----------------------------------------------------*/
      writesRemaining = write_range((byte *)&configPage15, (byte *)&configPage15+sizeof(configPage15), EEPROM_CONFIG15_START, writesRemaining);
      break;

    default:
      break;
  }

  setEepromWritePending(writesRemaining==0U);
}

//  ================================= Internal read support ===============================

/** Load range of bytes form EEPROM offset to memory.
 * @param address - start offset in EEPROM
 * @param pFirst - Start memory address
 * @param pLast - End memory address
 */
static inline uint16_t load_range(uint16_t address, byte *pFirst, const byte *pLast)
{
  return loadBlock(externalApi, address, pFirst, pLast);
}

static inline uint16_t load(table_row_iterator row, uint16_t address)
{
  return load_range(address, &*row, row.end());
}

static inline uint16_t load(table_value_iterator it, uint16_t address)
{
  while (!it.at_end())
  {
    address = load(*it, address);
    ++it;
  }
  return address; 
}

static inline uint16_t load(table_axis_iterator it, uint16_t address)
{
  while (!it.at_end())
  {
    *it = externalApi.read(address);
    ++address;
    ++it;
  }
  return address;    
}


static inline uint16_t loadTable(void *pTable, table_type_t key, uint16_t address)
{
  return load(y_rbegin(pTable, key),
                load(x_begin(pTable, key), 
                  load(rows_begin(pTable, key), address)));
}

//  ================================= End internal read support ===============================

void loadAllPages(void)
{
  loadTable(&fuelTable, decltype(fuelTable)::type_key, EEPROM_CONFIG1_MAP);
  load_range(EEPROM_CONFIG2_START, (byte *)&configPage2, (byte *)&configPage2+sizeof(configPage2));
  
  //*********************************************************************************************************************************************************************************
  //IGNITION CONFIG PAGE (2)

  loadTable(&ignitionTable, decltype(ignitionTable)::type_key, EEPROM_CONFIG3_MAP);
  load_range(EEPROM_CONFIG4_START, (byte *)&configPage4, (byte *)&configPage4+sizeof(configPage4));

  //*********************************************************************************************************************************************************************************
  //AFR TARGET CONFIG PAGE (3)

  loadTable(&afrTable, decltype(afrTable)::type_key, EEPROM_CONFIG5_MAP);
  load_range(EEPROM_CONFIG6_START, (byte *)&configPage6, (byte *)&configPage6+sizeof(configPage6));

  //*********************************************************************************************************************************************************************************
  // Boost and vvt tables load
  loadTable(&boostTable, decltype(boostTable)::type_key, EEPROM_CONFIG7_MAP1);
  loadTable(&vvtTable, decltype(vvtTable)::type_key,  EEPROM_CONFIG7_MAP2);
  loadTable(&stagingTable, decltype(stagingTable)::type_key, EEPROM_CONFIG7_MAP3);

  //*********************************************************************************************************************************************************************************
  // Fuel trim tables load
  loadTable(&trim1Table, decltype(trim1Table)::type_key, EEPROM_CONFIG8_MAP1);
  loadTable(&trim2Table, decltype(trim2Table)::type_key, EEPROM_CONFIG8_MAP2);
  loadTable(&trim3Table, decltype(trim3Table)::type_key, EEPROM_CONFIG8_MAP3);
  loadTable(&trim4Table, decltype(trim4Table)::type_key, EEPROM_CONFIG8_MAP4);
  loadTable(&trim5Table, decltype(trim5Table)::type_key, EEPROM_CONFIG8_MAP5);
  loadTable(&trim6Table, decltype(trim6Table)::type_key, EEPROM_CONFIG8_MAP6);
  loadTable(&trim7Table, decltype(trim7Table)::type_key, EEPROM_CONFIG8_MAP7);
  loadTable(&trim8Table, decltype(trim8Table)::type_key, EEPROM_CONFIG8_MAP8);

  //*********************************************************************************************************************************************************************************
  //canbus control page load
  load_range(EEPROM_CONFIG9_START, (byte *)&configPage9, (byte *)&configPage9+sizeof(configPage9));

  //*********************************************************************************************************************************************************************************

  //CONFIG PAGE (10)
  load_range(EEPROM_CONFIG10_START, (byte *)&configPage10, (byte *)&configPage10+sizeof(configPage10));

  //*********************************************************************************************************************************************************************************
  //Fuel table 2 (See storage.h for data layout)
  loadTable(&fuelTable2, decltype(fuelTable2)::type_key, EEPROM_CONFIG11_MAP);

  //*********************************************************************************************************************************************************************************
  // WMI, VVT2 and Dwell table load
  loadTable(&wmiTable, decltype(wmiTable)::type_key, EEPROM_CONFIG12_MAP);
  loadTable(&vvt2Table, decltype(vvt2Table)::type_key, EEPROM_CONFIG12_MAP2);
  loadTable(&dwellTable, decltype(dwellTable)::type_key, EEPROM_CONFIG12_MAP3);

  //*********************************************************************************************************************************************************************************
  //CONFIG PAGE (13)
  load_range(EEPROM_CONFIG13_START, (byte *)&configPage13, (byte *)&configPage13+sizeof(configPage13));

  //*********************************************************************************************************************************************************************************
  //SECOND IGNITION CONFIG PAGE (14)

  loadTable(&ignitionTable2, decltype(ignitionTable2)::type_key, EEPROM_CONFIG14_MAP);

  //*********************************************************************************************************************************************************************************
  //CONFIG PAGE (15) + boost duty lookup table (LUT)
  loadTable(&boostTableLookupDuty, decltype(boostTableLookupDuty)::type_key, EEPROM_CONFIG15_MAP);
  load_range(EEPROM_CONFIG15_START, (byte *)&configPage15, (byte *)&configPage15+sizeof(configPage15));  

  //*********************************************************************************************************************************************************************************
}

void loadAllCalibrationTables(void)
{
  // If you modify this function be sure to also modify saveAllCalibrationTables();
  // it should be a mirror image of this function. 
  (void)loadObject(externalApi, EEPROM_CALIBRATION_O2_BINS, o2CalibrationTable.axis);
  (void)loadObject(externalApi, EEPROM_CALIBRATION_O2_VALUES, o2CalibrationTable.values);

  (void)loadObject(externalApi, EEPROM_CALIBRATION_IAT_BINS, iatCalibrationTable.axis);
  (void)loadObject(externalApi, EEPROM_CALIBRATION_IAT_VALUES, iatCalibrationTable.values);

  (void)loadObject(externalApi, EEPROM_CALIBRATION_CLT_BINS, cltCalibrationTable.axis);
  (void)loadObject(externalApi, EEPROM_CALIBRATION_CLT_VALUES, cltCalibrationTable.values);
}

/** Write calibration tables to EEPROM.
This takes the values in the 3 calibration tables (Coolant, Inlet temp and O2)
and saves them to the EEPROM.
*/
void saveAllCalibrationTables(void)
{
  // If you modify this function be sure to also modify loadAllCalibrationTables();
  // it should be a mirror image of this function.

  saveCalibrationTable(SensorCalibrationTable::O2Sensor);
  saveCalibrationTable(SensorCalibrationTable::IntakeAirTempSensor);
  saveCalibrationTable(SensorCalibrationTable::CoolantSensor);
}

void saveCalibrationTable(SensorCalibrationTable sensor)
{
  if(sensor == SensorCalibrationTable::O2Sensor)
  {
    updateObject(externalApi, o2CalibrationTable.axis, EEPROM_CALIBRATION_O2_BINS);
    updateObject(externalApi, o2CalibrationTable.values, EEPROM_CALIBRATION_O2_VALUES);
  }
  else if(sensor == SensorCalibrationTable::IntakeAirTempSensor)
  {
    updateObject(externalApi, iatCalibrationTable.axis, EEPROM_CALIBRATION_IAT_BINS);
    updateObject(externalApi, iatCalibrationTable.values, EEPROM_CALIBRATION_IAT_VALUES);
  }
  else if(sensor == SensorCalibrationTable::CoolantSensor)
  {
    updateObject(externalApi, cltCalibrationTable.axis, EEPROM_CALIBRATION_CLT_BINS);
    updateObject(externalApi, cltCalibrationTable.values, EEPROM_CALIBRATION_CLT_VALUES);
  }
}

TESTABLE_INLINE_STATIC uint16_t getSensorCalibrationCrcAddress(SensorCalibrationTable sensor) {
  constexpr uint16_t EEPROM_CALIBRATION_CLT_CRC = 3674;
  constexpr uint16_t EEPROM_CALIBRATION_IAT_CRC = 3678;
  constexpr uint16_t EEPROM_CALIBRATION_O2_CRC = 3682;

  switch(sensor)
  {
    case SensorCalibrationTable::O2Sensor:
      return EEPROM_CALIBRATION_O2_CRC;
    case SensorCalibrationTable::IntakeAirTempSensor:
      return EEPROM_CALIBRATION_IAT_CRC;
    case SensorCalibrationTable::CoolantSensor:
    default: //Obviously should never happen
      return EEPROM_CALIBRATION_CLT_CRC;
  }
  return EEPROM_CALIBRATION_CLT_CRC;
}

void saveCalibrationCrc(SensorCalibrationTable sensor, uint32_t calibrationCRC)
{
  updateObject(externalApi, getSensorCalibrationCrcAddress(sensor), calibrationCRC);
}

/** Retrieves and returns the 4 byte CRC32 checksum for a given calibration page from EEPROM. */
uint32_t loadCalibrationCrc(SensorCalibrationTable sensor)
{
  uint32_t crc32_val;
  return loadObject(externalApi, getSensorCalibrationCrcAddress(sensor), crc32_val);
}

uint16_t getEEPROMSize(void)
{
  return externalApi.length();
}

// Utility functions.
// By having these in this file, it prevents other files from calling EEPROM functions directly. This is useful due to differences in the EEPROM libraries on different devces

void EEPROMWriteRaw(uint16_t address, byte data) { (void)update(externalApi, address, data); }
byte EEPROMReadRaw(uint16_t address) { return externalApi.read(address); }

uint8_t loadLastBaro(void) { return externalApi.read(EEPROM_LAST_BARO); }
void saveLastBaro(uint8_t newValue) { (void)update(externalApi, EEPROM_LAST_BARO, newValue); }

uint8_t loadEEPROMVersion(void) { return externalApi.read(EEPROM_DATA_VERSION); }
void saveEEPROMVersion(uint8_t newVersion) { (void)update(externalApi, EEPROM_DATA_VERSION, newVersion); }

void clearStorage(void) {
  #if defined(FLASH_AS_EEPROM_h)
    getEEPROM().read(0); //needed for SPI eeprom emulation.
    getEEPROM().clear(); 
  #else 
    for (uint16_t i = 0 ; i < externalApi.length() ; i++) { externalApi.write((uint16_t)i, UINT8_MAX);}
  #endif  
}

#if defined(CORE_AVR)
#pragma GCC pop_options
#endif