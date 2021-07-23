/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
/** @file
 * Lower level ConfigPage*, Table2D, Table3D and EEPROM storage operations.
 */

#include "globals.h"
#include "table.h"
#include "comms.h" // Is this needed at all ?
#include EEPROM_LIB_H //This is defined in the board .h files
#include "storage.h"
#include "table_iterator.h"
#include "pages.h"

//The maximum number of write operations that will be performed in one go. If we try to write to the EEPROM too fast (Each write takes ~3ms) then the rest of the system can hang)
#if defined(CORE_STM32) || defined(CORE_TEENSY) & !defined(USE_SPI_EEPROM)
#define EEPROM_MAX_WRITE_BLOCK 64
#else
#define EEPROM_MAX_WRITE_BLOCK 30
#endif

#define EEPROM_DATA_VERSION   0

// Calibration data is stored at the end of the EEPROM (This is in case any further calibration tables are needed as they are large blocks)
#define STORAGE_END 0xFFF       // Should be E2END?
#define EEPROM_CALIBRATION_CLT_VALUES (STORAGE_END-sizeof(cltCalibration_values))
#define EEPROM_CALIBRATION_CLT_BINS   (EEPROM_CALIBRATION_CLT_VALUES-sizeof(cltCalibration_bins))
#define EEPROM_CALIBRATION_IAT_VALUES (EEPROM_CALIBRATION_CLT_BINS-sizeof(iatCalibration_values))
#define EEPROM_CALIBRATION_IAT_BINS   (EEPROM_CALIBRATION_IAT_VALUES-sizeof(iatCalibration_bins))
#define EEPROM_CALIBRATION_O2_VALUES  (EEPROM_CALIBRATION_IAT_BINS-sizeof(o2Calibration_values))
#define EEPROM_CALIBRATION_O2_BINS    (EEPROM_CALIBRATION_O2_VALUES-sizeof(o2Calibration_bins))
#define EEPROM_LAST_BARO              (EEPROM_CALIBRATION_O2_BINS-1)

static bool eepromWritesPending = false;

bool isEepromWritePending()
{
  return eepromWritesPending;
}

/** Write all config pages to EEPROM.
 */
void writeAllConfig()
{
  uint8_t pageCount = getPageCount();
  uint8_t page = 1U;
  writeConfig(page++);
  while (page<pageCount && !eepromWritesPending)
  { 
    writeConfig(page++);
  }
}


namespace {

  /** Update byte to EEPROM by first comparing content and the need to write it.
  We only ever write to the EEPROM where the new value is different from the currently stored byte
  This is due to the limited write life of the EEPROM (Approximately 100,000 writes)
  */
  inline int16_t update(eeprom_address_t address, uint8_t value, int16_t counter)
  {
    if (EEPROM.read(address)!=value)
    {
      EEPROM.write(address, value);
      return counter+1;
    }
    return counter;
  }

  inline int16_t write_range_divisor(eeprom_address_t &address, int8_t divisor, int16_t *pStart, const int16_t *pEnd, int16_t counter)
  {
    while (counter<=EEPROM_MAX_WRITE_BLOCK && pStart!=pEnd)
    {
      counter = update(address, (*pStart)/divisor, counter);
      ++pStart; ++address;
    }
    return counter;
  }

  inline int16_t write_range(eeprom_address_t &address, byte *pStart, const byte *pEnd, int16_t counter)
  {
    while (counter<=EEPROM_MAX_WRITE_BLOCK && pStart!=pEnd)
    {
      counter = update(address, *pStart, counter);
      ++pStart; ++address;
    }
    return counter;
  }

  inline int16_t writeTableValues(const table3D *pTable, eeprom_address_t &address, int16_t counter)
  {
    byte **pRow = pTable->values + (pTable->xSize-1);
    byte **pRowEnd = pTable->values - 1;
    int rowSize = pTable->xSize;
    while (counter<=EEPROM_MAX_WRITE_BLOCK && pRow!=pRowEnd)
    {
      counter = write_range(address, *pRow, *pRow+rowSize, counter);
      --pRow;
    }
    return counter;
  }

