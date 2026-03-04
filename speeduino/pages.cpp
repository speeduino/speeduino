#include "pages.h"
#include "globals.h"
#include "preprocessor.h"
#include "table3d_visitor.h"

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
static constexpr uint16_t get_table_axisy_end(const table_t *table)
{
  UNUSED(table);
  return get_table_axisx_end<table_t>()+table_t::yaxis_t::length;
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
    return *((const byte*)entity.pRaw + offset);
  }
  return 0U;
}

static inline bool set_raw_location(page_iterator_t &entity, uint16_t offset, byte value)
{
  if (offset<entity.address.size)
  {
    *((byte*)entity.pRaw + offset) = value;
    return true;
  }
  return false;
}

struct get_table_value_visitor {
  uint16_t _offset;
  
  explicit get_table_value_visitor(uint16_t offset) 
    : _offset(offset) 
  {
  }

  template <typename TTable>
  byte visit(TTable &table) {
      return *offset_to_table<TTable>(&table, _offset);
  }
};

static inline byte get_table_value(const page_iterator_t &entity, uint16_t offset)
{
  if (offset<entity.address.size)
  {
    get_table_value_visitor visitor(offset);
    return visitTable3d<get_table_value_visitor, byte>(*entity.pTable, entity.table_key, visitor);
  }
  return 0U;
}

