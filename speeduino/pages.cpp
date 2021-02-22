#include "pages.h"

namespace page_to_type
{
  typedef enum { Raw, Table, None } page_subtype_t;

  typedef struct
  {
    typedef struct table_t {
      table3D *pTable;
      uint16_t offset;
    } table_t;
    typedef struct raw_t {
      void *pData;
      uint16_t offset;
    } raw_t;
    union {
      table_t table;
      raw_t raw;    
    };
    page_subtype_t type;
  } entity_address;

  // For some purposes a TS page is treated as a contiguous block of memory.
  // However, in Speeduino it's sometimes made up of multiple distinct and
  // non-contiguous chunks of data. This maps from the page address (number + offset)
  // to the type & position of the corresponding memory block.
  entity_address map_page_offset_to_memory(uint8_t pageNumber, uint16_t offset)
  {
    switch (pageNumber)
    {
      case veMapPage:
        return { { &fuelTable, offset }, .type = Table };
        break;

      case ignMapPage: //Ignition settings page (Page 2)
        return { { &ignitionTable, offset }, .type = Table };
        break;

      case afrMapPage: //Air/Fuel ratio target settings page
        return { { &afrTable, offset }, .type = Table };
        break;

      case boostvvtPage: //Boost, VVT and staging maps (all 8x8)
        if (offset < 80) //New value is on the Y (TPS) axis of the boost table
        {
          return { { &boostTable, offset }, .type = Table };
        }
        else if (offset < 160)
        {
          return { { &vvtTable, offset-80 }, .type = Table };
        }
        else  if (offset < 240)
        {
          return { { &stagingTable, offset-160 }, .type = Table };
        }
        break;

      case seqFuelPage:
        if (offset < 48) 
        {
          return { { &trim1Table, offset }, .type = Table };
        }
        //Trim table 2
        else if (offset < 96) 
        { 
          return { { &trim2Table, offset-48 }, .type = Table };
        }
        //Trim table 3
        else if (offset < 144)
        {
          return { { &trim3Table, offset-96 }, .type = Table };
        }
        //Trim table 4
        else if (offset < 192)
        {
          return { { &trim4Table, offset-144 }, .type = Table };
        }
        //Trim table 5
        else if (offset < 240)
        {
          return { { &trim5Table, offset-192 }, .type = Table };
        }
        //Trim table 6
        else if (offset < 288)
        {
          return { { &trim6Table, offset-240 }, .type = Table };
        }
        //Trim table 7
        else if (offset < 336)
        {
          return { { &trim7Table, offset-288 }, .type = Table };
        }
        //Trim table 8
        else if (offset<384)
        {
          return { { &trim8Table, offset-336 }, .type = Table };
        }
        break;

      case fuelMap2Page:
        return { { &fuelTable2, offset }, .type = Table };
        break;

      case wmiMapPage:
        if (offset < 80) //New value is on the Y (MAP) axis of the wmi table
        {
          return { { &wmiTable, offset }, .type = Table };
        }
        //End of wmi table
        else if (offset>159 && offset<240)
        {
          return { { &dwellTable, offset-160 }, .type = Table };
        }
        break;
      
      case ignMap2Page: //Ignition settings page (Page 2)
        return { { &ignitionTable2, offset }, .type = Table };
        break;

      case veSetPage: 
        return { { (table3D*)&configPage2, offset }, .type = Raw };
        break;

      case ignSetPage: 
        return { { (table3D*)&configPage4, offset }, .type = Raw };
        break;

      case afrSetPage: 
      return { { (table3D*)&configPage6, offset }, .type = Raw };
        break;

      case canbusPage:  
        return { { (table3D*)&configPage9, offset }, .type = Raw };
        break;

      case warmupPage: 
        return { { (table3D*)&configPage10, offset }, .type = Raw };
        break;

      case progOutsPage: 
        return { { (table3D*)&configPage13, offset }, .type = Raw };
        break;      

      default:
        break;
    }
    
    return { { nullptr, 0 }, .type = page_subtype_t::None };
  }
}

