#include "table3d_interpolate.h"
#include "maths.h"
#include "unit_testing.h"

// ============================= Axis Bin Searching =========================

static inline bool is_in_bin(const table3d_axis_t &testValue, const table3d_axis_t &min, const table3d_axis_t &max)
{
  return testValue > min && testValue <= max;
}

/**
 * @brief Perform a linear search on a 1D array.
 * 
 * @note Assume array is ordered [max...min]
 *
 * @param pStart Pointer to the start of the array.
 * @param length Length of the array.
 * @param value Value to search for.
 * @return Upper array index
 */
TESTABLE_INLINE_STATIC table3d_dim_t linear_search( const table3d_axis_t *pStart, 
                                                    const table3d_dim_t length,
                                                    const table3d_axis_t value) 
{
  const table3d_dim_t minBinIndex = length - 2U; // The minimum bin index is the last bin before the final value

  // At or above maximum - clamp to final value
  if (value>=pStart[0U])
  {
    return 0U;
  }
  // At or below minimum - clamp to lowest value
  if (value<=pStart[length-1U])
  {
    return minBinIndex;
  }

  // No hits above, so run a linear search.
  table3d_dim_t binUpperIndex = 0U;
  while (binUpperIndex!=minBinIndex && !is_in_bin(value, *(pStart+binUpperIndex+1U), *(pStart+binUpperIndex)))
  {
    ++binUpperIndex;
  }
  return binUpperIndex;
  // Performance note: on AVR it's much quicker to increment and compare 8-bit indices (and
  // dereference pointers) than to increment and compare 16-bit pointers!
  // Up to 80 loop/sec!
  // const table3d_axis_t * const pEnd = pStart + length -1U;
  // const table3d_axis_t *pLower = pStart + 1U;
  // while ((pLower != pEnd) && !is_in_bin(value, *pLower, *(pLower-1U))) { 
  //   ++pLower;
  // }
  // return pLower - pStart - 1U;
}

// Find the axis index for the top of the bin that covers the test value.
// E.g. 4 in { 1, 3, 5, 7, 9 } would be 2
// We assume the axis is in order.
static inline table3d_dim_t find_bin_max(
  const table3d_axis_t &value,        // Value to search for
  const table3d_axis_t *pAxis,  // The axis to search
  table3d_dim_t minElement,     // Axis index of the element with the lowest value (at one end of the array)
  table3d_dim_t maxElement,     // Axis index of the element with the highest value (at the other end of the array)
  table3d_dim_t lastBinMax)     // The last result from this call - used to speed up searches
{
  // It's quicker to increment/adjust this pointer than to repeatedly 
  // index the array - minimum 2%, often >5%
  const table3d_axis_t *pMax = nullptr;
  // minElement is at one end of the array, so the "lowest" bin 
  // is [minElement, minElement+stride]. Since we're working with the upper
  // index of the bin pair, we can't go below minElement + stride.
  table3d_dim_t minBinIndex = minElement - 1U;

  // Check the cached last bin and either side first - it's likely that this will give a hit under
  // real world conditions

  // Check if we're still in the same bin as last time
  pMax = pAxis + lastBinMax;
  if (is_in_bin(value, *(pMax + 1U), *pMax))
  {
    return lastBinMax;
  }
  // Check the bin above the last one
  pMax = pMax + 1U;
  if (lastBinMax!=minBinIndex && is_in_bin(value, *(pMax + 1U), *pMax))
  {
    return lastBinMax + 1U;    
  }
  // Check the bin below the last one
  pMax -= 2U;
  if (lastBinMax!=maxElement && is_in_bin(value, *(pMax + 1U), *pMax))
  {
    return lastBinMax - 1U;
  }

  return linear_search(pAxis, minElement + 1U, value);
}

table3d_dim_t find_xbin(const table3d_axis_t &value, const table3d_axis_t *pAxis, table3d_dim_t size, table3d_dim_t lastBin)
{
  return find_bin_max(value, pAxis, size-1U, 0U, lastBin);
}

