#pragma once

#include <Arduino.h>

// Overview
//
// Logical: each 3D table is a continuous height map spread over
// 2 orthogonal and bounded axes. The X & Y axes are non-linear.
// E.g. for a 3x3 table
//      Y-Max    V6      V7      V8
//      Y-Int    V3      V4      V5
//      Y-Min    V0      V1      V2
//              X-Min   X-Int   X-Max
//  Assumptions:
//      The axis values are increasing. I.e. x[n] >= x[n-1]
//
// Physical:
//      The X axis is conventional: [0] is the min
//      The Y-axis is inverted: y[0] is the max
//      The value locations match the axes. E.g.for a 3x3 table:
//          value[0][0] is the first value on the last row.
//          value[2][0] ia the first value on the first row.

// Table typedefs - for consistency
typedef byte table3d_dim_t;      // The x,y axis size & indices fit in this type
typedef byte table3d_value_t;    // The type of each table value
typedef int16_t table3d_axis_t;  // The type of each axis element

struct table3DGetValueCache {
  //Store the last X and Y coordinates in the table. This is used to make the next check faster
  byte lastXMax, lastXMin;
  byte lastYMax, lastYMin;

  //Store the last input and output values, again for caching purposes
  int16_t lastXInput, lastYInput;
  byte lastOutput; // This will need changing if we ever have 16-bit table values
  bool cacheIsValid; // This tracks whether the tables cache should be used. Ordinarily this is true, but is set to false whenever TunerStudio sends a new value for the table
};

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
int get3DTableValue(struct table3DGetValueCache *pTable, 
                    table3d_dim_t axisSize,
                    const table3d_value_t * const *pValues,
                    const table3d_axis_t *pXAxis,
                    const table3d_axis_t *pYAxis,
                    int y, int x);