byte getEntityValue(const page_iterator_t &entity, uint16_t offset)
{
  if (EntityType::Raw==entity.type)
  {
    return get_raw_location(entity, offset);
  }
  if (EntityType::Table==entity.type)
  {
    return get_table_value(entity, offset);
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

static inline bool set_table_value(page_iterator_t &entity, uint16_t offset, byte new_value)
{
  if (offset<entity.address.size)
  {
    set_table_value_visitor visitor(offset, new_value);
    visitTable3d<set_table_value_visitor, void>(*entity.pTable, entity.table_key, visitor);
    return true;
  }
  return false;
}

bool setEntityValue(page_iterator_t &entity, uint16_t offset, byte value)
{    
  if (EntityType::Raw==entity.type)
  {
    return set_raw_location(entity, offset, value);
  }
  else if (EntityType::Table==entity.type)
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

template <typename T>
static T& loadObject_P(const T *pAddress, T &t)
{
  (void)memcpy_P(&t, pAddress, sizeof(T));
  return t;
}

template <typename T>
static T loadObject_P(const T *pAddress)
{
  T t = {};
  return loadObject_P(pAddress, t);
}

// ========================= Table processing  ===================

template <typename table_t>
static constexpr page_iterator_t makeTableIterator(table_t *pTable, uint8_t pageNum)
{
  return page_iterator_t(pTable, 
                        table_t::type_key,
                        entity_page_location_t(pageNum, 0U),
                        entity_page_address_t(0U, getTableSize<table_t>()));
}

template <typename table_t>
static constexpr page_iterator_t makeTableIterator(table_t *pTable, const page_iterator_t &previous)
{
  return page_iterator_t(pTable, 
                        table_t::type_key,
                        previous.location.next(), previous.address.next(getTableSize<table_t>()));
}

// ========================= Raw memory block processing  ===================

static constexpr page_iterator_t makeRawIterator(config_page_t *pEntity, uint16_t entitySize, uint8_t pageNum)
{
  return page_iterator_t( pEntity, 
                        entity_page_location_t(pageNum, 0U),
                        entity_page_address_t(0U, entitySize));
}

static constexpr page_iterator_t makeRawIterator(config_page_t *pEntity, uint16_t entitySize, const page_iterator_t &previous)
{
  return page_iterator_t(pEntity, previous.location.next(), previous.address.next(entitySize));
}

// ========================= Empty entity processing  ===================

static constexpr page_iterator_t makeEmptyIterator(const page_iterator_t &previous, uint16_t entitySize)
{
  return page_iterator_t(EntityType::NoEntity, previous.location.next(), previous.address.next(entitySize));
}

// ========================= Page map ===============================

struct page_map_t
{
  const page_iterator_t *searchMap = nullptr;
  uint8_t mapSize = 0U;
};

static constexpr page_iterator_t vePageMap[] PROGMEM = {
  makeTableIterator(&fuelTable, veMapPage)
};

static constexpr page_iterator_t ignPageMap[] PROGMEM = {
  makeTableIterator(&ignitionTable, ignMapPage)
};

static constexpr page_iterator_t afrPageMap[] PROGMEM = {
  makeTableIterator(&afrTable, afrMapPage)
};

constexpr auto boostVvtEntity0 = makeTableIterator(&boostTable, boostvvtPage);
constexpr auto boostVvtEntity1 = makeTableIterator(&vvtTable, boostVvtEntity0);
constexpr auto boostVvtEntity2 = makeTableIterator(&stagingTable, boostVvtEntity1);
static constexpr page_iterator_t boostVvtPageMap[] PROGMEM = {
  boostVvtEntity0, boostVvtEntity1, boostVvtEntity2
};

constexpr auto seqEntity0 = makeTableIterator(&trim1Table, seqFuelPage);
constexpr auto seqEntity1 = makeTableIterator(&trim2Table, seqEntity0);
constexpr auto seqEntity2 = makeTableIterator(&trim3Table, seqEntity1);
constexpr auto seqEntity3 = makeTableIterator(&trim4Table, seqEntity2);
constexpr auto seqEntity4 = makeTableIterator(&trim5Table, seqEntity3);
constexpr auto seqEntity5 = makeTableIterator(&trim6Table, seqEntity4);
constexpr auto seqEntity6 = makeTableIterator(&trim7Table, seqEntity5);
constexpr auto seqEntity7 = makeTableIterator(&trim8Table, seqEntity6);
static constexpr page_iterator_t sequentialPageMap[] PROGMEM = {
  seqEntity0, seqEntity1, seqEntity2, seqEntity3, seqEntity4, seqEntity5, seqEntity6, seqEntity7
};

static constexpr page_iterator_t fuel2PageMap[] PROGMEM = {
  makeTableIterator(&fuelTable2, fuelMap2Page)
};

constexpr auto wmiEntity0 = makeTableIterator(&wmiTable, wmiMapPage);
constexpr auto wmiEntity1 = makeTableIterator(&vvt2Table, wmiEntity0);
constexpr auto wmiEntity2 = makeTableIterator(&dwellTable, wmiEntity1);
constexpr auto wmiEntity3 = makeEmptyIterator(wmiEntity2, 8U);
static constexpr page_iterator_t wmiPageMap[] PROGMEM = {
  wmiEntity0, wmiEntity1, wmiEntity2, wmiEntity3,
};

static constexpr page_iterator_t ign2PageMap[] PROGMEM = {
  makeTableIterator(&ignitionTable2, ignMap2Page)
};

static constexpr page_iterator_t veSetPageMap[] PROGMEM = {
  makeRawIterator(&configPage2, sizeof(configPage2), veSetPage)
};

static constexpr page_iterator_t ignSetPageMap[] PROGMEM = {
  makeRawIterator(&configPage4, sizeof(configPage4), ignSetPage)
};

static constexpr page_iterator_t afrSetPageMap[] PROGMEM = {
  makeRawIterator(&configPage6, sizeof(configPage6), afrSetPage)
};

static constexpr page_iterator_t canBusPageMap[] PROGMEM = {
  makeRawIterator(&configPage9, sizeof(configPage9), canbusPage)
};

static constexpr page_iterator_t warmUpPageMap[] PROGMEM = {
  makeRawIterator(&configPage10, sizeof(configPage10), warmupPage)
};

static constexpr page_iterator_t progOutsPageMap[] PROGMEM = {
  makeRawIterator(&configPage13, sizeof(configPage13), progOutsPage)
};

constexpr auto boostVvt2Entity0 = makeTableIterator(&boostTableLookupDuty, boostvvtPage2);
constexpr auto boostVvt2Entity1 = makeRawIterator(&configPage15, sizeof(configPage15), boostVvt2Entity0);
static constexpr page_iterator_t boostVvt2PageMap[] PROGMEM = {
  boostVvt2Entity0, boostVvt2Entity1
};

static page_map_t getPageMap(uint8_t pageNumber)
{
  static constexpr page_iterator_t pageZeroMap[] PROGMEM = {
    page_iterator_t(EntityType::End, entity_page_location_t(), entity_page_address_t()),
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
  return loadObject_P(&pageMaps[pageNumber]);
}

static page_iterator_t mapOffsetToEntity_P(page_map_t pageMap, uint16_t offset)
{
  page_iterator_t entityIter;
  for (uint8_t index=0; index<pageMap.mapSize; ++index)
  {
    if (loadObject_P(&pageMap.searchMap[index], entityIter).address.isOffsetInEntity(offset))
    {
      return entityIter;
    }
  }
  return page_iterator_t(EntityType::End, entityIter.location.next(), entityIter.address.next(0));
}

// ===============================================================================

// Does the heavy lifting of mapping page+offset to an entity
static page_iterator_t map_page_offset_to_entity(uint8_t pageNumber, uint16_t offset)
{
  return mapOffsetToEntity_P(getPageMap(pageNumber), offset);
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

static void setTableToEmpty(const page_iterator_t &entity)
{
  setTableAxisToEmpty(y_begin(entity));
  setTableAxisToEmpty(x_begin(entity));
  setTableValuesToEmpty(rows_begin(entity));
}


static void setEntityToEmpty(page_iterator_t entity) {
  switch (entity.type)
    {
    case EntityType::Raw:
        (void)memset(entity.pRaw, 0, entity.address.size);
        break;

    case EntityType::Table:
        setTableToEmpty(entity);
        break;

    default:
        // Do nothing
        break;
    }
}

// ====================================== External functions  ====================================

void __attribute__((noinline)) setTuneToEmpty(void) {
  for (uint8_t page=MIN_PAGE_NUM; page<MAX_PAGE_NUM; ++page) {
    page_iterator_t entity = page_begin(page);
    while (entity.type!=EntityType::End) {
      setEntityToEmpty(entity);
      entity = advance(entity);
    }
  }
}

uint16_t getPageSize(byte pageNum)
{
  auto pageMap = getPageMap(pageNum);
  page_iterator_t lastEntityOnPage;
  (void)loadObject_P(&pageMap.searchMap[pageMap.mapSize-1U], lastEntityOnPage);
  return lastEntityOnPage.address.start + lastEntityOnPage.address.size;
}

static inline uint16_t pageOffsetToEntityOffset(const page_iterator_t &entity, uint16_t pageOffset)
{
  return pageOffset-entity.address.start;
}

bool setPageValue(uint8_t pageNum, uint16_t offset, byte value)
{
  page_iterator_t entity = map_page_offset_to_entity(pageNum, offset);

  return setEntityValue(entity, pageOffsetToEntityOffset(entity, offset), value);
}

byte getPageValue(uint8_t pageNum, uint16_t offset)
{
  page_iterator_t entity = map_page_offset_to_entity(pageNum, offset);

  return getEntityValue(entity, pageOffsetToEntityOffset(entity, offset));
}

// LCOV_EXCL_START
// No need to have coverage on simple wrappers

// Support iteration over a pages entities.
page_iterator_t page_begin(uint8_t pageNum)
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
  return rows_begin(it.pTable, it.table_key);
}

/**
 * Convert page iterator to table x axis iterator.
 */
table_axis_iterator x_begin(const page_iterator_t &it)
{
  return x_begin(it.pTable, it.table_key);
}

/**
 * Convert page iterator to table x axis iterator.
 */
table_axis_iterator x_rbegin(const page_iterator_t &it)
{
  return x_rbegin(it.pTable, it.table_key);
}

/**
 * Convert page iterator to table y axis iterator.
 */
table_axis_iterator y_begin(const page_iterator_t &it)
{
  return y_begin(it.pTable, it.table_key);
}

// LCOV_EXCL_STOP

#if defined(CORE_AVR)
#pragma GCC pop_options
#endif