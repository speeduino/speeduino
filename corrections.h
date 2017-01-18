/*
All functions in the gamma file return 

*/
#ifndef CORRECTIONS_H
#define CORRECTIONS_H

void initialiseCorrections();

byte correctionsFuel();
static inline byte correctionWUE(); //Warmup enrichment
static inline byte correctionCranking(); //Cranking enrichment
static inline byte correctionASE(); //After Start Enrichment
static inline byte correctionAccel(); //Acceleration Enrichment
static inline byte correctionFloodClear(); //Check for flood clear on cranking
static inline byte correctionAFRClosedLoop(); //Closed loop AFR adjustment
static inline byte correctionFlex(); //Flex fuel adjustment
static inline byte correctionBatVoltage(); //Battery voltage correction
static inline byte correctionIATDensity(); //Inlet temp density correction
static inline byte correctionLaunch(); //Launch control correction
static inline bool correctionDFCO(); //Decelleration fuel cutoff

byte correctionsIgn();
static inline byte correctionFixedTiming(byte);
static inline byte correctionCrankingFixedTiming(byte);
static inline byte correctionFlexTiming(byte);
static inline byte correctionIATretard(byte);
static inline byte correctionSoftRevLimit(byte);
static inline byte correctionSoftLaunch(byte);
static inline byte correctionSoftFlatShift(byte);


#endif // CORRECTIONS_H