table3d_dim_t find_ybin(const table3d_axis_t &value, const table3d_axis_t *pAxis, table3d_dim_t size, table3d_dim_t lastBin)
{
  // Y axis is stored in reverse for performance purposes (not sure that's still valid). 
  // The minimum value is at the end & max at the start. So need to adjust for that. 
  return find_bin_max(value, pAxis, size-1U, 0U, lastBin);
}

// ========================= Fixed point math =========================

/**
 * @brief Unsigned fixed point number type with 1 integer bit & 8 fractional bits.
 * 
 * See https://en.wikipedia.org/wiki/Q_(number_format).
*
 * This is specialised for the number range 0..1 - a generic fixed point
 * class would miss some important optimisations. Specifically, we can avoid
 * type promotion during multiplication. 
 */
typedef uint16_t QU1X8_t;
/** @brief Integer shift to convert to/from QU1X8_t. */
static constexpr QU1X8_t QU1X8_INTEGER_SHIFT = 8;
/* @brief Precomputed value of 1 in QU1X8_t.
 * 
 * This is the value that represents 1.0 in the fixed point representation.
 */
static constexpr QU1X8_t QU1X8_ONE = (QU1X8_t)1U << QU1X8_INTEGER_SHIFT;
/* @brief Precomputed value of 0.5 in QU1X8_t. */
static constexpr QU1X8_t QU1X8_HALF = (QU1X8_t)1U << (QU1X8_INTEGER_SHIFT-1U);

/* @brief Multiply two QU1X8_t values. */
static inline QU1X8_t mulQU1X8(QU1X8_t a, QU1X8_t b)
{
    // 1x1 == 1....but the real reason for this is to avoid 16-bit multiplication overflow.
    //
    // We are using uint16_t as our underlying fixed point type. If we follow the regular
    // code path, we'd need to promote to uint32_t to avoid overflow.
    //
    // The overflow can only happen when *both* the X & Y inputs
    // are at the edge of a bin. 
    //
    // This is a rare condition, so most of the time we can use 16-bit multiplication and gain performance
    if (a==QU1X8_ONE && b==QU1X8_ONE)
    {
        return QU1X8_ONE;
    }
  // Add the equivalent of 0.5 to the final calculation pre-rounding.
  // This will have the effect of rounding to the nearest integer, rather
  // than always rounding down.
  return ((a * b) + QU1X8_HALF) >> QU1X8_INTEGER_SHIFT;
}

// ============================= Axis value to bin % =========================

static inline QU1X8_t compute_bin_position(const uint16_t value, const table3d_dim_t &bin, const table3d_axis_t *pAxis, const uint16_t &multiplier)
{
  uint16_t binMinValue = pAxis[bin+1U]*multiplier;
  if (value<=binMinValue) { return 0U; }
  uint16_t binMaxValue = pAxis[bin]*multiplier;
  if (value>=binMaxValue) { return QU1X8_ONE; }
  uint16_t binWidth = binMaxValue-binMinValue;

  // Since we can have bins of any width, we need to use 
  // 24.8 fixed point to avoid overflow
  uint16_t binPosition = value - binMinValue;
  uint32_t p = (uint32_t)binPosition << QU1X8_INTEGER_SHIFT;
  // But since we are computing the ratio (0 to 1), p is guaranteed to be
  // less than binWidth and thus the division below will result in a value
  // <=1. So we can reduce the data type from 24.8 (uint32_t) to 1.8 (uint16_t)
  return udiv_32_16(p, (uint16_t)binWidth);  
}

