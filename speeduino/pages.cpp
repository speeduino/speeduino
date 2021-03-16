#include "pages.h"
#include "src/FastCRC/FastCRC.h"
#include "globals.h"

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
  // Type of entity the offset mapped to
  enum struct entity_type : uint8_t { 
    Raw,    // A block of memory
    Table,  // A 3D table
    None,   // No entity, but a valid offset
    End     // The offset was past any known entity for the page
  };

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

  typedef struct entity_t {
    // The entity that the offset mapped to
    union {
      table_entity_t table;
      void *pData;
    };
    uint8_t page;   // The page the entity belongs to
    uint16_t start; // The start position of the entity, in bytes, from the start of the page
    uint16_t size;  // Size of the entity in bytes
    entity_type type;
  } entity_t;

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
  entity_t map_page_offset_to_entity(uint8_t pageNumber, uint16_t offset)
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
  inline int8_t getTableYAxisFactor(const table3D *pTable)
  {
    return pTable==&boostTable || pTable==&vvtTable ? 1 : TABLE_LOAD_MULTIPLIER;
  }

  inline byte get_table_value(const table_entity_t &table)
  {
    switch (table.section)
    {
      case table3D_section_t::Value: 
        return table.pTable->values[table.yIndex][table.xIndex];

      case table3D_section_t::axisX:
        return (byte)(table.pTable->axisX[table.xIndex] / TABLE_RPM_MULTIPLIER); 
      
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
        table.pTable->axisX[table.xIndex] = (int16_t)(value) * TABLE_RPM_MULTIPLIER; 
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
  entity_t entity = map_page_offset_to_entity(pageNum, offset);

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
  entity_t entity = map_page_offset_to_entity(page, offset);

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
    
    FastCRC32 CRC32;

}

/*
Calculates and returns the CRC32 value of a given page of memory
*/
uint32_t calculateCRC32(byte pageNo)
{
  uint32_t CRC32_val;
  byte raw_value;
  void* pnt_configPage;

  //This sucks (again) for all the 3D map pages that have to have a translation performed
  switch(pageNo)
  {
    case veMapPage:
      //Confirmed working
      raw_value = getPageValue(veMapPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[veMapPage]; x++)
      //for(uint16_t x=1; x< 288; x++)
      {
        raw_value = getPageValue(veMapPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    case veSetPage:
      //Confirmed working
      pnt_configPage = &configPage2; //Create a pointer to Page 1 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage2) );
      break;

    case ignMapPage:
      //Confirmed working
      raw_value = getPageValue(ignMapPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[ignMapPage]; x++)
      {
        raw_value = getPageValue(ignMapPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    case ignSetPage:
      //Confirmed working
      pnt_configPage = &configPage4; //Create a pointer to Page 4 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage4) );
      break;

    case afrMapPage:
      //Confirmed working
      raw_value = getPageValue(afrMapPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[afrMapPage]; x++)
      {
        raw_value = getPageValue(afrMapPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    case afrSetPage:
      //Confirmed working
      pnt_configPage = &configPage6; //Create a pointer to Page 4 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage6) );
      break;

    case boostvvtPage:
      //Confirmed working
      raw_value = getPageValue(boostvvtPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[boostvvtPage]; x++)
      {
        raw_value = getPageValue(boostvvtPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    case seqFuelPage:
      //Confirmed working
      raw_value = getPageValue(seqFuelPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[seqFuelPage]; x++)
      {
        raw_value = getPageValue(seqFuelPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    case canbusPage:
      //Confirmed working
      pnt_configPage = &configPage9; //Create a pointer to Page 9 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage9) );
      break;

    case warmupPage:
      //Confirmed working
      pnt_configPage = &configPage10; //Create a pointer to Page 10 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage10) );
      break;

    case fuelMap2Page:
      //Confirmed working
      raw_value = getPageValue(fuelMap2Page, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[fuelMap2Page]; x++)
      //for(uint16_t x=1; x< 288; x++)
      {
        raw_value = getPageValue(fuelMap2Page, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    case wmiMapPage:
      //Confirmed working
      raw_value = getPageValue(wmiMapPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[wmiMapPage]; x++)
      {
        raw_value = getPageValue(wmiMapPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;
      
    case progOutsPage:
      //Confirmed working
      pnt_configPage = &configPage13; //Create a pointer to Page 10 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage13) );
      break;
    
    case ignMap2Page:
      //Confirmed working
      raw_value = getPageValue(ignMap2Page, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[ignMap2Page]; x++)
      {
        raw_value = getPageValue(ignMap2Page, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    default:
      CRC32_val = 0;
      break;
  }
  
  return CRC32_val;
}
