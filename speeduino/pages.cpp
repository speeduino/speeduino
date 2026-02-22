#include "pages.h"
#include "globals.h"
#include "preprocessor.h"

#if defined(CORE_AVR)
#pragma GCC push_options
// This minimizes RAM usage at no performance cost
#pragma GCC optimize ("Os") 
#endif

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

// ========================= Offset to entity support  ===================

void nextEntity(page_iterator_t &entity, uint16_t nextBlockSize)
{
  ++entity.location.index;
  entity.address = entity.address.next(nextBlockSize);
}

// ========================= Table processing  ===================

template <class table_t>
static void checkIsInTable(page_iterator_t &result, table_t *pTable, uint16_t offset)
{
  if (result.type==End)
  {
    nextEntity(result, get_table_axisy_end(pTable));
    if (result.address.isOffsetInEntity(offset)) 
    { 
      result.setTable(pTable, pTable->type_key);
    }
  }
}

// ========================= Raw memory block processing  ===================

static void checkIsInRaw(page_iterator_t &result, void *pEntity, uint16_t entitySize, uint16_t offset)
{
  if (result.type==End)
  {
    nextEntity(result, entitySize);
    if (result.address.isOffsetInEntity(offset)) 
    { 
      result.setRaw(pEntity);
    }
  }
}

// ========================= Empty entity processing  ===================

static void checkIsInEmpty(page_iterator_t &result, uint16_t entitySize, uint16_t offset)
{
  if (result.type==End)
  {
    nextEntity(result, entitySize);
    if (result.address.isOffsetInEntity(offset)) 
    { 
      result.setNoEntity();
    }
  }
}

// ===============================================================================

// Does the heavy lifting of mapping page+offset to an entity
//
// Alternative implementation would be to encode the mapping into data structures
// That uses flash memory, which is scarce. And it was too slow.
static page_iterator_t map_page_offset_to_entity(uint8_t pageNumber, uint16_t offset)
{
  // This is mutated by the checkIsIn* functions to return the entity that matches the offset
  page_iterator_t result( End, // Signal that no entity has been found yet
                          entity_page_location_t(pageNumber, (uint8_t)-1 /* Deliberate, so we can increment index AND address as one operation */), 
                          entity_page_address_t(0U, 0U));

  switch (pageNumber)
  {
    case veMapPage:
      // LCOV_EXCL_BR_START
      // The first entity on the page has a missing branch not covered
      // No idea why, so exclude from branch coverage for the moment
      checkIsInTable(result, &fuelTable, offset);
      // LCOV_EXCL_BR_STOP
      break;

    case ignMapPage: //Ignition settings page (Page 2)
      // LCOV_EXCL_BR_START
      checkIsInTable(result, &ignitionTable, offset);
      // LCOV_EXCL_BR_STOP
      break;

    case afrMapPage: //Air/Fuel ratio target settings page
      // LCOV_EXCL_BR_START
      checkIsInTable(result, &afrTable, offset);
      // LCOV_EXCL_BR_STOP
      break;

    case boostvvtPage: //Boost, VVT and staging maps (all 8x8)
      // LCOV_EXCL_BR_START
      checkIsInTable(result, &boostTable, offset);
      // LCOV_EXCL_BR_STOP
      checkIsInTable(result, &vvtTable, offset);
      checkIsInTable(result, &stagingTable, offset);
      break;

    case seqFuelPage:
      // LCOV_EXCL_BR_START
      checkIsInTable(result, &trim1Table, offset);
      // LCOV_EXCL_BR_STOP
      checkIsInTable(result, &trim2Table, offset);
      checkIsInTable(result, &trim3Table, offset);
      checkIsInTable(result, &trim4Table, offset);
      checkIsInTable(result, &trim5Table, offset);
      checkIsInTable(result, &trim6Table, offset);
      checkIsInTable(result, &trim7Table, offset);
      checkIsInTable(result, &trim8Table, offset);
      break;

    case fuelMap2Page:
      // LCOV_EXCL_BR_START
      checkIsInTable(result, &fuelTable2, offset);
      // LCOV_EXCL_BR_STOP
      break;

    case wmiMapPage:
      // LCOV_EXCL_BR_START
      checkIsInTable(result, &wmiTable, offset);
      // LCOV_EXCL_BR_STOP
      checkIsInTable(result, &vvt2Table, offset);
      checkIsInTable(result, &dwellTable, offset);
      checkIsInEmpty(result, 8U, offset);
      break;
    
    case ignMap2Page:
      // LCOV_EXCL_BR_START
      checkIsInTable(result, &ignitionTable2, offset);
      // LCOV_EXCL_BR_STOP
      break;

    case veSetPage: 
      // LCOV_EXCL_BR_START
      checkIsInRaw(result, &configPage2, sizeof(configPage2), offset);
      // LCOV_EXCL_BR_STOP
      break;

    case ignSetPage: 
      // LCOV_EXCL_BR_START
      checkIsInRaw(result, &configPage4, sizeof(configPage4), offset);
      // LCOV_EXCL_BR_STOP
      break;
    
    case afrSetPage: 
      // LCOV_EXCL_BR_START
      checkIsInRaw(result, &configPage6, sizeof(configPage6), offset);
      // LCOV_EXCL_BR_STOP
      break;

    case canbusPage:  
      // LCOV_EXCL_BR_START
      checkIsInRaw(result, &configPage9, sizeof(configPage9), offset);
      // LCOV_EXCL_BR_STOP
      break;

    case warmupPage: 
      // LCOV_EXCL_BR_START
      checkIsInRaw(result, &configPage10, sizeof(configPage10), offset);
      // LCOV_EXCL_BR_STOP
      break;

    case progOutsPage: 
      // LCOV_EXCL_BR_START
      checkIsInRaw(result, &configPage13, sizeof(configPage13), offset);
      // LCOV_EXCL_BR_STOP
      break;
    
    case boostvvtPage2: //Boost, VVT and staging maps (all 8x8)
      // LCOV_EXCL_BR_START
      checkIsInTable(result, &boostTableLookupDuty, offset);
      // LCOV_EXCL_BR_STOP
      checkIsInRaw(result, &configPage15, sizeof(configPage15), offset);
      break;

    default:
      // Nothing to do
      break;
  }

  // Nothing matched, so we are at the end of the known entities for the page.
  if (result.type==End)
  {
    nextEntity(result, 0U);
  }

  return result;
}


// ====================================== External functions  ====================================

uint8_t getPageCount(void)
{
  return 16U;
}

uint16_t getPageSize(byte pageNum)
{
  page_iterator_t entity = map_page_offset_to_entity(pageNum, UINT16_MAX);
  return entity.address.start + entity.address.size;
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

#if defined(CORE_AVR)
#pragma GCC pop_options
#endif