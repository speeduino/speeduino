/*
This file has a few functions that are helpful for testing such as creating dummy maps and faking interrupts
*/

/*
Aim is to create an 8x8 table that looks like the below:
 VE
_____________________________________________
|100|    |    |    |    |    |    |    | 90 |
| 88|
| 75|
| 63|
| 50|
| 38|
| 25| 17 | 21 |
| 13| 15 | 20 |
|   | 500|1500|2000|2500|3000|4000|5000|6000| RPM

This is a fairly standard 8x8 VE vs RPM 2D map
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

void dummyFuelTable()
  {
    table myFuelTable;
    
    int tempXAxis[8] = {100,88,75,63,50,38,25,13};
    *myFuelTable.axisY = *tempXAxis;
    int tempYAxis[8] = {500,1500,2500,3000,4000,5000,6000};
    *myFuelTable.axisY = *tempYAxis;
  }
