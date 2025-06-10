/*
This file is used for everything related to maps/tables including their definition, functions etc
*/
#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>
#include <Arduino.h>

/// @cond
// private to table2D implementation

namespace _table2d_detail {

// The 2D table cache
template <typename axis_t, typename value_t>
struct Table2DCache 
{
  // Store the upper index of the bin we last found. This is used to make the next check faster
  // Since this is the *upper* index, it can never be 0.
  uint8_t lastBinUpperIndex = 1U; // The axis bin search algo relies on this being 1 initially

  //Store the last input and output for caching
  axis_t lastInput = 0;
  value_t lastOutput = 0;
  uint8_t cacheTime = 0U; //Tracks when the last cache value was set so it can expire after x seconds. A timeout is required to pickup when a tuning value is changed, otherwise the old cached value will continue to be returned as the X value isn't changing. 

  constexpr Table2DCache(void) = default;
};

extern uint8_t getCacheTime(void);

template <typename axis_t, typename value_t>
static inline bool cacheExpired(const Table2DCache<axis_t, value_t> &cache) {
  return (cache.cacheTime != getCacheTime());
}

} // _table2d_detail
/// @endcond


/**
 * @brief A 2D table.
 * 
 * The table is designed to be used with the table2D_getValue function to interpolate values from a 2D table.
 *  * Construct by calling one of the constructors below
 *    * The table is defined by providing the axis and value arrays 
 *    * The axis and value arrays must be the same length
 *    * The axis array **must** be sorted
 *    * The axis and values can be any integral type up to 16-bits wide.
 *    * Signed or unsigned.
 */
template <typename axis_t, typename value_t, uint8_t sizeT>
struct table2D
{
  using size_type = uint8_t;
  using axis_type = axis_t;
  using value_type = value_t;

  const axis_t *axis = nullptr;
  const value_t *values = nullptr;

  mutable _table2d_detail::Table2DCache<axis_t, value_t> cache;

  constexpr table2D(const axis_t (&axisBin)[sizeT], const value_t (&curve)[sizeT])
    : axis(axisBin) // cppcheck-suppress misra-c2012-14.4
    , values(curve)
  {
  }

  constexpr size_type size(void) const noexcept { return sizeT; }  
};


/// @cond
// private to table2D implementation

namespace _table2d_detail {

template <typename T>
struct Bin {
  constexpr Bin(const T *array, uint8_t binUpperIndex)
  : upperIndex(binUpperIndex)
  , _upperValue(array[binUpperIndex])
  , _lowerValue(array[binUpperIndex-1U])
  {
  } 
  constexpr Bin(uint8_t binUpperIndex, const T upperValue, const T lowerValue)
  : upperIndex(binUpperIndex)
  , _upperValue(upperValue)
  , _lowerValue(lowerValue)
  {
  } 

  const T upperValue(void) const noexcept { return _upperValue; }
  const T lowerValue(void) const noexcept { return _lowerValue; }

  bool withinBin(const T value) const noexcept {
    return (value <= _upperValue) && (value > _lowerValue);
  }

  uint8_t upperIndex;
  T _upperValue;
  T _lowerValue;
};


template <typename T, uint8_t sizeT>
static inline Bin<T> findBin(const T *const array, const T value) {
  // Loop from the upper end of the axis back down to the 1st bin [0,1]
  // Assume array is ordered [min...max]
  const T *binLower = array+sizeT-2U;
  while (*binLower>=value && binLower!=array) {
    --binLower;
  }
  return Bin<T>(binLower-array+1U, *(binLower+1U), *binLower);
}

// Generic interpolation
template <typename axis_t, typename value_t>
static inline value_t interpolate(const axis_t axisValue, const Bin<axis_t> &axisBin, const Bin<value_t> &valueBin) {
  return map(axisValue, axisBin.lowerValue(), axisBin.upperValue(), valueBin.lowerValue(), valueBin.upperValue());
}

// Specialized interpolation of uint8_t for performance
uint8_t interpolate(const uint8_t axisValue, const Bin<uint8_t> &axisBin, const Bin<uint8_t> &valueBin);

} // _table2d_detail

/// @endcond

/**
 * @brief Interpolate a value from a 2d table.
 * 
 * @tparam axis_t The type of the table axis
 * @tparam value_t The type of the table values
 * @tparam sizeT  The size of the table (number of axis and value elements)
 * @param fromTable the table to get the value from
 * @param axisValue the value to look up in the table axis
 * @return value_t table value corresponding to the axis value (possibly interpolated)
 */
template <typename axis_t, typename value_t, uint8_t sizeT>
static inline value_t table2D_getValue(const table2D<axis_t, value_t, sizeT> *fromTable, const axis_t axisValue) {
  //Check whether the X input is the same as last time this ran
  if( (axisValue == fromTable->cache.lastInput) && (!cacheExpired(fromTable->cache)) )
  {
    return fromTable->cache.lastOutput;
  // Test if above the max axis value, clip to max data value
  } else if(axisValue >= fromTable->axis[sizeT-1]) {
    fromTable->cache.lastOutput = fromTable->values[sizeT-1];
    fromTable->cache.lastBinUpperIndex = sizeT-1;
  // Test if below the min axis value, clip to min data value
  } else if (axisValue <= fromTable->axis[0]) {
    fromTable->cache.lastOutput = fromTable->values[0];
    fromTable->cache.lastBinUpperIndex = 1U;
  } else {
    // 1st check is whether we're still in the same X bin as last time
    _table2d_detail::Bin<axis_t> xBin = _table2d_detail::Bin<axis_t>(fromTable->axis, fromTable->cache.lastBinUpperIndex);
    if (!xBin.withinBin(axisValue))
    {
      //If we're not in the same bin, search 
      xBin = _table2d_detail::findBin<axis_t, sizeT>(fromTable->axis, axisValue);
    }

    // We are exactly at the bin upper bound, so no need to interpolate
    if (axisValue==xBin.upperValue()) {
      fromTable->cache.lastOutput = fromTable->values[xBin.upperIndex];
      fromTable->cache.lastBinUpperIndex = xBin.upperIndex;
    // Must be within the bin, interpolate
    } else {
      fromTable->cache.lastOutput = _table2d_detail::interpolate(axisValue, xBin, _table2d_detail::Bin<value_t>(fromTable->values, xBin.upperIndex));
      fromTable->cache.lastBinUpperIndex = xBin.upperIndex;
    }
    // Note: we cannot be at the bin lower bound here, as that would violate the bin definition of a non-inclusive lower bound
  }
  fromTable->cache.cacheTime = _table2d_detail::getCacheTime(); //As we're not using the cache value, set the current secl value to track when this new value was calculated
  fromTable->cache.lastInput = axisValue;

  return fromTable->cache.lastOutput;  
}

// Hide use of template in the header file
using table2du8u8_4 = table2D<uint8_t, uint8_t, 4U>;
using table2di8u8_4 = table2D<int8_t, uint8_t, 4U>;
using table2du8u8_10 = table2D<uint8_t, uint8_t, 10U>;
using table2du8u8_6 = table2D<uint8_t, uint8_t, 6U>;
using table2du8u8_9 = table2D<uint8_t, uint8_t, 9U>;
using table2du8u8_8 = table2D<uint8_t, uint8_t, 8U>;
using table2du8u16_4 = table2D<uint8_t, uint16_t, 4U>;
using table2du8s16_6 = table2D<uint8_t, int16_t, 6U>;
using table2du16u16_32 = table2D<uint16_t, uint16_t, 32U>;
using table2du16u8_32 = table2D<uint16_t, uint8_t, 32U>;

#endif // TABLE_H