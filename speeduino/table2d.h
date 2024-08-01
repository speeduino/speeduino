/*
This file is used for everything related to maps/tables including their definition, functions etc
*/
#ifndef TABLE_H
#define TABLE_H

#include "globals.h"

#define SIZE_SIGNED_BYTE    4
#define SIZE_BYTE           8
#define SIZE_INT            16

/**
 * @brief A polymorphic 2D table.
 * 
 * The table is designed to be used with the table2D_getValue function to interpolate values from a 2D table.
 *  * Construct by calling construct2dTable
 *    * The table is defined by providing the axis and value arrays 
 *    * The axis and value arrays must be the same length
 *  * The axis array **must** be sorted
 *  * The axis and values can be any integral type up to 16-bits wide.
 *    * Signed or unsigned.
 */
struct table2D {
  //Used 5414 RAM with original version
  uint8_t valueSize;
  uint8_t axisSize;
  uint8_t xSize;

  const void *values;
  const void *axisX;

  //int16_t *values16;
  //int16_t *axisX16;

  // Store the upper index of the bin we last found. This is used to make the next check faster
  // Since this is the *upper* index, it can never be 0.
  uint8_t lastBinUpperIndex;

  //Store the last input and output for caching
  int16_t lastInput;
  int16_t lastOutput;
  uint8_t cacheTime; //Tracks when the last cache value was set so it can expire after x seconds. A timeout is required to pickup when a tuning value is changed, otherwise the old cached value will continue to be returned as the X value isn't changing. 
};

/// @cond
// private to construct2dTable
void _construct2dTable(table2D &table, uint8_t length, const uint8_t *values, const uint8_t *bins);
void _construct2dTable(table2D &table, uint8_t length, const uint8_t *values, const int8_t *bins);
void _construct2dTable(table2D &table, uint8_t length, const uint16_t *values, const uint16_t *bins);
void _construct2dTable(table2D &table, uint8_t length, const uint8_t *values, const uint16_t *bins);
void _construct2dTable(table2D &table, uint8_t length, const uint16_t *values, const uint8_t *bins);
void _construct2dTable(table2D &table, uint8_t length, const int16_t *values, const uint8_t *bins);
/// @endcond

/**
 * @brief Wire up the 2D table struct to the axis (aka bins) and value arrays 
 * 
 * The 2 arrays must be the same length
 *
 * @tparam axis_t Integral type of the axis. E.g. uint8_t
 * @tparam value_t Integral type of the values. E.g. uint8_t
 * @tparam TSize Size of the arrays
 * @param table The table to wire up
 * @param values Array of values
 * @param bins Array of axis values
 */
template <typename axis_t, typename value_t, uint8_t TSize>
void construct2dTable(table2D &table, const value_t (&values)[TSize], const axis_t (&bins)[TSize]) {
  _construct2dTable(table, TSize, values, bins);
}

int16_t table2D_getAxisValue(struct table2D *fromTable, uint8_t X_in);
int16_t table2D_getRawValue(struct table2D *fromTable, uint8_t X_index);

int16_t table2D_getValue(struct table2D *fromTable, int16_t X_in);

#endif // TABLE_H
