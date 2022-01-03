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

bool eepromWritesPending = false;
/** Write all config pages to EEPROM.
 */
void writeAllConfig()
{
  writeConfig(veSetPage);
  if (eepromWritesPending == false) { writeConfig(veMapPage); }
  if (eepromWritesPending == false) { writeConfig(ignMapPage); }
  if (eepromWritesPending == false) { writeConfig(ignSetPage); }
  if (eepromWritesPending == false) { writeConfig(afrMapPage); }
  if (eepromWritesPending == false) { writeConfig(afrSetPage); }
  if (eepromWritesPending == false) { writeConfig(boostvvtPage); }
  if (eepromWritesPending == false) { writeConfig(seqFuelPage); }
  if (eepromWritesPending == false) { writeConfig(canbusPage); }
  if (eepromWritesPending == false) { writeConfig(warmupPage); }
  if (eepromWritesPending == false) { writeConfig(fuelMap2Page); }
  if (eepromWritesPending == false) { writeConfig(wmiMapPage); }
  if (eepromWritesPending == false) { writeConfig(progOutsPage); }
  if (eepromWritesPending == false) { writeConfig(ignMap2Page); }
}

namespace {

  /** Update byte to EEPROM by first comparing content and the need to write it.
  We only ever write to the EEPROM where the new value is different from the currently stored byte
  This is due to the limited write life of the EEPROM (Approximately 100,000 writes)
  */
  inline int16_t update(int index, uint8_t value, int16_t counter)
  {
    if (EEPROM.read(index)!=value)
    {
      EEPROM.write(index, value);
      return counter+1;
    }
    return counter;
  }

  inline int16_t write_range_divisor(int &index, int8_t divisor, int16_t *pStart, const int16_t *pEnd, int16_t counter)
  {
    while (counter<=EEPROM_MAX_WRITE_BLOCK && pStart!=pEnd)
    {
      counter = update(index, (*pStart)/divisor, counter);
      ++pStart; ++index;
    }
    return counter;
  }

  inline int16_t write_range(int &index, byte *pStart, const byte *pEnd, int16_t counter)
  {
    while (counter<=EEPROM_MAX_WRITE_BLOCK && pStart!=pEnd)
    {
      counter = update(index, *pStart, counter);
      ++pStart; ++index;
    }
    return counter;
  }

  inline int16_t writeTableValues(const table3D *pTable, int &index, int16_t counter)
  {
    byte **pRow = pTable->values + (pTable->xSize-1);
    byte **pRowEnd = pTable->values - 1;
    int rowSize = pTable->xSize;
    while (counter<=EEPROM_MAX_WRITE_BLOCK && pRow!=pRowEnd)
    {
      counter = write_range(index, *pRow, *pRow+rowSize, counter);
      --pRow;
    }
    return counter;
  }

