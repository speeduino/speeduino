#include "table3d_interpolate.h"
#include "maths.h"
#include "unit_testing.h"
#include "table2d.h"

/**
 * @file
 *
 * @brief Support functions for 3D table interpolation.
 */

/// @name Axis Bin Searching
/// @{

using table3d_bin_t = _table2d_detail::Bin<table3d_axis_t>;

/**
 * @brief Perform a linear search on an array for the bin that contains value
 * 
 * @note Assume array is ordered [max...min]
 *
 * @param pStart Pointer to the start of the array.
 * @param length Length of the array.
 * @param value Value to search for.
 * @return Upper array index of the bin
 */
TESTABLE_INLINE_STATIC table3d_dim_t linear_bin_search( const table3d_axis_t *pStart, 
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

  // Run the linear search.
  table3d_dim_t binUpperIndex = 0U;
  while (binUpperIndex!=minBinIndex && !table3d_bin_t::withinBin(value, *(pStart+binUpperIndex+1U), *(pStart+binUpperIndex)))
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

/**
 * @brief Find the bin that covers the test value.
 *
 * For example, 4 in { 1, 3, 5, 7, 9 } would be 2
 *
 * @note We assume the axis is in [max..min] order.
 *
 * @param value The value to search for.
 * @param pAxis The axis to search.
 * @param length The length of the axis.
 * @param lastBinMax The last bin max.
 * @return table3d_dim_t The axis index for the top of the bin.
 */
table3d_dim_t find_bin_max(
  const table3d_axis_t &value,
  const table3d_axis_t *pAxis,
  table3d_dim_t length,
  table3d_dim_t lastBinMax)
{
  // Check cached bin from last call to this function.
  if (table3d_bin_t::withinBin(value, *(pAxis+lastBinMax+1U), *(pAxis+lastBinMax)))
  {
    return lastBinMax;
  }

  // Note: we could check the bins above and below the lastBinMax, but this showed
  // no performance improvement in testing, so we just do a linear search.
  return linear_bin_search(pAxis, length, value);
}

/// @}

/// @name Fixed point math
/// @{

/// @brief Unsigned fixed point number type with 1 integer bit & 8 fractional bits.
/// 
/// @see https://en.wikipedia.org/wiki/Q_(number_format).
///
/// This is specialised for the number range 0..1 - a generic fixed point
/// class would miss some important optimisations. Specifically, we can avoid
/// type promotion during multiplication. 
typedef uint16_t QU1X8_t;

/** @brief Integer shift to convert to/from QU1X8_t. */
constexpr QU1X8_t QU1X8_INTEGER_SHIFT = 8;

static inline constexpr QU1X8_t toQU1X8(uint16_t base) {
  return base << QU1X8_INTEGER_SHIFT;
}
static inline constexpr uint16_t fromQU1X8(QU1X8_t base) {
  return base >> QU1X8_INTEGER_SHIFT;
}

/** @brief Precomputed value of 1 in QU1X8_t. */
TESTABLE_CONSTEXPR QU1X8_t QU1X8_ONE = toQU1X8(1U);

/** @brief Precomputed value of 0.5 in QU1X8_t. */
TESTABLE_CONSTEXPR QU1X8_t QU1X8_HALF = QU1X8_ONE/2U;

/** @brief Multiply two QU1X8_t values. */
TESTABLE_INLINE_STATIC QU1X8_t mulQU1X8(QU1X8_t a, QU1X8_t b)
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
  return fromQU1X8((a * b) + QU1X8_HALF);
}

/// @}

/// @name Interpolation
/// @{

/**
 * @brief Compute the % position of a value within a bin.
 * 
 *  - 0%==at/below the bin minimum 
 *  - 100%==at/above the bin maximum
 *  - 50%==in the middle of the bin.
 * 
 * @note The multiplier is used to scale the axis values to the same scale as the value being checked.
 * *This retains the full precision of the axis values, thus the computed position and eventually the final interpolated result*
 * 
 * @param value The value to check.
 * @param upperBinIndex The upper bin index into pAxis.
 * @param pAxis The axis array.
 * @param multiplier The multiplier for the axis values.
 * @return QU1X8_t The % position of the value within the bin.
 */
