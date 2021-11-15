#include "pages.h"
#include "globals.h"
#include "utilities.h"
#include "table_iterator.h"

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

// Page sizes as defined in the .ini file
constexpr const uint16_t PROGMEM ini_page_sizes[] = { 0, 128, 288, 288, 128, 288, 128, 240, 384, 192, 192, 288, 192, 128, 288 };

// What section of a 3D table the offset mapped to
enum table3D_section_t { 
  Value,  // The values
  axisX,  // X axis
  axisY,  // Y axis
  TableSectionNone    // Should never happen!
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

// This will fail AND print the page number and required size
template <uint8_t pageNum, uint16_t min>
static inline void check_size() {
  static_assert(ini_page_sizes[pageNum] >= min, "Size is off!");
}

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
  { nullptr, 0, 0, TableSectionNone }

#define CREATE_PAGE_END(pageNum, pageSize) \
  { NULL_TABLE, .page = pageNum, .start = 0, .size = pageSize, .type = End }

// Signal the end of a page
#define END_OF_PAGE(pageNum, pageSize) \
  check_size<pageNum, pageSize>(); \
  return CREATE_PAGE_END(pageNum, pageSize);

// If the offset is in range, create a None entity_t
#define CHECK_NOENTITY(offset, startByte, blockSize, pageNum) \
  if (offset < (startByte)+blockSize) \
  { \
    return { NULL_TABLE, .page = pageNum, .start = (startByte), .size = blockSize, .type = NoEntity }; \
  } 

// 
#define TABLE_VALUE(offset, startByte, pTable, tableSize) \
  { pTable, \
    OFFSET_TOVALUE_XINDEX((offset-(startByte)), tableSize), \
    OFFSET_TOVALUE_YINDEX((offset-(startByte)), tableSize), \
    Value }

#define TABLE_XAXIS(offset, startByte, pTable, tableSize) \
  { pTable, \
    OFFSET_TOAXIS_XINDEX((offset-(startByte)), tableSize), \
    0U, \
    axisX }

#define TABLE_YAXIS(offset, startByte, pTable, tableSize) \
  { pTable, \
    0U, \
    OFFSET_TOAXIS_YINDEX((offset-(startByte)), tableSize), \
    axisY }

#define TABLE_ENTITY(table_type, pageNum, startByte, tableSize) \
  { table_type, .page = pageNum, .start = (startByte), .size = TABLE_SIZE(tableSize), .type = Table }

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
    return { { (table3D*)pDataBlock, 0, 0, TableSectionNone }, .page = pageNum, .start = (startByte), .size = blockSize, .type = Raw }; \
  } 

// Does the heavy lifting of mapping page+offset to an entity
//
// Alternative implementation would be to encode the mapping into data structures
// That uses flash memory, which is scarce. And it was too slow.
static inline __attribute__((always_inline)) // <-- this is critical for performance
entity_t map_page_offset_to_entity_inline(uint8_t pageNumber, uint16_t offset)
{
  switch (pageNumber)
  {
    case 0:
      return CREATE_PAGE_END(0, 0);

    case veMapPage:
      CHECK_TABLE(offset, 0U, &fuelTable, 16, pageNumber)
      END_OF_PAGE(veMapPage, TABLE16_SIZE);
      break;

    case ignMapPage: //Ignition settings page (Page 2)
      CHECK_TABLE(offset, 0U, &ignitionTable, 16, pageNumber)
      END_OF_PAGE(ignMapPage, TABLE16_SIZE);
      break;

    case afrMapPage: //Air/Fuel ratio target settings page
      CHECK_TABLE(offset, 0U, &afrTable, 16, pageNumber)
      END_OF_PAGE(afrMapPage, TABLE16_SIZE);
      break;

    case boostvvtPage: //Boost, VVT and staging maps (all 8x8)
      CHECK_TABLE(offset, 0U, &boostTable, 8, pageNumber)
      CHECK_TABLE(offset, TABLE8_SIZE, &vvtTable, 8, pageNumber)
      CHECK_TABLE(offset, TABLE8_SIZE*2, &stagingTable, 8, pageNumber)
      END_OF_PAGE(boostvvtPage, TABLE8_SIZE*3);
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
      END_OF_PAGE(seqFuelPage, TABLE6_SIZE*8);
      break;

    case fuelMap2Page:
      CHECK_TABLE(offset, 0U, &fuelTable2, 16, pageNumber)
      END_OF_PAGE(fuelMap2Page, TABLE16_SIZE);
      break;

    case wmiMapPage:
      CHECK_TABLE(offset, 0U, &wmiTable, 8, pageNumber)
      CHECK_TABLE(offset, TABLE8_SIZE, &vvt2Table, 8, pageNumber)
      CHECK_TABLE(offset, TABLE8_SIZE*2, &dwellTable, 4, pageNumber)
      END_OF_PAGE(wmiMapPage, TABLE8_SIZE*2 + TABLE4_SIZE);
      break;
    
    case ignMap2Page:
      CHECK_TABLE(offset, 0U, &ignitionTable2, 16, pageNumber)
      END_OF_PAGE(ignMap2Page, TABLE16_SIZE);
      break;

    case veSetPage: 
      CHECK_RAW(offset, 0U, &configPage2, sizeof(configPage2), pageNumber)
      END_OF_PAGE(veSetPage, sizeof(configPage2));
      break;

    case ignSetPage: 
      CHECK_RAW(offset, 0U, &configPage4, sizeof(configPage4), pageNumber)
      END_OF_PAGE(ignSetPage, sizeof(configPage4));
      break;

    case afrSetPage: 
      CHECK_RAW(offset, 0U, &configPage6, sizeof(configPage6), pageNumber)
      END_OF_PAGE(afrSetPage, sizeof(configPage6));
      break;

    case canbusPage:  
      CHECK_RAW(offset, 0U, &configPage9, sizeof(configPage9), pageNumber)
      END_OF_PAGE(canbusPage, sizeof(configPage9));
      break;

    case warmupPage: 
      CHECK_RAW(offset, 0U, &configPage10, sizeof(configPage10), pageNumber)
      END_OF_PAGE(warmupPage, sizeof(configPage10));
      break;

    case progOutsPage: 
      CHECK_RAW(offset, 0U, &configPage13, sizeof(configPage13), pageNumber)
      END_OF_PAGE(progOutsPage, sizeof(configPage13));
      break;      

    default:
      abort(); // Unkown page number. Not a lot  we can do.
      break;
  }
}


