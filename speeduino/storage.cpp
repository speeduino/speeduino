/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
/** @file
 * @brief Lower level ConfigPage*, Table2D, Table3D and EEPROM storage operations.
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
#include "preprocessor.h"
#include "unit_testing.h"
#include EEPROM_LIB_H //This is defined in the board .h files

// Should be defined in a CPP file elsewhere. Usually the board CPP file.
extern EEPROM_t EEPROM;

#if defined(CORE_AVR)
#pragma GCC push_options
// This minimizes RAM usage at no performance cost
#pragma GCC optimize ("Os") 
#endif

static constexpr eeprom_address_t EEPROM_DATA_VERSION = 0;

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
eeprom_address_t MAX_PAGE_ADDRESS = EEPROM_LAST_BARO-sizeof(uint8_t);
uint16_t STORAGE_SIZE = STORAGE_END;
#endif

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
  eeprom_address_t address = 0U;
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

struct write_location {
  eeprom_address_t address; // EEPROM address to write next
  uint16_t counter; // Number of bytes written
  uint8_t write_block_size; // Maximum number of bytes to write

  /** Update byte to EEPROM by first comparing content and the need to write it.
  We only ever write to the EEPROM where the new value is different from the currently stored byte
  This is due to the limited write life of the EEPROM (Approximately 100,000 writes)
  */
  void update(uint8_t value)
  {
    if (EEPROM.read(address)!=value)
    {
      EEPROM.write(address, value);
      ++counter;
    }
  }

  /** Create a copy with a different write address.
   * Allows chaining of instances.
   */
  write_location changeWriteAddress(eeprom_address_t newAddress) const {
    return { newAddress, counter, write_block_size };
  }

  write_location& operator++(void)
  {
    ++address;
    return *this;
  }

  bool can_write(void) const
  {
    bool canWrite = false;
    if(currentStatus.RPM > 0U) { canWrite = (counter <= write_block_size); }
    else { canWrite = (counter <= (write_block_size * 8U)); } //Write to EEPROM more aggressively if the engine is not running

    return canWrite;
  }
};

static inline write_location write_range(const byte *pStart, const byte *pEnd, write_location location)
{
  while ( location.can_write() && pStart!=pEnd)
  {
    location.update(*pStart);
    ++pStart; 
    ++location;
  }
  return location;
}

static inline write_location write(const table_row_iterator &row, write_location location)
{
  return write_range(&*row, row.end(), location);
}

static inline write_location write(table_value_iterator it, write_location location)
{
  while (location.can_write() && !it.at_end())
  {
    location = write(*it, location); //cppcheck-suppress misra-c2012-17.2
    ++it;
  }
  return location;
}

static inline write_location write(table_axis_iterator it, write_location location)
{
  while (location.can_write() && !it.at_end())
  {
    location.update(*it);
    ++location;
    ++it;
  }
  return location;
}

static inline write_location writeTable(void *pTable, table_type_t key, const write_location &location)
{
  return write(y_rbegin(pTable, key), 
                write(x_begin(pTable, key), 
                  write(rows_begin(pTable, key), location)));
}

//The maximum number of write operations that will be performed in one go.
//If we try to write to the EEPROM too fast (Eg Each write takes ~3ms on the AVR) then 
//the rest of the system can hang)
static uint8_t getMaxWriteBlockSize(void) {
#if defined(USE_SPI_EEPROM)
  //For use with common Winbond SPI EEPROMs Eg W25Q16JV
  return 20; //This needs tuning
#elif defined(CORE_STM32) || defined(CORE_TEENSY)
  return 64;
#else
  uint8_t blockSize = 18;
  if(currentStatus.commCompat) { blockSize = 8; } //If comms compatibility mode is on, slow the burn rate down even further

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
  return blockSize;
#endif
}


static write_location writeEntity(page_iterator_t entity, write_location location) {
  if (location.address>(eeprom_address_t)0U) {
    switch (entity.type) {
    case Raw:
      location = write_range((byte *)entity.pData, (byte *)entity.pData+entity.size, location);
      break;
      
    case Table:
      location = writeTable(entity.pData, entity.table_key, location);
      break;
    
    case NoEntity:
    default:
        // Do nothing
      break;
  }
  }
  return location;
}


//  ================================= End write support ===============================

void savePage(uint8_t pageNum)
{
  write_location result = { 0, 0, getMaxWriteBlockSize() };

  page_iterator_t entity = page_begin(pageNum);
  while ((entity.type!=End) && (result.can_write())) {
    result = writeEntity(entity, result.changeWriteAddress(getEntityStartAddress(entity)));

    entity = advance(entity);
  }

  setEepromWritePending(!result.can_write());
}

//  ================================= Internal read support ===============================

/** Load range of bytes form EEPROM offset to memory.
 * @param address - start offset in EEPROM
 * @param pFirst - Start memory address
 * @param pLast - End memory address
 */
