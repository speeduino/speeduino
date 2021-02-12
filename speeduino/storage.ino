/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/


#include "globals.h"
#include "table.h"
#include "comms.h"
#include EEPROM_LIB_H //This is defined in the board .h files
#include "storage.h"

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

  /*
  We only ever write to the EEPROM where the new value is different from the currently stored byte
  This is due to the limited write life of the EEPROM (Approximately 100,000 writes)
  */
  inline int16_t update(EERef eeRef, uint8_t value, int16_t counter)
  {
    if (eeRef!=value)
    {
      eeRef = value;
      return counter+1;
    }
    return counter;
  }

  template<class _InIt>
  inline int16_t write_range_divisor(EEPtr &eePtr, int8_t divisor, _InIt pStart, const _InIt pEnd, int16_t counter)
  {
    while (counter<=EEPROM_MAX_WRITE_BLOCK && pStart!=pEnd)
    {
      counter = update(*eePtr, (*pStart)/divisor, counter);
      ++pStart; ++eePtr;
    }
    return counter;
  }

  template<class _InIt>
  inline int16_t write_range(EEPtr &eePtr, _InIt pStart, const _InIt pEnd, int16_t counter)
  {
    return write_range_divisor(eePtr, 1, pStart, pEnd, counter);
  }

  inline int16_t writeTableValues(const table3D *pTable, EEPtr &eePtr, int16_t counter)
  {
    byte **pRow = pTable->values + (pTable->xSize-1);
    byte **pRowEnd = pTable->values - 1;
    int rowSize = pTable->xSize;
    while (counter<=EEPROM_MAX_WRITE_BLOCK && pRow!=pRowEnd)
    {
      counter = write_range(eePtr, *pRow, *pRow+rowSize, counter);
      --pRow;
    }
    return counter;
  }

  inline int16_t writeTable(const table3D *pTable, int16_t xAxisDivisor, int16_t yAxisDivisor, EEPtr &eePtr, int16_t counter)
  {
    counter = update(*eePtr, pTable->xSize, counter); ++eePtr;
    counter = update(*eePtr, pTable->ySize, counter); ++eePtr;
    counter = writeTableValues(pTable, eePtr, counter);
    counter = write_range_divisor(eePtr, xAxisDivisor, pTable->axisX, pTable->axisX+pTable->xSize, counter);
    return write_range_divisor(eePtr, yAxisDivisor, pTable->axisY, pTable->axisY+pTable->ySize, counter);
  }
}