// Tables do not map linearly to the TS page address space, so special 
// handling is necessary (we do not use the normal array layout for
// performance reasons elsewhere)
//
// We take the offset & map it to a single value, x-axis or y-axis element
static inline byte get_table_value(const table_entity_t &table)
{
  switch (table.section)
  {
    case Value: 
      return table.pTable->values[table.yIndex][table.xIndex];

    case axisX:
      return (byte)(table.pTable->axisX[table.xIndex] / getTableXAxisFactor(table.pTable)); 
    
    case axisY:
      return (byte)(table.pTable->axisY[table.yIndex] / getTableYAxisFactor(table.pTable)); 
    
    default: return 0; // no-op
  }
  return 0U;
}

static inline void set_table_value(const table_entity_t &table, int8_t value)
{
  switch (table.section)
  {
    case Value: 
      table.pTable->values[table.yIndex][table.xIndex] = value;
      break;

    case axisX:
      table.pTable->axisX[table.xIndex] = (int16_t)(value) * getTableXAxisFactor(table.pTable); 
      break;
    
    case axisY:
      table.pTable->axisY[table.yIndex]= (int16_t)(value) * getTableYAxisFactor(table.pTable);
      break;
    
    default: ; // no-op
  }    
  table.pTable->cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
}

static inline byte* get_raw_value(const entity_t &entity, uint16_t offset)
{
  return (byte*)entity.pData + offset;
}

// ============================ Page iteration support ======================

// Because the page iterators will not be called for every single byte
// inlining the mapping function is not performance critical.
//
// So save some memory.
static entity_t map_page_offset_to_entity(uint8_t pageNumber, uint16_t offset)
{
  return map_page_offset_to_entity_inline(pageNumber, offset);
}

static inline page_iterator_t to_page_entity(entity_t mapped)
{
  return { { mapped.type==Table ? mapped.table.pTable : (table3D*)mapped.pData }, 
          .page=mapped.page, .start = mapped.start, .size = mapped.size, .type = mapped.type };
}

// ====================================== External functions  ====================================

uint8_t getPageCount()
{
  return _countof(ini_page_sizes);
}

uint16_t getPageSize(byte pageNum)
{
  return pgm_read_word(&(ini_page_sizes[pageNum]));
}

void setPageValue(byte pageNum, uint16_t offset, byte value)
{
  entity_t entity = map_page_offset_to_entity_inline(pageNum, offset);

  switch (entity.type)
  {
  case Table:
    set_table_value(entity.table, value);
    break;
  
  case Raw:
    *get_raw_value(entity, offset-entity.start) = value;
    break;
      
  default:
    break;
  }
}

byte getPageValue(byte page, uint16_t offset)
{
  entity_t entity = map_page_offset_to_entity_inline(page, offset);

  switch (entity.type)
  {
    case Table:
      return get_table_value(entity.table);
      break;

    case Raw:
      return *get_raw_value(entity, offset);
      break;

    default: return 0U;
  }
  return 0U;
}

// Support iteration over a pages entities.
// Check for entity.type==End
page_iterator_t page_begin(byte pageNum)
{
  return to_page_entity(map_page_offset_to_entity(pageNum, 0U));
}

page_iterator_t advance(const page_iterator_t &it)
{
    return to_page_entity(map_page_offset_to_entity(it.page, it.start+it.size));
}