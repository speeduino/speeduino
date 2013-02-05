/*
This file is used for everything related to maps/tables including their definition, functions etc
*/

struct table {
  //All tables must be the same size for simplicity
  const static int xSize = 8;
  const static int ySize = 8;
  
  int values[ySize][xSize];
  //int axisX[xSize];
  int axisX[8];
  int axisY[8];
  //static boolean useInterp = false; //Whether or not interpolation should be used (Assuming we have enough CPU for it)
  
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
    int xMin = fromTable.axisX[0];
    int xMax = fromTable.axisX[fromTable.xSize-1];
    int xMinValue = 0;
    int xMaxValue = 0;    
    
    for (int x = fromTable.xSize-1; x > 0; x--)
    {
      if ( (X <= fromTable.axisX[x]) && (X >= fromTable.axisX[x-1]) )
      {
        xMaxValue = fromTable.axisX[x];
        xMinValue = fromTable.axisX[x-1];
        xMax = x;
        xMin = x-1;
        break;
      }   
    }
    
    //Loop through the Y axis bins for the min/max pair
    int yMin = fromTable.axisY[0];
    int yMax = fromTable.axisY[fromTable.ySize-1];
    int yMinValue = 0;
    int yMaxValue = 0;
    
    for (int y = fromTable.ySize-1; y > 0; y--)
    {
      if ( (Y >= fromTable.axisY[y]) && (Y <= fromTable.axisY[y-1]) )
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
    float p = ((float)(X - xMinValue)) / (float)(xMaxValue - xMinValue);
    float q = ((float)(Y - yMaxValue)) / (float)(yMinValue - yMaxValue);
    
    float m = (1.0-p) * (1.0-q);
    float n = p * (1-q);
    float o = (1-p) * q;
    float r = p * q;
    
    
    return ( (A * m) + (B * n) + (C * o) + (D * r) ); 
  }
