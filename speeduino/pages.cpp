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
  enum struct entity_type : uint8_t { Raw, Table, None, End };

  typedef struct entity_t {
    union {
      void *pData;
      table3D *pTable;
    };
    uint16_t size;
    entity_type type;
  } entity_t;
  typedef struct
  {
    entity_t entity;
    uint16_t offset;
  } entity_address;

  #define TABLE_VALUE_SIZE(size) (size*size)
  #define TABLE_AXISX_END(size) (TABLE_VALUE_SIZE(size)+size)
  #define TABLE_AXISY_END(size) (TABLE_AXISX_END(size)+size)
  #define TABLE_SIZE(size) TABLE_AXISY_END(size)

  #define END_ADDRESS entity_address { { nullptr, 0, entity_type::End },  0 }

  #define TABLE_ADDRESS(table, offset, size) \
    (offset<TABLE_SIZE(size) ? \
      entity_address { { &table, TABLE_SIZE(size), entity_type::Table }, offset } \
      : END_ADDRESS)

  #define RAW_ADDRESS(entity, offset) \
    (offset<sizeof(entity) ? \
      entity_address { { &entity, sizeof(entity), entity_type::Raw }, offset  } \
      : END_ADDRESS)

  #define NO_ADDRESS(size, offset) { { nullptr, size, entity_type::None }, offset }

  // For some purposes a TS page is treated as a contiguous block of memory.
  // However, in Speeduino it's sometimes made up of multiple distinct and
  // non-contiguous chunks of data. This maps from the page address (number + offset)
  // to the type & position of the corresponding memory block.
  inline entity_address map_page_offset_to_memory(uint8_t pageNumber, uint16_t offset)
  {
    switch (pageNumber)
    {
      case veMapPage:
        return TABLE_ADDRESS(fuelTable, offset, 16);
        break;

      case ignMapPage: //Ignition settings page (Page 2)
        return TABLE_ADDRESS(ignitionTable, offset, 16);
        break;

      case afrMapPage: //Air/Fuel ratio target settings page
        return TABLE_ADDRESS(afrTable, offset, 16);
        break;

      case boostvvtPage: //Boost, VVT and staging maps (all 8x8)
        if (offset < 80) //New value is on the Y (TPS) axis of the boost table
        {
          return TABLE_ADDRESS(boostTable, offset, 8);
        }
        else if (offset < 160)
        {
          return TABLE_ADDRESS(vvtTable, offset-80, 8);
        }
        else  if (offset < 240)
        {
          return TABLE_ADDRESS(stagingTable, offset-160, 8);
        }
        break;

      case seqFuelPage:
        if (offset < 48) 
        {
          return TABLE_ADDRESS(trim1Table, offset, 6);
        }
        //Trim table 2
        else if (offset < 96) 
        { 
          return TABLE_ADDRESS(trim2Table, offset-48, 6);
        }
        //Trim table 3
        else if (offset < 144)
        {
          return TABLE_ADDRESS(trim3Table, offset-96, 6);
        }
        //Trim table 4
        else if (offset < 192)
        {
          return TABLE_ADDRESS(trim4Table, offset-144, 6);
        }
        //Trim table 5
        else if (offset < 240)
        {
          return TABLE_ADDRESS(trim5Table, offset-192, 6);
        }
        //Trim table 6
        else if (offset < 288)
        {
          return TABLE_ADDRESS(trim6Table, offset-240, 6);
        }
        //Trim table 7
        else if (offset < 336)
        {
          return TABLE_ADDRESS(trim7Table, offset-288, 6);
        }
        //Trim table 8
        else if (offset<384)
        {
          return TABLE_ADDRESS(trim8Table, offset-336, 6);
        }
        break;

      case fuelMap2Page:
        return TABLE_ADDRESS(fuelTable2, offset, 16);
        break;

      case wmiMapPage:
        if (offset < 80) 
        {
          return TABLE_ADDRESS(wmiTable, offset, 8);
        }
        else if (offset<160)
        {
          return NO_ADDRESS(160-80, offset-80);
        }
        else if (offset<184)
        {
          return TABLE_ADDRESS(dwellTable, offset-160, 4);
        }
        break;
      
      case ignMap2Page:
        return TABLE_ADDRESS(ignitionTable2, offset, 16);
        break;

      case veSetPage: 
        return RAW_ADDRESS(configPage2, offset);
        break;

      case ignSetPage: 
        return RAW_ADDRESS(configPage4, offset);
        break;

      case afrSetPage: 
      return RAW_ADDRESS(configPage6, offset);
        break;

      case canbusPage:  
        return RAW_ADDRESS(configPage9, offset);
        break;

      case warmupPage: 
        return RAW_ADDRESS(configPage10, offset);
        break;

      case progOutsPage: 
        return RAW_ADDRESS(configPage13, offset);
        break;      

      default:
        break;
    }
    
    return END_ADDRESS;
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
  inline table_address_t to_table_address(const entity_address &location)
  {
    if (location.offset < TABLE_VALUE_SIZE(_Size))
    {
      return { location.entity.pTable->values[OFFSET_TOVALUE_YINDEX(location.offset, _Size)] + OFFSET_TOVALUE_XINDEX(location.offset, _Size), table3D_section_t::Value };
    }
    else if (location.offset < TABLE_AXISX_END(_Size))
    {
      return { (byte*)(location.entity.pTable->axisX + OFFSET_TOAXIS_XINDEX(location.offset, _Size)), table3D_section_t::axisX };
    }
    else if (location.offset < TABLE_AXISY_END(_Size))
    {
      return { (byte*)(location.entity.pTable->axisY + OFFSET_TOAXIS_YINDEX(location.offset, _Size)), table3D_section_t::axisY };
    }
    return { nullptr, table3D_section_t::None }; 
  }

  #define TO_TABLE_ADDRESS(size, location) case size: return to_table_address<size>(location); break;

  inline table_address_t to_table_address(const entity_address &location)
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
    switch (location.entity.pTable->xSize)
    {
      TO_TABLE_ADDRESS(16, location);
      TO_TABLE_ADDRESS(8, location);
      TO_TABLE_ADDRESS(6, location);
      TO_TABLE_ADDRESS(4, location);
    }
    return { nullptr, table3D_section_t::None }; 
  }

  inline byte get_table_value(const entity_address &location)
  {
    auto table_address = to_table_address(location);
    switch (table_address.section)
    {
      case table3D_section_t::Value: 
        return *table_address.pValue;

      case table3D_section_t::axisX:
        return byte((*table_address.pAxis) / getTableXAxisFactor(location.entity.pTable)); 
      
      case table3D_section_t::axisY:
        return byte((*table_address.pAxis) / getTableYAxisFactor(location.entity.pTable)); 
      
      default: return 0; // no-op
    }

    return 0U;
  }

  inline uint8_t get_value(const entity_address &location)
  {
    switch (location.entity.type)
    {
      case entity_type::Table:
        return get_table_value(location);
        break;
  
      case entity_type::Raw:
        return *((byte*)location.entity.pData + location.offset);
        break;

      default: return 0U;
    }
    return 0U;
  }

  inline void set_table_value(const entity_address &location, int8_t value)
  {
    auto table_address = to_table_address(location);
    switch (table_address.section)
    {
      case table3D_section_t::Value: 
        *table_address.pValue = value;
        break;

      case table3D_section_t::axisX:
        *table_address.pAxis = (int)(value) * getTableXAxisFactor(location.entity.pTable); 
        break;
      
      case table3D_section_t::axisY:
        *table_address.pAxis= (int)(value) * getTableYAxisFactor(location.entity.pTable);
        break;
      
      default: ; // no-op
    }
    location.entity.pTable->cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
  }
}