/*
Takes the current configuration (config pages and maps)
and writes them to EEPROM as per the layout defined in storage.h
*/
void writeConfig(byte tableNum)
{
  /*
  We only ever write to the EEPROM where the new value is different from the currently stored byte
  This is due to the limited write life of the EEPROM (Approximately 100,000 writes)
  */
  int writeCounter = 0;
  EEPtr eePtr(0);

  switch(tableNum)
  {
    case veMapPage:
      /*---------------------------------------------------
      | Fuel table (See storage.h for data layout) - Page 1
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      eePtr = EEPROM_CONFIG1_XSIZE;
      writeCounter = writeTable(&fuelTable, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER, eePtr, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;
      //That concludes the writing of the VE table

    case veSetPage:
      /*---------------------------------------------------
      | Config page 2 (See storage.h for data layout)
      | 64 byte long config table
      -----------------------------------------------------*/
      eePtr = EEPROM_CONFIG2_START;
      writeCounter = write_range(eePtr, (byte *)&configPage2, (byte *)&configPage2+sizeof(configPage2), writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case ignMapPage:
      /*---------------------------------------------------
      | Ignition table (See storage.h for data layout) - Page 1
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the Ignition table, basically the same thing as above
      eePtr = EEPROM_CONFIG3_XSIZE;
      writeCounter = writeTable(&ignitionTable, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER, eePtr, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case ignSetPage:
      /*---------------------------------------------------
      | Config page 2 (See storage.h for data layout)
      | 64 byte long config table
      -----------------------------------------------------*/
      eePtr = EEPROM_CONFIG4_START;
      writeCounter = write_range(eePtr, (byte *)&configPage4, (byte *)&configPage4+sizeof(configPage4), writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case afrMapPage:
      /*---------------------------------------------------
      | AFR table (See storage.h for data layout) - Page 5
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the Ignition table, basically the same thing as above
      eePtr = EEPROM_CONFIG5_XSIZE;
      writeCounter = writeTable(&afrTable, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER, eePtr, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case afrSetPage:
      /*---------------------------------------------------
      | Config page 3 (See storage.h for data layout)
      | 64 byte long config table
      -----------------------------------------------------*/
      eePtr = EEPROM_CONFIG6_START;
      writeCounter = write_range(eePtr, (byte *)&configPage6, (byte *)&configPage6+sizeof(configPage6), writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case boostvvtPage:
      /*---------------------------------------------------
      | Boost and vvt tables (See storage.h for data layout) - Page 8
      | 8x8 table itself + the 8 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the 2 tables, basically the same thing as above but we're doing these 2 together (2 tables per page instead of 1)
      eePtr = EEPROM_CONFIG7_XSIZE1;
      writeCounter = writeTable(&boostTable, TABLE_RPM_MULTIPLIER, 1, eePtr, writeCounter);
      eePtr = EEPROM_CONFIG7_XSIZE2;
      writeCounter = writeTable(&vvtTable, TABLE_RPM_MULTIPLIER, 1, eePtr, writeCounter);
      eePtr = EEPROM_CONFIG7_XSIZE3;
      writeCounter = writeTable(&stagingTable, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER, eePtr, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case seqFuelPage:
      /*---------------------------------------------------
      | Fuel trim tables (See storage.h for data layout) - Page 9
      | 6x6 tables itself + the 6 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the 2 tables, basically the same thing as above but we're doing these 2 together (2 tables per page instead of 1)
      eePtr = EEPROM_CONFIG8_XSIZE1;
      writeCounter = writeTable(&trim1Table, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER, eePtr, writeCounter);
      eePtr = EEPROM_CONFIG8_XSIZE2;
      writeCounter = writeTable(&trim1Table, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER, eePtr, writeCounter);
      eePtr = EEPROM_CONFIG8_XSIZE3;
      writeCounter = writeTable(&trim3Table, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER, eePtr, writeCounter);
      eePtr = EEPROM_CONFIG8_XSIZE4;
      writeCounter = writeTable(&trim4Table, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER, eePtr, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case canbusPage:
      /*---------------------------------------------------
      | Config page 10 (See storage.h for data layout)
      | 192 byte long config table
      -----------------------------------------------------*/
      eePtr = EEPROM_CONFIG9_START;
      writeCounter = write_range(eePtr, (byte *)&configPage9, (byte *)&configPage9+sizeof(configPage9), writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case warmupPage:
      /*---------------------------------------------------
      | Config page 11 (See storage.h for data layout)
      | 192 byte long config table
      -----------------------------------------------------*/
      eePtr = EEPROM_CONFIG10_START;
      writeCounter = write_range(eePtr, (byte *)&configPage10, (byte *)&configPage10+sizeof(configPage10), writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    case fuelMap2Page:
      /*---------------------------------------------------
      | Fuel table 2 (See storage.h for data layout)
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      eePtr = EEPROM_CONFIG11_XSIZE;
      writeCounter = writeTable(&fuelTable2, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER, eePtr, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;
      //That concludes the writing of the 2nd fuel table

    case wmiMapPage:
      /*---------------------------------------------------
      | WMI tables (See storage.h for data layout) - Page 12
      | 8x8 table itself + the 8 values along each of the axis
      -----------------------------------------------------*/
      eePtr = EEPROM_CONFIG12_XSIZE;
      writeCounter = writeTable(&wmiTable, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER, eePtr, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;
      
  case progOutsPage:
      /*---------------------------------------------------
      | Config page 13 (See storage.h for data layout)
      -----------------------------------------------------*/
      eePtr = EEPROM_CONFIG13_START;
      writeCounter = write_range(eePtr, (byte *)&configPage13, (byte *)&configPage13+sizeof(configPage13), writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;
    
    case ignMap2Page:
      /*---------------------------------------------------
      | Ignition table (See storage.h for data layout) - Page 1
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the Ignition table, basically the same thing as above
      eePtr = EEPROM_CONFIG14_XSIZE;
      writeCounter = writeTable(&ignitionTable2, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER, eePtr, writeCounter);
      eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
      break;

    default:
      break;
  }
}

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
  template<class _InIt,
	class _OutIt> inline
	_InIt copy_advance_input(_InIt _Source, _OutIt _First, _OutIt _Last)
  {
	  for (; _First != _Last; ++_Source, (void)++_First)
		{
		  *_First = *_Source;
		}
    return _Source;
  }

  template<class _InIt,
	class _OutIt,
	class _Fn> inline
	_InIt transform_advance_input(_InIt _Source, _OutIt _First, _OutIt _Last, _Fn _Func)
	{	// transform [_First, _Last) with _Func
  	for (; _First != _Last; ++_Source, (void)++_First)
		{
		  *_First = _Func(*_Source);
		}
    return _Source;
  }

  inline EEPtr loadTableValues(table3D *pTable, EEPtr eePtr)
  {
    byte **pRow = pTable->values + (pTable->xSize-1);
    byte **pRowEnd = pTable->values - 1;
    int rowSize = pTable->xSize;
    for(; pRow!=pRowEnd; --pRow)
    {
      eePtr = copy_advance_input(eePtr, *pRow, *pRow+rowSize);
    }
    return eePtr; 
  }

  inline EEPtr loadTableAxisX(table3D *pTable, EEPtr eePtr, int xAxisMultiplier)
  {
    return transform_advance_input(eePtr, pTable->axisX, pTable->axisX+pTable->xSize, [xAxisMultiplier](uint8_t value) { return value * xAxisMultiplier; } );
  }

  inline EEPtr loadTableAxisY(table3D *pTable, EEPtr eePtr, int yAxisMultiplier)
  {
    return transform_advance_input(eePtr, pTable->axisY, pTable->axisY+pTable->ySize, [yAxisMultiplier](uint8_t value) { return value * yAxisMultiplier; } );
  }

  inline EEPtr loadTable(table3D *pTable, EEPtr eePtr, int xAxisMultiplier, int yAxisMultiplier)
  {
    return loadTableAxisY(pTable,
                          loadTableAxisX(pTable, 
                                          loadTableValues(pTable, eePtr), 
                                          xAxisMultiplier),
                          yAxisMultiplier);
  }
}

void loadConfig()
{
  loadTable(&fuelTable, EEPtr(EEPROM_CONFIG1_MAP), TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);
  copy_advance_input(EEPtr(EEPROM_CONFIG2_START), (byte *)&configPage2, (byte *)&configPage2+sizeof(configPage2));
  //That concludes the reading of the VE table
  
  //*********************************************************************************************************************************************************************************
  //IGNITION CONFIG PAGE (2)

  //Begin writing the Ignition table, basically the same thing as above
  loadTable(&ignitionTable, EEPtr(EEPROM_CONFIG3_MAP), TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);
  copy_advance_input(EEPtr(EEPROM_CONFIG4_START), (byte *)&configPage4, (byte *)&configPage4+sizeof(configPage4));

  //*********************************************************************************************************************************************************************************
  //AFR TARGET CONFIG PAGE (3)

  //Begin writing the Ignition table, basically the same thing as above
  loadTable(&afrTable, EEPtr(EEPROM_CONFIG5_MAP), TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);
  copy_advance_input(EEPtr(EEPROM_CONFIG6_START), (byte *)&configPage6, (byte *)&configPage6+sizeof(configPage6));

  //*********************************************************************************************************************************************************************************
  // Boost and vvt tables load
  loadTable(&boostTable, EEPtr(EEPROM_CONFIG7_MAP1), TABLE_RPM_MULTIPLIER, 1);
  loadTable(&vvtTable, EEPtr(EEPROM_CONFIG7_MAP2), TABLE_RPM_MULTIPLIER, 1);
  loadTable(&stagingTable, EEPtr(EEPROM_CONFIG7_MAP3), TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);

  //*********************************************************************************************************************************************************************************
  // Fuel trim tables load
  loadTable(&trim1Table, EEPtr(EEPROM_CONFIG8_MAP1), TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);
  loadTable(&trim2Table, EEPtr(EEPROM_CONFIG8_MAP2), TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);
  loadTable(&trim3Table, EEPtr(EEPROM_CONFIG8_MAP3), TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);
  loadTable(&trim4Table, EEPtr(EEPROM_CONFIG8_MAP4), TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);

  //*********************************************************************************************************************************************************************************
  //canbus control page load
  copy_advance_input(EEPtr(EEPROM_CONFIG9_START), (byte *)&configPage9, (byte *)&configPage9+sizeof(configPage9));

  //*********************************************************************************************************************************************************************************

  //CONFIG PAGE (10)
  copy_advance_input(EEPtr(EEPROM_CONFIG10_START), (byte *)&configPage10, (byte *)&configPage10+sizeof(configPage10));

  //*********************************************************************************************************************************************************************************
  //Fuel table 2 (See storage.h for data layout)
  loadTable(&fuelTable2, EEPtr(EEPROM_CONFIG11_MAP), TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);

  //*********************************************************************************************************************************************************************************
  // WMI table load
  loadTable(&wmiTable, EEPtr(EEPROM_CONFIG12_MAP), TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);
    
  //*********************************************************************************************************************************************************************************
  //CONFIG PAGE (13)
  copy_advance_input(EEPtr(EEPROM_CONFIG13_START), (byte *)&configPage13, (byte *)&configPage13+sizeof(configPage13));

  //*********************************************************************************************************************************************************************************
  //SECOND IGNITION CONFIG PAGE (14)

  //Begin writing the Ignition table, basically the same thing as above
  loadTable(&ignitionTable2, EEPtr(EEPROM_CONFIG14_MAP), TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);

  //*********************************************************************************************************************************************************************************
}

/*
Reads the calibration information from EEPROM.
This is separate from the config load as the calibrations do not exist as pages within the ini file for Tuner Studio
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

/*
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

/*
Takes a page number and CRC32 value then stores it in the relevant place in EEPROM
Note: Each pages requires 4 bytes for its CRC32. These are stored in reverse page order (ie the last page is store first in EEPROM)
*/
void storePageCRC32(byte pageNo, uint32_t crc32_val)
{
  uint16_t address; //Start address for the relevant page
  address = EEPROM_PAGE_CRC32 + ((NUM_PAGES - pageNo) * 4);

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

/*
Retrieves and returns the 4 byte CRC32 for a given page from EEPROM
*/
uint32_t readPageCRC32(byte pageNo)
{
  uint16_t address; //Start address for the relevant page
  address = EEPROM_PAGE_CRC32 + ((NUM_PAGES - pageNo) * 4);

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
byte readLastBaro() { return EEPROM.read(EEPROM_LAST_BARO); }
void storeLastBaro(byte newValue) { EEPROM.update(EEPROM_LAST_BARO, newValue); }
void storeCalibrationValue(uint16_t location, byte value) { EEPROM.update(location, value); } //This is essentially just an abstraction for EEPROM.update()
byte readEEPROMVersion() { return EEPROM.read(EEPROM_DATA_VERSION); }
void storeEEPROMVersion(byte newVersion) { EEPROM.update(EEPROM_DATA_VERSION, newVersion); }
