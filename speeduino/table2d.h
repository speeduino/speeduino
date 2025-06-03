/*
This file is used for everything related to maps/tables including their definition, functions etc
*/
#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>

/// @cond
// private to table2D implementation

// The 2D table cache
struct Table2DCache {
  // Store the upper index of the bin we last found. This is used to make the next check faster
  // Since this is the *upper* index, it can never be 0.
  uint8_t lastBinUpperIndex = 1U; // The algorithms rely on this being less than the length of the table AND non-zero

  //Store the last input and output for caching
  int16_t lastInput = INT16_MAX;
  int16_t lastOutput = 0;
  uint8_t cacheTime = 0U; //Tracks when the last cache value was set so it can expire after x seconds. A timeout is required to pickup when a tuning value is changed, otherwise the old cached value will continue to be returned as the X value isn't changing. 

  constexpr Table2DCache(void) = default;
};

// Captures the concept of an opaque array
// (at some point this needs to be replaced with C++ templates)
struct OpaqueArray {
  enum TypeIndicator {
    TYPE_UINT8,
    TYPE_INT8,
    TYPE_UINT16,
    TYPE_INT16,
  };
  TypeIndicator type = TYPE_UINT8;
  const void *data = nullptr;

  explicit constexpr OpaqueArray(const uint8_t *data)
    : type(TYPE_UINT8)
    , data((const void *)data)
  {
  };
  explicit constexpr OpaqueArray(const int8_t *data)
    : type(TYPE_INT8)
    , data((const void *)data)
  {
  };
  explicit constexpr OpaqueArray(const uint16_t *data)
    : type(TYPE_UINT16)
    , data((const void *)data)
  {
  };
  explicit constexpr OpaqueArray(const int16_t *data)
    : type(TYPE_INT16)
    , data((const void *)data)
  {
  };
  
};

/// @endcond

/**
 * @brief A polymorphic 2D table.
 * 
 * The table is designed to be used with the table2D_getValue function to interpolate values from a 2D table.
 *  * Construct by calling one of the constructors below
 *    * The table is defined by providing the axis and value arrays 
 *    * The axis and value arrays must be the same length
 *    * The axis array **must** be sorted
 *    * The axis and values can be any integral type up to 16-bits wide.
 *    * Signed or unsigned.
 */
struct table2D { // cppcheck-suppress ctuOneDefinitionRuleViolation; false positive
  uint8_t length;

  OpaqueArray values;
  OpaqueArray axis;

  mutable Table2DCache cache;

  constexpr table2D(uint8_t length, const OpaqueArray &values, const OpaqueArray &bins) 
    : length(length), values(values), axis(bins) {  
  }
  constexpr table2D(uint8_t length, const uint8_t *values, const uint8_t *bins) 
    : table2D(length, OpaqueArray(values), OpaqueArray(bins)) {
  }
  constexpr table2D(uint8_t length, const uint8_t *values, const int8_t *bins)
    : table2D(length, OpaqueArray(values), OpaqueArray(bins)) {
  }
  constexpr table2D(uint8_t length, const uint16_t *values, const uint16_t *bins)
    : table2D(length, OpaqueArray(values), OpaqueArray(bins)) {
  }
  constexpr table2D(uint8_t length, const uint8_t *values, const uint16_t *bins)
    : table2D(length, OpaqueArray(values), OpaqueArray(bins)) {
  }
  constexpr table2D(uint8_t length, const uint16_t *values, const uint8_t *bins)
    : table2D(length, OpaqueArray(values), OpaqueArray(bins)) {
  }
  constexpr table2D(uint8_t length, const int16_t *values, const uint8_t *bins)
    : table2D(length, OpaqueArray(values), OpaqueArray(bins)) {
  } 
};

int16_t table2D_getAxisValue(const struct table2D *fromTable, uint8_t index);
int16_t table2D_getRawValue(const struct table2D *fromTable, uint8_t index);

int16_t table2D_getValue(const struct table2D *fromTable, const int16_t X_in);

#endif // TABLE_H
