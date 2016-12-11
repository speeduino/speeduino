/*
All functions in the gamma file return 

*/
#ifndef CORRECTIONS_H
#define CORRECTIONS_H

void initialiseCorrections();

byte correctionsFuel();
static inline byte correctionWUE(); //Warmup enrichment
static inline byte correctionASE(); //After Start Enrichment
static inline byte correctionAccel(); //Acceleration Enrichment
static inline byte correctionsFloodClear(); //Check for flood clear on cranking
static inline byte correctionsAFRClosedLoop(); //Closed loop AFR adjustment
static inline byte correctionsFlex(); //Flex fuel adjustment

byte correctionsIgn();
static inline byte correctionsFixedTiming(byte);
static inline byte correctionsCrankingFixedTiming(byte);
static inline byte correctionsFlexTiming(byte);
static inline byte correctionsIATretard(byte);
static inline byte correctionsSoftRevLimit(byte);
static inline byte correctionsSoftLaunch(byte);


#endif // CORRECTIONS_H
