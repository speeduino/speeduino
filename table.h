/*
This file is used for everything related to maps/tables including their definition, functions etc
*/
#include <Arduino.h>

struct table2Dx4 {
  const static byte xSize = 4;
  
  byte values[xSize];
  int axisX[xSize];
};

struct table2Dx10 {
  const static byte xSize = 10;
  
  byte values[xSize];
  int axisX[xSize];
};

struct table3D {
  //All tables must be the same size for simplicity
  const static byte xSize = 8;
  const static byte ySize = 8;
  
  byte values[ySize][xSize];
  int axisX[xSize];
  int axisY[ySize];
  
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
int get3DTableValue(struct table3D, int, int);
int get2DTableValue(struct table2D, int);
