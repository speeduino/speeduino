/*
All functions in the gamma file return 

*/
//static byte numCorrections = 2;

byte correctionsTotal();
byte correctionWUE(); //Warmup enrichment
byte correctionASE(); //After Start Enrichment
byte correctionAccel(); //Acceleration Enrichment
byte correctionsFloodClear(); //Check for flood clear on cranking
byte correctionsAFRClosedLoop(); //Closed loop AFR adjustment
