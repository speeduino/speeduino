#include "pages.h"
#include "src/FastCRC/FastCRC.h"

const uint16_t npage_size[NUM_PAGES] = {0,128,288,288,128,288,128,240,384,192,192,288,192,128,288}; /**< This array stores the size (in bytes) of each configuration page */

namespace 
{
  enum struct page_subtype_t : uint8_t { Raw, Table, None };

  typedef struct
  {
    union {
      void *pData;
      table3D *pTable;
    };
    uint16_t offset;
    page_subtype_t type;
  } entity_address;

  #define TABLE_ADDRESS(table, offset) \
    { { &table }, offset, .type = page_subtype_t::Table }

  #define RAW_ADDRESS(entity, offset) \
    { { &entity }, offset, .type = page_subtype_t::Raw }

  #define NO_ADDRESS { { nullptr }, 0, .type = page_subtype_t::None }
    
  #define TABLE_VALUE_SIZE(size) (size*size)
  #define TABLE_AXISX_END(size) (TABLE_VALUE_SIZE(size)+size)
  #define TABLE_AXISY_END(size) (TABLE_AXISX_END(size)+size)
  #define TABLE_SIZE(size) TABLE_AXISY_END(size)

  // For some purposes a TS page is treated as a contiguous block of memory.
  // However, in Speeduino it's sometimes made up of multiple distinct and
  // non-contiguous chunks of data. This maps from the page address (number + offset)
  // to the type & position of the corresponding memory block.
  inline entity_address map_page_offset_to_memory(uint8_t pageNumber, uint16_t offset)
  {
    switch (pageNumber)
    {
      case veMapPage:
        return TABLE_ADDRESS(fuelTable, offset);
        break;

      case ignMapPage: //Ignition settings page (Page 2)
        return TABLE_ADDRESS(ignitionTable, offset);
        break;

      case afrMapPage: //Air/Fuel ratio target settings page
        return TABLE_ADDRESS(afrTable, offset);
        break;

      case boostvvtPage: //Boost, VVT and staging maps (all 8x8)
        if (offset < 80) //New value is on the Y (TPS) axis of the boost table
        {
          return TABLE_ADDRESS(boostTable, offset);
        }
        else if (offset < 160)
        {
          return TABLE_ADDRESS(vvtTable, offset-80);
        }
        else  if (offset < 240)
        {
          return TABLE_ADDRESS(stagingTable, offset-160);
        }
        break;

      case seqFuelPage:
        if (offset < 48) 
        {
          return TABLE_ADDRESS(trim1Table, offset);
        }
        //Trim table 2
        else if (offset < 96) 
        { 
          return TABLE_ADDRESS(trim2Table, offset-48);
        }
        //Trim table 3
        else if (offset < 144)
        {
          return TABLE_ADDRESS(trim3Table, offset-96);
        }
        //Trim table 4
        else if (offset < 192)
        {
          return TABLE_ADDRESS(trim4Table, offset-144);
        }
        //Trim table 5
        else if (offset < 240)
        {
          return TABLE_ADDRESS(trim5Table, offset-192);
        }
        //Trim table 6
        else if (offset < 288)
        {
          return TABLE_ADDRESS(trim6Table, offset-240);
        }
        //Trim table 7
        else if (offset < 336)
        {
          return TABLE_ADDRESS(trim7Table, offset-288);
        }
        //Trim table 8
        else if (offset<384)
        {
          return TABLE_ADDRESS(trim8Table, offset-336);
        }
        break;

      case fuelMap2Page:
        return TABLE_ADDRESS(fuelTable2, offset);
        break;

      case wmiMapPage:
        if (offset < 80) //New value is on the Y (MAP) axis of the wmi table
        {
          return TABLE_ADDRESS(wmiTable, offset);
        }
        else if (offset<160)
        {
          return NO_ADDRESS;
        }
        //End of wmi table
        else if (offset<184)
        {
          return TABLE_ADDRESS(dwellTable, offset-160);
        }
        break;
      
      case ignMap2Page:
        return TABLE_ADDRESS(ignitionTable2, offset);
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
    
    return NO_ADDRESS;
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
      return { location.pTable->values[OFFSET_TOVALUE_YINDEX(location.offset, _Size)] + OFFSET_TOVALUE_XINDEX(location.offset, _Size), table3D_section_t::Value };
    }
    else if (location.offset < TABLE_AXISX_END(_Size))
    {
      return { (byte*)(location.pTable->axisX + OFFSET_TOAXIS_XINDEX(location.offset, _Size)), table3D_section_t::axisX };
    }
    else if (location.offset < TABLE_AXISY_END(_Size))
    {
      return { (byte*)(location.pTable->axisY + OFFSET_TOAXIS_YINDEX(location.offset, _Size)), table3D_section_t::axisY };
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
    switch (location.pTable->xSize)
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
        return byte((*table_address.pAxis) / getTableXAxisFactor(location.pTable)); 
      
      case table3D_section_t::axisY:
        return byte((*table_address.pAxis) / getTableYAxisFactor(location.pTable)); 
      
      default: return 0; // no-op
    }

    return 0;
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
        *table_address.pAxis = (int)(value) * getTableXAxisFactor(location.pTable); 
        break;
      
      case table3D_section_t::axisY:
        *table_address.pAxis= (int)(value) * getTableYAxisFactor(location.pTable);
        break;
      
      default: ; // no-op
    }
    location.pTable->cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
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
  entity_address location = map_page_offset_to_memory(page, valueAddress);

  switch (location.type)
  {
  case page_subtype_t::Table:
    return get_table_value(location);
    break;
  
  case page_subtype_t::Raw:
    return *((byte*)location.pData + location.offset);
    break;
      
  default:
      return 0;
      break;
  }
  return 0;
}


void setPageValue(byte pageNum, uint16_t offset, byte value)
{
  entity_address location = map_page_offset_to_memory(pageNum, offset);

  switch (location.type)
  {
  case page_subtype_t::Table:
    set_table_value(location, value);
    break;
  
  case page_subtype_t::Raw:
    if (offset < npage_size[pageNum])
    {
      *((byte*)location.pData + location.offset) = value;
    }
    break;
      
  default:
    break;
  }
}

namespace {
  FastCRC32 CRC32;
}
/*
Calculates and returns the CRC32 value of a given page of memory
*/
uint32_t calculateCRC32(byte pageNum)
{
  byte raw_value = getPageValue(pageNum, 0);
  uint32_t CRC32_val = CRC32.crc32(&raw_value, 1, false);
  for(uint16_t x=1; x< npage_size[pageNum]; x++)
  {
    raw_value = getPageValue(pageNum, x);
    CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
  }
  //Do a manual reflection of the CRC32 value
  CRC32_val = ~CRC32_val;  
  return CRC32_val; 
}
