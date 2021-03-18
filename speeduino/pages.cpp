#include "pages.h"
#include "src/FastCRC/FastCRC.h"
#include "globals.h"
#include "utilities.h"
#include "table_iterator.h"

const uint16_t npage_size[NUM_PAGES] = {0,128,288,288,128,288,128,240,384,192,192,288,192,128,288}; /**< This array stores the size (in bytes) of each configuration page */

// This namespace maps from virtual page "addresses" to addresses/bytes of real in memory entities
//
// For TunerStudio:
// 1. Each page has a numeric identifier (0 to N-1)
// 2. A single page is a continguous block of data.
// So individual bytes are identified by a page number + offset
//
// The TS layout is not what is in memory. E.g.
//
//    TS Page 2               |0123456789ABCD|0123456789ABCDEF|
//                                |                       |
//    Arduino In Memory  |--- Entity A ---|         |--- Entity B -----|
//
// Further, the in memory entity may also not be contiguous or in the same
// order that TS expects
//
// So there is a 2 stage mapping:
//  1. Page # + Offset to entity
//  2. Offset to intra-entity byte
// This namespace encapsulates step 1
namespace 
{
  // What section of a 3D table the offset mapped to
  enum struct table3D_section_t : uint8_t { 
    Value,  // The values
    axisX,  // X axis
    axisY,  // Y axis
    None    // Should never happen!
  };

  // Stores enough information to access a table element
  struct table_entity_t {
    table3D *pTable;
    uint8_t xIndex; // Value X index or X axis index
    uint8_t yIndex; // Value Y index or Y axis index
    table3D_section_t section;
  };  

  struct entity_t {
    // The entity that the offset mapped to
    union {
      table_entity_t table;
      void *pData;
    };
    uint8_t page;   // The page the entity belongs to
    uint16_t start; // The start position of the entity, in bytes, from the start of the page
    uint16_t size;  // Size of the entity in bytes
    entity_type type;
  };

  // Handy table macros
  #define TABLE_VALUE_END(size) ((uint16_t)size*(uint16_t)size)
  #define TABLE_AXISX_END(size) (TABLE_VALUE_END(size)+(uint16_t)size)
  #define TABLE_AXISY_END(size) (TABLE_AXISX_END(size)+(uint16_t)size)
  #define TABLE_SIZE(size) TABLE_AXISY_END(size)

  // Precompute for performance
  #define TABLE16_SIZE TABLE_SIZE(16)
  #define TABLE8_SIZE TABLE_SIZE(8)
  #define TABLE6_SIZE TABLE_SIZE(6)
  #define TABLE4_SIZE TABLE_SIZE(4)

  // Macros + compile time constants = fast division/modulus
  //
  // The various fast division libraries, E.g. libdivide, use
  // 32-bit operations for 16-bit division. Super slow.
  #define OFFSET_TOVALUE_YINDEX(offset, size) ((uint8_t)((size-1) - (offset / size)))
  #define OFFSET_TOVALUE_XINDEX(offset, size) ((uint8_t)(offset % size))
  #define OFFSET_TOAXIS_XINDEX(offset, size) ((uint8_t)(offset - TABLE_VALUE_END(size)))
  #define OFFSET_TOAXIS_YINDEX(offset, size) ((uint8_t)((size-1) - (offset - TABLE_AXISX_END(size))))

  #define NULL_TABLE \
    { nullptr, 0, 0, table3D_section_t::None }

  // Create an End entity_t
  #define PAGE_END(pageNum, pageSize) \
    { NULL_TABLE, .page = pageNum, .start = 0, .size = pageSize, .type = entity_type::End };
 
  // If the offset is in range, create a None entity_t
#define CHECK_NOENTITY(offset, startByte, blockSize, pageNum) \
    if (offset < (startByte)+blockSize) \
    { \
      return { NULL_TABLE, .page = pageNum, .start = (startByte), .size = blockSize, .type = entity_type::None }; \
    } 

  // 
  #define TABLE_VALUE(offset, startByte, pTable, tableSize) \
    { pTable, \
      OFFSET_TOVALUE_XINDEX((offset-(startByte)), tableSize), \
      OFFSET_TOVALUE_YINDEX((offset-(startByte)), tableSize), \
      table3D_section_t::Value }

