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
| 25|
| 13|
|  1| 15 | 20 |
|   | 500|1500|2000|2500|3000|4000|5000|6000| RPM

This is a fairly standard 8x8 VE vs RPM 2D map

*/

void dummyFuelTable()
  {
    table myFuelTable;
    
    //myFuelTable.values
    //myFuelTable.axisY = {100,88,75,63,50,38,25,13,1};
    //myFuelTable.axisX = {500,1500,2500,3000,4000,5000,6000};
  }
