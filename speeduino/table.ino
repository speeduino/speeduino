/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

/*
Because the size of the table is dynamic, this functino is required to reallocate the array sizes
Note that this may clear some of the existing values of the table
*/
#include "table.h"
#include "globals.h"

/*
void table2D_setSize(struct table2D* targetTable, byte newSize)
{
  //Table resize is ONLY permitted during system initialisation.
  //if(initialisationComplete == false)
  {
    //2D tables can contain either bytes or ints, depending on the value of the valueSize field
    if(targetTable->valueSize == SIZE_BYTE)
    {
      //The following lines have MISRA suppressions as realloc is otherwise forbidden. These calls have been verified as unable to be executed from anywhere but controlled areas. 
      //cppcheck-suppress misra-21.3
      targetTable->values = (byte *)realloc(targetTable->values, newSize * sizeof(byte)); //cppcheck-suppress misra_21.3
      targetTable->axisX = (byte *)realloc(targetTable->axisX, newSize * sizeof(byte));
      targetTable->xSize = newSize;
    }
    else
    {
      targetTable->values16 = (int16_t *)realloc(targetTable->values16, newSize * sizeof(int16_t));
      targetTable->axisX16 = (int16_t *)realloc(targetTable->axisX16, newSize * sizeof(int16_t));
      targetTable->xSize = newSize;
    } //Byte or int
  } //initialisationComplete
}
*/

static uint8_t _3DTable_heap[TABLE_HEAP_SIZE];
static uint16_t _heap_pointer = 0;

void* heap_alloc(uint16_t size)
 {
     uint8_t* value = nullptr;
     if (size < (TABLE_HEAP_SIZE - _heap_pointer))
     {
         value = &_3DTable_heap[_heap_pointer];
         _heap_pointer += size;
     }
     return value;
 }


void table3D_setSize(struct table3D *targetTable, byte newSize)
{
  if(initialisationComplete == false)
  {
    /*
    targetTable->values = (byte **)malloc(newSize * sizeof(byte*));
    for(byte i = 0; i < newSize; i++) { targetTable->values[i] = (byte *)malloc(newSize * sizeof(byte)); }
    */
    targetTable->values = (byte **)heap_alloc(newSize * sizeof(byte*));
    for(byte i = 0; i < newSize; i++) { targetTable->values[i] = (byte *)heap_alloc(newSize * sizeof(byte)); }

    /*
    targetTable->axisX = (int16_t *)malloc(newSize * sizeof(int16_t));
    targetTable->axisY = (int16_t *)malloc(newSize * sizeof(int16_t));
    */
    targetTable->axisX = (int16_t *)heap_alloc(newSize * sizeof(int16_t));
    targetTable->axisY = (int16_t *)heap_alloc(newSize * sizeof(int16_t));
    targetTable->xSize = newSize;
    targetTable->ySize = newSize;
    targetTable->cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
  } //initialisationComplete
}

