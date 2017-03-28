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
static inline int8_t correctionFixedTiming(int8_t);
static inline int8_t correctionCrankingFixedTiming(int8_t);
static inline int8_t correctionFlexTiming(int8_t);
static inline int8_t correctionIATretard(int8_t);
static inline int8_t correctionSoftRevLimit(int8_t);
static inline int8_t correctionSoftLaunch(int8_t);
static inline int8_t correctionSoftFlatShift(int8_t);


#endif // CORRECTIONS_H
