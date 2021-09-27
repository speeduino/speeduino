#include "pages.h"
#include "globals.h"
#include "utilities.h"
#include "table3d_iterator.h"

// Maps from virtual page "addresses" to addresses/bytes of real in memory entities
//
// For TunerStudio:
// 1. Each page has a numeric identifier (0 to N-1)
// 2. A single page is a continguous block of data.
// So individual bytes are identified by a (page number, offset)
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

// ========================= Offset to Entity Mapping types =========================

typedef enum __attribute__ ((__packed__)) /* Packed is required to minimize to 8-bit */  { 
    location_raw, location_table_values, location_table_axis, location_none
} offset_location_t;

struct entity_byte_address_t {
  // The byte address that the offset mapped to.
  offset_location_t location_type;
  union {
      table_axis_iterator_t axis_iterator;
      table_row_t row_iterator;
      byte *pData;
  };  
};

struct entity_t {
  // Entity details
  page_iterator_t page_iterator;

  // The byte address that the offset mapped to.
  entity_byte_address_t entity_byte_address;
};

#define CREATE_ENTITY_T(page_it, byte_address) entity_t { .page_iterator = page_it, .entity_byte_address = byte_address }

inline byte get_value(const entity_t &entity)
{
  // Multiple 'if' statements are faster than a switch on Mega2560
  if (location_raw==entity.entity_byte_address.location_type)
  {
    return *entity.entity_byte_address.pData;
  }
  if (location_table_values==entity.entity_byte_address.location_type)
  {
    return get_value(entity.entity_byte_address.row_iterator);
  }
  if (location_table_axis==entity.entity_byte_address.location_type)
  {
    return get_value(entity.entity_byte_address.axis_iterator); 
  }
  return 0U;
}

inline void invalidate_table_cache(void *pTable, table_type_t table_key)
{
    #define GEN_INVALIDATE_CACHE(size, xDomain, yDomain, pTable) \
        invalidate_cache(&((DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)*)pTable)->get_value_cache); \
        break;

    CONCRETE_TABLE_ACTION(table_key, GEN_INVALIDATE_CACHE, pTable); 
}

inline void set_value(entity_t &entity, byte value)
{    
  // Multiple 'if' statements are faster than a switch on Mega2560
  if (location_raw==entity.entity_byte_address.location_type)
  {
    *entity.entity_byte_address.pData = value;
  }
  else if (location_table_values==entity.entity_byte_address.location_type)
  {
    set_value(entity.entity_byte_address.row_iterator, value);
    invalidate_table_cache(entity.page_iterator.pData, entity.page_iterator.table_key);
  }
  else if (location_table_axis==entity.entity_byte_address.location_type)
  {
    set_value(entity.entity_byte_address.axis_iterator, value); 
    invalidate_table_cache(entity.page_iterator.pData, entity.page_iterator.table_key);
  }
}

// ========================= Static page size computation & checking ===================

// This will fail AND print the page number and required size
template <uint8_t pageNum, uint16_t min>
static inline void check_size() {
  static_assert(ini_page_sizes[pageNum] >= min, "Size is off!");
}

// Since pages are a logical contiguous block, we can automatically compute the 
// logical start address of every item: the first one starts at zero, following
// items must start at the end of the previous.
#define _ENTITY_START(entityNum) entity ## entityNum ## Start
#define ENTITY_START_VAR(entityNum) _ENTITY_START(entityNum)
// Compute the start address of the next entity. We need this to be a constexpr
// so we can static assert on it later. So we cannot increment an exiting var.
#define DECLARE_NEXT_ENTITY_START(entityIndex, entitySize) \
  constexpr uint16_t ENTITY_START_VAR( PP_INC(entityIndex) ) = ENTITY_START_VAR(entityIndex)+entitySize;

#define PAGEOFFSET_TO_ENTITYOFFSET(offset, entityNum) (offset-ENTITY_START_VAR(entityNum))

// ========================= Logical page end processing ===================

