/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

/*
Because the size of the table is dynamic, this function is required to reallocate the array sizes
Note that this may clear some of the existing values of the table
*/
#include <Arduino.h>
#include "table2d.h"
#if !defined(UNIT_TEST)
#include "globals.h"
#endif



static inline uint8_t getCacheTime(void) {
#if !defined(UNIT_TEST)
  return currentStatus.secl;
#else
  return 0;
#endif
}

static inline bool cacheExpired(const Table2DCache &cache) {
  return (cache.cacheTime != getCacheTime());
}

// OpaqueArray support: conveniently access the underlying array. 
// Ideally this should be the one and only place where the array type is known.
#define OPAQUE_ARRAY_DISPATCH(opaqueArray, action, ...) \
  if((opaqueArray).type == OpaqueArray::TYPE_UINT8) { return (action)((const uint8_t*)((opaqueArray).data), __VA_ARGS__); } \
  if((opaqueArray).type == OpaqueArray::TYPE_UINT16) { return (action)((const uint16_t*)((opaqueArray).data), __VA_ARGS__); } \
  if((opaqueArray).type == OpaqueArray::TYPE_INT8) { return (action)((const int8_t*)((opaqueArray).data), __VA_ARGS__); } \
  if((opaqueArray).type == OpaqueArray::TYPE_INT16) { return (action)((const int16_t*)((opaqueArray).data), __VA_ARGS__); }

template <typename TArray>
static inline TArray getValue(const TArray *array, uint8_t index) {
  return array[index];
}
 
static inline int16_t getValue(const OpaqueArray &array, uint8_t index) {
  OPAQUE_ARRAY_DISPATCH(array, getValue, index)
  return 0;
}

struct Table2DBin {
  uint8_t upperIndex;
  int16_t upperValue;
  int16_t lowerValue;
};

static inline int16_t range(const Table2DBin &bin) {
  return bin.upperValue - bin.lowerValue;
}

static inline bool isInBin(int16_t value, const Table2DBin &bin) {
  return (value <= bin.upperValue) && (value > bin.lowerValue);
}

template <typename TArray>
static inline Table2DBin constructBin(const TArray *axis, uint8_t binUpperIndex) {
  return { .upperIndex = binUpperIndex , .upperValue = (int16_t)axis[binUpperIndex], .lowerValue = (int16_t)axis[binUpperIndex-1U] }; 
}

static inline Table2DBin constructBin(const OpaqueArray &array, uint8_t binUpperIndex) {
  OPAQUE_ARRAY_DISPATCH(array, constructBin, binUpperIndex)
  return Table2DBin();
}  

static inline Table2DBin getAxisBin(const struct table2D *fromTable, uint8_t binUpperIndex)
{
  return constructBin(fromTable->axis, binUpperIndex);
}

static inline Table2DBin getValueBin(const struct table2D *fromTable, uint8_t binUpperIndex)
{
  return constructBin(fromTable->values, binUpperIndex);
}

static inline Table2DBin findAxisBin(const struct table2D *fromTable, const int16_t axisValue) {
  // Loop from the upper end of the axis back down to the 1st bin [0,1]
  Table2DBin xBin = getAxisBin(fromTable, fromTable->length-1U);
  while ((!isInBin(axisValue, xBin)) && (xBin.upperIndex!=1U)) {
    --xBin.upperIndex;
    xBin = getAxisBin(fromTable, xBin.upperIndex);
  }

  return xBin;
}

/*
This function pulls a 1D linear interpolated (ie averaged) value from a 2D table
ie: Given a value on the X axis, it returns a Y value that corresponds to the point on the curve between the nearest two defined X values

This function must take into account whether a table contains 8-bit or 16-bit values.
Unfortunately this means many of the lines are duplicated depending on this
*/
int16_t table2D_getValue(const struct table2D *fromTable, const int16_t X_in)
{
  //Check whether the X input is the same as last time this ran
  if( (X_in == fromTable->cache.lastInput) && (!cacheExpired(fromTable->cache)) )
  {
    return fromTable->cache.lastOutput;
  }

  const uint8_t xMax = fromTable->length-1U;

  // 1st check is whether we're still in the same X bin as last time
  Table2DBin xBin = getAxisBin(fromTable, fromTable->cache.lastBinUpperIndex);
  if (!isInBin(X_in, xBin))
  {
    //If we're not in the same bin, search 
    xBin = findAxisBin(fromTable, X_in);
  }

  // We are exactly at the bin upper bound, so no need to interpolate
  if (X_in==xBin.upperValue) {
    fromTable->cache.lastOutput = getValue(fromTable->values, xBin.upperIndex);
    fromTable->cache.lastBinUpperIndex = xBin.upperIndex;
  // Within the bin, interpolate
  } else if (isInBin(X_in, xBin)) {
    const Table2DBin valueBin = getValueBin(fromTable, xBin.upperIndex);
    fromTable->cache.lastOutput = (int16_t)map(X_in, xBin.lowerValue, xBin.upperValue, valueBin.lowerValue, valueBin.upperValue);
    fromTable->cache.lastBinUpperIndex = xBin.upperIndex;
  // Above max axis value, clip to max data value
  } else if(X_in >= getValue(fromTable->axis, xMax)) {
    fromTable->cache.lastOutput = getValue(fromTable->values, xMax);
    fromTable->cache.lastBinUpperIndex = xMax;
  // Only choice left is below min axis value, clip to min data value
  } else {
    fromTable->cache.lastOutput = getValue(fromTable->values, 0U);
    fromTable->cache.lastBinUpperIndex = 1U;
  }

  fromTable->cache.cacheTime = getCacheTime(); //As we're not using the cache value, set the current secl value to track when this new value was calculated
  fromTable->cache.lastInput = X_in;

  return fromTable->cache.lastOutput;
}



/**
 * @brief Returns an axis (bin) value from the 2D table. This works regardless of whether that axis is bytes or int16_ts
 * 
 * @param fromTable 
 * @param X_in 
 * @return int16_t 
 */
int16_t table2D_getAxisValue(const struct table2D *fromTable, uint8_t index)
{
  return getValue(fromTable->axis, index);
}

/**
 * @brief Returns an value from the 2D table given an index value. No interpolation is performed
 * 
 * @param fromTable 
 * @param X_index 
 * @return int16_t 
 */
int16_t table2D_getRawValue(const struct table2D *fromTable, uint8_t index)
{
  return getValue(fromTable->values, index);
}