  #define TABLE_XAXIS(offset, startByte, pTable, tableSize) \
    { pTable, \
      OFFSET_TOAXIS_XINDEX((offset-(startByte)), tableSize), \
      0U, \
      table3D_section_t::axisX }

  #define TABLE_YAXIS(offset, startByte, pTable, tableSize) \
    { pTable, \
      0U, \
      OFFSET_TOAXIS_YINDEX((offset-(startByte)), tableSize), \
      table3D_section_t::axisY }

  #define TABLE_ENTITY(table_type, pageNum, startByte, tableSize) \
    { table_type, .page = pageNum, .start = (startByte), .size = TABLE_SIZE(tableSize), .type = entity_type::Table }

  // If the offset is in range, create a Table entity_t
  #define CHECK_TABLE(offset, startByte, pTable, tableSize, pageNum) \
    if (offset < (startByte)+TABLE_VALUE_END(tableSize)) \
    { \
      return TABLE_ENTITY(TABLE_VALUE(offset, startByte, pTable, tableSize), pageNum, startByte, tableSize); \
    } \
    if (offset < (startByte)+TABLE_AXISX_END(tableSize)) \
    { \
      return TABLE_ENTITY(TABLE_XAXIS(offset, startByte, pTable, tableSize), pageNum, startByte, tableSize); \
    } \
    if (offset < (startByte)+TABLE_AXISY_END(tableSize)) \
    { \
      return TABLE_ENTITY(TABLE_YAXIS(offset, startByte, pTable, tableSize), pageNum, startByte, tableSize); \
    }

  // If the offset is in range, create a Raw entity_t
  #define CHECK_RAW(offset, startByte, pDataBlock, blockSize, pageNum) \
    if (offset < (startByte)+blockSize) \
    { \
      return { { (table3D*)pDataBlock, 0, 0, table3D_section_t::None }, .page = pageNum, .start = (startByte), .size = blockSize, .type = entity_type::Raw }; \
    } 

  // Does the heavy lifting of mapping page+offset to an entity
  //
  // Alternative implementation would be to encode the mapping into data structures
  // That uses flash memory, which is scarce. And it was too slow.
  inline __attribute__((always_inline)) // <-- this is critical for performance
  entity_t map_page_offset_to_entity_inline(uint8_t pageNumber, uint16_t offset)
  {
    switch (pageNumber)
    {
      case veMapPage:
        CHECK_TABLE(offset, 0U, &fuelTable, 16, pageNumber)
        return PAGE_END(pageNumber, TABLE16_SIZE);
        break;

      case ignMapPage: //Ignition settings page (Page 2)
        CHECK_TABLE(offset, 0U, &ignitionTable, 16, pageNumber)
        return PAGE_END(pageNumber, TABLE16_SIZE);
        break;

      case afrMapPage: //Air/Fuel ratio target settings page
        CHECK_TABLE(offset, 0U, &afrTable, 16, pageNumber)
        return PAGE_END(pageNumber, TABLE16_SIZE);
        break;

      case boostvvtPage: //Boost, VVT and staging maps (all 8x8)
        CHECK_TABLE(offset, 0U, &boostTable, 8, pageNumber)
        CHECK_TABLE(offset, TABLE8_SIZE, &vvtTable, 8, pageNumber)
        CHECK_TABLE(offset, TABLE8_SIZE*2, &stagingTable, 8, pageNumber)
        return PAGE_END(pageNumber, TABLE8_SIZE*3);
        break;

      case seqFuelPage:
        CHECK_TABLE(offset, 0U, &trim1Table, 6, pageNumber)
        CHECK_TABLE(offset, TABLE6_SIZE*1, &trim2Table, 6, pageNumber)
        CHECK_TABLE(offset, TABLE6_SIZE*2, &trim3Table, 6, pageNumber)
        CHECK_TABLE(offset, TABLE6_SIZE*3, &trim4Table, 6, pageNumber)
        CHECK_TABLE(offset, TABLE6_SIZE*4, &trim5Table, 6, pageNumber)
        CHECK_TABLE(offset, TABLE6_SIZE*5, &trim6Table, 6, pageNumber)
        CHECK_TABLE(offset, TABLE6_SIZE*6, &trim7Table, 6, pageNumber)
        CHECK_TABLE(offset, TABLE6_SIZE*7, &trim8Table, 6, pageNumber)
        return PAGE_END(pageNumber, TABLE6_SIZE*8);
        break;

      case fuelMap2Page:
        CHECK_TABLE(offset, 0U, &fuelTable2, 16, pageNumber)
        return PAGE_END(pageNumber, TABLE16_SIZE);
        break;

      case wmiMapPage:
        CHECK_TABLE(offset, 0U, &wmiTable, 8, pageNumber)
        CHECK_NOENTITY(offset, TABLE8_SIZE, 80, pageNumber)
        CHECK_TABLE(offset, TABLE8_SIZE + 80, &dwellTable, 4, pageNumber)
        return PAGE_END(pageNumber, TABLE8_SIZE + 80 + TABLE4_SIZE);
        break;
      
      case ignMap2Page:
        CHECK_TABLE(offset, 0U, &ignitionTable2, 16, pageNumber)
        return PAGE_END(pageNumber, TABLE16_SIZE);
        break;

      case veSetPage: 
        CHECK_RAW(offset, 0U, &configPage2, sizeof(configPage2), pageNumber)
        return PAGE_END(pageNumber, sizeof(configPage2));
        break;

      case ignSetPage: 
        CHECK_RAW(offset, 0U, &configPage4, sizeof(configPage4), pageNumber)
        return PAGE_END(pageNumber, sizeof(configPage4));
        break;

      case afrSetPage: 
        CHECK_RAW(offset, 0U, &configPage6, sizeof(configPage6), pageNumber)
        return PAGE_END(pageNumber, sizeof(configPage6));
        break;

      case canbusPage:  
        CHECK_RAW(offset, 0U, &configPage9, sizeof(configPage9), pageNumber)
        return PAGE_END(pageNumber, sizeof(configPage9));
        break;

      case warmupPage: 
        CHECK_RAW(offset, 0U, &configPage10, sizeof(configPage10), pageNumber)
        return PAGE_END(pageNumber, sizeof(configPage10));
        break;

      case progOutsPage: 
        CHECK_RAW(offset, 0U, &configPage13, sizeof(configPage13), pageNumber)
        return PAGE_END(pageNumber, sizeof(configPage13));
        break;      

      default:
        break;
    }

    return PAGE_END(pageNumber, 0);
  }
}