TESTABLE_INLINE_STATIC QU1X8_t compute_bin_position(const uint16_t &value, const table3d_dim_t &upperBinIndex, const table3d_axis_t *pAxis, const uint16_t &multiplier)
{
  uint16_t binMinValue = (uint16_t)pAxis[upperBinIndex+1U]*multiplier;
  if (value<=binMinValue) { return 0U; }
  uint16_t binMaxValue = (uint16_t)pAxis[upperBinIndex]*multiplier;
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

/** @brief Row and column coordinates in a 2D table */
struct row_col2d {
  table3d_dim_t row;
  table3d_dim_t col;
};

/** @brief Get the top right corner of the *value* coordinates in a 3D table, based on x/y axis coords. */
static inline row_col2d toTopRight(const xy_coord2d &axisCoords, const table3d_dim_t &axisSize)
{
  return { (table3d_dim_t)(axisCoords.y * axisSize), (table3d_dim_t)(axisSize - axisCoords.x - UINT8_C(1)) };
}

/** @brief Get the bottom left corner of the *value* coordinates in a 3D table, based on top right corner. */
static inline row_col2d toBottomLeft(const row_col2d &topRight, const table3d_dim_t &axisSize)
{
  return { (table3d_dim_t)(topRight.row + axisSize), (table3d_dim_t)(topRight.col - UINT8_C(1)) };
}

/**
 * @brief 2d interpolation, given 4 corner values and x/y percentages
 *
 * <pre>
 *  tl----------------tr
 *  |                 |
 *  |                 |
 *  |                 |
 *  |>>>>>dx>>>>?     |
 *  |           ^     |
 *  |           dy    |
 *  |           ^     |
 *  bl----------------br
 * </pre>
 * 
 * @param tl Top left value 
 * @param tr Top right value 
 * @param bl Bottom left value 
 * @param br Bottom right value 
 * @param dx X distance
 * @param dy Y distance
 * @return table3d_value_t 
 */
TESTABLE_INLINE_STATIC table3d_value_t bilinear_interpolation( const table3d_value_t &tl,
                                                      const table3d_value_t &tr,
                                                      const table3d_value_t &bl,
                                                      const table3d_value_t &br,
                                                      const QU1X8_t &dx,
                                                      const QU1X8_t &dy) {
  // Compute corner weights
  const QU1X8_t m = mulQU1X8(QU1X8_ONE-dx, dy);
  const QU1X8_t n = mulQU1X8(dx, dy);
  const QU1X8_t o = mulQU1X8(QU1X8_ONE-dx, QU1X8_ONE-dy);
  const QU1X8_t r = mulQU1X8(dx, QU1X8_ONE-dy);
  // Apply weights and shift from fixed point
  return fromQU1X8( (tl * m) + (tr * n) + (bl * o) + (br * r) );
}

/**
 * @brief Interpolate a table value from axis bins & values.
 * 
 * @param lookUpValues The x & y axis values we are interpolating
 * @param upperBinIndices The x & y axis bin indices that contain lookUpValues
 * @param axisSize The length of an axis
 * @param pValues The interpolation source values
 * @param pXAxis The x-axis
 * @param xMultiplier The x-axis multiplier
 * @param pYAxis The y-axis
 * @param yMultiplier The y-axis multiplier
 * @return table3d_value_t 
 */
table3d_value_t interpolate_3d_value(const xy_values &lookUpValues, 
                    const xy_coord2d &upperBinIndices,
                    const table3d_dim_t &axisSize,
                    const table3d_value_t *pValues,
                    const table3d_axis_t *pXAxis,
                    const uint16_t xMultiplier,
                    const table3d_axis_t *pYAxis,
                    const uint16_t yMultiplier)
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
  row_col2d tr = toTopRight(upperBinIndices, axisSize);
  row_col2d bl = toBottomLeft(tr, axisSize);

  /*
  At this point we have 2 corners of the map where the interpolated value will fall in
  Eg: ()          (tr.x,tr.y)

      (bl.x,bl.y) ()

  Translate those 2 corners into the actual values:
            A          B

            C          D
  Note that the values are stored in a 1D array, so we need to calculate the indices 
  appropriately based on the array layout.
  */
  table3d_value_t A = pValues[tr.row + bl.col];
  table3d_value_t B = pValues[tr.row + tr.col];
  table3d_value_t C = pValues[bl.row + bl.col];
  table3d_value_t D = pValues[bl.row + tr.col];  
  
  //Check that all values aren't just the same (This regularly happens with things like the fuel trim maps)
  if( (A == B) && (A == C) && (A == D) ) 
  { 
    return A;
  }
  else
  {
    //Create some normalised position values
    //These are essentially percentages (between 0 and 1) of where the desired value falls between the nearest bins on each axis
    const QU1X8_t p = compute_bin_position(lookUpValues.x, upperBinIndices.x, pXAxis, xMultiplier);
    const QU1X8_t q = compute_bin_position(lookUpValues.y, upperBinIndices.y, pYAxis, yMultiplier);
    return bilinear_interpolation(A, B, C, D, p, q);
  }
}

/// @}