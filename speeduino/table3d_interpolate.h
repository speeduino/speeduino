#pragma once

#include "table3d_typedefs.h"

// A table location. 
struct coord2d
{
    table3d_axis_t x;
    table3d_axis_t y;
};

// Table axis bin. Stores 2 axis *indices* that define a range
struct table3d_bin_t
{
    table3d_dim_t min;
    table3d_dim_t max;
};

struct table3DGetValueCache {
  // Store the last X and Y coordinates in the table. This is used to make the next check faster
  table3d_bin_t lastXBins;
  table3d_bin_t lastYBins;

  //Store the last input and output values, again for caching purposes
  coord2d last_lookup = { INT16_MAX, INT16_MAX };
  table3d_value_t lastOutput;
};


inline void invalidate_cache(table3DGetValueCache *pCache)
{
    pCache->last_lookup.x = INT16_MAX;
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