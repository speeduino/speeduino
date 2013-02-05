/*
This file is used for everything related to maps/tables including their definition, functions etc
*/

struct table {
  //All tables must be the same size for simplicity
  const static int xSize = 8;
  const static int ySize = 8;
  
  int values[xSize][ySize];
  int axisX[xSize];
  int axisY[ySize];
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

int getTableValue(struct table fromTable, int Y, int X)
  {
    for (int x = 0; x < fromTable.xSize; x++)
    {
      
    }
    return 1; 
  }