static inline eeprom_address_t load_range(eeprom_address_t address, byte *pFirst, const byte *pLast)
{
#if defined(CORE_AVR)
  // The generic code in the #else branch works but this provides a 45% speed up on AVR
  size_t size = pLast-pFirst;
  eeprom_read_block(pFirst, (const void*)(size_t)address, size);
  return address+(eeprom_address_t)size;
#else
  for (; pFirst != pLast; ++address, (void)++pFirst)
  {
    *pFirst = EEPROM.read(address);
  }
  return address;
#endif
}

static inline eeprom_address_t load(table_row_iterator row, eeprom_address_t address)
{
  return load_range(address, &*row, row.end());
}

static inline eeprom_address_t load(table_value_iterator it, eeprom_address_t address)
{
  while (!it.at_end())
  {
    address = load(*it, address); //cppcheck-suppress misra-c2012-17.2
    ++it;
  }
  return address; 
}

static inline eeprom_address_t load(table_axis_iterator it, eeprom_address_t address)
{
  while (!it.at_end())
  {
    *it = EEPROM.read(address);
    ++address;
    ++it;
  }
  return address;    
}


static inline eeprom_address_t loadTable(void *pTable, table_type_t key, eeprom_address_t address)
{
  return load(y_rbegin(pTable, key),
                load(x_begin(pTable, key), 
                  load(rows_begin(pTable, key), address)));
}


static eeprom_address_t loadPageEntity(page_iterator_t entity, eeprom_address_t address) {
  switch (entity.type)
  {
  case Raw:
      address = load_range(address, (byte *)entity.pData, (byte *)entity.pData+entity.size);
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

  EEPROM.get(EEPROM_CALIBRATION_O2_BINS, o2CalibrationTable.axis);
  EEPROM.get(EEPROM_CALIBRATION_O2_VALUES, o2CalibrationTable.values);

  EEPROM.get(EEPROM_CALIBRATION_IAT_BINS, iatCalibrationTable.axis);
  EEPROM.get(EEPROM_CALIBRATION_IAT_VALUES, iatCalibrationTable.values);

  EEPROM.get(EEPROM_CALIBRATION_CLT_BINS, cltCalibrationTable.axis);
  EEPROM.get(EEPROM_CALIBRATION_CLT_VALUES, cltCalibrationTable.values);
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
    EEPROM.put(EEPROM_CALIBRATION_O2_BINS, o2CalibrationTable.axis);
    EEPROM.put(EEPROM_CALIBRATION_O2_VALUES, o2CalibrationTable.values);
  }
  else if(sensor == IntakeAirTempSensor)
  {
    EEPROM.put(EEPROM_CALIBRATION_IAT_BINS, iatCalibrationTable.axis);
    EEPROM.put(EEPROM_CALIBRATION_IAT_VALUES, iatCalibrationTable.values);
  }
  else if(sensor == CoolantSensor)
  {
    EEPROM.put(EEPROM_CALIBRATION_CLT_BINS, cltCalibrationTable.axis);
    EEPROM.put(EEPROM_CALIBRATION_CLT_VALUES, cltCalibrationTable.values);
  }
}

TESTABLE_INLINE_STATIC eeprom_address_t getSensorCalibrationCrcAddress(SensorCalibrationTable sensor) {
  constexpr eeprom_address_t EEPROM_CALIBRATION_CLT_CRC = 3674;
  constexpr eeprom_address_t EEPROM_CALIBRATION_IAT_CRC = 3678;
  constexpr eeprom_address_t EEPROM_CALIBRATION_O2_CRC = 3682;

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
  EEPROM.put(getSensorCalibrationCrcAddress(sensor), calibrationCRC);
}

/** Retrieves and returns the 4 byte CRC32 checksum for a given calibration page from EEPROM. */
uint32_t loadCalibrationCrc(SensorCalibrationTable sensor)
{
  uint32_t crc32_val;
  EEPROM.get(getSensorCalibrationCrcAddress(sensor), crc32_val);
  return crc32_val;
}

uint16_t getEEPROMSize(void)
{
  return EEPROM.length();
}

// Utility functions.
// By having these in this file, it prevents other files from calling EEPROM functions directly. This is useful due to differences in the EEPROM libraries on different devces

void EEPROMWriteRaw(uint16_t address, byte data) { EEPROM.update(address, data); }
byte EEPROMReadRaw(uint16_t address) { return (byte)EEPROM.read(address); }

uint8_t loadLastBaro(void) { return EEPROM.read(EEPROM_LAST_BARO); }
void saveLastBaro(uint8_t newValue) { EEPROM.update(EEPROM_LAST_BARO, newValue); }

uint8_t loadEEPROMVersion(void) { return EEPROM.read(EEPROM_DATA_VERSION); }
void saveEEPROMVersion(uint8_t newVersion) { EEPROM.update(EEPROM_DATA_VERSION, newVersion); }

void clearStorage(void) {
  #if defined(FLASH_AS_EEPROM_h)
    EEPROM.read(0); //needed for SPI eeprom emulation.
    EEPROM.clear(); 
  #else 
    for (uint16_t i = 0 ; i < EEPROM.length() ; i++) { EEPROM.write((eeprom_address_t)i, UINT8_MAX);}
  #endif  
}

#if defined(CORE_AVR)
#pragma GCC pop_options
#endif