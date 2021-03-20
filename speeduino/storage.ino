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
#include "pages.h"
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

  uint16_t page_start_index(uint8_t pageNumber)
  {
    // Pages start at index 1 & are packed end-to-end
    uint16_t index = 1;
    while (pageNumber>0)
    {
      index += getPageSize(--pageNumber);
    }
    // Page 0 has no size
    return index;
  }
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

  inline int16_t write_range(int &index, const byte *pStart, const byte *pEnd, int16_t counter)
  {
    while (counter<=EEPROM_MAX_WRITE_BLOCK && pStart!=pEnd)
    {
      counter = update(index, *pStart, counter);
      ++pStart; ++index;
    }
    return counter;
  }

  inline int16_t write(const table_row_t &row, int &index, int16_t counter)
  {
    return write_range(index, row.pValue, row.pEnd, counter);
  }

  inline int16_t write(table_row_iterator_t it, int &index, int16_t counter)
  {
    while (counter<=EEPROM_MAX_WRITE_BLOCK && !at_end(it))
    {
      counter = write(get_row(it), index, counter);
      advance_row(it);
    }
    return counter;
  }

  inline int16_t write(table_axis_iterator_t it, int &index, int16_t counter)
  {
    while (counter<=EEPROM_MAX_WRITE_BLOCK && !at_end(it))
    {
      counter = update(index++, get_value(it), counter);
      advance_axis(it);
    }
    return counter;
  }

  inline int16_t write(const table3D *pTable, int &index, int16_t counter)
  {
    counter = write(rows_begin(pTable), index, counter);
    counter = write(x_begin(pTable), index, counter);
    return write(y_begin(pTable), index, counter);
  }

  inline int16_t write(const page_iterator_t &entity, int &index, int16_t counter)
  {
    switch (entity.type)
    {
    case Raw:
      return write_range(index, (byte *)entity.pData, ((byte *)entity.pData)+entity.size, counter);
      break;

    case Table:
      return write(entity.pTable, index, counter);
      break;

    case NoEntity:
      index += entity.size;
      return counter;
      break;
    
    default:
      abort();  // Code error
      break;
    }
  }
}

/** Write a table or map to EEPROM storage.
Takes the current configuration (config pages and maps)
and writes them to EEPROM as per the layout defined in storage.h.
*/
void writeConfig(byte pageNum)
{
  int index = page_start_index(pageNum);
  int writeCounter = 0;
  page_iterator_t entity = page_begin(pageNum);
  while (writeCounter<=EEPROM_MAX_WRITE_BLOCK && entity.type!=End)
  {
    writeCounter = write(entity, index, writeCounter);
    entity = advance(entity);
  }
  eepromWritesPending = writeCounter > EEPROM_MAX_WRITE_BLOCK;
}
/** Reset all configPage* structs (2,4,6,9,10,13) and write them full of null-bytes.
 */
void resetConfigPages()
{
  for (uint8_t page=1; page<getPageCount(); ++page)
  {
    page_iterator_t entity = page_begin(page);
    while (entity.type!=End)
    {
      if (entity.type==Raw)
      {
        memset(entity.pData, 0, entity.size);
      }
      entity = advance(entity);
    }
  }
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

  inline int load(table_row_t row, int index)
  {
    return load_range(index, row.pValue, row.pEnd);;
  }

  inline int load(table_row_iterator_t it, int index)
  {
    while (!at_end(it))
    {
      index = load(get_row(it), index);
      advance_row(it);
    }
    return index; 
  }

  inline int load(table_axis_iterator_t it, int index)
  {
    while (!at_end(it))
    {
      set_value(it, EEPROM.read(index++));
      advance_axis(it);
    }
    return index;    
  }

  inline int load(table3D *pTable, int index)
  {
    return load(y_begin(pTable),
                  load(x_begin(pTable), 
                    load(rows_begin(pTable), index)));
  }

  
  inline int load(page_iterator_t &entity, int index)
  {
    switch (entity.type)
    {
    case Raw: return load_range(index, (byte *)entity.pData, (byte *)entity.pData+entity.size);

    case Table: return load(entity.pTable, index);

    case NoEntity: return index + entity.size;
    
    default:
      abort();  // Code error
      break;
    }
  }

  inline void load_page(uint8_t page)
  {
    int index = page_start_index(page);
    page_iterator_t entity = page_begin(page);
    while (entity.type!=End)
    {
      index = load(entity, index);
      entity = advance(entity);
    }
  }
}
/** Load all config tables from storage.
 */
void loadConfig()
{
  for (uint8_t page=1; page<getPageCount(); ++page)
  {
    load_page(page);
  }
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
