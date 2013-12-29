/*
This file is used for everything related to maps/tables including their definition, functions etc
*/
#include <Arduino.h>
struct table {
  //All tables must be the same size for simplicity
  const static byte xSize = 8;
  const static byte ySize = 8;
  
  byte values[ySize][xSize];
  int axisX[xSize];
  int axisY[ySize];
  
};

/*
Tables have an origin (0,0) in the top left hand corner. Vertical axis is expressed first.
Eg: 2x2 table
-----
|2 7|
|1 4|
-----

(0,1) = 7
(0,0) = 2
(1,0) = 1

*/

//This function pulls a value from a table given a target for X and Y coordinates.
//It performs a 2D linear interpolation as descibred in: http://www.megamanual.com/v22manual/ve_tuner.pdf
int getTableValue(struct table fromTable, int Y, int X)
  {
    //Loop through the X axis bins for the min/max pair
    //Note: For the X axis specifically, rather than looping from tableAxisX[0] up to tableAxisX[max], we start at tableAxisX[Max] and go down. 
    //      This is because the important tables (fuel and injection) will have the highest RPM at the top of the X axis, so starting there will mean the best case occurs when the RPM is highest (And hence the CPU is needed most)
    int xMinValue = fromTable.axisX[0];
    int xMaxValue = fromTable.axisX[fromTable.xSize-1];
    int xMin = 0;
    int xMax = 0;
    
    //If the requested X value is greater/small than the maximum/minimum bin, reset X to be that value
    if(X > xMaxValue) { X = xMaxValue; }
    if(X < xMinValue) { X = xMinValue; }
    
    for (int x = fromTable.xSize-1; x >= 0; x--)
    {
      //Checks the case where the X value is exactly what was requested
      if ( (X == fromTable.axisX[x]) || (x == 0) )
      {
        xMaxValue = fromTable.axisX[x];
        xMinValue = fromTable.axisX[x];
        xMax = x;
        xMin = x;
        break;
      }
      //Normal case
      if ( (X <= fromTable.axisX[x]) && (X > fromTable.axisX[x-1]) )
      {
        xMaxValue = fromTable.axisX[x];
        xMinValue = fromTable.axisX[x-1];
        xMax = x;
        xMin = x-1;
        break;
      }   
    }
    
    //Loop through the Y axis bins for the min/max pair
    int yMaxValue = fromTable.axisY[0];
    int yMinValue = fromTable.axisY[fromTable.ySize-1];
    int yMin = 0;
    int yMax = 0;
    
    //If the requested Y value is greater/small than the maximum/minimum bin, reset Y to be that value
    if(Y > yMaxValue) { Y = yMaxValue; }
    if(Y < yMinValue) { Y = yMinValue; }
    
    for (int y = fromTable.ySize-1; y >= 0; y--)
    {
      //Checks the case where the Y value is exactly what was requested
      if ( (Y == fromTable.axisY[y]) || (y==0) )
      {
        yMaxValue = fromTable.axisY[y];
        yMinValue = fromTable.axisY[y];
        yMax = y;
        yMin = y;
        break;
      }
      //Normal case
      if ( (Y >= fromTable.axisY[y]) && (Y < fromTable.axisY[y-1]) )
      {
        yMaxValue = fromTable.axisY[y];
        yMinValue = fromTable.axisY[y-1];
        yMax = y;
        yMin = y-1;
        break;
      }   
    }
        
    
    /* 
    At this point we have the 4 corners of the map where the interpolated value will fall in
    Eg: (yMin,xMin)  (yMin,xMax)
    
        (yMax,xMin)  (yMax,xMax)
        
    In the following calculation the table values are referred to by the following variables:
              A          B
              
              C          D
    
    */
    int A = fromTable.values[yMin][xMin];
    int B = fromTable.values[yMin][xMax];
    int C = fromTable.values[yMax][xMin];
    int D = fromTable.values[yMax][xMax];
    
    //Create some normalised position values
    //These are essentially percentages (between 0 and 1) of where the desired value falls between the nearest bins on each axis
    
    // Float version
    /*
    float p, q;
    if (xMaxValue == xMinValue)
    { p = (float)(X-xMinValue); }
    else { p = ((float)(X - xMinValue)) / (float)(xMaxValue - xMinValue); }
    
    if (yMaxValue == yMinValue)
    { q = (float)(Y - yMinValue); }
    else { q = ((float)(Y - yMaxValue)) / (float)(yMinValue - yMaxValue); }
    
    float m = (1.0-p) * (1.0-q);
    float n = p * (1-q);
    float o = (1-p) * q;
    float r = p * q;
    
    
    return ( (A * m) + (B * n) + (C * o) + (D * r) ); 
    */
    
    // Non-Float version:
    //Initial check incase the values were hit straight on
    long p;
    if (xMaxValue == xMinValue)
    { p = ((long)(X - xMinValue) << 8); } //This only occurs if the requested X value was equal to one of the X axis bins
    else 
    { 
      p = ((long)(X - xMinValue) << 8) / (xMaxValue - xMinValue); } //This is the standard case
    
    long q;
    if (yMaxValue == yMinValue)
    { q = ((long)(Y - yMinValue) << 8); }
    else
    { q = ((long)(Y - yMaxValue) << 8) / (yMinValue - yMaxValue); }
      
    int m = ((257-p) * (257-q)) >> 8;
    int n = (p * (257-q)) >> 8;
    int o = ((257-p) * q) >> 8;
    int r = (p * q) >> 8;

    return ( (A * m) + (B * n) + (C * o) + (D * r) ) >> 8;
  }
