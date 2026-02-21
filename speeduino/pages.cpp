#include "pages.h"
#include "globals.h"
#include "preprocessor.h"

// Maps from virtual page "addresses" to addresses/bytes of real in memory entities
//
// For TunerStudio:
// 1. Each page has a numeric identifier (0 to N-1)
// 2. A single page is a contiguous block of data.
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
constexpr const uint16_t PROGMEM ini_page_sizes[] = { 0, 128, 288, 288, 128, 288, 128, 240, 384, 192, 192, 288, 192, 128, 288, 256 };

// ========================= Table size calculations =========================
// Note that these should be computed at compile time, assuming the correct
// calling context.

template <class table_t>
static inline constexpr uint16_t get_table_value_end(void)
{
  return table_t::xaxis_t::length*table_t::yaxis_t::length;
}
template <class table_t>
static inline constexpr uint16_t get_table_axisx_end(void)
{
  return get_table_value_end<table_t>()+table_t::xaxis_t::length;
}
template <class table_t>
static inline constexpr uint16_t get_table_axisy_end(const table_t *table)
{
  UNUSED(table);
  return get_table_axisx_end<table_t>()+table_t::yaxis_t::length;
}

// ========================= Intra-table offset to byte class =========================

template<class table_t>
class offset_to_table
{
public:

  // This class encapsulates mapping a linear offset to the various parts of a table
  // and exposing the linear offset as an mutable byte.
  //
  // Tables do not map linearly to the TS page address space, so special 
  // handling is necessary (we do not use the normal array layout for
  // performance reasons elsewhere)
  //
  // We take the offset & map it to a single value, x-axis or y-axis element
  //
  // Using a template here is a performance boost - we can call functions that
  // are specialised per table type, which allows the compiler more optimisation
  // opportunities. See get_table_value().

  offset_to_table(const table_t *pTable, uint16_t table_offset)
  : _pTable(const_cast<table_t *>(pTable)), // cppcheck-suppress misra-c2012-10.4
    _table_offset(min(table_offset, get_table_axisy_end(pTable)))
  {    
  }

  // Getter
  inline byte operator*(void) const 
  { 
    switch (get_table_location())
    {
      case table_location_values:
        return get_value_value();
      case table_location_xaxis:
        return get_xaxis_value();
      case table_location_yaxis:
      default:
        return get_yaxis_value();
    }
  }

  // Setter
  inline offset_to_table &operator=( byte new_value )
  {
    switch (get_table_location())
    {
      case table_location_values:
        get_value_value() = new_value;
        break;

      case table_location_xaxis:
        get_xaxis_value() = new_value;
        break;

      case table_location_yaxis:
      default:
        get_yaxis_value() = new_value;
        break; 
    }
    invalidate_cache(&_pTable->get_value_cache);
    return *this;
  }  

private: 

  inline byte& get_value_value(void) const
  {
    return _pTable->values.value_at((uint8_t)_table_offset);
  }

  inline table3d_axis_t& get_xaxis_value(void) const
  {
    // LCOV_EXCL_BR_START
    // Can't figure out the missing branches, so exclude for the moment
    return *(_pTable->axisX.begin().advance(_table_offset - get_table_value_end<table_t>()));
    // LCOV_EXCL_BR_STOP
  }

  inline table3d_axis_t& get_yaxis_value(void) const
  {
    // LCOV_EXCL_BR_START
    // Can't figure out the missing branches, so exclude for the moment
    return *(_pTable->axisY.begin().advance(_table_offset - get_table_axisx_end<table_t>()));
    // LCOV_EXCL_BR_STOP
}

  enum table_location {
      table_location_values, table_location_xaxis, table_location_yaxis 
  };
  
  inline table_location get_table_location(void) const
  {
    if (_table_offset<get_table_value_end<table_t>())
    {
      return table_location_values;
    }
    if (_table_offset<get_table_axisx_end<table_t>())
    {
      return table_location_xaxis;
    }
    return table_location_yaxis;
  }

  table_t *_pTable;
  uint16_t _table_offset;
};

// ========================= Offset to entity byte mapping =========================

static inline byte get_raw_location(const page_iterator_t &entity, uint16_t offset)
{
  if (offset<entity.address.size)
  {
    return *((const byte*)entity.pData + offset);
  }
  return 0U;
}

static inline bool set_raw_location(page_iterator_t &entity, uint16_t offset, byte value)
{
  if (offset<entity.address.size)
  {
    *((byte*)entity.pData + offset) = value;
    return true;
  }
  return false;
}