  inline int16_t writeTable(const table3D *pTable, eeprom_address_t &address, int16_t counter)
  {
    counter = update(address, pTable->xSize, counter); ++address;
    counter = update(address, pTable->ySize, counter); ++address;
    counter = writeTableValues(pTable, address, counter);
    counter = write_range_divisor(address, getTableXAxisFactor(pTable), pTable->axisX, pTable->axisX+pTable->xSize, counter);
    return write_range_divisor(address, getTableYAxisFactor(pTable), pTable->axisY, pTable->axisY+pTable->ySize, counter);
  }
}


/** Write a table or map to EEPROM storage.
Takes the current configuration (config pages and maps)
and writes them to EEPROM as per the layout defined in storage.h.
*/
void writeConfig(byte tableNum)
{
  /*
  We only ever write to the EEPROM where the new value is different from the currently stored byte
  This is due to the limited write life of the EEPROM (Approximately 100,000 writes)
  */
  int writeCounter = 0;
  eeprom_address_t address;

  switch(tableNum)
  {
    case veMapPage:
      /*---------------------------------------------------
      | Fuel table (See storage.h for data layout) - Page 1
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      address = EEPROM_CONFIG1_XSIZE;
      writeCounter = writeTable(&fuelTable, address, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;
      //That concludes the writing of the VE table

    case veSetPage:
      /*---------------------------------------------------
      | Config page 2 (See storage.h for data layout)
      | 64 byte long config table
      -----------------------------------------------------*/
      address = EEPROM_CONFIG2_START;
      writeCounter = write_range(address, (byte *)&configPage2, (byte *)&configPage2+sizeof(configPage2), writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case ignMapPage:
      /*---------------------------------------------------
      | Ignition table (See storage.h for data layout) - Page 1
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the Ignition table, basically the same thing as above
      address = EEPROM_CONFIG3_XSIZE;
      writeCounter = writeTable(&ignitionTable, address, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case ignSetPage:
      /*---------------------------------------------------
      | Config page 2 (See storage.h for data layout)
      | 64 byte long config table
      -----------------------------------------------------*/
      address = EEPROM_CONFIG4_START;
      writeCounter = write_range(address, (byte *)&configPage4, (byte *)&configPage4+sizeof(configPage4), writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case afrMapPage:
      /*---------------------------------------------------
      | AFR table (See storage.h for data layout) - Page 5
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the Ignition table, basically the same thing as above
      address = EEPROM_CONFIG5_XSIZE;
      writeCounter = writeTable(&afrTable, address, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case afrSetPage:
      /*---------------------------------------------------
      | Config page 3 (See storage.h for data layout)
      | 64 byte long config table
      -----------------------------------------------------*/
      address = EEPROM_CONFIG6_START;
      writeCounter = write_range(address, (byte *)&configPage6, (byte *)&configPage6+sizeof(configPage6), writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case boostvvtPage:
      /*---------------------------------------------------
      | Boost and vvt tables (See storage.h for data layout) - Page 8
      | 8x8 table itself + the 8 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the 2 tables, basically the same thing as above but we're doing these 2 together (2 tables per page instead of 1)
      address = EEPROM_CONFIG7_XSIZE1;
      writeCounter = writeTable(&boostTable, address, writeCounter);
      address = EEPROM_CONFIG7_XSIZE2;
      writeCounter = writeTable(&vvtTable, address, writeCounter);
      address = EEPROM_CONFIG7_XSIZE3;
      writeCounter = writeTable(&stagingTable, address, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case seqFuelPage:
      /*---------------------------------------------------
      | Fuel trim tables (See storage.h for data layout) - Page 9
      | 6x6 tables itself + the 6 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the 2 tables, basically the same thing as above but we're doing these 2 together (2 tables per page instead of 1)
      address = EEPROM_CONFIG8_XSIZE1;
      writeCounter = writeTable(&trim1Table, address, writeCounter);
      address = EEPROM_CONFIG8_XSIZE2;
      writeCounter = writeTable(&trim2Table, address, writeCounter);
      address = EEPROM_CONFIG8_XSIZE3;
      writeCounter = writeTable(&trim3Table, address, writeCounter);
      address = EEPROM_CONFIG8_XSIZE4;
      writeCounter = writeTable(&trim4Table, address, writeCounter);
      address = EEPROM_CONFIG8_XSIZE5;
      writeCounter = writeTable(&trim5Table, address, writeCounter);
      address = EEPROM_CONFIG8_XSIZE6;
      writeCounter = writeTable(&trim6Table, address, writeCounter);
      address = EEPROM_CONFIG8_XSIZE7;
      writeCounter = writeTable(&trim7Table, address, writeCounter);
      address = EEPROM_CONFIG8_XSIZE8;
      writeCounter = writeTable(&trim8Table, address, writeCounter);
      
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case canbusPage:
      /*---------------------------------------------------
      | Config page 10 (See storage.h for data layout)
      | 192 byte long config table
      -----------------------------------------------------*/
      address = EEPROM_CONFIG9_START;
      writeCounter = write_range(address, (byte *)&configPage9, (byte *)&configPage9+sizeof(configPage9), writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case warmupPage:
      /*---------------------------------------------------
      | Config page 11 (See storage.h for data layout)
      | 192 byte long config table
      -----------------------------------------------------*/
      address = EEPROM_CONFIG10_START;
      writeCounter = write_range(address, (byte *)&configPage10, (byte *)&configPage10+sizeof(configPage10), writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case fuelMap2Page:
      /*---------------------------------------------------
      | Fuel table 2 (See storage.h for data layout)
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      address = EEPROM_CONFIG11_XSIZE;
      writeCounter = writeTable(&fuelTable2, address, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;
      //That concludes the writing of the 2nd fuel table

    case wmiMapPage:
      /*---------------------------------------------------
      | WMI and Dwell tables (See storage.h for data layout) - Page 12
      | 8x8 WMI table itself + the 8 values along each of the axis
      | 8x8 VVT2 table + the 8 values along each of the axis
      | 4x4 Dwell table itself + the 4 values along each of the axis
      -----------------------------------------------------*/
      address = EEPROM_CONFIG12_XSIZE;
      writeCounter = writeTable(&wmiTable, address, writeCounter);
      address = EEPROM_CONFIG12_XSIZE2;
      writeCounter = writeTable(&vvt2Table, address, writeCounter);
      address = EEPROM_CONFIG12_XSIZE3;
      writeCounter = writeTable(&dwellTable, address, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;
      
  case progOutsPage:
      /*---------------------------------------------------
      | Config page 13 (See storage.h for data layout)
      -----------------------------------------------------*/
      address = EEPROM_CONFIG13_START;
      writeCounter = write_range(address, (byte *)&configPage13, (byte *)&configPage13+sizeof(configPage13), writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;
    
    case ignMap2Page:
      /*---------------------------------------------------
      | Ignition table (See storage.h for data layout) - Page 1
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the Ignition table, basically the same thing as above
      address = EEPROM_CONFIG14_XSIZE;
      writeCounter = writeTable(&ignitionTable2, address, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    default:
      break;
  }
}
/** Reset all configPage* structs (2,4,6,9,10,13) and write them full of null-bytes.
 */
void resetConfigPages()
{
  memset(&configPage2, 0, sizeof(config2));
  memset(&configPage4, 0, sizeof(config4));
  memset(&configPage6, 0, sizeof(config6));
  memset(&configPage9, 0, sizeof(config9));
  memset(&configPage10, 0, sizeof(config10));
  memset(&configPage13, 0, sizeof(config13));
}

namespace
{
  /** Load range of bytes form EEPROM offset to memory.
   * @param address - start offset in EEPROM
   * @param pFirst - Start memory address
   * @param pLast - End memory address
   */
  inline eeprom_address_t load_range(eeprom_address_t address, byte *pFirst, byte *pLast)
  {
	  for (; pFirst != pLast; ++address, (void)++pFirst)
		{
		  *pFirst = EEPROM.read(address);
		}
    return address;
  }

  inline eeprom_address_t load_range_multiplier(eeprom_address_t address, int16_t *pFirst, int16_t *pLast, int16_t multiplier)
	{
  	for (; pFirst != pLast; ++address, (void)++pFirst)
		{
		  *pFirst = EEPROM.read(address) * multiplier;
		}
    return address;
  }

  inline eeprom_address_t loadTableValues(table3D *pTable, eeprom_address_t address)
  {
    byte **pRow = pTable->values + (pTable->xSize-1);
    byte **pRowEnd = pTable->values - 1;
    int rowSize = pTable->xSize;
    for(; pRow!=pRowEnd; --pRow)
    {
      address = load_range(address, *pRow, *pRow+rowSize);
    }
    return address; 
  }

  inline eeprom_address_t loadTableAxisX(table3D *pTable, eeprom_address_t address)
  {
    return load_range_multiplier(address, pTable->axisX, pTable->axisX+pTable->xSize, getTableXAxisFactor(pTable));
  }

  inline eeprom_address_t loadTableAxisY(table3D *pTable, eeprom_address_t address)
  {
    return load_range_multiplier(address, pTable->axisY, pTable->axisY+pTable->ySize, getTableYAxisFactor(pTable));
  }

  inline eeprom_address_t loadTable(table3D *pTable, eeprom_address_t address)
  {
    return loadTableAxisY(pTable,
                          loadTableAxisX(pTable, 
                                          loadTableValues(pTable, address)));
  }
}
/** Load all config tables from storage.
 */
void loadConfig()
{
  loadTable(&fuelTable, EEPROM_CONFIG1_MAP);
  load_range(EEPROM_CONFIG2_START, (byte *)&configPage2, (byte *)&configPage2+sizeof(configPage2));
  //That concludes the reading of the VE table
  
  //*********************************************************************************************************************************************************************************
  //IGNITION CONFIG PAGE (2)

  //Begin writing the Ignition table, basically the same thing as above
  loadTable(&ignitionTable, EEPROM_CONFIG3_MAP);
  load_range(EEPROM_CONFIG4_START, (byte *)&configPage4, (byte *)&configPage4+sizeof(configPage4));

  //*********************************************************************************************************************************************************************************
  //AFR TARGET CONFIG PAGE (3)

  //Begin writing the Ignition table, basically the same thing as above
  loadTable(&afrTable, EEPROM_CONFIG5_MAP);
  load_range(EEPROM_CONFIG6_START, (byte *)&configPage6, (byte *)&configPage6+sizeof(configPage6));

  //*********************************************************************************************************************************************************************************
  // Boost and vvt tables load
  loadTable(&boostTable, EEPROM_CONFIG7_MAP1);
  loadTable(&vvtTable, EEPROM_CONFIG7_MAP2);
  loadTable(&stagingTable, EEPROM_CONFIG7_MAP3);

  //*********************************************************************************************************************************************************************************
  // Fuel trim tables load
  loadTable(&trim1Table, EEPROM_CONFIG8_MAP1);
  loadTable(&trim2Table, EEPROM_CONFIG8_MAP2);
  loadTable(&trim3Table, EEPROM_CONFIG8_MAP3);
  loadTable(&trim4Table, EEPROM_CONFIG8_MAP4);
  loadTable(&trim5Table, EEPROM_CONFIG8_MAP5);
  loadTable(&trim6Table, EEPROM_CONFIG8_MAP6);
  loadTable(&trim7Table, EEPROM_CONFIG8_MAP7);
  loadTable(&trim8Table, EEPROM_CONFIG8_MAP8);

  //*********************************************************************************************************************************************************************************
  //canbus control page load
  load_range(EEPROM_CONFIG9_START, (byte *)&configPage9, (byte *)&configPage9+sizeof(configPage9));

  //*********************************************************************************************************************************************************************************

  //CONFIG PAGE (10)
  load_range(EEPROM_CONFIG10_START, (byte *)&configPage10, (byte *)&configPage10+sizeof(configPage10));

  //*********************************************************************************************************************************************************************************
  //Fuel table 2 (See storage.h for data layout)
  loadTable(&fuelTable2, EEPROM_CONFIG11_MAP);

  //*********************************************************************************************************************************************************************************
  // WMI, VVT2 and Dwell table load
  loadTable(&wmiTable, EEPROM_CONFIG12_MAP);
  loadTable(&vvt2Table, EEPROM_CONFIG12_MAP2);
  loadTable(&dwellTable, EEPROM_CONFIG12_MAP3);

  //*********************************************************************************************************************************************************************************
  //CONFIG PAGE (13)
  load_range(EEPROM_CONFIG13_START, (byte *)&configPage13, (byte *)&configPage13+sizeof(configPage13));

  //*********************************************************************************************************************************************************************************
  //SECOND IGNITION CONFIG PAGE (14)

  //Begin writing the Ignition table, basically the same thing as above
  loadTable(&ignitionTable2, EEPROM_CONFIG14_MAP);

  //*********************************************************************************************************************************************************************************
}

/** Read the calibration information from EEPROM.
This is separate from the config load as the calibrations do not exist as pages within the ini file for Tuner Studio.
*/
void loadCalibration()
{
  // If you modify this function be sure to also modify writeCalibration();
  // it should be a mirror image of this function.

  EEPROM.get(EEPROM_CALIBRATION_O2_BINS, o2Calibration_bins);
  EEPROM.get(EEPROM_CALIBRATION_O2_VALUES, o2Calibration_values);
  
  EEPROM.get(EEPROM_CALIBRATION_IAT_BINS, iatCalibration_bins);
  EEPROM.get(EEPROM_CALIBRATION_IAT_VALUES, iatCalibration_values);

  EEPROM.get(EEPROM_CALIBRATION_CLT_BINS, cltCalibration_bins);
  EEPROM.get(EEPROM_CALIBRATION_CLT_VALUES, cltCalibration_values);
}

/** Write calibration tables to EEPROM.
This takes the values in the 3 calibration tables (Coolant, Inlet temp and O2)
and saves them to the EEPROM.
*/
void writeCalibration()
{
  // If you modify this function be sure to also modify loadCalibration();
  // it should be a mirror image of this function.

  EEPROM.put(EEPROM_CALIBRATION_O2_BINS, o2Calibration_bins);
  EEPROM.put(EEPROM_CALIBRATION_O2_VALUES, o2Calibration_values);
  
  EEPROM.put(EEPROM_CALIBRATION_IAT_BINS, iatCalibration_bins);
  EEPROM.put(EEPROM_CALIBRATION_IAT_VALUES, iatCalibration_values);

  EEPROM.put(EEPROM_CALIBRATION_CLT_BINS, cltCalibration_bins);
  EEPROM.put(EEPROM_CALIBRATION_CLT_VALUES, cltCalibration_values);
}

static eeprom_address_t compute_crc_address(byte pageNo)
{
  return EEPROM_LAST_BARO-((getPageCount() - pageNo)*sizeof(uint32_t));
}

/** Write CRC32 checksum to EEPROM.
Takes a page number and CRC32 value then stores it in the relevant place in EEPROM
Note: Each pages requires 4 bytes for its CRC32. These are stored in reverse page order (ie the last page is store first in EEPROM).
@param pageNo - Config page number
@param crc32_val - CRC32 checksum
*/
void storePageCRC32(byte pageNo, uint32_t crc32_val)
{
  EEPROM.put(compute_crc_address(pageNo), crc32_val);
}

/** Retrieves and returns the 4 byte CRC32 checksum for a given page from EEPROM.
@param pageNo - Config page number
*/
uint32_t readPageCRC32(byte pageNo)
{
  uint32_t crc32_val;
  return EEPROM.get(compute_crc_address(pageNo), crc32_val);
}

// Utility functions.
// By having these in this file, it prevents other files from calling EEPROM functions directly. This is useful due to differences in the EEPROM libraries on different devces
/// Read last stored barometer reading from EEPROM.
byte readLastBaro() { return EEPROM.read(EEPROM_LAST_BARO); }
/// Write last acquired arometer reading to EEPROM.
void storeLastBaro(byte newValue) { EEPROM.update(EEPROM_LAST_BARO, newValue); }
/// Read EEPROM current data format version (from offset EEPROM_DATA_VERSION).
byte readEEPROMVersion() { return EEPROM.read(EEPROM_DATA_VERSION); }
/// Store EEPROM current data format version (to offset EEPROM_DATA_VERSION).
void storeEEPROMVersion(byte newVersion) { EEPROM.update(EEPROM_DATA_VERSION, newVersion); }
