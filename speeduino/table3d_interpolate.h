#pragma once

#include "table3d_typedefs.h"
#include "maths.h"

/**
 * @file 
 * @brief Functions for interpolating values from 3D tables. @see get3DTableValue
 */

/** @brief A pair of x and y values used for lookups in 3D tables. */
struct xy_values
{
  uint16_t x;
  uint16_t y;
};

/** @brief Equality operator for xy_values */
static inline bool operator==(const xy_values& lhs, const xy_values& rhs)
{
    return lhs.x==rhs.x && lhs.y==rhs.y;
}

/** @brief 2D coordinate structure for table lookups */
struct xy_coord2d
{
  /** @brief X axis coordinate */
  table3d_dim_t x;
  /** @brief Y axis coordinate */
  table3d_dim_t y;
};

/** @brief Cache structure for 3D table value lookups. */
struct table3DGetValueCache {
  // Store the upper *index* of the X and Y axis bins that were last hit.
  // This is used to make the next check faster since very likely the x & y values have
  // only changed by a small amount & are in the same bin (or an adjacent bin).
  //
  // It's implicit that the other bin index is max bin index - 1 (a single axis
  // value can't span 2 axis bins). This saves 1 byte.
  //
  // E.g. 6 element x-axis contents:
  //   [ 8| 9|12|15|18|21]
  // indices:
  //     0, 1, 2, 3, 4, 5
  // If lastXBinMax==3, the min index must be 2. I.e. the last X value looked
  // up was between 12<X<=15.
  xy_coord2d lastBinMax = { 1U, 1U };

  //Store the last input and output values, again for caching purposes
  xy_values last_lookup = { UINT16_MAX, UINT16_MAX };
  table3d_value_t lastOutput;
};

/** @brief Invalidate the cache by resetting the last lookup values. */
static inline void invalidate_cache(table3DGetValueCache *pCache)
{
    pCache->last_lookup.x = UINT16_MAX;
}

/// @cond
// private to table3D implementation

extern table3d_dim_t find_bin_max(
  const table3d_axis_t &value,
  const table3d_axis_t *pAxis,
  table3d_dim_t length,
  table3d_dim_t lastBinMax);

extern table3d_value_t interpolate_3d_value(const xy_values &lookUpValues, 
                    const xy_coord2d &axisCoords,
                    const table3d_dim_t &axisSize,
                    const table3d_value_t *pValues,
                    const table3d_axis_t *pXAxis,
                    const uint16_t xMultiplier,
                    const table3d_axis_t *pYAxis,
                    const uint16_t yMultiplier);

/// @endcond

/** @brief Get a value from a 3D table using the specified lookup values.
 *
 * Conceptually, a 3d table consists of a 2D table (rows & columns) and 2 axes. E.g. 6x6 table (tables are always square):
 *  - X: x axis value (e.g. RPM)
 *  - Y: y axis value (e.g. Load)
 *  - V: values of a 2d table (e.g. VE)
 * <pre>
 * YMax | V  V  V  V  V  V
 * Y    | V  V  V  V  V  V
 * Y    | V  V  V  V  V  V
 * Y    | V  V  V  V  V  V
 * Y    | V  V  V  V  V  V
 * YMin | V  V  V  V  V  V
 * -----+-------------------
 *      | X  X  X  X  X  X
 *      |Min            Max
 * </pre>
 * 
 * Our overall task is to accurately interpolate a value from the table, given X and Y axis values.
 * The x/y values will likely be in-between axis values. The function performs a 2D linear interpolation 
 * as described in: www.megamanual.com/v22manual/ve_tuner.pdf
 *
 * @note [x|y]Factor are multipliers used to convert the lookup values to the same
 * scale as the axis values.
 * @note Background: the axes are sent from TunerStudio compressed into a byte.
 * E.g. RPM is stored /100 (I.e. 2500->25) in the table x-axis. We can save a lot of SRAM
 * by *not* rehydrating the axis values. However, we do *not* want to simply divide the axis lookup value
 * by 100, as that would result in a loss of fidelity *when interpolating the x-axis position*: see compute_bin_position()
 * @note Instead, we:
 * 1. Divide the axis *lookup value* when searching for the axis bin (no loss of fidelity, since we're comparing bin thresholds). E.g RPM of 2153/100 -> 22
 * 2. Multiply the *axis values* when interpolating the axis position (retain fidelity). E.g. bin [20,25] becomes [2000,2500] which gives a bin position of 31% (instead of 40%)
 * 
 * @tparam xFactor The factor used to scale the lookup value to/from the same units as the axis values.
 * @tparam yFactor The factor for the Y axis values.
 * @param pValueCache Pointer to the value cache structure.
 * @param axisSize The size of the axis.
 * @param pValues Pointer to the table values.
 * @param pXAxis Pointer to the X axis array.
 * @param pYAxis Pointer to the Y axis array.
 * @param lookupValues The X axis and Y axis values to look up.
 * @return The interpolated value from the table.
 */
template <uint16_t xFactor, uint16_t yFactor>
table3d_value_t get3DTableValue(struct table3DGetValueCache *pValueCache, 
                    const table3d_dim_t axisSize,
                    const table3d_value_t *pValues,
                    const table3d_axis_t *pXAxis,
                    const table3d_axis_t *pYAxis,
                    const xy_values &lookupValues) {
  

  // Check if the lookup values are the same as the last time we looked up a value
  // If they are, we can return the cached value
  if( lookupValues == pValueCache->last_lookup)
  {
    return pValueCache->lastOutput;
  }

  // Figure out where on the axes the incoming coord are
  pValueCache->lastBinMax.x = find_bin_max(div_round_closest_u16<xFactor>(lookupValues.x), pXAxis, axisSize, pValueCache->lastBinMax.x);
  pValueCache->lastBinMax.y = find_bin_max(div_round_closest_u16<yFactor>(lookupValues.y), pYAxis, axisSize, pValueCache->lastBinMax.y);
  // Interpolate based on the bin positions
  pValueCache->lastOutput = interpolate_3d_value(lookupValues, pValueCache->lastBinMax, axisSize, pValues, pXAxis, xFactor, pYAxis, yFactor);
  // Store the last lookup values so we can check them next time
  pValueCache->last_lookup = lookupValues;

  return pValueCache->lastOutput;

}
