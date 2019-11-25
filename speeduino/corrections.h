/*
All functions in the gamma file return

*/
#ifndef CORRECTIONS_H
#define CORRECTIONS_H

void initialiseCorrections();

static inline byte correctionsFuel() __attribute__((always_inline));
static inline byte correctionWUE() __attribute__((always_inline)); //Warmup enrichment
static inline byte correctionCranking() __attribute__((always_inline)); //Cranking enrichment
static inline byte correctionASE() __attribute__((always_inline)); //After Start Enrichment
static inline int16_t correctionAccel() __attribute__((always_inline)); //Acceleration Enrichment
static inline byte correctionFloodClear() __attribute__((always_inline)); //Check for flood clear on cranking
static inline byte correctionAFRClosedLoop() __attribute__((always_inline)); //Closed loop AFR adjustment
static inline byte correctionFlex() __attribute__((always_inline)); //Flex fuel adjustment
static inline byte correctionBatVoltage() __attribute__((always_inline)); //Battery voltage correction
static inline byte correctionIATDensity() __attribute__((always_inline)); //Inlet temp density correction
static inline byte correctionBaro() __attribute__((always_inline)); //Barometric pressure correction
static inline byte correctionLaunch() __attribute__((always_inline)); //Launch control correction
static inline bool correctionDFCO() __attribute__((always_inline)); //Decelleration fuel cutoff

int8_t correctionsIgn(int8_t advance);
static inline int8_t correctionFixedTiming(int8_t);
static inline int8_t correctionCrankingFixedTiming(int8_t);
static inline int8_t correctionFlexTiming(int8_t);
static inline int8_t correctionIATretard(int8_t);
static inline int8_t correctionCLTadvance(int8_t);
static inline int8_t correctionIdleAdvance(int8_t);
static inline int8_t correctionSoftRevLimit(int8_t);
static inline int8_t correctionNitrous(int8_t);
static inline int8_t correctionSoftLaunch(int8_t);
static inline int8_t correctionSoftFlatShift(int8_t);
static inline int8_t correctionKnock(int8_t);

uint16_t correctionsDwell(uint16_t dwell);

uint16_t AFRnextCycle;
unsigned long knockStartTime;
byte lastKnockCount;
int16_t knockWindowMin; //The current minimum crank angle for a knock pulse to be valid
int16_t knockWindowMax;//The current maximum crank angle for a knock pulse to be valid

#endif // CORRECTIONS_H