/*
This function pulls a 1D linear interpolated (ie averaged) value from a 2D table
ie: Given a value on the X axis, it returns a Y value that coresponds to the point on the curve between the nearest two defined X values

This function must take into account whether a table contains 8-bit or 16-bit values.
Unfortunately this means many of the lines are duplicated depending on this
*/
int table2D_getValue(struct table2D *fromTable, int X_in)
{
  //Orig memory usage = 5414
  int returnValue = 0;
  bool valueFound = false;

  int X = X_in;
  int xMinValue, xMaxValue;
  int xMin = 0;
  int xMax = fromTable->xSize-1;

  //Check whether the X input is the same as last time this ran
  if( (X_in == fromTable->lastInput) && (fromTable->cacheTime == currentStatus.secl) )
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
  else if(X <= table2D_getAxisValue(fromTable, xMin))
  {
    returnValue = table2D_getRawValue(fromTable, xMin);
    valueFound = true;
  }
  //Finally if none of that is found
  else
  {
    fromTable->cacheTime = currentStatus.secl; //As we're not using the cache value, set the current secl value to track when this new value was calc'd

    //1st check is whether we're still in the same X bin as last time
    xMaxValue = table2D_getAxisValue(fromTable, fromTable->lastXMax);
    xMinValue = table2D_getAxisValue(fromTable, fromTable->lastXMin);
    if ( (X <= xMaxValue) && (X > xMinValue) )
    {
      xMax = fromTable->lastXMax;
      xMin = fromTable->lastXMin;
    }
    else
    {
      //If we're not in the same bin, loop through to find where we are
      xMaxValue = table2D_getAxisValue(fromTable, fromTable->xSize-1); // init xMaxValue in preparation for loop.
      for (int x = fromTable->xSize-1; x > 0; x--)
      {
        xMinValue = table2D_getAxisValue(fromTable, x-1); // fetch next Min

        //Checks the case where the X value is exactly what was requested
        if (X == xMaxValue)
        {
          returnValue = table2D_getRawValue(fromTable, x); //Simply return the coresponding value
          valueFound = true;
          break;
        }
        else if (X > xMinValue)
        {
          // Value is in the current bin
          xMax = x;
          fromTable->lastXMax = xMax;
          xMin = x-1;
          fromTable->lastXMin = xMin;
          break;
        }
        // Otherwise, continue to next bin
        xMaxValue = xMinValue; // for the next bin, our Min is their Max
      }
    }
  } //X_in same as last time

  if (valueFound == false)
  {
    int16_t m = X - xMinValue;
    int16_t n = xMaxValue - xMinValue;

    int16_t yMax = table2D_getRawValue(fromTable, xMax);
    int16_t yMin = table2D_getRawValue(fromTable, xMin);

    /* Float version (if m, yMax, yMin and n were float's)
       int yVal = (m * (yMax - yMin)) / n;
    */
    
    //Non-Float version
    int16_t yVal = ( ((int32_t) m) * (yMax-yMin) ) / n;
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
int16_t table2D_getAxisValue(struct table2D *fromTable, byte X_in)
{
  int returnValue = 0;

  if(fromTable->axisSize == SIZE_INT) { returnValue = ((int16_t*)fromTable->axisX)[X_in]; }
  else if(fromTable->axisSize == SIZE_BYTE) { returnValue = ((uint8_t*)fromTable->axisX)[X_in]; }

  return returnValue;
}

/**
 * @brief Returns an value from the 2D table given an index value. No interpolation is performed
 * 
 * @param fromTable 
 * @param X_in 
 * @return int16_t 
 */
int16_t table2D_getRawValue(struct table2D *fromTable, byte X_index)
{
  int returnValue = 0;

  if(fromTable->valueSize == SIZE_INT) { returnValue = ((int16_t*)fromTable->values)[X_index]; }
  else if(fromTable->valueSize == SIZE_BYTE) { returnValue = ((uint8_t*)fromTable->values)[X_index]; }

  return returnValue;
}


//This function pulls a value from a 3D table given a target for X and Y coordinates.
//It performs a 2D linear interpolation as descibred in: www.megamanual.com/v22manual/ve_tuner.pdf
int get3DTableValue(struct table3D *fromTable, int Y_in, int X_in)
{
  //0th check is whether the same X and Y values are being sent as last time. If they are, this not only prevents a lookup of the axis, but prevents the interpolation calcs being performed
  if( (X_in == fromTable->lastXInput) && (Y_in == fromTable->lastYInput) && (fromTable->cacheIsValid == true))
  {
    return fromTable->lastOutput;
  }

  byte xMin, xMax;
  byte yMin, yMax;
  int tableResult = 0;
  int now, start, end;

	if (X_in <= fromTable->axisX[0]) { xMin = xMax = 0; }
	else if (X_in >= fromTable->axisX[fromTable->xSize-1]) { xMin = xMax = fromTable->xSize-1; }
  else
  {
    start = 0;
    end = fromTable->xSize-1;
    if(fromTable->lastXMin != 0) { now = fromTable->lastXMin; }
    else { now = end >> 1; }

    while(true)
    {
      if (now == start) { break; }
      if (X_in < fromTable->axisX[now]) { end = now; }
      else if (X_in > fromTable->axisX[now]) { start = now; }
      else { break;	}

      now = (start + end) >> 1;
    }
    xMin = xMax = now;
    if((X_in != fromTable->axisX[xMin]) && (xMin != fromTable->xSize-1)) { xMax++; }
  }

	if (Y_in <= fromTable->axisY[fromTable->ySize-1]) { yMin = yMax = fromTable->ySize-1; }
	else if (Y_in >= fromTable->axisY[0]) { yMin = yMax = 0; }
  else
  {
    start = 0;
    end = fromTable->ySize-1;
    if(fromTable->lastYMin != 0) { now = fromTable->lastYMin; }
    else { now = end >> 1; }

    while(true)
    {
      if (now == start) { break; }
      if (Y_in < fromTable->axisY[now]) { start = now; }
      else if (Y_in > fromTable->axisY[now]) { end = now; }
      else { break;	}

      now = (start + end) >> 1;
    }
    yMin = yMax = now;
    if((Y_in != fromTable->axisY[yMin]) && (yMin != fromTable->ySize-1)) { yMax++; }
  }

  fromTable->lastXMin = xMin;
  fromTable->lastYMin = yMin;

	if(((xMin == fromTable->ySize - 1) && (yMin == fromTable->ySize - 1)) || ((xMin == xMax) && (yMin == yMax)))
  {
		tableResult = fromTable->values[yMin][xMin];
	}
  else
  {
    /*
    At this point we have the 4 corners of the map where the interpolated value will fall in
    Eg: (yMin,xMin)  (yMin,xMax)
        (yMax,xMin)  (yMax,xMax)

    In the following calculation the table values are referred to by the following variables:
              A          B
              C          D
    */
    int A = fromTable->values[yMin][xMin];
    int B = fromTable->values[yMin][xMax];
    int C = fromTable->values[yMax][xMin];
    int D = fromTable->values[yMax][xMax];

    //Check that all values aren't just the same (This regularly happens with things like the fuel trim maps)
    if( (A == B) && (A == C) && (A == D) ) { tableResult = A; }
    else if(xMin == xMax) // Same RPM axys do a simple 2D (27% faster on Mega on worst case, usually 60%)
    {
      int yMaxValue = fromTable->axisY[yMax];
      int yMinValue = fromTable->axisY[yMin];
      long q = (((long)(Y_in - yMinValue) << 6)  * (C - A)) / (yMaxValue - yMinValue);
      tableResult = A + (q >> 6);
    }
    else if(yMin == yMax) // Same MAP axys do a simple 2D (27% faster on Mega on worst case, usually 60%)
    {
      int xMaxValue = fromTable->axisX[xMax];
      int xMinValue = fromTable->axisX[xMin];
      long q = (((long)(X_in - xMinValue) << 6) * (B - A)) / (xMaxValue - xMinValue);
      tableResult = A + (q >> 6);
    }
    else
    {
      int yMaxValue = fromTable->axisY[yMax];
      int yMinValue = fromTable->axisY[yMin];
      int xMaxValue = fromTable->axisX[xMax];
      int xMinValue = fromTable->axisX[xMin];

      //Create some normalised position values
      //These are essentially percentages (between 0 and 1) of where the desired value falls between the nearest bins on each axis
      unsigned long p = (long)X_in - xMinValue;
      if (xMaxValue == xMinValue) { p = (p << TABLE_SHIFT_FACTOR); }  //This only occurs if the requested X value was equal to one of the X axis bins
      else { p = ( (p << TABLE_SHIFT_FACTOR) / (xMaxValue - xMinValue) ); } //This is the standard case

      unsigned long q;
      if (yMaxValue == yMinValue)
      {
        q = (long)Y_in - yMinValue;
        q = (q << TABLE_SHIFT_FACTOR);
      }
      //Standard case
      else
      {
        q = long(Y_in) - yMaxValue;
        q = TABLE_SHIFT_POWER - ( (q << TABLE_SHIFT_FACTOR) / (yMinValue - yMaxValue) );
      }
      uint32_t m = (TABLE_SHIFT_POWER-p) * (TABLE_SHIFT_POWER-q);
      uint32_t n = p * (TABLE_SHIFT_POWER-q);
      uint32_t o = (TABLE_SHIFT_POWER-p) * q;
      uint32_t r = p * q;
      uint32_t sum = ( ((A * m)) + (B * n) + (C * o) + (D * r) );
      tableResult = ((int16_t*)&sum)[1]; //ugly hack to skip 16 shifts ;)
    }
  }
  //Update the tables cache data
  fromTable->lastXInput = X_in;
  fromTable->lastYInput = Y_in;
  fromTable->lastOutput = tableResult;
  fromTable->cacheIsValid = true;

 return tableResult;
 }
