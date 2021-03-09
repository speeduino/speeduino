#include "pages.h"
#include "src/FastCRC/FastCRC.h"
#include "utilities.h"

const uint16_t npage_size[NUM_PAGES] = {0,128,288,288,128,288,128,240,384,192,192,288,192,128,288}; /**< This array stores the size (in bytes) of each configuration page */

// #define DEBUG_PRINT(x) Serial.print(x);
// #define DEBUG_PRINTLN(x) Serial.println(x);
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)

namespace 
{
  enum entity_type { Raw, Table, None, End };

  typedef struct entity_t {
    union {
      void *pData;
      table3D *pTable;
    };
    uint8_t page;
    uint16_t start;
    uint16_t size;
    entity_type type;
  } entity_t;

  #define TABLE_VALUE_SIZE(size) (size*size)
  #define TABLE_AXISX_END(size) (TABLE_VALUE_SIZE(size)+size)
  #define TABLE_AXISY_END(size) (TABLE_AXISX_END(size)+size)
  #define TABLE_SIZE(size) TABLE_AXISY_END(size)

  #define TABLE16_SIZE TABLE_SIZE(16)
  #define TABLE8_SIZE TABLE_SIZE(8)
  #define TABLE6_SIZE TABLE_SIZE(6)
  #define TABLE4_SIZE TABLE_SIZE(4)

  inline entity_t page_end_entity(byte page)
  {
    return { { nullptr }, .page = page, .start = 0, .size = 0, .type = entity_type::End };
  }

  inline entity_t table_entity(table3D *pTable, byte page, uint16_t start)
  {
      return { { pTable }, .page = page, .start = start, .size = TABLE_SIZE((uint16_t)pTable->xSize), .type = entity_type::Table };
  }
  
  inline entity_t raw_entity(void *pEntity, byte page, uint16_t start, uint16_t size)
  {
    return { { pEntity }, .page = page, .start = start, .size = size, .type = entity_type::Raw };
  }

  inline entity_t no_entity(byte page, uint16_t start, uint16_t size)
  {
    return { nullptr, .page = page, .start = start, .size = size, .type = entity_type::None };
  }

  // For some purposes a TS page is treated as a contiguous block of memory.
  // However, in Speeduino it's sometimes made up of multiple distinct and
  // non-contiguous chunks of data. This maps from the page address (number + offset)
  // to the type & position of the corresponding memory block.
  inline entity_t map_page_offset_to_entity(uint8_t pageNumber, uint16_t offset)
  {
    switch (pageNumber)
    {
      case veMapPage:
        if (offset < TABLE16_SIZE)
        {
          return table_entity(&fuelTable, veMapPage, 0);
        }
        break;

      case ignMapPage: //Ignition settings page (Page 2)
        if (offset < TABLE16_SIZE)
        {
          return table_entity(&ignitionTable, ignMapPage, 0);
        }
        break;

      case afrMapPage: //Air/Fuel ratio target settings page
        if (offset < TABLE16_SIZE)
        {
          return table_entity(&afrTable, afrMapPage, 0);
        }
        break;

      case boostvvtPage: //Boost, VVT and staging maps (all 8x8)
        if (offset < TABLE8_SIZE) //New value is on the Y (TPS) axis of the boost table
        {
          return table_entity(&boostTable, boostvvtPage, 0);
        }
        if (offset < TABLE8_SIZE*2)
        {
          return table_entity(&vvtTable, boostvvtPage, TABLE8_SIZE);
        }
        else  if (offset < TABLE8_SIZE*3)
        {
          return table_entity(&stagingTable, boostvvtPage, TABLE8_SIZE*2);
        }
        break;

      case seqFuelPage:
        if (offset < TABLE6_SIZE) 
        {
          return table_entity(&trim1Table, seqFuelPage, 0);
        }
        //Trim table 2
        if (offset < TABLE6_SIZE*2) 
        { 
          return table_entity(&trim2Table, seqFuelPage, TABLE6_SIZE);
        }
        //Trim table 3
        if (offset < TABLE6_SIZE*3)
        {
          return table_entity(&trim3Table, seqFuelPage, TABLE6_SIZE*2);
        }
        //Trim table 4
        if (offset < TABLE6_SIZE*4)
        {
          return table_entity(&trim4Table, seqFuelPage, TABLE6_SIZE*3);
        }
        //Trim table 5
        if (offset < TABLE6_SIZE*5)
        {
          return table_entity(&trim5Table, seqFuelPage, TABLE6_SIZE*4);
        }
        //Trim table 6
        if (offset < TABLE6_SIZE*6)
        {
          return table_entity(&trim6Table, seqFuelPage, TABLE6_SIZE*5);
        }
        //Trim table 7
        if (offset < TABLE6_SIZE*7)
        {
          return table_entity(&trim7Table, seqFuelPage, TABLE6_SIZE*6);
        }
        //Trim table 8
        if (offset<TABLE6_SIZE*8)
        {
          return table_entity(&trim8Table, seqFuelPage, TABLE6_SIZE*7);
        }
        break;

      case fuelMap2Page:
        if (offset < TABLE16_SIZE)
        {
          return table_entity(&fuelTable2, fuelMap2Page, 0);
        }
        break;

      case wmiMapPage:
        if (offset < TABLE8_SIZE) 
        {
          return table_entity(&wmiTable, wmiMapPage, 0);
        }
        if (offset<TABLE8_SIZE+80)
        {
          return no_entity(wmiMapPage, TABLE8_SIZE, 80);
        }
        if (offset<TABLE8_SIZE+80+TABLE4_SIZE)
        {
          return table_entity(&dwellTable, wmiMapPage, TABLE8_SIZE+80);
        }
        break;
      
      case ignMap2Page:
        if (offset < TABLE16_SIZE)
        {
          return table_entity(&ignitionTable2, ignMap2Page, 0);
        }
        break;

      case veSetPage: 
        if (offset<sizeof(configPage2))
        {
          return raw_entity(&configPage2, veSetPage, 0, sizeof(configPage2));
        }
        break;

      case ignSetPage: 
        if (offset<sizeof(configPage4))
        {
          return raw_entity(&configPage4, ignSetPage, 0, sizeof(configPage4));
        }
        break;

      case afrSetPage: 
        if (offset<sizeof(configPage6))
        {
          return raw_entity(&configPage6, afrSetPage, 0, sizeof(configPage6));
        }
        break;

      case canbusPage:  
        if (offset<sizeof(configPage9))
        {
          return raw_entity(&configPage9, canbusPage, 0, sizeof(configPage9));
        }
        break;

      case warmupPage: 
        if (offset<sizeof(configPage10))
        {
          return raw_entity(&configPage10, warmupPage, 0, sizeof(configPage10));
        }
        break;

      case progOutsPage: 
        if (offset<sizeof(configPage13))
        {
          return raw_entity(&configPage13, progOutsPage, 0, sizeof(configPage13));
        }
        break;      

      default:
        break;
    }
    
    return page_end_entity(pageNumber);
  }
}

