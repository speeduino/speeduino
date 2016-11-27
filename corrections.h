/*
All functions in the gamma file return 

*/
#ifndef CORRECTIONS_H
#define CORRECTIONS_H

byte correctionsTotal();
byte correctionWUE(); //Warmup enrichment
byte correctionASE(); //After Start Enrichment
byte correctionAccel(); //Acceleration Enrichment
byte correctionsFloodClear(); //Check for flood clear on cranking
byte correctionsAFRClosedLoop(); //Closed loop AFR adjustment

#endif // CORRECTIONS_H
