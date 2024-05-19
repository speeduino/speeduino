/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
/** @file
 * @brief Lower level ConfigPage* and Table3D storage operations.
 * 
 * Storage notes
 * =============
 *
 * The external unit of storage is "page". We have to use this because that's what 
 * TunerStudio uses: its can send an entire page as a binary data block, sends page based write 
 * commands and uses page based CRC values (it's not a bad choice, there has to be some unit
 * of work/storage defined).
 *
 * Our in memory representation is more granular. Each page is split into 1 or more 
 * entities. Either
 * a. A contiguous block of memory (a struct instance)
 * b. A 3D table
 * 
 * The combination of page+entity is unique: this is enforced in pages.cpp 
 */


#include "globals.h"
#include "storage.h"
#include "pages.h"
#include "sensors.h"
#include "utilities.h"
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

static constexpr uint16_t EEPROM_DATA_VERSION = 0;

void setStorageAPI(const storage_api_t &api) {
  externalApi = api;
}

// Calibration data is stored at the end of the EEPROM (This is in case any further calibration tables are needed as they are large blocks)
static constexpr uint16_t STORAGE_END = 0xFFF;
static constexpr uint16_t EEPROM_CALIBRATION_CLT_VALUES = STORAGE_END-(uint16_t)sizeof(decltype(cltCalibrationTable)::values);
static constexpr uint16_t EEPROM_CALIBRATION_CLT_BINS =  EEPROM_CALIBRATION_CLT_VALUES-(uint16_t)sizeof(decltype(cltCalibrationTable)::axis);
static constexpr uint16_t EEPROM_CALIBRATION_IAT_VALUES = EEPROM_CALIBRATION_CLT_BINS-(uint16_t)sizeof(decltype(iatCalibrationTable)::values);
static constexpr uint16_t EEPROM_CALIBRATION_IAT_BINS = EEPROM_CALIBRATION_IAT_VALUES-(uint16_t)sizeof(decltype(iatCalibrationTable)::axis);
static constexpr uint16_t EEPROM_CALIBRATION_O2_VALUES = EEPROM_CALIBRATION_IAT_BINS-(uint16_t)sizeof(decltype(o2CalibrationTable)::values);
static constexpr uint16_t EEPROM_CALIBRATION_O2_BINS =   EEPROM_CALIBRATION_O2_VALUES-(uint16_t)sizeof(decltype(o2CalibrationTable)::axis);
static constexpr uint16_t EEPROM_LAST_BARO = (EEPROM_CALIBRATION_O2_BINS-(uint16_t)1);

#if defined(UNIT_TEST)
uint16_t MAX_PAGE_ADDRESS = EEPROM_LAST_BARO-sizeof(uint8_t);
uint16_t STORAGE_SIZE = STORAGE_END;
#endif

// Maps an entity to it's storage start address on the EEPROM.
//
// This is *THE* single source of truth for mapping the tune
// (I.e page entities) to EEPROM locations.
TESTABLE_STATIC uint16_t getEntityStartAddress(page_iterator_t entity) {
  static constexpr uint16_t EEPROM_CONFIG1_MAP    = 3;
  static constexpr uint16_t EEPROM_CONFIG2_START  = 291;
  static constexpr uint16_t EEPROM_CONFIG3_MAP    = 421;
  static constexpr uint16_t EEPROM_CONFIG4_START  = 709;
  static constexpr uint16_t EEPROM_CONFIG5_MAP    = 839;
  static constexpr uint16_t EEPROM_CONFIG6_START  = 1127;
  static constexpr uint16_t EEPROM_CONFIG7_MAP1   = 1257;
  static constexpr uint16_t EEPROM_CONFIG7_MAP2   = 1339;
  static constexpr uint16_t EEPROM_CONFIG7_MAP3   = 1421;
  static constexpr uint16_t EEPROM_CONFIG8_MAP1   = 1503;
  static constexpr uint16_t EEPROM_CONFIG8_MAP2   = 1553;
  static constexpr uint16_t EEPROM_CONFIG8_MAP3   = 1603;
  static constexpr uint16_t EEPROM_CONFIG8_MAP4   = 1653;
  static constexpr uint16_t EEPROM_CONFIG9_START  = 1710;
  static constexpr uint16_t EEPROM_CONFIG10_START = 1902;
  static constexpr uint16_t EEPROM_CONFIG11_MAP   = 2096;
  static constexpr uint16_t EEPROM_CONFIG12_MAP   = 2387;
  static constexpr uint16_t EEPROM_CONFIG12_MAP2  = 2469;
  static constexpr uint16_t EEPROM_CONFIG12_MAP3  = 2551;
  static constexpr uint16_t EEPROM_CONFIG13_START = 2580;
  static constexpr uint16_t EEPROM_CONFIG14_MAP   = 2710;
  //This is OUT OF ORDER as Page 8 was expanded to add fuel trim tables 5-8. The EEPROM for them is simply added here so as not to impact existing tunes
  static constexpr uint16_t EEPROM_CONFIG8_MAP5   = 3001;
  static constexpr uint16_t EEPROM_CONFIG8_MAP6   = 3051;
  static constexpr uint16_t EEPROM_CONFIG8_MAP7   = 3101;
  static constexpr uint16_t EEPROM_CONFIG8_MAP8   = 3151;
  //Page 15 added after OUT OF ORDER page 8
  static constexpr uint16_t EEPROM_CONFIG15_MAP   = 3199;
  static constexpr uint16_t EEPROM_CONFIG15_START = 3281;

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
  uint8_t page = 0U;
  while (page<pageCount && !isEepromWritePending())
  {
    savePage(page);
    page = page + 1U;
  }
}