static const entity_t page_end_template = { 
  .page_iterator = page_iterator_t { 
    .pData = nullptr,
    .table_key = table_type_None,
    .page = 0U,
    .start = 0U,
    .size = 0U,
    .type = End,
  },
  .entity_byte_address = entity_byte_address_t {
    .location_type = location_none,
    { .pData = nullptr }
  }, 
};

// Signal the end of a page
#define END_OF_PAGE(pageNum, entityNum) \
  check_size<pageNum, ENTITY_START_VAR(entityNum)>(); \
  return page_end_template; \

// ========================= Table processing  ===================

// Tables do not map linearly to the TS page address space, so special 
// handling is necessary (we do not use the normal array layout for
// performance reasons elsewhere)
//
// We take the offset & map it to a single value, x-axis or y-axis element

// Handy table macros
#define TABLE_AXIS_LENGTH(pTable) (pTable)->_metadata.axis_length
#define TABLE_VALUE_END(pTable) (sq((uint16_t)TABLE_AXIS_LENGTH(pTable)))
#define TABLE_AXISX_END(pTable) (TABLE_VALUE_END(pTable)+(uint16_t)TABLE_AXIS_LENGTH(pTable))
#define TABLE_AXISY_END(pTable) (TABLE_AXISX_END(pTable)+(uint16_t)TABLE_AXIS_LENGTH(pTable))
#define TABLE_SIZE(pTable) TABLE_AXISY_END((pTable))

#define CREATE_TABLE_PAGEITERATOR(pTable, entityNum) \
  page_iterator_t { \
    .pData = pTable, \
    .table_key = (pTable)->_metadata.type_key, \
    .page = 0U, \
    .start = ENTITY_START_VAR(entityNum), \
    .size = TABLE_SIZE(pTable), \
    .type = Table, \
  }

#define CREATE_VALUE_BYTEACCESSOR(row, col, pTable, entityNum) \
  entity_byte_address_t { \
    .location_type = location_table_values, \
    { .row_iterator = advance_intra_row(get_row(advance_row(rows_begin(pTable), row)), col) }  \
  }

#define CREATE_XAXIS_BYTEACCESSOR(offset, pTable, entityNum) \
 entity_byte_address_t { \
    .location_type = location_table_axis, \
    { .axis_iterator = advance_axis(x_begin(pTable), PAGEOFFSET_TO_ENTITYOFFSET(offset, entityNum) - TABLE_VALUE_END(pTable)) } \
 }

#define CREATE_YAXIS_BYTEACCESSOR(offset, pTable, entityNum) \
  entity_byte_address_t { \
    .location_type = location_table_axis, \
    { .axis_iterator = advance_axis(y_begin(pTable), PAGEOFFSET_TO_ENTITYOFFSET(offset, entityNum) - TABLE_AXISX_END(pTable)) } \
  }

// If the offset is in range, create a Table entity_t
//
// Since table values aren't laid out linearily, converting a linear 
// TS offset to the equivalent memory address requires a divison AND modulus 
// operations.