static inline byte get_table_value(const page_iterator_t &entity, uint16_t offset)
{
  if (offset<entity.address.size)
  {
    // LCOV_EXCL_BR_START
    // Can't figure out the missing branches, so exclude for the moment
    #define CTA_GET_TABLE_VALUE(size, xDomain, yDomain, pTable, offset) \
        return *offset_to_table<TABLE3D_TYPENAME_BASE(size, xDomain, yDomain)>((const TABLE3D_TYPENAME_BASE(size, xDomain, yDomain)*)(pTable), (offset));
    #define CTA_GET_TABLE_VALUE_DEFAULT ({ return 0U; })
    CONCRETE_TABLE_ACTION(entity.table_key, CTA_GET_TABLE_VALUE, CTA_GET_TABLE_VALUE_DEFAULT, entity.pData, offset);  
    // LCOV_EXCL_BR_STOP
  }
  return 0U;
}

byte getEntityValue(const page_iterator_t &entity, uint16_t offset)
{
  if (Raw==entity.type)
  {
    return get_raw_location(entity, offset);
  }
  if (Table==entity.type)
  {
    return get_table_value(entity, offset);
  }
  // Entity has no data
  return 0U;
}

static inline bool set_table_value(page_iterator_t &entity, uint16_t offset, byte new_value)
{
  if (offset<entity.address.size)
  {
    // LCOV_EXCL_BR_START
    // Can't figure out the missing branches, so exclude for the moment
    #define CTA_SET_TABLE_VALUE(size, xDomain, yDomain, pTable, offset, new_value) \
        offset_to_table<TABLE3D_TYPENAME_BASE(size, xDomain, yDomain)>((TABLE3D_TYPENAME_BASE(size, xDomain, yDomain)*)(pTable), (offset)) = (new_value); break;
    #define CTA_SET_TABLE_VALUE_DEFAULT ({ })
    CONCRETE_TABLE_ACTION(entity.table_key, CTA_SET_TABLE_VALUE, CTA_SET_TABLE_VALUE_DEFAULT, entity.pData, offset, new_value);  
    // LCOV_EXCL_BR_STOP
    return true;
  }
  return false;
}

bool setEntityValue(page_iterator_t &entity, uint16_t offset, byte value)
{    
  if (Raw==entity.type)
  {
    return set_raw_location(entity, offset, value);
  }
  else if (Table==entity.type)
  {
    return set_table_value(entity, offset, value);
  }
  else
  {
    // Unsettable entity type 
    return false;
  }
}

// ========================= Static page size computation & checking ===================

