#include "pages.h"
#include "globals.h"
#include "preprocessor.h"
#include "table3d_visitor.h"
#include "prog_mem_support.h"

// This minimizes RAM usage at no performance cost
#pragma GCC optimize ("Os") 

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
static constexpr uint16_t get_table_value_end(void)
{
  return table_t::xaxis_t::length*table_t::yaxis_t::length;
}
template <class table_t>
static constexpr uint16_t get_table_axisx_end(void)
{
  return get_table_value_end<table_t>()+table_t::xaxis_t::length;
}
template <class table_t>
static constexpr uint16_t getTableSize(void)
{
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
    _table_offset(min(table_offset, getTableSize<table_t>()))
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

static inline byte get_raw_location(const entity_t &entity, uint16_t entityOffset)
{
  if (entity.isEntityAddressWithin(entityOffset))
  {
    return *((const byte*)entity.pRaw + entityOffset);
  }
  return 0U;
}

static inline bool set_raw_location(entity_t &entity, uint16_t entityOffset, byte value)
{
  if (entity.isEntityAddressWithin(entityOffset))
  {
    *((byte*)entity.pRaw + entityOffset) = value;
    return true;
  }
  return false;
}

struct get_table_value_visitor {
  uint16_t _tableOffset;
  
  explicit get_table_value_visitor(uint16_t tableOffset) 
    : _tableOffset(tableOffset) 
  {
  }

  template <typename TTable>
  byte visit(TTable &table) {
      return *offset_to_table<TTable>(&table, _tableOffset);
  }
};

static inline byte get_table_value(const entity_t &entity, uint16_t entityOffset)
{
  if (entity.isEntityAddressWithin(entityOffset))
  {
    get_table_value_visitor visitor(entityOffset);
    return visitTable3d<get_table_value_visitor, byte>(*entity.pTable, entity.table_key, visitor);
  }
  return 0U;
}

byte getEntityValue(const entity_t &entity, uint16_t entityOffset)
{
  if (EntityType::Raw==entity.type)
  {
    return get_raw_location(entity, entityOffset);
  }
  if (EntityType::Table==entity.type)
  {
    return get_table_value(entity, entityOffset);
  }
  // Entity has no data
  return 0U;
}

struct set_table_value_visitor {
  uint16_t _offset;
  byte _newValue;

  explicit set_table_value_visitor(uint16_t offset, byte newValue) 
    : _offset(offset) 
    , _newValue(newValue)
  {
  }

  template <typename TTable>
  void visit(TTable &table) {
      offset_to_table<TTable>(&table, _offset) = _newValue;
  }
};

static inline bool set_table_value(entity_t &entity, uint16_t entityOffset, byte new_value)
{
  if (entity.isEntityAddressWithin(entityOffset))
  {
    set_table_value_visitor visitor(entityOffset, new_value);
    visitTable3d<set_table_value_visitor, void>(*entity.pTable, entity.table_key, visitor);
    return true;
  }
  return false;
}

bool setEntityValue(entity_t &entity, uint16_t entityOffset, byte value)
{    
  if (EntityType::Raw==entity.type)
  {
    return set_raw_location(entity, entityOffset, value);
  }
  else if (EntityType::Table==entity.type)
  {
    return set_table_value(entity, entityOffset, value);
  }
  else
  {
    // Unsettable entity type 
    return false;
  }
}


// ========================= Table processing  ===================

template <typename table_t>
static constexpr entity_t makeEntity(table_t *pTable)
{
  return entity_t((table3d_t*)pTable, table_t::type_key, getTableSize<table_t>());
}

// ========================= Raw memory block processing  ===================

static constexpr entity_t makeEntity(config_page_t *pEntity, uint16_t entitySize)
{
  return entity_t(pEntity, entitySize);
}

// ========================= Empty entity processing  ===================

static constexpr entity_t makeEntity(uint16_t entitySize)
{
  return entity_t(EntityType::NoEntity, entitySize);
}

// =========================== Page Mapping ===============================

struct page_map_t
{
  const entity_t *searchMap = nullptr;
  uint8_t mapSize = 0U;
};

static page_map_t getPageMap(uint8_t pageNumber)
{
  static constexpr entity_t pageZeroMap[] PROGMEM = {
    makeEntity(0U),
  };
  static constexpr entity_t vePageMap[] PROGMEM = {
    makeEntity(&fuelTable),
  };
  static constexpr entity_t ignPageMap[] PROGMEM = {
    makeEntity(&ignitionTable),
  };
  static constexpr entity_t afrPageMap[] PROGMEM = {
    makeEntity(&afrTable),
  };
  static constexpr entity_t boostVvtPageMap[] PROGMEM = {
    makeEntity(&boostTable), 
    makeEntity(&vvtTable), 
    makeEntity(&stagingTable),
  };
  static constexpr entity_t sequentialPageMap[] PROGMEM = {
    makeEntity(&trim1Table), 
    makeEntity(&trim2Table), 
    makeEntity(&trim3Table),
    makeEntity(&trim4Table),
    makeEntity(&trim5Table),
    makeEntity(&trim6Table),
    makeEntity(&trim7Table),
    makeEntity(&trim8Table),
  };
  static constexpr entity_t fuel2PageMap[] PROGMEM = {
    makeEntity(&fuelTable2)
  };
  static constexpr entity_t wmiPageMap[] PROGMEM = {
    makeEntity(&wmiTable),
    makeEntity(&vvt2Table),
    makeEntity(&dwellTable),
    makeEntity(8U),
  };
  static constexpr entity_t ign2PageMap[] PROGMEM = {
    makeEntity(&ignitionTable2),
  };
  static constexpr entity_t veSetPageMap[] PROGMEM = {
    makeEntity(&configPage2, sizeof(configPage2)),
  };
  static constexpr entity_t ignSetPageMap[] PROGMEM = {
    makeEntity(&configPage4, sizeof(configPage4)),
  };
  static constexpr entity_t afrSetPageMap[] PROGMEM = {
    makeEntity(&configPage6, sizeof(configPage6)),
  };
  static constexpr entity_t canBusPageMap[] PROGMEM = {
    makeEntity(&configPage9, sizeof(configPage9)),
  };
  static constexpr entity_t warmUpPageMap[] PROGMEM = {
    makeEntity(&configPage10, sizeof(configPage10)),
  };
  static constexpr entity_t progOutsPageMap[] PROGMEM = {
    makeEntity(&configPage13, sizeof(configPage13)),
  };
  static constexpr entity_t boostVvt2PageMap[] PROGMEM = {
    makeEntity(&boostTableLookupDuty),
    makeEntity(&configPage15, sizeof(configPage15)),
  };

  static constexpr page_map_t pageMaps[MAX_PAGE_NUM] PROGMEM = {
    { pageZeroMap, _countof(pageZeroMap) },
    { veSetPageMap, _countof(veSetPageMap) },
    { vePageMap, _countof(vePageMap) },
    { ignPageMap, _countof(ignPageMap) },
    { ignSetPageMap, _countof(ignSetPageMap) },    
    { afrPageMap, _countof(afrPageMap) },
    { afrSetPageMap, _countof(afrSetPageMap) },
    { boostVvtPageMap, _countof(boostVvtPageMap) },
    { sequentialPageMap, _countof(sequentialPageMap) },
    { canBusPageMap, _countof(canBusPageMap) },
    { warmUpPageMap, _countof(warmUpPageMap) },
    { fuel2PageMap, _countof(fuel2PageMap) },
    { wmiPageMap, _countof(wmiPageMap) },
    { progOutsPageMap, _countof(progOutsPageMap) },    
    { ign2PageMap, _countof(ign2PageMap) },
    { boostVvt2PageMap, _countof(boostVvt2PageMap) },
  };

  if (pageNumber>=MAX_PAGE_NUM)
  {
    pageNumber = 0U;
  }
  return copyObject_P(&pageMaps[pageNumber]);
}

/**
 * @brief Search for the page_iterator_t that spans pageOffset */
static page_iterator_t mapOffsetToEntity_P(const entity_t *pEntityMap, uint8_t mapLength, uint8_t pageNumber, uint16_t pageOffset)
{
  entity_page_location_t pageLocation(pageNumber, 0U);
  uint16_t entityOffset = 0U;

  for (uint8_t index=0; index<mapLength; ++index)
  {
    page_entity_t mappedEntity(copyObject_P(&pEntityMap[index]), entityOffset);
    if (mappedEntity.isPageAddressWithin(pageOffset))
    {
      return page_iterator_t(mappedEntity, pageLocation);
    }
    entityOffset = entityOffset + mappedEntity.size;
    pageLocation = pageLocation.next();
  }

  constexpr entity_t END_ENTITY = entity_t(EntityType::End, 0U);
  return page_iterator_t(page_entity_t(END_ENTITY, entityOffset), pageLocation);
}

// ===============================================================================

// Does the heavy lifting of mapping page+offset to an entity
static page_iterator_t map_page_offset_to_entity(uint8_t pageNumber, uint16_t offset)
{
  auto pageMap = getPageMap(pageNumber);
  return mapOffsetToEntity_P(pageMap.searchMap, pageMap.mapSize, pageNumber, offset);
}

// ========================= Set tune to empty support  ===================

static void setTableRowToEmpty(table_row_iterator row)
{
  (void)memset(&*row, 0, row.size());
}

static void setTableValuesToEmpty(table_value_iterator it)
{
  while (!it.at_end())
  {
    setTableRowToEmpty(*it);
    ++it;
  }
}

static void setTableAxisToEmpty(table_axis_iterator it)
{
  while (!it.at_end())
  {
    *it = 0;
    ++it;
  }
}

static void setTableToEmpty(const page_iterator_t &iter)
{
  setTableAxisToEmpty(y_begin(iter));
  setTableAxisToEmpty(x_begin(iter));
  setTableValuesToEmpty(rows_begin(iter));
}


static void setEntityToEmpty(page_iterator_t iter) {
  switch (iter.entity.type)
    {
    case EntityType::Raw:
        (void)memset(iter.entity.pRaw, 0, iter.entity.size);
        break;

    case EntityType::Table:
        setTableToEmpty(iter);
        break;

    default:
        // Do nothing
        break;
    }
}

// ====================================== External functions  ====================================

void __attribute__((noinline)) setTuneToEmpty(void) {
  for (uint8_t page=MIN_PAGE_NUM; page<MAX_PAGE_NUM; ++page) {
    page_iterator_t iter = page_begin(page);
    while (iter.entity.type!=EntityType::End) {
      setEntityToEmpty(iter);
      iter = advance(iter);
    }
  }
}

uint16_t getPageSize(byte pageNum)
{
  page_iterator_t iter = map_page_offset_to_entity(pageNum, UINT16_MAX);
  return iter.entity.start;
}

static inline uint16_t pageOffsetToEntityOffset(const page_iterator_t &iter, uint16_t pageOffset)
{
  return pageOffset-iter.entity.start;
}

bool setPageValue(uint8_t pageNum, uint16_t pageOffset, byte value)
{
  page_iterator_t iter = map_page_offset_to_entity(pageNum, pageOffset);

  return setEntityValue(iter.entity, pageOffsetToEntityOffset(iter, pageOffset), value);
}

byte getPageValue(uint8_t pageNum, uint16_t pageOffset)
{
  page_iterator_t iter = map_page_offset_to_entity(pageNum, pageOffset);

  return getEntityValue(iter.entity, pageOffsetToEntityOffset(iter, pageOffset));
}

// LCOV_EXCL_START
// No need to have coverage on simple wrappers

// Support iteration over a pages entities.
page_iterator_t page_begin(uint8_t pageNum)
{
  return map_page_offset_to_entity(pageNum, 0U);
}

page_iterator_t advance(const page_iterator_t &iter)
{
    return map_page_offset_to_entity(iter.location.page, iter.entity.start+iter.entity.size);
}

/**
 * Convert page iterator to table value iterator.
 */
table_value_iterator rows_begin(const page_iterator_t &iter)
{
  return rows_begin(iter.entity.pTable, iter.entity.table_key);
}

/**
 * Convert page iterator to table x axis iterator.
 */
table_axis_iterator x_begin(const page_iterator_t &iter)
{
  return x_begin(iter.entity.pTable, iter.entity.table_key);
}

/**
 * Convert page iterator to table x axis iterator.
 */
table_axis_iterator x_rbegin(const page_iterator_t &iter)
{
  return x_rbegin(iter.entity.pTable, iter.entity.table_key);
}

/**
 * Convert page iterator to table y axis iterator.
 */
table_axis_iterator y_begin(const page_iterator_t &iter)
{
  return y_begin(iter.entity.pTable, iter.entity.table_key);
}

// LCOV_EXCL_STOP