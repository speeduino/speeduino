  /*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
/** @file
 * Lower level ConfigPage*, Table2D, Table3D and EEPROM storage operations.
 */

#include "globals.h"
#include EEPROM_LIB_H //This is defined in the board .h files
#include "storage.h"
#include "pages.h"
#include "table_iterator.h"

static bool eepromWritesPending = false;

bool isEepromWritePending()
{
  return eepromWritesPending;
}

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

//The maximum number of write operations that will be performed in one go. If we try to write to the EEPROM too fast (Each write takes ~3ms) then the rest of the system can hang)
#if defined(CORE_STM32) || defined(CORE_TEENSY) & !defined(USE_SPI_EEPROM)
#define EEPROM_MAX_WRITE_BLOCK 64 
#else
#define EEPROM_MAX_WRITE_BLOCK 30
#endif

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

//  ================================= Internal read & write support ===============================

static eeprom_address_t page_start_index(uint8_t pageNum)
{
  // Pages start at index 1 & are packed end-to-end
  eeprom_address_t index = 1U;
  while (pageNum!=0U)
  {
    --pageNum;
    index += getPageSize(pageNum);
  }
  // Page 0 has no size
  return index;
}

//  ================================= Internal write support ===============================


/** Update byte to EEPROM by first comparing content and the need to write it.
We only ever write to the EEPROM where the new value is different from the currently stored byte
This is due to the limited write life of the EEPROM (Approximately 100,000 writes)
*/
static inline uint8_t update(eeprom_address_t index, uint8_t value, uint8_t counter)
{
  if (EEPROM.read(index)!=value)
  {
    EEPROM.write(index, value);
    ++counter;
  }
  return counter;
}

static inline uint8_t write_range(eeprom_address_t &index, const byte *pStart, const byte * const pEnd, uint8_t counter)
{
  while ((counter<=EEPROM_MAX_WRITE_BLOCK) && pStart!=pEnd)
  {
    counter = update(index, *pStart, counter);
    ++pStart; ++index;
  }
  return counter;
}

static inline uint8_t write(const table_row_t &row, eeprom_address_t &index, uint8_t counter)
{
  return write_range(index, row.pValue, row.pEnd, counter);
}

static inline uint8_t write(table_row_iterator_t it, eeprom_address_t &index, uint8_t counter)
{
  while ((counter<=EEPROM_MAX_WRITE_BLOCK) && !at_end(it))
  {
    counter = write(get_row(it), index, counter);
    it = advance_row(it);
  }
  return counter;
}

static inline uint8_t write(table_axis_iterator_t it, eeprom_address_t &index, uint8_t counter)
{
  while ((counter<=EEPROM_MAX_WRITE_BLOCK) && !at_end(it))
  {
    counter = update(index, get_value(it), counter);
    ++index;
    it = advance_axis(it);
  }
  return counter;
}

static inline uint8_t write_raw(const page_iterator_t &entity, eeprom_address_t &index, uint8_t counter)
{
  const byte *pStart = (byte *)(entity.pData);
  return write_range(index, pStart, pStart+entity.size, counter);
}

static inline uint8_t write_table(const table3D * const pTable, eeprom_address_t &index, uint8_t counter)
{
  return write(y_begin(pTable), index, 
                write(x_begin(pTable), index, 
                  write(rows_begin(pTable), index, counter)));
}

static inline uint8_t write(const page_iterator_t &entity, eeprom_address_t &index, uint8_t counter)
{
  switch (entity.type)
  {
  case Raw:
    counter = write_raw(entity, index, counter);
    break;

  case Table:
    counter = write_table(entity.pTable, index, counter);
    break;

  case NoEntity:
    index += entity.size;
    break;
  
  default:
    abort();  // Code error
    break;
  }

    return counter;
  }

//  ================================= End write support ===============================

/** Write a table or map to EEPROM storage.
Takes the current configuration (config pages and maps)
and writes them to EEPROM as per the layout defined in storage.h.
*/
void writeConfig(uint8_t pageNum)
{
  eeprom_address_t index = page_start_index(pageNum);
  uint8_t writeCounter = 0;
  page_iterator_t entity = page_begin(pageNum);
  while ((writeCounter<=EEPROM_MAX_WRITE_BLOCK) && entity.type!=End)
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
  uint8_t pageCount = getPageCount();
  for (uint8_t page=1; page<pageCount; ++page)
  {
    page_iterator_t entity = page_begin(page);
    while (entity.type!=End)
    {
      if (entity.type==Raw)
      {
        (void)memset(entity.pData, 0, entity.size);
      }
      entity = advance(entity);
    }
  }
}

//  ================================= Internal read support ===============================

/** Load range of bytes form EEPROM offset to memory.
 * @param index - start offset in EEPROM
 * @param pFirst - Start memory address
 * @param pLast - End memory address
 */