/**
 * @brief Retrieves a single value from a memory page, with data aligned as per the ini file
 * 
 * @param page The page number to retrieve data from
 * @param valueAddress The address in the page that should be returned. This is as per the page definition in the ini
 * @return byte The requested value
 */
byte getPageValue(byte page, uint16_t valueAddress)
{
  return get_value(map_page_offset_to_memory(page, valueAddress));
}


void setPageValue(byte pageNum, uint16_t offset, byte value)
{
  entity_address location = map_page_offset_to_memory(pageNum, offset);

  switch (location.entity.type)
  {
  case entity_type::Table:
    set_table_value(location, value);
    break;
  
  case entity_type::Raw:
    *((byte*)location.entity.pData + location.offset) = value;
    break;
      
  default:
    break;
  }
}

namespace {
  FastCRC32 CRC32;

  inline uint32_t initialize_crc(entity_address &address)
  {
    uint8_t startValue = get_value(address);
    address.offset++;
    return CRC32.crc32(&startValue, 1, false);
  }

  inline uint32_t compute_crc_block(entity_address &address)
  {
    uint8_t buffer[128];
    uint8_t *pElement = buffer;  
    uint8_t *pEnd = buffer + min(_countof(buffer), address.entity.size-address.offset);     
    while (pElement!=pEnd)
    {
      *pElement = get_value(address);
      ++address.offset;
      ++pElement;
    }
    return CRC32.crc32_upd(buffer, pEnd-buffer, false);
  }

  inline uint32_t compute_crc(entity_address &address, uint32_t crc)
  {
    while (address.offset<address.entity.size)
    {  
      crc = compute_crc_block(address);
    }
    return crc;
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
  uint16_t totalOffset = 0;
  entity_address location = map_page_offset_to_memory(pageNum, totalOffset);
  uint32_t crc = initialize_crc(location);

  while (location.entity.type!=entity_type::End)
  {
    crc = compute_crc(location, crc);
    totalOffset += location.entity.size;
    location = map_page_offset_to_memory(pageNum, totalOffset);
  }
  return ~pad_crc(npage_size[pageNum] - totalOffset, crc);
}