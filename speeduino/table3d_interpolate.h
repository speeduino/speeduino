#pragma once

#include "table3d_typedefs.h"

struct table3DGetValueCache {
  //Store the last X and Y coordinates in the table. This is used to make the next check faster
  table3d_dim_t lastXMax, lastXMin;
  table3d_dim_t lastYMax, lastYMin;

  //Store the last input and output values, again for caching purposes
  table3d_axis_t lastXInput = INT16_MAX, lastYInput;
  table3d_value_t lastOutput; // This will need changing if we ever have 16-bit table values
};

inline void invalidate_cache(table3DGetValueCache *pCache)
{
    pCache->lastXInput = INT16_MAX;
}

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

*/
table3d_value_t get3DTableValue(struct table3DGetValueCache *pValueCache, 
                    table3d_dim_t axisSize,
                    const table3d_value_t *pValues,
                    const table3d_axis_t *pXAxis,
                    const table3d_axis_t *pYAxis,
                    table3d_axis_t y, table3d_axis_t x);