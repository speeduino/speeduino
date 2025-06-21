#pragma once

#include "table3d_typedefs.h"

/**
 * @file 
 * @brief Functions for interpolating values from 3D tables.
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

/** @brief Get a value from a 3D table using the specified lookup values.
 * 
 * @tparam xFactor The factor used to scale the lookup value to/from the same dimension as the axis values.
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
                    const xy_values &lookupValues);

/** @brief Specialization of get3DTableValue with x-axis scale factor of 100 and y-axis scale factor of 2. 
* 
* This is used for tables with RPM on the X axis and Load on the Y axis - which is the only case at the moment.
*/
template <>
table3d_value_t get3DTableValue<100U, 2U>(struct table3DGetValueCache *pValueCache, 
                    const table3d_dim_t axisSize,
                    const table3d_value_t *pValues,
                    const table3d_axis_t *pXAxis,
                    const table3d_axis_t *pYAxis,
                    const xy_values &lookupValues);