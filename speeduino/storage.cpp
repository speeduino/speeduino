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
#include EEPROM_LIB_H //This is defined in the board .h files

// Should be defined in a CPP file elsewhere. Usually the board CPP file.
extern EEPROM_t EEPROM;

#define EEPROM_DATA_VERSION   0

#if defined(CORE_AVR)
#pragma GCC push_options
// This minimizes RAM usage at no performance cost
#pragma GCC optimize ("Os") 
#endif


// Calibration data is stored at the end of the EEPROM (This is in case any further calibration tables are needed as they are large blocks)
static constexpr uint16_t STORAGE_END = 0xFFF;
static constexpr uint16_t EEPROM_CALIBRATION_CLT_VALUES = STORAGE_END-(uint16_t)sizeof(decltype(cltCalibrationTable)::values);
static constexpr uint16_t EEPROM_CALIBRATION_CLT_BINS =  EEPROM_CALIBRATION_CLT_VALUES-(uint16_t)sizeof(decltype(cltCalibrationTable)::axis);
static constexpr uint16_t EEPROM_CALIBRATION_IAT_VALUES = EEPROM_CALIBRATION_CLT_BINS-(uint16_t)sizeof(decltype(iatCalibrationTable)::values);
static constexpr uint16_t EEPROM_CALIBRATION_IAT_BINS = EEPROM_CALIBRATION_IAT_VALUES-(uint16_t)sizeof(decltype(iatCalibrationTable)::axis);
static constexpr uint16_t EEPROM_CALIBRATION_O2_VALUES = EEPROM_CALIBRATION_IAT_BINS-(uint16_t)sizeof(decltype(o2CalibrationTable)::values);
static constexpr uint16_t EEPROM_CALIBRATION_O2_BINS =   EEPROM_CALIBRATION_O2_VALUES-(uint16_t)sizeof(decltype(o2CalibrationTable)::axis);
static constexpr uint16_t EEPROM_LAST_BARO = (EEPROM_CALIBRATION_O2_BINS-(uint16_t)1);


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
struct write_location {
  eeprom_address_t address; // EEPROM address to write next
  uint16_t writeCounter; // Number of bytes written
  uint16_t write_block_size; // Maximum number of bytes to write

  /** Update byte to EEPROM by first comparing content and the need to write it.
  We only ever write to the EEPROM where the new value is different from the currently stored byte
  This is due to the limited write life of the EEPROM (Approximately 100,000 writes)
  */
  void update(uint8_t value)
  {
    if (EEPROM.read(address)!=value)
    {
      EEPROM.write(address, value);
      ++writeCounter;
    }
  }

  /** Create a copy with a different write address.
   * Allows chaining of instances.
   */
  write_location changeWriteAddress(eeprom_address_t newAddress) const {
    return { newAddress, writeCounter, write_block_size };
  }

  write_location& operator++()
  {
    ++address;
    return *this;
  }