//  ================================= Internal write support ===============================

struct write_location{
  uint16_t address;
  uint16_t maxWrites;
};
static inline write_location write(const table_row_iterator &row, const write_location &location)
{
  return { (uint16_t)(location.address+row.size()), updateBlockLimitWriteOps(externalApi, location.address, &*row, row.end(), location.maxWrites) };
}

static inline write_location write(table_value_iterator it, write_location location)
{
  while (location.maxWrites>0U && !it.at_end())
  {
    location = write(*it, location); //cppcheck-suppress misra-c2012-17.2
    ++it;
  }
  return location;
}

static inline write_location write(table_axis_iterator it, write_location location)
{
  while (location.maxWrites>0U && !it.at_end())
  {
    if (update(externalApi, location.address, *it)) {
      --location.maxWrites;
    }
    ++location.address;
    ++it;
  }
  return location;
}

static inline uint16_t writeTable(void *pTable, table_type_t key, uint16_t address, uint16_t maxWrites)
{
  return write(y_rbegin(pTable, key), 
                write(x_begin(pTable, key), 
                  write(rows_begin(pTable, key), { address, maxWrites }))).maxWrites;
}

//The maximum number of write operations that will be performed in one go.
//If we try to write to the EEPROM too fast (Eg Each write takes ~3ms on the AVR) then 
//the rest of the system can hang)
static uint8_t getMaxWriteBlockSize(void) {
  // External fixed sized
#if defined(MAX_BLOCK_WRITE_BYTES)
  uint8_t blockSize = MAX_BLOCK_WRITE_BYTES;
#else
  // Dynamically determine size
  uint8_t blockSize = 18;
  if(currentStatus.commCompat) { blockSize = 8; } //If comms compatibility mode is on, slow the burn rate down even further
#endif

#ifdef CORE_AVR
  //In order to prevent missed pulses during EEPROM writes on AVR, scale the
  //maximum write block size based on the RPM.
  //This calculation is based on EEPROM writes taking approximately 4ms per byte
  //(Actual value is 3.8ms, so 4ms has some safety margin) 
  if(currentStatus.RPM > 65U) //Min RPM of 65 prevents overflow of uint8_t
  { 
    blockSize = (uint8_t)(15000U / currentStatus.RPM);
    blockSize = constrain(blockSize, 1U, 15U); //Any higher than this will cause comms timeouts on AVR
  }
#endif

  //Write to EEPROM more aggressively if the engine is not running
  if(currentStatus.RPM==0U) {
    blockSize = blockSize * 8;
  } 
 
 return blockSize;
}


static uint16_t writeEntity(page_iterator_t entity, uint16_t address, uint16_t maxWrites) {
  if (address>0U) {
    switch (entity.type) {
    case Raw:
      maxWrites = updateBlockLimitWriteOps(externalApi, address, (byte *)entity.pData, ((byte *)entity.pData)+entity.size, maxWrites);
      break;
      
    case Table:
      maxWrites = writeTable(entity.pData, entity.table_key, address, maxWrites);
      break;
    
    case NoEntity:
    default:
        // Do nothing
      break;
  }
  }
  return maxWrites;
}


//  ================================= End write support ===============================

void savePage(uint8_t pageNum)
{
  uint16_t maxWrites = getMaxWriteBlockSize();

  page_iterator_t entity = page_begin(pageNum);
  while ((entity.type!=End) && (maxWrites>0U)) {
    maxWrites = writeEntity(entity, getEntityStartAddress(entity), maxWrites);

    entity = advance(entity);
  }

  setEepromWritePending(maxWrites==0);
}

//  ================================= Internal read support ===============================

static inline uint16_t load(table_row_iterator row, uint16_t address)
{
  return loadBlock(externalApi, address, &*row, row.end());
}