namespace
{
  enum struct table3D_section_t : uint8_t{ Value, axisX, axisY, None } ;

  typedef struct table_address_t {
    union {
      byte *pValue;
      int16_t *pAxis;
    };
    table3D_section_t section;
  } table_address_t;

  #define OFFSET_TOVALUE_YINDEX(offset, size) ((size-1) - (offset / size))
  #define OFFSET_TOVALUE_XINDEX(offset, size) (offset % size)
  #define OFFSET_TOAXIS_XINDEX(offset, size) (offset - (size*size))
  #define OFFSET_TOAXIS_YINDEX(offset, size) ((size-1) - (offset - ((size*size) + size)))

  template <int8_t _Size>
  inline table_address_t to_table_address(const table3D *pTable, uint16_t offset)
  {
    if (offset < TABLE_VALUE_SIZE(_Size))
    {
      return { pTable->values[OFFSET_TOVALUE_YINDEX(offset, _Size)] + OFFSET_TOVALUE_XINDEX(offset, _Size), table3D_section_t::Value };
    }
    else if (offset < TABLE_AXISX_END(_Size))
    {
      return { (byte*)(pTable->axisX + OFFSET_TOAXIS_XINDEX(offset, _Size)), table3D_section_t::axisX };
    }
    else if (offset < TABLE_AXISY_END(_Size))
    {
      return { (byte*)(pTable->axisY + OFFSET_TOAXIS_YINDEX(offset, _Size)), table3D_section_t::axisY };
    }
    return { nullptr, table3D_section_t::None }; 
  }

  #define TO_TABLE_ADDRESS(size, pTable, offset) case size: return to_table_address<size>(pTable, offset); break;

  inline table_address_t to_table_address(const table3D *pTable, uint16_t offset)
  {
    // You might be tempted to remove the switch: DON'T
    //
    // Some processors do not have division hardware. E.g. ATMega2560.
    //
    // So in the general case, division is done in software. I.e. the compiler
    // emits code to perform the division. 
    //
    // However, by using compile time constants the compiler can do a ton of
    // optimization of the division and modulus operations below
    // (likely converting them to multiply & shift operations).
    //
    // So this is a massive performance win (2x to 3x).
    switch (pTable->xSize)
    {
      TO_TABLE_ADDRESS(16, pTable, offset);
      TO_TABLE_ADDRESS(8, pTable, offset);
      TO_TABLE_ADDRESS(6, pTable, offset);
      TO_TABLE_ADDRESS(4, pTable, offset);
    }
    return { nullptr, table3D_section_t::None }; 
  }