// Tables do not map linearly to the TS page address space, so special 
// handling is necessary (we do not use the normal array layout for
// performance reasons elsewhere)
//
// We take the offset & map it to a single value, x-axis or y-axis element
namespace
{
  inline byte get_table_value(const table_entity_t &table)
  {
    switch (table.section)
    {
      case table3D_section_t::Value: 
        return table.pTable->values[table.yIndex][table.xIndex];

      case table3D_section_t::axisX:
        return (byte)(table.pTable->axisX[table.xIndex] / getTableXAxisFactor(table.pTable)); 
      
      case table3D_section_t::axisY:
        return (byte)(table.pTable->axisY[table.yIndex] / getTableYAxisFactor(table.pTable)); 
      
      default: return 0; // no-op
    }
    return 0U;
  }

  inline void set_table_value(const table_entity_t &table, int8_t value)
  {
    switch (table.section)
    {
      case table3D_section_t::Value: 
        table.pTable->values[table.yIndex][table.xIndex] = value;
        break;

      case table3D_section_t::axisX:
        table.pTable->axisX[table.xIndex] = (int16_t)(value) * getTableXAxisFactor(table.pTable); 
        break;
      
      case table3D_section_t::axisY:
        table.pTable->axisY[table.yIndex]= (int16_t)(value) * getTableYAxisFactor(table.pTable);
        break;
      
      default: ; // no-op
    }    
    table.pTable->cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
  }
}

namespace {
  
  inline byte* get_raw_value(const entity_t &entity, uint16_t offset)
  {
    return (byte*)entity.pData + offset;
  }

}

void setPageValue(byte pageNum, uint16_t offset, byte value)
{
  entity_t entity = map_page_offset_to_entity_inline(pageNum, offset);

  switch (entity.type)
  {
  case entity_type::Table:
    set_table_value(entity.table, value);
    break;
  
  case entity_type::Raw:
    *get_raw_value(entity, offset-entity.start) = value;
    break;
      
  default:
    break;
  }
}


/**
 * @brief Retrieves a single value from a memory page, with data aligned as per the ini file
 * 
 * @param page The page number to retrieve data from
 * @param valueAddress The address in the page that should be returned. This is as per the page definition in the ini
 * @return byte The requested value
 */