// This is slow, since AVR hardware has no divider. We can gain performance
// by forcing uint8_t calculations. GCC will recognize that the code below
// is doing both division & modulus on the same operands and call the
// __udivmodqi4 builtin to do both at once. 
//
// THIS IS WORTH 20% to 30% speed up
//
// This limits us to 16x16 tables. If we need bigger and move to 16-bit 
// operations, consider using libdivide.
#define CHECK_TABLE(offset, pTable, entityNum) \
  if (offset < ENTITY_START_VAR(entityNum)+TABLE_SIZE(pTable)) \
  { \
    constexpr uint16_t start_address = ENTITY_START_VAR(entityNum); \
    constexpr uint8_t axis_length = TABLE_AXIS_LENGTH(pTable); \
    static_assert(axis_length<17, "Table is too big"); \
    if (offset < start_address+TABLE_VALUE_END(pTable)) \
    { \
      const uint8_t entity_offset = PAGEOFFSET_TO_ENTITYOFFSET(offset, entityNum); \
      const uint8_t row = entity_offset / axis_length; \
      const uint8_t col = entity_offset % axis_length; \
      return CREATE_ENTITY_T(CREATE_TABLE_PAGEITERATOR(pTable, entityNum), CREATE_VALUE_BYTEACCESSOR(row, col, pTable, entityNum)); \
    } \
    if (offset < start_address+TABLE_AXISX_END(pTable)) \
    { \
      return CREATE_ENTITY_T(CREATE_TABLE_PAGEITERATOR(pTable, entityNum), CREATE_XAXIS_BYTEACCESSOR(offset, pTable, entityNum)); \
    } \
    return CREATE_ENTITY_T(CREATE_TABLE_PAGEITERATOR(pTable, entityNum), CREATE_YAXIS_BYTEACCESSOR(offset, pTable, entityNum)); \
  } \
  DECLARE_NEXT_ENTITY_START(entityNum, TABLE_SIZE(pTable))

// ========================= Raw memory block processing  ===================

#define CREATE_RAW_PAGEIT(pDataBlock, blockSize, entityNum) \
  page_iterator_t { \
    .pData = pDataBlock, \
    .table_key = table_type_None, \
    .page = 0U, \
    .start = ENTITY_START_VAR(entityNum), \
    .size = blockSize, \
    .type = Raw, \
  }

#define CREATE_RAW_BYTEADDRESS(offset, pDataBlock, entityNum) \
  entity_byte_address_t { \
    .location_type = location_raw, \
    { .pData = (byte*)pDataBlock+PAGEOFFSET_TO_ENTITYOFFSET(offset, entityNum) } \
  }

// If the offset is in range, create a Raw entity_t
#define CHECK_RAW(offset, pDataBlock, blockSize, entityNum) \
  if (offset < ENTITY_START_VAR(entityNum)+blockSize) \
  { \
    return CREATE_ENTITY_T(CREATE_RAW_PAGEIT(pDataBlock, blockSize, entityNum), CREATE_RAW_BYTEADDRESS(offset, pDataBlock, entityNum)); \
  } \
  DECLARE_NEXT_ENTITY_START(entityNum, blockSize)

// ===============================================================================

