/*
This file has a few functions that are helpful for testing such as creating dummy maps and faking interrupts
*/

/*
Aim is to create an 8x8 table that looks like the below:
 MAP
______________________________________________
|100|| 78 | 88 | 92 | 95 | 97 |101 |107 |110 |
| 88|| 58 | 88 | 75 | 63 | 50 | 38 | 25 | 13 | 
| 75|| 45 | 88 | 75 | 63 | 50 | 38 | 25 | 13 | 
| 63|| 35 | 88 | 75 | 63 | 50 | 38 | 25 | 13 | 
| 50|| 28 | 88 | 75 | 63 | 50 | 38 | 25 | 13 | 
| 38|| 22 | 23 | 75 | 63 | 50 | 38 | 25 | 13 |  
| 25|| 17 | 21 | 75 | 63 | 50 | 38 | 25 | 13 | 
| 13|| 15 | 20 | 25 | 63 | 50 | 38 | 25 | 13 | 
|   || 500|1500|2000|2500|3000|4000|5000|6000| RPM

This is not really a realistic 8x8 VE vs RPM 2D map, but it's enough for testing
Here is a run through of how the interpolation would flow:
Pass in:
x = 1000
y = 20

yMax = 13
yMin = 25

xMax = 1500
xMin = 500

A 17
B 21
C 15
D 20
p (1000 - 500) / (1500 - 500) = 1/2 = 0.5
q (20 - 13) / (25 - 13) = 7/12 = 0.583
m (1 - 0.5) * (1 - 0.583) = 0.2083
n 0.5 * (1 - 0.583) = 0.2083
o (1 - 0.5) * 0.583 = 0.29166
r (0.5 * 0.583) = 0.29166

VE = ( (17 * 0.2083) + (21 * 0.2083) + (15 * 0.9166) + (20 * 0.29166) ) = 18.125

*/

void dummyFuelTable(struct table3D *myFuelTable)
  {
    //table myFuelTable;
    
    int tempXAxis[8] = {500,1500,2000,2500,3000,4000,5000,6000};
    for (byte x = 0; x< myFuelTable->xSize; x++) { myFuelTable->axisX[x] = tempXAxis[x]; }
    //*myFuelTable->axisX = *tempXAxis;
    int tempYAxis[8] = {100,88,75,63,50,30,20,10};
    for (byte x = 0; x< myFuelTable->ySize; x++) { myFuelTable->axisY[x] = tempYAxis[x]; }
    //*myFuelTable->axisY = *tempYAxis;
    
    //Go through the 8 rows and add the column values
    byte tempRow1[8] = {78,88,92,95,97,101,107,110};
    byte tempRow2[8] = {58,88,75,63,50,38,25,13};
    byte tempRow3[8] = {45,88,75,63,50,38,25,13};
    byte tempRow4[8] = {35,88,75,63,50,38,25,13};
    byte tempRow5[8] = {28,88,75,63,50,38,25,13};
    byte tempRow6[8] = {22,23,75,63,50,38,25,13};
    byte tempRow7[8] = {17,21,75,63,50,38,25,13};
    byte tempRow8[8] = {15,20,25,63,50,38,25,13};
    for (byte x = 0; x< myFuelTable->xSize; x++) { myFuelTable->values[0][x] = tempRow1[x]; }
    for (byte x = 0; x< myFuelTable->xSize; x++) { myFuelTable->values[1][x] = tempRow2[x]; }
    for (byte x = 0; x< myFuelTable->xSize; x++) { myFuelTable->values[2][x] = tempRow3[x]; }
    for (byte x = 0; x< myFuelTable->xSize; x++) { myFuelTable->values[3][x] = tempRow4[x]; }
    for (byte x = 0; x< myFuelTable->xSize; x++) { myFuelTable->values[4][x] = tempRow5[x]; }
    for (byte x = 0; x< myFuelTable->xSize; x++) { myFuelTable->values[5][x] = tempRow6[x]; }
    for (byte x = 0; x< myFuelTable->xSize; x++) { myFuelTable->values[6][x] = tempRow7[x]; }
    for (byte x = 0; x< myFuelTable->xSize; x++) { myFuelTable->values[7][x] = tempRow8[x]; }
        
  }
  
/*
Populates a table with some reasonably realistic ignition advance data
*/
void dummyIgnitionTable(struct table3D *mySparkTable)
  { 
    int tempXAxis[8] = {500,1500,2000,2500,3000,4000,5000,6000};
    for (byte x = 0; x< mySparkTable->xSize; x++) { mySparkTable->axisX[x] = tempXAxis[x]; }
    //*myFuelTable->axisX = *tempXAxis;
    int tempYAxis[8] = {100,88,75,63,50,30,20,10};
    for (byte x = 0; x< mySparkTable->ySize; x++) { mySparkTable->axisY[x] = tempYAxis[x]; }
    //*myFuelTable->axisY = *tempYAxis;
    
    //Go through the 8 rows and add the column values
    byte tempRow1[8] = {10,15,20,26,35,40,43,44};
    byte tempRow2[8] = {10,88,75,63,50,38,25,44};
    byte tempRow3[8] = {12,88,75,63,50,38,25,40};
    byte tempRow4[8] = {12,88,75,63,50,38,25,36};
    byte tempRow5[8] = {28,88,75,63,50,38,25,13};
    byte tempRow6[8] = {22,23,75,63,50,38,25,13};
    byte tempRow7[8] = {17,21,75,63,50,38,25,13};
    byte tempRow8[8] = {15,20,25,63,50,38,25,13};
    for (byte x = 0; x< mySparkTable->xSize; x++) { mySparkTable->values[0][x] = tempRow1[x]; }
    for (byte x = 0; x< mySparkTable->xSize; x++) { mySparkTable->values[1][x] = tempRow2[x]; }
    for (byte x = 0; x< mySparkTable->xSize; x++) { mySparkTable->values[2][x] = tempRow3[x]; }
    for (byte x = 0; x< mySparkTable->xSize; x++) { mySparkTable->values[3][x] = tempRow4[x]; }
    for (byte x = 0; x< mySparkTable->xSize; x++) { mySparkTable->values[4][x] = tempRow5[x]; }
    for (byte x = 0; x< mySparkTable->xSize; x++) { mySparkTable->values[5][x] = tempRow6[x]; }
    for (byte x = 0; x< mySparkTable->xSize; x++) { mySparkTable->values[6][x] = tempRow7[x]; }
    for (byte x = 0; x< mySparkTable->xSize; x++) { mySparkTable->values[7][x] = tempRow8[x]; }
        
  }