byte getPageValue(byte page, uint16_t offset)
{
  entity_t entity = map_page_offset_to_entity_inline(page, offset);

  switch (entity.type)
  {
    case entity_type::Table:
      return get_table_value(entity.table);
      break;

    case entity_type::Raw:
      return *get_raw_value(entity, offset);
      break;

    default: return 0U;
  }
  return 0U;
}

namespace {
  
  // Because trhe page iterators will not be called for every single byte
  // inlining the mapping function is not performance critical.
  //
  // So save some memory.
  entity_t map_page_offset_to_entity(uint8_t pageNumber, uint16_t offset)
  {
    return map_page_offset_to_entity_inline(pageNumber, offset);
  }

  inline page_entity_t to_page_entity(entity_t mapped)
  {
    return { { mapped.type==entity_type::Table ? mapped.table.pTable : (table3D*)mapped.pData }, 
           .page=mapped.page, .start = mapped.start, .size = mapped.size, .type = mapped.type };
  }
}

// Support iteration over a pages entities.
// Check for entity.type==entity_type::End
page_entity_t page_begin(byte pageNum)
{
  return to_page_entity(map_page_offset_to_entity(pageNum, 0U));
}

page_entity_t advance(const page_entity_t &it)
{
    return to_page_entity(map_page_offset_to_entity(it.page, it.start+it.size));
}

namespace {

  FastCRC32 CRC32;

  typedef uint32_t (FastCRC32::*pCrcCalc)(const uint8_t *, const uint16_t, bool);

  inline uint32_t compute_raw_crc(page_entity_t &entity, pCrcCalc calcFunc)
  {
    return (CRC32.*calcFunc)((uint8_t*)entity.pData, entity.size, false);
  }

  inline uint32_t compute_tablevalues_crc(table_row_iterator_t it, pCrcCalc calcFunc)
  {
    table_row_t row = get_row(it);
    uint32_t crc = (CRC32.*calcFunc)(row.pValue, row.pEnd-row.pValue, false);
    advance_row(it);

    while (!at_end(it))
    {
      row = get_row(it);
      crc = CRC32.crc32_upd(row.pValue, row.pEnd-row.pValue, false);
      advance_row(it);
    }
    return crc;
  }

  inline uint32_t compute_tableaxis_crc(table_axis_iterator_t it, uint32_t crc)
  {
    byte values[32]; // Fingers crossed we don't have a table bigger than 32x32
    byte *pValue = values;
    while (!at_end(it))
    {
      *pValue++ = get_value(it);
      it = advance_axis(it);
    }
    return pValue-values==0 ? crc : CRC32.crc32_upd(values, pValue-values, false);
  }

  inline uint32_t compute_table_crc(table3D *pTable, pCrcCalc calcFunc)
  {
    return compute_tableaxis_crc(y_begin(pTable), 
              compute_tableaxis_crc(x_begin(pTable),
                  compute_tablevalues_crc(rows_begin(pTable), calcFunc)));
  }

  inline uint32_t pad_crc(uint16_t padding, uint32_t crc)
  {
    uint8_t raw_value = 0u;
    while (padding>0)
    {
      crc = CRC32.crc32_upd(&raw_value, 1, false);
      --padding;
    }
    return crc;
  }

  inline uint32_t compute_crc(page_entity_t &entity, pCrcCalc calcFunc)
  {
    switch (entity.type)
    {
    case entity_type::Raw:
      return compute_raw_crc(entity, calcFunc);
      break;

    case entity_type::Table:
      return compute_table_crc(entity.pTable, calcFunc);
      break;
    
    case entity_type::None:
      return pad_crc(entity.size, 0U);
      break;

    default:
      abort();
      break;
    }
 }
}

/*
Calculates and returns the CRC32 value of a given page of memory
*/
uint32_t calculateCRC32(byte pageNum)
{
  page_entity_t entity = page_begin(pageNum);
  // Initial CRC calc
  uint32_t crc = compute_crc(entity, &FastCRC32::crc32);

  entity = advance(entity);
  while (entity.type!=entity_type::End)
  {
    crc = compute_crc(entity, &FastCRC32::crc32_upd /* Note that we are *updating* */);
    entity = advance(entity);
  }
  return ~pad_crc(npage_size[pageNum] - entity.size, crc);
}