template <uint16_t xMultiplier, uint16_t yMultiplier>
static inline table3d_value_t interpolate_3d_value(const xy_values &lookUpValues, 
                    const xy_coord2d &axisCoords,
                    const table3d_dim_t &axisSize,
                    const table3d_value_t *pValues,
                    const table3d_axis_t *pXAxis,
                    const table3d_axis_t *pYAxis)
{
  /*
  3D Tables have an origin (0,0) in the top left hand corner. Vertical axis is expressed first.
  Eg: 2x2 table
  -----
  |2 7|
  |1 4|
  -----

  (0,1) = 7
  (0,0) = 2
  (1,0) = 1
  (1,1) = 4
  */  
  table3d_dim_t rowMax = axisCoords.y * axisSize;
  table3d_dim_t rowMin = rowMax + axisSize;
  table3d_dim_t colMax = axisSize - axisCoords.x - 1U;
  table3d_dim_t colMin = colMax - 1U;
  table3d_value_t A = pValues[rowMax + colMin];
  table3d_value_t B = pValues[rowMax + colMax];
  table3d_value_t C = pValues[rowMin + colMin];
  table3d_value_t D = pValues[rowMin + colMax];  

  /*
  At this point we have the 4 corners of the map where the interpolated value will fall in
  Eg: (yMax,xMin)  (yMax,xMax)

      (yMin,xMin)  (yMin,xMax)

  In the following calculation the table values are referred to by the following variables:
            A          B

            C          D
  */
  
  //Check that all values aren't just the same (This regularly happens with things like the fuel trim maps)
  if( (A == B) && (A == C) && (A == D) ) 
  { 
    return A;
  }
  else
  {
    //Create some normalised position values
    //These are essentially percentages (between 0 and 1) of where the desired value falls between the nearest bins on each axis
    const QU1X8_t p = compute_bin_position(lookUpValues.x, axisCoords.x, pXAxis, xMultiplier);
    const QU1X8_t q = compute_bin_position(lookUpValues.y, axisCoords.y, pYAxis, yMultiplier);

    const QU1X8_t m = mulQU1X8(QU1X8_ONE-p, q);
    const QU1X8_t n = mulQU1X8(p, q);
    const QU1X8_t o = mulQU1X8(QU1X8_ONE-p, QU1X8_ONE-q);
    const QU1X8_t r = mulQU1X8(p, QU1X8_ONE-q);
    return ( (A * m) + (B * n) + (C * o) + (D * r) ) >> QU1X8_INTEGER_SHIFT;
  }
}

// ============================= End internal support functions =========================

template <>
table3d_value_t get3DTableValue<100U, 2U>(struct table3DGetValueCache *pValueCache, 
                    table3d_dim_t axisSize,
                    const table3d_value_t *pValues,
                    const table3d_axis_t *pXAxis,
                    const table3d_axis_t *pYAxis,
                    const xy_values &lookupValues)
{
  // (x|y)Factor
  // -----------
  // These are multipliers used to convert the lookup values to the same
  // scale as the axis values. The axes are sent from TunerStudio compressed 
  // into a byte. E.g. RPM is stored /100 (so 2500->25) in the table x-axis.
  // We do *not* want to divide the x-axis lookup value by 100, as that would
  // result in a loss of fidelity when interpolating the x-axis position (see
  // compute_bin_position). Instead, we:
  // 1. Divide x-axis lookup value when searching for the axis bin
  // 2. Multiply the x-axis values when interpolating the x-axis position

  // Check if the lookup values are the same as the last time we looked up a value
  // If they are, we can return the cached value
  if( lookupValues == pValueCache->last_lookup)
  {
    return pValueCache->lastOutput;
  }

  // Figure out where on the axes the incoming coord are
  pValueCache->lastBinMax.x = find_xbin(div100(lookupValues.x), pXAxis, axisSize, pValueCache->lastBinMax.x);
  pValueCache->lastBinMax.y = find_ybin(lookupValues.y>>1U, pYAxis, axisSize, pValueCache->lastBinMax.y);
  // Interpolate based on the bin positions
  pValueCache->lastOutput = interpolate_3d_value<100U, 2U>(lookupValues, pValueCache->lastBinMax, axisSize, pValues, pXAxis, pYAxis);
  // Store the last lookup values so we can check them next time
  pValueCache->last_lookup = lookupValues;

  return pValueCache->lastOutput;
}
