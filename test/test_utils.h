
#pragma once
#include "src/utils/minmax.h"
#include <stdint.h>
#include <unity.h>
#include <algorithm>
#include <Arduino.h>
#include "table3d.h"
#include "table2d.h"
#include "maths.h"
#include "file_name_guard_t.h"

template<size_t MAX_LEN, size_t N>
constexpr void STR_LEN_CHECK(char const (&)[N]) 
{
    static_assert(N < MAX_LEN, "String overflow!");
}

#if !defined(_countof)
#define _countof(x) (sizeof(x) / sizeof (x[0]))
#endif

// Unity macro to reduce memory usage (RAM, .bss)
//
// Unity supplied RUN_TEST captures the function name
// using #func directly in the call to UnityDefaultTestRun.
// This is a raw string that is placed in the data segment,
// which consumes RAM.
//
// So instead, place the function name in flash memory and
// load it at run time.
#define RUN_TEST_P(func) \
  { \
    char funcName[128]; \
    constexpr size_t bufferLen = _countof(funcName); \
    STR_LEN_CHECK<bufferLen>(#func); \
    strcpy_P(funcName, PSTR(#func)); \
    UnityDefaultTestRun(func, funcName, __LINE__); \
  }

#define RUN_TEST_POSTFIX_P(func, postFix) \
  { \
    char funcName[256]; \
    constexpr size_t bufferLen = _countof(funcName); \
    STR_LEN_CHECK<bufferLen>(#func); \
    strcpy_P(funcName, PSTR(#func)); \
    strcat(funcName, postFix); \
    UnityDefaultTestRun(func, funcName, __LINE__); \
  }


// ============================ SET_UNITY_FILENAME ============================ 

// This is an older style. New code should just use
//    unity_filename_guard_t(__FILE__);
// instead.
#define SET_UNITY_FILENAME()                                                        \
for ( struct { unity_filename_guard_t a; uint8_t b; } guard = { .a = unity_filename_guard_t(__FILE__), .b = 1 }; \
    guard.b; guard.b = 0 )

// ============================ end SET_UNITY_FILENAME ============================ 

// Store test data in flash, if feasible.
#if defined(PROGMEM)
#define TEST_DATA_P static constexpr PROGMEM
#else
#define TEST_DATA_P static constexpr
#endif

template <typename table3d_t>
static inline void fill_table_values(table3d_t &table, table3d_value_t value) {
  // for (uint8_t i=0; i<table.values.row_size*table.values.num_rows; ++i) {
  //   table.values.values[i] = value;
  // }
  table_value_iterator itZ = table.values.begin();
  while (!itZ.at_end())
  {
    table_row_iterator itRow = *itZ;
    while (!itRow.at_end())
    {
      *itRow = value;
      ++itRow;
    }
    ++itZ;
  }  
  invalidate_cache(&table.get_value_cache);
}

template<size_t N>
static inline void populate_table_axis(std::array<table3d_axis_t, N> &axis, table3d_axis_t value) {
  std::fill(axis.begin(), axis.end(), value); 
}

template<size_t N>
static inline void populate_table_axis_P(std::array<table3d_axis_t, N> &axis, 
                                         const table3d_axis_t *pXValues) {   // PROGMEM if available
  for (auto &elenent: axis) {
#if defined(PROGMEM)
    elenent = (table3d_axis_t)pgm_read_word(pXValues);
#else
    elenent = *pXValues;
#endif      
    ++pXValues;
  }
}

// Populate a 3d table (from PROGMEM if available)
// You wuld typically declare the 3 source arrays usin TEST_DATA_P
template <typename table3d_t>
static inline void populate_table_P(table3d_t &table, 
                                  const table3d_axis_t *pXValues,   // PROGMEM if available
                                  const table3d_axis_t *pYValues,   // PROGMEM if available
                                  const table3d_value_t *pZValues)  // PROGMEM if available
{
  populate_table_axis_P(table.axisX, pXValues);
  populate_table_axis_P(table.axisY, pYValues);
  {
    table_value_iterator itZ = table.values.begin();
    while (!itZ.at_end())
    {
      table_row_iterator itRow = *itZ;
      while (!itRow.at_end())
      {
#if defined(PROGMEM)
        *itRow = pgm_read_byte(pZValues);
#else
        *itRow = *pZValues;
#endif
        ++pZValues;
        ++itRow;
      }
      ++itZ;
    }
  }
}

// Populate a 2d table with constant values
template <typename axis_t, typename value_t, uint8_t sizeT>
static inline void populate_2dtable(table2D<axis_t, value_t, sizeT> *pTable, value_t value, axis_t bin) {
  for (uint8_t index=0; index<sizeT; ++index) {
    (value_t&)(pTable->values[index]) = value;
    (axis_t&)(pTable->axis[index]) = bin;
  }
  pTable->cache.cacheTime = UINT8_MAX;
}

template <typename axis_t, typename value_t, uint8_t sizeT>
static inline void populate_2dtable(table2D<axis_t, value_t, sizeT> *pTable, const value_t (&values)[sizeT], const axis_t (&bins)[sizeT]) {
  memcpy((void*)pTable->axis, bins, sizeT * sizeof(axis_t));
  memcpy((void*)pTable->values, values, sizeT * sizeof(value_t));
  pTable->cache.cacheTime = UINT8_MAX;
}

// Populate a 2d table (from PROGMEM if available)
// You would typically declare the 2 source arrays using TEST_DATA_P
template <typename axis_t, typename value_t, uint8_t sizeT>
static inline void populate_2dtable_P(table2D<axis_t, value_t, sizeT> *pTable, const value_t (&values)[sizeT], const axis_t (&bins)[sizeT]) {
#if defined(PROGMEM)
  memcpy_P((void*)pTable->axis, bins, sizeT * sizeof(axis_t));
  memcpy_P((void*)pTable->values, values, sizeT * sizeof(value_t));
  pTable->cache.cacheTime = UINT8_MAX;
#else
  populate_2dtable(pTable, values, bins)
#endif
}

template <typename T>
T intermediate(T const& min, T const& max, uint8_t const& frac)
{
  if (max<min) {
    return min - percentage(frac, (min - max));
  }
  return min + percentage(frac, (max - min));
}