namespace table_address
{
  typedef enum { Value, axisX, axisY, None } table3D_section_t;

  inline table3D_section_t offsetToTableSection(const page_to_type::entity_address::table_t &location)
  {
    // Precompute for a small performance gain.
    uint8_t size = location.pTable->xSize;
    uint16_t area = sq(size);
    return location.offset < area ? Value :
            location.offset <  area+size ? axisX :
              location.offset < area+size+size ? axisY :
                None;
  }

#define GET_TABLE_VALUE(size) \
  case size: return &location.pTable->values[(size-1) - (location.offset / size)][location.offset % size]

  inline byte* offsetToValue(const page_to_type::entity_address::table_t &location)
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
        GET_TABLE_VALUE(16);
        GET_TABLE_VALUE(15);
        GET_TABLE_VALUE(14);
        GET_TABLE_VALUE(13);
        GET_TABLE_VALUE(12);
        GET_TABLE_VALUE(11);
        GET_TABLE_VALUE(10);
        GET_TABLE_VALUE(9);
        GET_TABLE_VALUE(8);
        GET_TABLE_VALUE(7);
        GET_TABLE_VALUE(6);
        GET_TABLE_VALUE(5);
        GET_TABLE_VALUE(4);
        default: ;
    }
    // Default slow path
    return &location.pTable->values[(location.pTable->xSize-1) - (location.offset/location.pTable->xSize)][location.offset % location.pTable->xSize];
  }

  inline int8_t offsetToAxisXIndex(const page_to_type::entity_address::table_t &location)
  {
    return location.offset - sq(location.pTable->xSize);
  }

  inline int8_t offsetToAxisYIndex(const page_to_type::entity_address::table_t &location)
  {
    // Need to do a translation to flip the order (Due to us using (0,0) in the top left rather than bottom right
    return (location.pTable->xSize-1) - (location.offset - (sq(location.pTable->xSize) + location.pTable->xSize));
  }

  inline int8_t getTableValueFromOffset(const page_to_type::entity_address::table_t &location)
  {
    switch (offsetToTableSection(location))
    {
      case Value: 
        return *offsetToValue(location);
      
      case axisX:
        return int8_t(location.pTable->axisX[offsetToAxisXIndex(location)] / getTableXAxisFactor(location.pTable)); 
      
      case axisY:
        return int8_t(location.pTable->axisY[offsetToAxisYIndex(location)] / getTableYAxisFactor(location.pTable)); 
      
      default: ; // no-op
    }

    return 0;
  }

  inline void setTableValueFromOffset(const page_to_type::entity_address::table_t &location, int8_t value)
  {
    switch (offsetToTableSection(location))
    {
      case Value: 
        *offsetToValue(location) = value;
        break;

      case axisX:
        location.pTable->axisX[offsetToAxisXIndex(location)]  = (int)(value) * getTableXAxisFactor(location.pTable); 
        break;

      case axisY:
        location.pTable->axisY[offsetToAxisYIndex(location)] = (int)(value) * getTableYAxisFactor(location.pTable);
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
  page_to_type::entity_address location = page_to_type::map_page_offset_to_memory(page, valueAddress);

  switch (location.type)
  {
  case page_to_type::page_subtype_t::Table:
    return table_address::getTableValueFromOffset(location.table);
    break;
  
  case page_to_type::page_subtype_t::Raw:
    return *((byte*)location.raw.pData + location.raw.offset);
    break;
      
  default:
      break;
  }
  return 0;
}


void setPageValue(byte pageNum, uint16_t offset, byte value)
{
  page_to_type::entity_address location = page_to_type::map_page_offset_to_memory(pageNum, offset);

  switch (location.type)
  {
  case page_to_type::page_subtype_t::Table:
    table_address::setTableValueFromOffset(location.table, value);
    break;
  
  case page_to_type::page_subtype_t::Raw:
    if (offset < npage_size[pageNum])
    {
      *((byte*)location.raw.pData + location.raw.offset) = value;
    }
    break;
      
  default:
    break;
  }
}