// Does the heavy lifting of mapping page+offset to an entity
//
// Alternative implementation would be to encode the mapping into data structures
// That uses flash memory, which is scarce. And it was too slow.
static inline __attribute__((always_inline)) // <-- this is critical for performance
entity_t map_page_offset_to_entity_inline(uint8_t pageNumber, uint16_t offset)
{
  // The start address of the 1st entity in any page.
  static constexpr uint16_t ENTITY_START_VAR(0) = 0U;

  switch (pageNumber)
  {
    case 0:
      END_OF_PAGE(0, 0)

    case veMapPage:
    {
      CHECK_TABLE(offset, &fuelTable, 0)
      END_OF_PAGE(veMapPage, 1)
    }

    case ignMapPage: //Ignition settings page (Page 2)
    {
      CHECK_TABLE(offset, &ignitionTable, 0)
      END_OF_PAGE(ignMapPage, 1)
    }

    case afrMapPage: //Air/Fuel ratio target settings page
    {
      CHECK_TABLE(offset, &afrTable, 0)
      END_OF_PAGE(afrMapPage, 1)
    }

    case boostvvtPage: //Boost, VVT and staging maps (all 8x8)
    {
      CHECK_TABLE(offset, &boostTable, 0)
      CHECK_TABLE(offset, &vvtTable, 1)
      CHECK_TABLE(offset, &stagingTable, 2)
      END_OF_PAGE(boostvvtPage, 3)
    }

    case seqFuelPage:
    {
      CHECK_TABLE(offset, &trim1Table, 0)
      CHECK_TABLE(offset, &trim2Table, 1)
      CHECK_TABLE(offset, &trim3Table, 2)
      CHECK_TABLE(offset, &trim4Table, 3)
      CHECK_TABLE(offset, &trim5Table, 4)
      CHECK_TABLE(offset, &trim6Table, 5)
      CHECK_TABLE(offset, &trim7Table, 6)
      CHECK_TABLE(offset, &trim8Table, 7)
      END_OF_PAGE(seqFuelPage, 8)
    }

    case fuelMap2Page:
    {
      CHECK_TABLE(offset, &fuelTable2, 0)
      END_OF_PAGE(fuelMap2Page, 1)
    }

    case wmiMapPage:
    {
      CHECK_TABLE(offset, &wmiTable, 0)
      CHECK_TABLE(offset, &vvt2Table, 1)
      CHECK_TABLE(offset, &dwellTable, 2)
      END_OF_PAGE(wmiMapPage, 3)
    }
    
    case ignMap2Page:
    {
      CHECK_TABLE(offset, &ignitionTable2, 0)
      END_OF_PAGE(ignMap2Page, 1)
    }

    case veSetPage: 
    {
      CHECK_RAW(offset, &configPage2, sizeof(configPage2), 0)
      END_OF_PAGE(veSetPage, 1)
    }

    case ignSetPage: 
    {
      CHECK_RAW(offset, &configPage4, sizeof(configPage4), 0)
      END_OF_PAGE(ignSetPage, 1)
    }
    
    case afrSetPage: 
    {
      CHECK_RAW(offset, &configPage6, sizeof(configPage6), 0)
      END_OF_PAGE(afrSetPage, 1)
    }

    case canbusPage:  
    {
      CHECK_RAW(offset, &configPage9, sizeof(configPage9), 0)
      END_OF_PAGE(canbusPage, 1)
    }

    case warmupPage: 
    {
      CHECK_RAW(offset, &configPage10, sizeof(configPage10), 0)
      END_OF_PAGE(warmupPage, 1)
    }

    case progOutsPage: 
    {
      CHECK_RAW(offset, &configPage13, sizeof(configPage13), 0)
      END_OF_PAGE(progOutsPage, 1)
    }

    default:
      abort(); // Unkown page number. Not a lot we can do.
      break;
  }
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

static inline page_iterator_t to_page_entity(const page_iterator_t &priorIt, entity_t mapped)
{
  page_iterator_t page_it = mapped.page_iterator;
  page_it.page = priorIt.page;
  page_it.start = page_it.type==End ? priorIt.start+priorIt.size : page_it.start;
  // Hit the end? Then size is the total actual page size (might be different than declared in the INI)
  page_it.size = page_it.type==End ? priorIt.start+priorIt.size : page_it.size;
  return page_it;
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

  set_value(entity, value);
}

byte getPageValue(byte pageNum, uint16_t offset)
{
  return get_value(map_page_offset_to_entity_inline(pageNum, offset));
}

// Support iteration over a pages entities.
// Check for entity.type==End
page_iterator_t page_begin(byte pageNum)
{
  page_iterator_t start_page_it = {
    .pData = nullptr,
    .table_key = table_type_None,
    .page = pageNum,
    .start = 0U,
    .size = 0U,
    .type = NoEntity
  };
  return to_page_entity(start_page_it, map_page_offset_to_entity(pageNum, 0U));
}

page_iterator_t advance(const page_iterator_t &it)
{
    return to_page_entity(it, map_page_offset_to_entity(it.page, it.start+it.size));
}

/**
 * Convert page iterator to table value iterator.
 */
table_row_iterator_t rows_begin(const page_iterator_t &it)
{
  return rows_begin(it.pData, it.table_key);
}

/**
 * Convert page iterator to table x axis iterator.
 */
table_axis_iterator_t x_begin(const page_iterator_t &it)
{
  return x_begin(it.pData, it.table_key);
}

/**
 * Convert page iterator to table y axis iterator.
 */
table_axis_iterator_t y_begin(const page_iterator_t &it)
{
  return y_begin(it.pData, it.table_key);
}