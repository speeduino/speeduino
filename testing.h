/*
This file has a few functions that are helpful for testing such as creating dummy maps and faking interrupts
*/

/*
Aim is to create an 8x8 table that looks like the below:
 MAP
______________________________________________
|100|| 78 | 88 | 92 | 95 | 97 |101 |107 |110 |
| 88|| 58 | 
| 75|| 45 |
| 63|| 35 |  
| 50|| 28 | 30
| 38|| 22 | 23 | 
| 25|| 17 | 21 |
| 13|| 15 | 20 | 25 | 
|   || 500|1500|2000|2500|3000|4000|5000|6000| RPM

This is a fairly realistic 8x8 VE vs RPM 2D map
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

void dummyFuelTable(struct table *myFuelTable)
  {
    //table myFuelTable;
    
    int tempXAxis[8] = {500,1500,2000,2500,3000,4000,5000,6000};
    for (int x = 0; x< myFuelTable->xSize; x++) { myFuelTable->axisX[x] = tempXAxis[x]; }
    //*myFuelTable->axisX = *tempXAxis;
    int tempYAxis[8] = {100,88,75,63,50,38,25,13};
    for (int x = 0; x< myFuelTable->ySize; x++) { myFuelTable->axisY[x] = tempYAxis[x]; }
    //*myFuelTable->axisY = *tempYAxis;
    
    //Go through the 8 rows and add the column values
    int tempRow1[8] = {78,88,92,95,97,101,107,110};
    int tempRow2[8] = {58,88,75,63,50,38,25,13};
    int tempRow3[8] = {45,88,75,63,50,38,25,13};
    int tempRow4[8] = {35,88,75,63,50,38,25,13};
    int tempRow5[8] = {28,88,75,63,50,38,25,13};
    int tempRow6[8] = {22,23,75,63,50,38,25,13};
    int tempRow7[8] = {17,21,75,63,50,38,25,13};
    int tempRow8[8] = {15,20,25,63,50,38,25,13};
    for (int x = 0; x< myFuelTable->xSize; x++) { myFuelTable->values[0][x] = tempRow1[x]; }
    for (int x = 0; x< myFuelTable->xSize; x++) { myFuelTable->values[1][x] = tempRow2[x]; }
    for (int x = 0; x< myFuelTable->xSize; x++) { myFuelTable->values[2][x] = tempRow3[x]; }
    for (int x = 0; x< myFuelTable->xSize; x++) { myFuelTable->values[3][x] = tempRow4[x]; }
    for (int x = 0; x< myFuelTable->xSize; x++) { myFuelTable->values[4][x] = tempRow5[x]; }
    for (int x = 0; x< myFuelTable->xSize; x++) { myFuelTable->values[5][x] = tempRow6[x]; }
    for (int x = 0; x< myFuelTable->xSize; x++) { myFuelTable->values[6][x] = tempRow7[x]; }
    for (int x = 0; x< myFuelTable->xSize; x++) { myFuelTable->values[7][x] = tempRow8[x]; }
        
  }
