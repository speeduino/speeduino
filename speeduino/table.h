/*
This file is used for everything related to maps/tables including their definition, functions etc
*/
#ifndef TABLE_H
#define TABLE_H

#define TABLE_RPM_MULTIPLIER  100
#define TABLE_LOAD_MULTIPLIER 2

/*
The 2D table can contain either 8-bit (byte) or 16-bit (int) values
The valueSize variable should be set to either 8 or 16 to indicate this BEFORE the table is used
*/
struct table2D {
  //Used 5414 RAM with original version
  byte valueSize;
  byte axisSize;
  byte xSize;

  void *values;
  void *axisX;

  //int16_t *values16;
  //int16_t *axisX16;

  //Store the last X and Y coordinates in the table. This is used to make the next check faster
  int16_t lastXMax;
  int16_t lastXMin;

  //Store the last input and output for caching
  int16_t lastInput;
  int16_t lastOutput;
  byte cacheTime; //Tracks when the last cache value was set so it can expire after x seconds. A timeout is required to pickup when a tuning value is changed, otherwise the old cached value will continue to be returned as the X value isn't changing. 
};

//void table2D_setSize(struct table2D targetTable, byte newSize);
// void table2D_setSize(struct table2D*, byte);
int16_t table2D_getAxisValue(struct table2D*, byte);
int16_t table2D_getRawValue(struct table2D*, byte);

// Declare a container for the internal storage of table3D
#define DECLARE_TABLE3D(name, size)\
struct {\
  byte values[size*size];\
  byte* rows[size];\
  int16_t axisX[size];\
  int16_t axisY[size];\
} _##name;\
struct table3D name = { .xSize=size, .ySize=size, .values=_##name.rows, .axisX=_##name.axisX, .axisY=_##name.axisY ,0,0,0,0,0,0,0,0 };

// Link the table to its container
#define INIT_TABLE3D(name) do {\
  for(int _i = 0; _i < name.ySize; _i++) { name.values[_i] = &_##name.values[_i*name.xSize]; }\
  name.cacheIsValid = false;\
} while(0)

struct table3D {

  //All tables must be the same size for simplicity

  byte xSize;
  byte ySize;

  byte **values;
  int16_t *axisX;
  int16_t *axisY;

  //Store the last X and Y coordinates in the table. This is used to make the next check faster
  byte lastXMax, lastXMin;
  byte lastYMax, lastYMin;

  //Store the last input and output values, again for caching purposes
  int16_t lastXInput, lastYInput;
  byte lastOutput; //This will need changing if we ever have 16-bit table values
  bool cacheIsValid; ///< This tracks whether the tables cache should be used. Ordinarily this is true, but is set to false whenever TunerStudio sends a new value for the table
};

//void table3D_setSize(struct table3D *targetTable, byte);
// void table3D_setSize(struct table3D *targetTable, byte);

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
int get3DTableValue(struct table3D *fromTable, int, int);
int table2D_getValue(struct table2D *fromTable, int);

#endif // TABLE_H
