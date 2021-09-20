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

struct intra_entity_address_t {
  // The byte where the requested offset maps to
  void *pData;
  // In some cases we convert 16-bit values to 8-bit. This is the factor applied during that conversion.
  uint8_t factor; 
};

inline byte get_value(const intra_entity_address_t &address)
{
  if (address.factor==1)
  {
    return *(byte*)(address.pData);
  }
  else if (address.factor>1)
  {
    return  (*(int16_t*)(address.pData)) / address.factor;
  }

  return 0U;
}

inline void set_value(byte value, const intra_entity_address_t &address)
{
  if (address.factor==1)
  {
    *(byte*)(address.pData) = value;
  }
  else if (address.factor>1)
  {
    (*(int16_t*)(address.pData)) = value * address.factor;
  }
}

struct entity_t {
  entity_type type;
  void *pEntity; // The start of the entity
  table_type_t table_key;
  uint16_t start; // The start position of the entity, in bytes, from the start of the page
  uint16_t size;  // Size of the entity in bytes

  // The byte address that the offset mapped to.
  intra_entity_address_t address;
};

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

// ========================= Logical page end processing ===================

static const entity_t page_end_template = { 
  .type = End,
  .pEntity = nullptr, 
  .table_key = table_type_None, 
  .start = 0U,
  .size = 0U, 
  .address = { nullptr, 0U } 
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
#define TABLE_VALUE_END(size) ((uint16_t)size*(uint16_t)size)
#define TABLE_AXISX_END(size) (TABLE_VALUE_END(size)+(uint16_t)size)
#define TABLE_AXISY_END(size) (TABLE_AXISX_END(size)+(uint16_t)size)
#define TABLE_SIZE(pTable) TABLE_AXISY_END((pTable)->_metadata.axis_length)

#define OFFSET_TO_XAXIS_INDEX(offset, size) (offset - sq(size))
#define OFFSET_TO_YAXIS_INDEX(offset, size) ((size-1) - (offset - (sq(size)+size)))

// This computes the index of the logical first element of the first row
// Which isn't at values[0]
#define FIRST_ELEMENT_INDEX(size) ((uint8_t)((size*size)-size))

// Since table values aren't laid out linearily, converting a linear 
// TS offset to the equivalent memory address requires a modulus operation
// This is slow, since AVR hardware has no divider. We can gain performance
// by forcing uint8_t calculations.
//
// THIS IS WORTH 20% to 30% speed up
//
// This limits us to 16x16 tables. If we need bigger and move to 16-bit 
// operations, consider using libdivide.
#define OFFSET_TO_VALUE_INDEX(offset, size) \
  (FIRST_ELEMENT_INDEX(size)+(2*((uint8_t)offset % (uint8_t)size))-(uint8_t)offset)

#define CREATE_TABLE_ENTITY(pTable, intra_address, entityNum) \
  { .type = Table, .pEntity = pTable, .table_key = (pTable)->_metadata.type_key, \
    .start = ENTITY_START_VAR(entityNum),\
    .size = TABLE_SIZE(pTable), \
    .address = intra_address }

#define CREATE_VALUE_ADDRESS(offset, pTable, entityNum) \
  { (pTable)->values + OFFSET_TO_VALUE_INDEX((offset-ENTITY_START_VAR(entityNum)), (pTable)->_metadata.axis_length), \
     1 }
#define CREATE_XAXIS_ADDRESS(offset, pTable, entityNum) \
  { (pTable)->axisX + OFFSET_TO_XAXIS_INDEX((offset-ENTITY_START_VAR(entityNum)), (pTable)->_metadata.axis_length), \
    (pTable)->_metadata.xaxis_io_factor}
#define CREATE_YAXIS_ADDRESS(offset, pTable, entityNum) \
  { (pTable)->axisY + OFFSET_TO_YAXIS_INDEX((offset-ENTITY_START_VAR(entityNum)), (pTable)->_metadata.axis_length), \
    (pTable)->_metadata.yaxis_io_factor}

// If the offset is in range, create a Table entity_t
#define CHECK_TABLE(offset, pTable, entityNum) \
  if (offset < ENTITY_START_VAR(entityNum)+TABLE_SIZE(pTable)) \
  { \
    constexpr uint16_t axis_length = (pTable)->_metadata.axis_length; \
    constexpr uint16_t start_address = ENTITY_START_VAR(entityNum); \
    /* Limit table size to max 16 so we can force 8-bit calculations */ \
    /* for performance. See comments around OFFSET_TO_VALUE_INDEX */ \
    static_assert(axis_length<17, "Table is too big"); \
    if (offset < start_address+TABLE_VALUE_END(axis_length)) \
    { \
      return CREATE_TABLE_ENTITY(pTable, CREATE_VALUE_ADDRESS(offset, pTable, entityNum), entityNum); \
    } \
    if (offset < start_address+TABLE_AXISX_END(axis_length)) \
    { \
      return CREATE_TABLE_ENTITY(pTable, CREATE_XAXIS_ADDRESS(offset, pTable, entityNum), entityNum); \
    } \
    return CREATE_TABLE_ENTITY(pTable, CREATE_YAXIS_ADDRESS(offset, pTable, entityNum), entityNum); \
  } \
  DECLARE_NEXT_ENTITY_START(entityNum, TABLE_SIZE(pTable))

// ========================= Raw memory block processing  ===================

// If the offset is in range, create a Raw entity_t
#define CHECK_RAW(offset, pDataBlock, blockSize, entityNum) \
  if (offset < ENTITY_START_VAR(entityNum)+blockSize) \
  { \
    return  { .type = Raw, .pEntity = pDataBlock, .table_key = table_type_None, \
              .start = ENTITY_START_VAR(entityNum), \
              .size = blockSize,  \
              .address = { (byte*)pDataBlock+offset, 1 } } ; \
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
  return { .pData = mapped.pEntity,
           .table_key = mapped.table_key, 
           .page = priorIt.page, 
           .start =mapped.type==End ? priorIt.start+priorIt.size : mapped.start, 
           // Hit the end? Then size is the total actual page size (might be different than declared in the INI)
           .size = mapped.type==End ? priorIt.start+priorIt.size : mapped.size, 
           .type = mapped.type };
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
  const entity_t entity = map_page_offset_to_entity_inline(pageNum, offset);

  set_value(value, entity.address);
  if (entity.type==Table)
  {
    #define GEN_INVALIDATE_CACHE(size, xDomain, yDomain, pTable) \
        invalidate_cache(&((DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)*)pTable)->get_value_cache); \
        break;

    CONCRETE_TABLE_ACTION(entity.table_key, GEN_INVALIDATE_CACHE, entity.pEntity);      
  }    
}

byte getPageValue(byte page, uint16_t offset)
{
  return get_value(map_page_offset_to_entity_inline(page, offset).address);
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