  inline int16_t writeTable(const table3D *pTable, int &index, int16_t counter)
  {
    counter = update(index, pTable->xSize, counter); ++index;
    counter = update(index, pTable->ySize, counter); ++index;
    counter = writeTableValues(pTable, index, counter);
    counter = write_range_divisor(index, getTableXAxisFactor(pTable), pTable->axisX, pTable->axisX+pTable->xSize, counter);
    return write_range_divisor(index, getTableYAxisFactor(pTable), pTable->axisY, pTable->axisY+pTable->ySize, counter);
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
  int index;

  switch(tableNum)
  {
    case veMapPage:
      /*---------------------------------------------------
      | Fuel table (See storage.h for data layout) - Page 1
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      index = EEPROM_CONFIG1_XSIZE;
      writeCounter = writeTable(&fuelTable, index, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;
      //That concludes the writing of the VE table

    case veSetPage:
      /*---------------------------------------------------
      | Config page 2 (See storage.h for data layout)
      | 64 byte long config table
      -----------------------------------------------------*/
      index = EEPROM_CONFIG2_START;
      writeCounter = write_range(index, (byte *)&configPage2, (byte *)&configPage2+sizeof(configPage2), writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case ignMapPage:
      /*---------------------------------------------------
      | Ignition table (See storage.h for data layout) - Page 1
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the Ignition table, basically the same thing as above
      index = EEPROM_CONFIG3_XSIZE;
      writeCounter = writeTable(&ignitionTable, index, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case ignSetPage:
      /*---------------------------------------------------
      | Config page 2 (See storage.h for data layout)
      | 64 byte long config table
      -----------------------------------------------------*/
      index = EEPROM_CONFIG4_START;
      writeCounter = write_range(index, (byte *)&configPage4, (byte *)&configPage4+sizeof(configPage4), writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case afrMapPage:
      /*---------------------------------------------------
      | AFR table (See storage.h for data layout) - Page 5
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the Ignition table, basically the same thing as above
      index = EEPROM_CONFIG5_XSIZE;
      writeCounter = writeTable(&afrTable, index, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case afrSetPage:
      /*---------------------------------------------------
      | Config page 3 (See storage.h for data layout)
      | 64 byte long config table
      -----------------------------------------------------*/
      index = EEPROM_CONFIG6_START;
      writeCounter = write_range(index, (byte *)&configPage6, (byte *)&configPage6+sizeof(configPage6), writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case boostvvtPage:
      /*---------------------------------------------------
      | Boost and vvt tables (See storage.h for data layout) - Page 8
      | 8x8 table itself + the 8 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the 2 tables, basically the same thing as above but we're doing these 2 together (2 tables per page instead of 1)
      index = EEPROM_CONFIG7_XSIZE1;
      writeCounter = writeTable(&boostTable, index, writeCounter);
      index = EEPROM_CONFIG7_XSIZE2;
      writeCounter = writeTable(&vvtTable, index, writeCounter);
      index = EEPROM_CONFIG7_XSIZE3;
      writeCounter = writeTable(&stagingTable, index, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case seqFuelPage:
      /*---------------------------------------------------
      | Fuel trim tables (See storage.h for data layout) - Page 9
      | 6x6 tables itself + the 6 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the 2 tables, basically the same thing as above but we're doing these 2 together (2 tables per page instead of 1)
      index = EEPROM_CONFIG8_XSIZE1;
      writeCounter = writeTable(&trim1Table, index, writeCounter);
      index = EEPROM_CONFIG8_XSIZE2;
      writeCounter = writeTable(&trim2Table, index, writeCounter);
      index = EEPROM_CONFIG8_XSIZE3;
      writeCounter = writeTable(&trim3Table, index, writeCounter);
      index = EEPROM_CONFIG8_XSIZE4;
      writeCounter = writeTable(&trim4Table, index, writeCounter);
      index = EEPROM_CONFIG8_XSIZE5;
      writeCounter = writeTable(&trim5Table, index, writeCounter);
      index = EEPROM_CONFIG8_XSIZE6;
      writeCounter = writeTable(&trim6Table, index, writeCounter);
      index = EEPROM_CONFIG8_XSIZE7;
      writeCounter = writeTable(&trim7Table, index, writeCounter);
      index = EEPROM_CONFIG8_XSIZE8;
      writeCounter = writeTable(&trim8Table, index, writeCounter);
      
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case canbusPage:
      /*---------------------------------------------------
      | Config page 10 (See storage.h for data layout)
      | 192 byte long config table
      -----------------------------------------------------*/
      index = EEPROM_CONFIG9_START;
      writeCounter = write_range(index, (byte *)&configPage9, (byte *)&configPage9+sizeof(configPage9), writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case warmupPage:
      /*---------------------------------------------------
      | Config page 11 (See storage.h for data layout)
      | 192 byte long config table
      -----------------------------------------------------*/
      index = EEPROM_CONFIG10_START;
      writeCounter = write_range(index, (byte *)&configPage10, (byte *)&configPage10+sizeof(configPage10), writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case fuelMap2Page:
      /*---------------------------------------------------
      | Fuel table 2 (See storage.h for data layout)
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      index = EEPROM_CONFIG11_XSIZE;
      writeCounter = writeTable(&fuelTable2, index, writeCounter);
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
      index = EEPROM_CONFIG12_XSIZE;
      writeCounter = writeTable(&wmiTable, index, writeCounter);
      index = EEPROM_CONFIG12_XSIZE2;
      writeCounter = writeTable(&vvt2Table, index, writeCounter);
      index = EEPROM_CONFIG12_XSIZE3;
      writeCounter = writeTable(&dwellTable, index, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;
      
  case progOutsPage:
      /*---------------------------------------------------
      | Config page 13 (See storage.h for data layout)
      -----------------------------------------------------*/
      index = EEPROM_CONFIG13_START;
      writeCounter = write_range(index, (byte *)&configPage13, (byte *)&configPage13+sizeof(configPage13), writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;
    
    case ignMap2Page:
      /*---------------------------------------------------
      | Ignition table (See storage.h for data layout) - Page 1
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the Ignition table, basically the same thing as above
      index = EEPROM_CONFIG14_XSIZE;
      writeCounter = writeTable(&ignitionTable2, index, writeCounter);
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
   * @param index - start offset in EEPROM
   * @param pFirst - Start memory address
   * @param pLast - End memory address
   */
  inline int load_range(int index, byte *pFirst, byte *pLast)
  {
	  for (; pFirst != pLast; ++index, (void)++pFirst)
		{
		  *pFirst = EEPROM.read(index);
		}
    return index;
  }

  inline int load_range_multiplier(int index, int16_t *pFirst, int16_t *pLast, int16_t multiplier)
	{
  	for (; pFirst != pLast; ++index, (void)++pFirst)
		{
		  *pFirst = EEPROM.read(index) * multiplier;
		}
    return index;
  }

  inline int loadTableValues(table3D *pTable, int index)
  {
    byte **pRow = pTable->values + (pTable->xSize-1);
    byte **pRowEnd = pTable->values - 1;
    int rowSize = pTable->xSize;
    for(; pRow!=pRowEnd; --pRow)
    {
      index = load_range(index, *pRow, *pRow+rowSize);
    }
    return index; 
  }

  inline int loadTableAxisX(table3D *pTable, int index)
  {
    return load_range_multiplier(index, pTable->axisX, pTable->axisX+pTable->xSize, getTableXAxisFactor(pTable));
  }

  inline int loadTableAxisY(table3D *pTable, int index)
  {
    return load_range_multiplier(index, pTable->axisY, pTable->axisY+pTable->ySize, getTableYAxisFactor(pTable));
  }

  inline int loadTable(table3D *pTable, int index)
  {
    return loadTableAxisY(pTable,
                          loadTableAxisX(pTable, 
                                          loadTableValues(pTable, index)));
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

  for(int x=0; x<32; x++) //Each calibration table is 32 bytes long
  {
    int y = EEPROM_CALIBRATION_CLT + (x * 2);
    EEPROM.get(y, cltCalibration_bins[x]);
    y += 64; 
    EEPROM.get(y, cltCalibration_values[x]);

    y = EEPROM_CALIBRATION_IAT + (x * 2);
    EEPROM.get(y, iatCalibration_bins[x]);
    y += 64; 
    EEPROM.get(y, iatCalibration_values[x]);

    y = EEPROM_CALIBRATION_O2 + (x * 2);
    EEPROM.get(y, o2Calibration_bins[x]);
    y = EEPROM_CALIBRATION_O2 + 64 + x;
    o2Calibration_values[x] = EEPROM.read(y); //Byte values

  }

}

/** Write calibration tables to EEPROM.
This takes the values in the 3 calibration tables (Coolant, Inlet temp and O2)
and saves them to the EEPROM.
*/
void writeCalibration()
{

  for(int x=0; x<32; x++) //Each calibration table is 32 bytes long
  {
    int y = EEPROM_CALIBRATION_CLT + (x * 2);
    EEPROM.put(y, cltCalibration_bins[x]);
    y += 64; 
    EEPROM.put(y, cltCalibration_values[x]);

    y = EEPROM_CALIBRATION_IAT + (x * 2);
    EEPROM.put(y, iatCalibration_bins[x]);
    y += 64; 
    EEPROM.put(y, iatCalibration_values[x]);

    y = EEPROM_CALIBRATION_O2 + (x * 2);
    EEPROM.put(y, o2Calibration_bins[x]);
    y = EEPROM_CALIBRATION_O2 + 64 + x; 
    EEPROM.update(y, o2Calibration_values[x]);
  }

}

/** Write CRC32 checksum to EEPROM.
Takes a page number and CRC32 value then stores it in the relevant place in EEPROM
Note: Each pages requires 4 bytes for its CRC32. These are stored in reverse page order (ie the last page is store first in EEPROM).
@param pageNo - Config page number
@param crc32_val - CRC32 checksum
*/
void storePageCRC32(byte pageNo, uint32_t crc32_val)
{
  uint16_t address; //Start address for the relevant page
  address = EEPROM_PAGE_CRC32 + ((getPageCount() - pageNo) * 4);

  //One = Most significant -> Four = Least significant byte
  byte four = (crc32_val & 0xFF);
  byte three = ((crc32_val >> 8) & 0xFF);
  byte two = ((crc32_val >> 16) & 0xFF);
  byte one = ((crc32_val >> 24) & 0xFF);

  //Write the 4 bytes into the eeprom memory.
  EEPROM.update(address, four);
  EEPROM.update(address + 1, three);
  EEPROM.update(address + 2, two);
  EEPROM.update(address + 3, one);
}

/** Retrieves and returns the 4 byte CRC32 checksum for a given page from EEPROM.
@param pageNo - Config page number
*/
uint32_t readPageCRC32(byte pageNo)
{
  uint16_t address; //Start address for the relevant page
  address = EEPROM_PAGE_CRC32 + ((getPageCount() - pageNo) * 4);

  //Read the 4 bytes from the eeprom memory.
  uint32_t four = EEPROM.read(address);
  uint32_t three = EEPROM.read(address + 1);
  uint32_t two = EEPROM.read(address + 2);
  uint32_t one = EEPROM.read(address + 3);

  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

// Utility functions.
// By having these in this file, it prevents other files from calling EEPROM functions directly. This is useful due to differences in the EEPROM libraries on different devces
/// Read last stored barometer reading from EEPROM.
byte readLastBaro() { return EEPROM.read(EEPROM_LAST_BARO); }
/// Write last acquired arometer reading to EEPROM.
void storeLastBaro(byte newValue) { EEPROM.update(EEPROM_LAST_BARO, newValue); }
/// Store calibration value byte into EEPROM (offset "location").
void storeCalibrationValue(uint16_t location, byte value) { EEPROM.update(location, value); } //This is essentially just an abstraction for EEPROM.update()
/// Read EEPROM current data format version (from offset EEPROM_DATA_VERSION).
byte readEEPROMVersion() { return EEPROM.read(EEPROM_DATA_VERSION); }
/// Store EEPROM current data format version (to offset EEPROM_DATA_VERSION).
void storeEEPROMVersion(byte newVersion) { EEPROM.update(EEPROM_DATA_VERSION, newVersion); }
