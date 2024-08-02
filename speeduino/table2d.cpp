/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

/*
Because the size of the table is dynamic, this function is required to reallocate the array sizes
Note that this may clear some of the existing values of the table
*/
#include "table2d.h"
#if !defined(UNIT_TEST)
#include "globals.h"
#endif

#define TYPE_INT8    1U
#define TYPE_UINT8   2U
#define TYPE_UINT16  3U

static void _construct2dTable(table2D &table, uint8_t valueType, uint8_t axisType, uint8_t length, const void *values, const void *bins) {
  table.valueType = valueType;
  table.axisType = axisType;
  table.length = length;
  table.values = values;
  table.axisX = bins;
  table.lastInput = INT16_MAX;
  table.lastBinUpperIndex = 1U;
}

void _construct2dTable(table2D &table, uint8_t length, const uint8_t *values, const uint8_t *bins) {
  _construct2dTable(table, TYPE_UINT8, TYPE_UINT8, length, values, bins);
}
void _construct2dTable(table2D &table, uint8_t length, const uint8_t *values, const int8_t *bins) {
  _construct2dTable(table, TYPE_UINT8, TYPE_INT8, length, values, bins);
}
void _construct2dTable(table2D &table, uint8_t length, const uint16_t *values, const uint16_t *bins) {
  _construct2dTable(table, TYPE_UINT16, TYPE_UINT16, length, values, bins);
}
void _construct2dTable(table2D &table, uint8_t length, const uint8_t *values, const uint16_t *bins) {
  _construct2dTable(table, TYPE_UINT8, TYPE_UINT16, length, values, bins);
}
void _construct2dTable(table2D &table, uint8_t length, const uint16_t *values, const uint8_t *bins) {
  _construct2dTable(table, TYPE_UINT16, TYPE_UINT8, length, values, bins);
}
void _construct2dTable(table2D &table, uint8_t length, const int16_t *values, const uint8_t *bins) {
  _construct2dTable(table, TYPE_UINT16, TYPE_UINT8, length, values, bins);
}

static inline uint8_t getCacheTime(void) {
#if !defined(UNIT_TEST)
  return currentStatus.secl;
#else
  return 0;
#endif
}

/*
This function pulls a 1D linear interpolated (ie averaged) value from a 2D table
ie: Given a value on the X axis, it returns a Y value that corresponds to the point on the curve between the nearest two defined X values

This function must take into account whether a table contains 8-bit or 16-bit values.
Unfortunately this means many of the lines are duplicated depending on this
*/
int16_t table2D_getValue(struct table2D *fromTable, int16_t X_in)
{
  //Orig memory usage = 5414
  int16_t returnValue = 0;
  bool valueFound = false;

  int16_t X = X_in;
  int16_t xMinValue, xMaxValue;
  uint8_t xMax = fromTable->length-1U;

  //Check whether the X input is the same as last time this ran
  if( (X_in == fromTable->lastInput) && (fromTable->cacheTime == getCacheTime()) )
  {
    returnValue = fromTable->lastOutput;
    valueFound = true;
  }
  //If the requested X value is greater/small than the maximum/minimum bin, simply return that value
  else if(X >= table2D_getAxisValue(fromTable, xMax))
  {
    returnValue = table2D_getRawValue(fromTable, xMax);
    valueFound = true;
  }
  else if(X <= table2D_getAxisValue(fromTable, 0U))
  {
    returnValue = table2D_getRawValue(fromTable, 0U);
    valueFound = true;
  }
  //Finally if none of that is found
  else
  {
    fromTable->cacheTime = getCacheTime(); //As we're not using the cache value, set the current secl value to track when this new value was calculated

    //1st check is whether we're still in the same X bin as last time
    xMaxValue = table2D_getAxisValue(fromTable, fromTable->lastBinUpperIndex);
    xMinValue = table2D_getAxisValue(fromTable, fromTable->lastBinUpperIndex-1U);
    if ( (X <= xMaxValue) && (X > xMinValue) )
    {
      xMax = fromTable->lastBinUpperIndex;
    }
    else
    {
      //If we're not in the same bin, loop through to find where we are
      xMaxValue = table2D_getAxisValue(fromTable, fromTable->length-1U); // init xMaxValue in preparation for loop.
      for (uint8_t x = fromTable->length-1U; x > 0U; x--)
      {
        xMinValue = table2D_getAxisValue(fromTable, x-1U); // fetch next Min

        //Checks the case where the X value is exactly what was requested
        if (X == xMaxValue)
        {
          returnValue = table2D_getRawValue(fromTable, x); //Simply return the corresponding value
          valueFound = true;
          break;
        }
        else if (X > xMinValue)
        {
          // Value is in the current bin
          xMax = x;
          fromTable->lastBinUpperIndex = xMax;
          break;
        } else {
          // Otherwise, continue to next bin
          xMaxValue = xMinValue; // for the next bin, our Min is their Max
        }
      }
    }
  } //X_in same as last time

  if (valueFound == false)
  {
    int32_t m = (int32_t)X - (int32_t)xMinValue;
    int32_t n = (int32_t)xMaxValue - (int32_t)xMinValue;

    int16_t yMax = table2D_getRawValue(fromTable, xMax);
    int16_t yMin = table2D_getRawValue(fromTable, xMax-1U);
    int32_t yRange = (int32_t) yMax - (int32_t) yMin;

    /* Float version (if m, yMax, yMin and n were float's)
       int yVal = (m * (yMax - yMin)) / n;
    */
    
    //Non-Float version
    int16_t yVal = (int16_t)(( m * yRange ) / n);
    returnValue = yMin + yVal;
  }

  fromTable->lastInput = X_in;
  fromTable->lastOutput = returnValue;

  return returnValue;
}

/**
 * @brief Returns an axis (bin) value from the 2D table. This works regardless of whether that axis is bytes or int16_ts
 * 
 * @param fromTable 
 * @param X_in 
 * @return int16_t 
 */
int16_t table2D_getAxisValue(struct table2D *fromTable, uint8_t X_in)
{
  int returnValue = 0;

  if(fromTable->axisType == TYPE_UINT16) { returnValue = ((int16_t*)fromTable->axisX)[X_in]; }
  else if(fromTable->axisType == TYPE_UINT8) { returnValue = ((uint8_t*)fromTable->axisX)[X_in]; }
  else if(fromTable->axisType == TYPE_INT8) { returnValue = ((int8_t*)fromTable->axisX)[X_in]; }
  else { /* Keep MISRA checker happy*/ }  

  return returnValue;
}

/**
 * @brief Returns an value from the 2D table given an index value. No interpolation is performed
 * 
 * @param fromTable 
 * @param X_index 
 * @return int16_t 
 */
int16_t table2D_getRawValue(struct table2D *fromTable, uint8_t X_index)
{
  int returnValue = 0;

  if(fromTable->valueType == TYPE_UINT16) { returnValue = ((int16_t*)fromTable->values)[X_index]; }
  else if(fromTable->valueType == TYPE_UINT8) { returnValue = ((uint8_t*)fromTable->values)[X_index]; }
  else if(fromTable->valueType == TYPE_INT8) { returnValue = ((int8_t*)fromTable->values)[X_index]; }
  else { /* Keep MISRA checker happy*/ }  

  return returnValue;
}