// This will fail AND print the page number and required size
template <uint8_t pageNum, uint16_t pageSize>
static inline void check_size(void) {
  static_assert(ini_page_sizes[pageNum] == pageSize, "Size is off!");
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

// The members of all page_iterator_t instances are compile time constants and
// thus all page_iterator_t instances *could* be compile time constants. 
//
// If we declare them inline as part of return statements, gcc recognises they 
// are constants (even without constexpr). Constants need to be stored somewhere:
// gcc places them in the .data section, which is placed in SRAM :-(. 
//
// So we would end up using several hundred bytes of SRAM. 
//
// Instead we use this (and other) intermediate factory function(s) - it provides a barrier that
// forces GCC to construct the page_iterator_t instance at runtime.
static inline page_iterator_t create_end_iterator(uint8_t pageNum, uint16_t start)
{
  return page_iterator_t( End, 
                          entity_page_location_t(pageNum, UINT8_MAX),
                          entity_page_address_t(start, 0U));
}

// Signal the end of a page
#define END_OF_PAGE(pageNum, entityNum) \
  check_size<(pageNum), ENTITY_START_VAR(entityNum)>(); \
  return create_end_iterator((pageNum), ENTITY_START_VAR(entityNum)); \

// ========================= Table processing  ===================

static inline page_iterator_t create_table_iterator(void *pTable, table_type_t key, uint8_t pageNum, uint8_t index, uint16_t start, uint16_t size)
{
  return page_iterator_t( pTable, key,
                          entity_page_location_t(pageNum, index),
                          entity_page_address_t(start, size));
}

// If the offset is in range, create a Table entity_t
#define CHECK_TABLE(pageNum, offset, pTable, entityNum) \
  if (offset < ENTITY_START_VAR(entityNum)+get_table_axisy_end(pTable)) \
  { \
    return create_table_iterator(pTable, (pTable)->type_key, \
                                  (pageNum), (entityNum), \
                                  ENTITY_START_VAR(entityNum), get_table_axisy_end((pTable))); \
  } \
  DECLARE_NEXT_ENTITY_START(entityNum, get_table_axisy_end(pTable))

// ========================= Raw memory block processing  ===================

static inline page_iterator_t create_raw_iterator(void *pBuffer, uint8_t pageNum, uint8_t index, uint16_t start, uint16_t size)
{
  return page_iterator_t( pBuffer,
                          entity_page_location_t(pageNum, index),    
                          entity_page_address_t(start, size));
}

// If the offset is in range, create a Raw entity_t
#define CHECK_RAW(pageNum, offset, pDataBlock, blockSize, entityNum) \
  if (offset < ENTITY_START_VAR(entityNum)+blockSize) \
  { \
    return create_raw_iterator((pDataBlock), (pageNum), (entityNum), ENTITY_START_VAR(entityNum), (blockSize));\
  } \
  DECLARE_NEXT_ENTITY_START(entityNum, blockSize)

// ========================= Empty entity processing  ===================

static inline page_iterator_t create_empty_iterator(uint8_t pageNum, uint8_t index, uint16_t start, uint16_t size)
{
  return page_iterator_t( NoEntity,
                          entity_page_location_t(pageNum, index),    
                          entity_page_address_t(start, size));
}

// If the offset is in range, create a "no entity"
#define CHECK_NOENTITY(pageNum, offset, blockSize, entityNum) \
  if (offset < ENTITY_START_VAR(entityNum)+blockSize) \
  { \
    return create_empty_iterator((pageNum), (entityNum), ENTITY_START_VAR(entityNum), (blockSize));\
  } \
  DECLARE_NEXT_ENTITY_START(entityNum, blockSize)

// ===============================================================================

// Does the heavy lifting of mapping page+offset to an entity
//
// Alternative implementation would be to encode the mapping into data structures
// That uses flash memory, which is scarce. And it was too slow.
static page_iterator_t map_page_offset_to_entity(uint8_t pageNumber, uint16_t offset)
{
  // The start address of the 1st entity in any page.
  static constexpr uint16_t ENTITY_START_VAR(0) = 0U;

  switch (pageNumber)
  {
    default:
    case 0:
      return create_end_iterator(pageNumber, 0);

    case veMapPage:
    {
      // LCOV_EXCL_BR_START
      // The first entity on the page has a missing branch not covered
      // No idea why, so exclude froom branch coverage for the moment
      CHECK_TABLE(veMapPage, offset, &fuelTable, 0)
      // LCOV_EXCL_BR_STOP
      END_OF_PAGE(veMapPage, 1)
    }

    case ignMapPage: //Ignition settings page (Page 2)
    {
      // LCOV_EXCL_BR_START
      CHECK_TABLE(ignMapPage, offset, &ignitionTable, 0)
      // LCOV_EXCL_BR_STOP
      END_OF_PAGE(ignMapPage, 1)
    }

    case afrMapPage: //Air/Fuel ratio target settings page
    {
      // LCOV_EXCL_BR_START
      CHECK_TABLE(afrMapPage, offset, &afrTable, 0)
      // LCOV_EXCL_BR_STOP
      END_OF_PAGE(afrMapPage, 1)
    }

    case boostvvtPage: //Boost, VVT and staging maps (all 8x8)
    {
      // LCOV_EXCL_BR_START
      CHECK_TABLE(boostvvtPage, offset, &boostTable, 0)
      // LCOV_EXCL_BR_STOP
      CHECK_TABLE(boostvvtPage, offset, &vvtTable, 1)
      CHECK_TABLE(boostvvtPage, offset, &stagingTable, 2)
      END_OF_PAGE(boostvvtPage, 3)
    }

    case seqFuelPage:
    {
      // LCOV_EXCL_BR_START
      CHECK_TABLE(seqFuelPage, offset, &trim1Table, 0)
      // LCOV_EXCL_BR_STOP
      CHECK_TABLE(seqFuelPage, offset, &trim2Table, 1)
      CHECK_TABLE(seqFuelPage, offset, &trim3Table, 2)
      CHECK_TABLE(seqFuelPage, offset, &trim4Table, 3)
      CHECK_TABLE(seqFuelPage, offset, &trim5Table, 4)
      CHECK_TABLE(seqFuelPage, offset, &trim6Table, 5)
      CHECK_TABLE(seqFuelPage, offset, &trim7Table, 6)
      CHECK_TABLE(seqFuelPage, offset, &trim8Table, 7)
      END_OF_PAGE(seqFuelPage, 8)
    }

    case fuelMap2Page:
    {
      // LCOV_EXCL_BR_START
      CHECK_TABLE(fuelMap2Page, offset, &fuelTable2, 0)
      // LCOV_EXCL_BR_STOP
      END_OF_PAGE(fuelMap2Page, 1)
    }

    case wmiMapPage:
    {
      // LCOV_EXCL_BR_START
      CHECK_TABLE(wmiMapPage, offset, &wmiTable, 0)
      // LCOV_EXCL_BR_STOP
      CHECK_TABLE(wmiMapPage, offset, &vvt2Table, 1)
      CHECK_TABLE(wmiMapPage, offset, &dwellTable, 2)
      CHECK_NOENTITY(wmiMapPage, offset, 8U, 3)
      END_OF_PAGE(wmiMapPage, 4)
    }
    
    case ignMap2Page:
    {
      // LCOV_EXCL_BR_START
      CHECK_TABLE(ignMap2Page, offset, &ignitionTable2, 0)
      // LCOV_EXCL_BR_STOP
      END_OF_PAGE(ignMap2Page, 1)
    }

    case veSetPage: 
    {
      // LCOV_EXCL_BR_START
      CHECK_RAW(veSetPage, offset, &configPage2, sizeof(configPage2), 0)
      // LCOV_EXCL_BR_STOP
      END_OF_PAGE(veSetPage, 1)
    }

    case ignSetPage: 
    {
      // LCOV_EXCL_BR_START
      CHECK_RAW(ignSetPage, offset, &configPage4, sizeof(configPage4), 0)
      // LCOV_EXCL_BR_STOP
      END_OF_PAGE(ignSetPage, 1)
    }
    
    case afrSetPage: 
    {
      // LCOV_EXCL_BR_START
      CHECK_RAW(afrSetPage, offset, &configPage6, sizeof(configPage6), 0)
      // LCOV_EXCL_BR_STOP
      END_OF_PAGE(afrSetPage, 1)
    }

    case canbusPage:  
    {
      // LCOV_EXCL_BR_START
      CHECK_RAW(canbusPage, offset, &configPage9, sizeof(configPage9), 0)
      // LCOV_EXCL_BR_STOP
      END_OF_PAGE(canbusPage, 1)
    }

    case warmupPage: 
    {
      // LCOV_EXCL_BR_START
      CHECK_RAW(warmupPage, offset, &configPage10, sizeof(configPage10), 0)
      // LCOV_EXCL_BR_STOP
      END_OF_PAGE(warmupPage, 1)
    }

    case progOutsPage: 
    {
      // LCOV_EXCL_BR_START
      CHECK_RAW(progOutsPage, offset, &configPage13, sizeof(configPage13), 0)
      // LCOV_EXCL_BR_STOP
      END_OF_PAGE(progOutsPage, 1)
    }

    case boostvvtPage2: //Boost, VVT and staging maps (all 8x8)
    {
      // LCOV_EXCL_BR_START
      CHECK_TABLE(boostvvtPage2, offset, &boostTableLookupDuty, 0)
      // LCOV_EXCL_BR_STOP
      CHECK_RAW(boostvvtPage2, offset, &configPage15, sizeof(configPage15), 1)
      END_OF_PAGE(boostvvtPage2, 2)
    }
  }
}


// ====================================== External functions  ====================================

uint8_t getPageCount(void)
{
  return _countof(ini_page_sizes);
}

uint16_t getPageSize(byte pageNum)
{
  return pageNum<_countof(ini_page_sizes) ? pgm_read_word(&(ini_page_sizes[pageNum])) : 0U;
}

static inline uint16_t pageOffsetToEntityOffset(const page_iterator_t &entity, uint16_t pageOffset)
{
  return pageOffset-entity.address.start;
}

bool setPageValue(byte pageNum, uint16_t offset, byte value)
{
  page_iterator_t entity = map_page_offset_to_entity(pageNum, offset);

  return setEntityValue(entity, pageOffsetToEntityOffset(entity, offset), value);
}

byte getPageValue(byte pageNum, uint16_t offset)
{
  page_iterator_t entity = map_page_offset_to_entity(pageNum, offset);

  return getEntityValue(entity, pageOffsetToEntityOffset(entity, offset));
}

// LCOV_EXCL_START
// No need to have coverage on simple wrappers

// Support iteration over a pages entities.
page_iterator_t page_begin(byte pageNum)
{
  return map_page_offset_to_entity(pageNum, 0U);
}

page_iterator_t advance(const page_iterator_t &it)
{
    return map_page_offset_to_entity(it.location.page, it.address.start+it.address.size);
}

/**
 * Convert page iterator to table value iterator.
 */
table_value_iterator rows_begin(const page_iterator_t &it)
{
  return rows_begin(it.pData, it.table_key);
}

/**
 * Convert page iterator to table x axis iterator.
 */
table_axis_iterator x_begin(const page_iterator_t &it)
{
  return x_begin(it.pData, it.table_key);
}

/**
 * Convert page iterator to table x axis iterator.
 */
table_axis_iterator x_rbegin(const page_iterator_t &it)
{
  return x_rbegin(it.pData, it.table_key);
}

/**
 * Convert page iterator to table y axis iterator.
 */
table_axis_iterator y_begin(const page_iterator_t &it)
{
  return y_begin(it.pData, it.table_key);
}

// LCOV_EXCL_STOP