  bool can_write() const
  {
    return writeCounter <= write_block_size;
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
    location = write(*it, location);
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


static inline write_location writeTable(void *pTable, table_type_t key, write_location location)
{
  return write(y_rbegin(pTable, key), 
                write(x_begin(pTable, key), 
                  write(rows_begin(pTable, key), location)));
}

//  ================================= End write support ===============================

void savePage(uint8_t pageNum)
{
  write_location result = { 0, 0, getEepromWriteBlockSize(currentStatus) };

  switch(pageNum)
  {
    case veMapPage:
      /*---------------------------------------------------
      | Fuel table (See storage.h for data layout) - Page 1
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      result = writeTable(&fuelTable, decltype(fuelTable)::type_key, result.changeWriteAddress(EEPROM_CONFIG1_MAP));
      break;

    case veSetPage:
      /*---------------------------------------------------
      | Config page 2 (See storage.h for data layout)
      | 64 byte long config table
      -----------------------------------------------------*/
      result = write_range((byte *)&configPage2, (byte *)&configPage2+sizeof(configPage2), result.changeWriteAddress(EEPROM_CONFIG2_START));
      break;

    case ignMapPage:
      /*---------------------------------------------------
      | Ignition table (See storage.h for data layout) - Page 1
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      result = writeTable(&ignitionTable, decltype(ignitionTable)::type_key, result.changeWriteAddress(EEPROM_CONFIG3_MAP));
      break;

    case ignSetPage:
      /*---------------------------------------------------
      | Config page 2 (See storage.h for data layout)
      | 64 byte long config table
      -----------------------------------------------------*/
      result = write_range((byte *)&configPage4, (byte *)&configPage4+sizeof(configPage4), result.changeWriteAddress(EEPROM_CONFIG4_START));
      break;

    case afrMapPage:
      /*---------------------------------------------------
      | AFR table (See storage.h for data layout) - Page 5
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      result = writeTable(&afrTable, decltype(afrTable)::type_key, result.changeWriteAddress(EEPROM_CONFIG5_MAP));
      break;

    case afrSetPage:
      /*---------------------------------------------------
      | Config page 3 (See storage.h for data layout)
      | 64 byte long config table
      -----------------------------------------------------*/
      result = write_range((byte *)&configPage6, (byte *)&configPage6+sizeof(configPage6), result.changeWriteAddress(EEPROM_CONFIG6_START));
      break;

    case boostvvtPage:
      /*---------------------------------------------------
      | Boost and vvt tables (See storage.h for data layout) - Page 8
      | 8x8 table itself + the 8 values along each of the axis
      -----------------------------------------------------*/
      result = writeTable(&boostTable, decltype(boostTable)::type_key, result.changeWriteAddress(EEPROM_CONFIG7_MAP1));
      result = writeTable(&vvtTable, decltype(vvtTable)::type_key, result.changeWriteAddress(EEPROM_CONFIG7_MAP2));
      result = writeTable(&stagingTable, decltype(stagingTable)::type_key, result.changeWriteAddress(EEPROM_CONFIG7_MAP3));
      break;

    case seqFuelPage:
      /*---------------------------------------------------
      | Fuel trim tables (See storage.h for data layout) - Page 9
      | 6x6 tables itself + the 6 values along each of the axis
      -----------------------------------------------------*/
      result = writeTable(&trim1Table, decltype(trim1Table)::type_key, result.changeWriteAddress(EEPROM_CONFIG8_MAP1));
      result = writeTable(&trim2Table, decltype(trim2Table)::type_key, result.changeWriteAddress(EEPROM_CONFIG8_MAP2));
      result = writeTable(&trim3Table, decltype(trim3Table)::type_key, result.changeWriteAddress(EEPROM_CONFIG8_MAP3));
      result = writeTable(&trim4Table, decltype(trim4Table)::type_key, result.changeWriteAddress(EEPROM_CONFIG8_MAP4));
      result = writeTable(&trim5Table, decltype(trim5Table)::type_key, result.changeWriteAddress(EEPROM_CONFIG8_MAP5));
      result = writeTable(&trim6Table, decltype(trim6Table)::type_key, result.changeWriteAddress(EEPROM_CONFIG8_MAP6));
      result = writeTable(&trim7Table, decltype(trim7Table)::type_key, result.changeWriteAddress(EEPROM_CONFIG8_MAP7));
      result = writeTable(&trim8Table, decltype(trim8Table)::type_key, result.changeWriteAddress(EEPROM_CONFIG8_MAP8));
      break;

    case canbusPage:
      /*---------------------------------------------------
      | Config page 10 (See storage.h for data layout)
      | 192 byte long config table
      -----------------------------------------------------*/
      result = write_range((byte *)&configPage9, (byte *)&configPage9+sizeof(configPage9), result.changeWriteAddress(EEPROM_CONFIG9_START));
      break;

    case warmupPage:
      /*---------------------------------------------------
      | Config page 11 (See storage.h for data layout)
      | 192 byte long config table
      -----------------------------------------------------*/
      result = write_range((byte *)&configPage10, (byte *)&configPage10+sizeof(configPage10), result.changeWriteAddress(EEPROM_CONFIG10_START));
      break;

    case fuelMap2Page:
      /*---------------------------------------------------
      | Fuel table 2 (See storage.h for data layout)
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      result = writeTable(&fuelTable2, decltype(fuelTable2)::type_key, result.changeWriteAddress(EEPROM_CONFIG11_MAP));
      break;

    case wmiMapPage:
      /*---------------------------------------------------
      | WMI and Dwell tables (See storage.h for data layout) - Page 12
      | 8x8 WMI table itself + the 8 values along each of the axis
      | 8x8 VVT2 table + the 8 values along each of the axis
      | 4x4 Dwell table itself + the 4 values along each of the axis
      -----------------------------------------------------*/
      result = writeTable(&wmiTable, decltype(wmiTable)::type_key, result.changeWriteAddress(EEPROM_CONFIG12_MAP));
      result = writeTable(&vvt2Table, decltype(vvt2Table)::type_key, result.changeWriteAddress(EEPROM_CONFIG12_MAP2));
      result = writeTable(&dwellTable, decltype(dwellTable)::type_key, result.changeWriteAddress(EEPROM_CONFIG12_MAP3));
      break;
      
    case progOutsPage:
      /*---------------------------------------------------
      | Config page 13 (See storage.h for data layout)
      -----------------------------------------------------*/
      result = write_range((byte *)&configPage13, (byte *)&configPage13+sizeof(configPage13), result.changeWriteAddress(EEPROM_CONFIG13_START));
      break;
    
    case ignMap2Page:
      /*---------------------------------------------------
      | Ignition table (See storage.h for data layout) - Page 1
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      result = writeTable(&ignitionTable2, decltype(ignitionTable2)::type_key, result.changeWriteAddress(EEPROM_CONFIG14_MAP));
      break;

    case boostvvtPage2:
      /*---------------------------------------------------
      | Boost duty cycle lookuptable (See storage.h for data layout) - Page 15
      | 8x8 table itself + the 8 values along each of the axis
      -----------------------------------------------------*/
      result = writeTable(&boostTableLookupDuty, decltype(boostTableLookupDuty)::type_key, result.changeWriteAddress(EEPROM_CONFIG15_MAP));

      /*---------------------------------------------------
      | Config page 15 (See storage.h for data layout)
      -----------------------------------------------------*/
      result = write_range((byte *)&configPage15, (byte *)&configPage15+sizeof(configPage15), result.changeWriteAddress(EEPROM_CONFIG15_START));
      break;

    default:
      break;
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
  return address+size;
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
    address = load(*it, address);
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

  saveCalibrationTable(SensorCalibrationTable::O2Sensor);
  saveCalibrationTable(SensorCalibrationTable::IntakeAirTempSensor);
  saveCalibrationTable(SensorCalibrationTable::CoolantSensor);
}

void saveCalibrationTable(SensorCalibrationTable sensor)
{
  if(sensor == SensorCalibrationTable::O2Sensor)
  {
    EEPROM.put(EEPROM_CALIBRATION_O2_BINS, o2CalibrationTable.axis);
    EEPROM.put(EEPROM_CALIBRATION_O2_VALUES, o2CalibrationTable.values);
  }
  else if(sensor == SensorCalibrationTable::IntakeAirTempSensor)
  {
    EEPROM.put(EEPROM_CALIBRATION_IAT_BINS, iatCalibrationTable.axis);
    EEPROM.put(EEPROM_CALIBRATION_IAT_VALUES, iatCalibrationTable.values);
  }
  else if(sensor == SensorCalibrationTable::CoolantSensor)
  {
    EEPROM.put(EEPROM_CALIBRATION_CLT_BINS, cltCalibrationTable.axis);
    EEPROM.put(EEPROM_CALIBRATION_CLT_VALUES, cltCalibrationTable.values);
  }
}

static inline eeprom_address_t getSensorCalibrationCrcAddress(SensorCalibrationTable sensor) {
  constexpr eeprom_address_t EEPROM_CALIBRATION_CLT_CRC = 3674;
  constexpr eeprom_address_t EEPROM_CALIBRATION_IAT_CRC = 3678;
  constexpr eeprom_address_t EEPROM_CALIBRATION_O2_CRC = 3682;

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
byte EEPROMReadRaw(uint16_t address) { return EEPROM.read(address); }

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