static inline uint16_t load(table_value_iterator it, uint16_t address)
{
  while (!it.at_end())
  {
    address = load(*it, address); //cppcheck-suppress misra-c2012-17.2
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


static uint16_t loadPageEntity(page_iterator_t entity, uint16_t address) {
  switch (entity.type)
  {
  case Raw:
      address = loadBlock(externalApi, address, (byte *)entity.pData, (byte *)entity.pData+entity.size);
      break;

  case Table:
      address = loadTable(entity.pData, entity.table_key, address);
      break;

  case NoEntity:
  default:
      // Do nothing
      break;
  }
  return address;
}

static void loadPage(uint8_t pageNum) {
  page_iterator_t entity = page_begin(pageNum);
  while (entity.type!=End) {
    (void)loadPageEntity(entity, getEntityStartAddress(entity));
    entity = advance(entity);
  }
}

//  ================================= End internal read support ===============================

void loadAllPages(void)
{
  uint8_t pageCount = getPageCount();
  for (uint8_t page = 0U; page<pageCount; ++page) {
    loadPage(page);
  }
}

void loadAllCalibrationTables(void)
{
  // If you modify this function be sure to also modify saveAllCalibrationTables();
  // it should be a mirror image of this function.

  loadObject(externalApi, EEPROM_CALIBRATION_O2_BINS, o2CalibrationTable.axis);
  loadObject(externalApi, EEPROM_CALIBRATION_O2_VALUES, o2CalibrationTable.values);
  
  loadObject(externalApi, EEPROM_CALIBRATION_IAT_BINS, iatCalibrationTable.axis);
  loadObject(externalApi, EEPROM_CALIBRATION_IAT_VALUES, iatCalibrationTable.values);

  loadObject(externalApi, EEPROM_CALIBRATION_CLT_BINS, cltCalibrationTable.axis);
  loadObject(externalApi, EEPROM_CALIBRATION_CLT_VALUES, cltCalibrationTable.values);
}

/** Write calibration tables to EEPROM.
This takes the values in the 3 calibration tables (Coolant, Inlet temp and O2)
and saves them to the EEPROM.
*/
void saveAllCalibrationTables(void)
{
  // If you modify this function be sure to also modify loadAllCalibrationTables();
  // it should be a mirror image of this function.

  saveCalibrationTable(O2Sensor);
  saveCalibrationTable(IntakeAirTempSensor);
  saveCalibrationTable(CoolantSensor);
}

void saveCalibrationTable(SensorCalibrationTable sensor)
{
  if(sensor == O2Sensor)
  {
    updateObject(externalApi, o2CalibrationTable.axis, EEPROM_CALIBRATION_O2_BINS);
    updateObject(externalApi, o2CalibrationTable.values, EEPROM_CALIBRATION_O2_VALUES);
  }
  else if(sensor == IntakeAirTempSensor)
  {
    updateObject(externalApi, iatCalibrationTable.axis, EEPROM_CALIBRATION_IAT_BINS);
    updateObject(externalApi, iatCalibrationTable.values, EEPROM_CALIBRATION_IAT_VALUES);
  }
  else if(sensor == CoolantSensor)
  {
    updateObject(externalApi, cltCalibrationTable.axis, EEPROM_CALIBRATION_CLT_BINS);
    updateObject(externalApi, cltCalibrationTable.values, EEPROM_CALIBRATION_CLT_VALUES);
  } else {
    // Unknown sensor identifier - do nothing but keep MISRA checker happy
  }
}

TESTABLE_INLINE_STATIC uint16_t getSensorCalibrationCrcAddress(SensorCalibrationTable sensor) {
  constexpr uint16_t EEPROM_CALIBRATION_CLT_CRC = 3674;
  constexpr uint16_t EEPROM_CALIBRATION_IAT_CRC = 3678;
  constexpr uint16_t EEPROM_CALIBRATION_O2_CRC = 3682;

  switch(sensor)
  {
    case O2Sensor:
      return EEPROM_CALIBRATION_O2_CRC;
    case IntakeAirTempSensor:
      return EEPROM_CALIBRATION_IAT_CRC;
    case CoolantSensor:
    default: //Obviously should never happen
      return EEPROM_CALIBRATION_CLT_CRC;
  }
  return EEPROM_CALIBRATION_CLT_CRC;
}

void saveCalibrationCrc(SensorCalibrationTable sensor, uint32_t calibrationCRC)
{
  updateObject(externalApi, calibrationCRC, getSensorCalibrationCrcAddress(sensor));
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

uint8_t loadLastBaro(void) { return EEPROMReadRaw(EEPROM_LAST_BARO); }
void saveLastBaro(uint8_t newValue) { EEPROMWriteRaw(EEPROM_LAST_BARO, newValue); }

uint8_t loadEEPROMVersion(void) { return EEPROMReadRaw(EEPROM_DATA_VERSION); }
void saveEEPROMVersion(uint8_t newVersion) { EEPROMWriteRaw(EEPROM_DATA_VERSION, newVersion); }

void clearStorage(void) {
  for (uint16_t i = 0 ; i < getEEPROMSize() ; i++) { EEPROMWriteRaw((uint16_t)i, UINT8_MAX);} 
}

#if defined(CORE_AVR)
#pragma GCC pop_options
#endif