  inline byte get_table_value(const table3D *pTable, uint16_t offset)
  {
    auto table_address = to_table_address(pTable, offset);
    switch (table_address.section)
    {
      case table3D_section_t::Value: 
        return *table_address.pValue;

      case table3D_section_t::axisX:
        return byte((*table_address.pAxis) / getTableXAxisFactor(pTable)); 
      
      case table3D_section_t::axisY:
        return byte((*table_address.pAxis) / getTableYAxisFactor(pTable)); 
      
      default: return 0; // no-op
    }

    return 0U;
  }

  inline byte* get_raw_value(const entity_t &entity, uint16_t offset)
  {
    return (byte*)entity.pData + offset;
  }

  inline uint8_t get_value(const entity_t &entity, uint16_t offset)
  {
    switch (entity.type)
    {
      case entity_type::Table:
        return get_table_value(entity.pTable, offset);
        break;
  
      case entity_type::Raw:
        return *get_raw_value(entity, offset);
        break;

      default: return 0U;
    }
    return 0U;
  }

  inline void set_table_value(table3D *pTable, uint16_t offset, int8_t value)
  {
    auto table_address = to_table_address(pTable, offset);
    switch (table_address.section)
    {
      case table3D_section_t::Value: 
        *table_address.pValue = value;
        break;

      case table3D_section_t::axisX:
        *table_address.pAxis = (int)(value) * getTableXAxisFactor(pTable); 
        break;
      
      case table3D_section_t::axisY:
        *table_address.pAxis= (int)(value) * getTableYAxisFactor(pTable);
        break;
      
      default: ; // no-op
    }
    pTable->cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
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
  entity_t address = map_page_offset_to_entity(page, offset);
  return get_value(address, offset-address.start);
}


void setPageValue(byte pageNum, uint16_t offset, byte value)
{
  entity_t location = map_page_offset_to_entity(pageNum, offset);

  switch (location.type)
  {
  case entity_type::Table:
    set_table_value(location.pTable, offset-location.start, value);
    break;
  
  case entity_type::Raw:
    *get_raw_value(location, offset-location.start) = value;
    break;
      
  default:
    break;
  }
}

namespace {
  FastCRC32 CRC32;

  inline uint16_t copy_to(entity_t &entity, uint16_t &offset, byte *pStart, byte*pEnd)
  {
    uint16_t num_bytes = min((uint16_t)(pEnd-pStart), entity.size-offset);
    pEnd = pStart + num_bytes;
    while (pStart!=pEnd)
    {
      *pStart = get_value(entity, offset);
      ++offset;
      ++pStart;
    }
    return num_bytes;
  }

  inline uint32_t compute_raw_crc(entity_t &entity)
  {
    return CRC32.crc32((uint8_t*)entity.pData, entity.size, false);
  }

  inline uint32_t update_raw_crc(entity_t &entity)
  {
    return CRC32.crc32_upd((uint8_t*)entity.pData, entity.size, false);
  }

  inline uint32_t compute_crc_block(entity_t &entity, uint16_t &offset)
  {
    uint8_t buffer[128];
    return CRC32.crc32(buffer, copy_to(entity, offset, buffer, buffer+_countof(buffer)), false);
  }

  inline uint32_t update_crc_block(entity_t &entity, uint16_t &offset)
  {
    uint8_t buffer[128];
    return CRC32.crc32_upd(buffer, copy_to(entity, offset, buffer, buffer+_countof(buffer)), false);
  }

  inline uint32_t compute_crc(entity_t &entity)
  {
    if (entity.type==entity_type::Raw)
    {
      return compute_raw_crc(entity);
    }
    else
    {
      uint16_t offset = 0;
      uint32_t crc = compute_crc_block(entity, offset);
      while (offset<entity.size)
      {  
        crc = update_crc_block(entity, offset);
      }
      return crc;
    }
  }
  
  inline uint32_t update_crc(entity_t &entity)
  {
    if (entity.type==entity_type::Raw)
    {
      return update_raw_crc(entity);
    }
    else
    {
      uint16_t offset = 0;
      uint32_t crc = update_crc_block(entity, offset);
      while (offset<entity.size)
      {  
        crc = update_crc_block(entity, offset);
      }
      return crc;
    }
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
}
/*
Calculates and returns the CRC32 value of a given page of memory
*/
uint32_t calculateCRC32(byte pageNum)
{
  entity_t location = map_page_offset_to_entity(pageNum, 0);
  uint32_t crc = compute_crc(location);

  uint16_t pageOffset = location.size;
  location = map_page_offset_to_entity(pageNum, pageOffset);
  while (location.type!=entity_type::End)
  {
    crc = update_crc(location);
    pageOffset += location.size;
    location = map_page_offset_to_entity(pageNum, pageOffset);
  }
  return ~pad_crc(npage_size[pageNum] - pageOffset, crc);
}