static inline eeprom_address_t load_range(eeprom_address_t index, byte *pFirst, const byte * const pLast)
{
  for (; pFirst != pLast; ++index, (void)++pFirst)
  {
    *pFirst = EEPROM.read(index);
  }
  return index;
}

static inline eeprom_address_t load(table_row_t row, eeprom_address_t index)
{
  return load_range(index, row.pValue, row.pEnd);
}

static inline eeprom_address_t load(table_row_iterator_t it, eeprom_address_t index)
{
  while (!at_end(it))
  {
    index = load(get_row(it), index);
    it = advance_row(it);
  }
  return index; 
}

static inline eeprom_address_t load(table_axis_iterator_t it, eeprom_address_t index)
{
  while (!at_end(it))
  {
    set_value(it, EEPROM.read(index));
    ++index;
    it = advance_axis(it);
  }
  return index;    
}

static inline eeprom_address_t load_raw(page_iterator_t &entity, eeprom_address_t index)
{
  byte *pStart = (byte *)(entity.pData);
  return load_range(index, pStart, pStart+entity.size);
}

static inline eeprom_address_t load(const table3D * const pTable, eeprom_address_t index)
{
  return load(y_begin(pTable),
                load(x_begin(pTable), 
                  load(rows_begin(pTable), index)));
}


static inline eeprom_address_t load(page_iterator_t &entity, eeprom_address_t index)
{
  switch (entity.type)
  {
  case Raw:
    index = load_raw(entity, index);
    break;

  case Table: 
    index = load(entity.pTable, index);
    break;

  case NoEntity: 
    index += (eeprom_address_t)(entity.size);
    break;
  
  default:
    abort();  // Code error
    break;
  }

  return index;
}

static inline void load_page(uint8_t pageNum)
{
  eeprom_address_t index = page_start_index(pageNum);
  page_iterator_t entity = page_begin(pageNum);
  while (entity.type!=End)
  {
    index = load(entity, index);
    entity = advance(entity);
  }
}

//  ================================= End internal read support ===============================

/** Load all config tables from storage.
 */
void loadConfig()
{
  uint8_t pageCount = getPageCount();
  for (uint8_t page=1; page<pageCount; ++page)
  {
    load_page(page);
  }
}

/** Read the calibration information from EEPROM.
This is separate from the config load as the calibrations do not exist as pages within the ini file for Tuner Studio.
*/
void loadCalibration()
{
  // If you modify this function be sure to also modify writeCalibration()
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
  // If you modify this function be sure to also modify loadCalibration()
  // it should be a mirror image of this function.

  EEPROM.put(EEPROM_CALIBRATION_O2_BINS, o2Calibration_bins);
  EEPROM.put(EEPROM_CALIBRATION_O2_VALUES, o2Calibration_values);
  
  EEPROM.put(EEPROM_CALIBRATION_IAT_BINS, iatCalibration_bins);
  EEPROM.put(EEPROM_CALIBRATION_IAT_VALUES, iatCalibration_values);

  EEPROM.put(EEPROM_CALIBRATION_CLT_BINS, cltCalibration_bins);
  EEPROM.put(EEPROM_CALIBRATION_CLT_VALUES, cltCalibration_values);
}

static eeprom_address_t compute_crc_address(uint8_t pageNum)
{
  return EEPROM_LAST_BARO-((getPageCount() - pageNum)*sizeof(uint32_t));
}

/** Write CRC32 checksum to EEPROM.
Takes a page number and CRC32 value then stores it in the relevant place in EEPROM
Note: Each pages requires 4 bytes for its CRC32. These are stored in reverse page order (ie the last page is store first in EEPROM).
@param pageNo - Config page number
@param crc32_val - CRC32 checksum
*/
void storePageCRC32(uint8_t pageNum, uint32_t crc32_val)
{
  EEPROM.put(compute_crc_address(pageNum), crc32_val);
}

/** Retrieves and returns the 4 byte CRC32 checksum for a given page from EEPROM.
@param pageNo - Config page number
*/
uint32_t readPageCRC32(uint8_t pageNum)
{
  uint32_t crc32_val;
  return EEPROM.get(compute_crc_address(pageNum), crc32_val);
}

// Utility functions.
// By having these in this file, it prevents other files from calling EEPROM functions directly. This is useful due to differences in the EEPROM libraries on different devces
/// Read last stored barometer reading from EEPROM.
byte readLastBaro() { return EEPROM.read(EEPROM_LAST_BARO); }
/// Write last acquired arometer reading to EEPROM.
void storeLastBaro(byte newValue) { EEPROM.update(EEPROM_LAST_BARO, newValue); }
/// Read EEPROM current data format version (from offset EEPROM_DATA_VERSION).
uint8_t readEEPROMVersion() { return EEPROM.read(EEPROM_DATA_VERSION); }
/// Store EEPROM current data format version (to offset EEPROM_DATA_VERSION).
void storeEEPROMVersion(uint8_t version) { EEPROM.update(EEPROM_DATA_VERSION, version); }
