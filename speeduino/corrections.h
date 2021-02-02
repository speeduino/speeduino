/*
All functions in the gamma file return

*/
#ifndef CORRECTIONS_H
#define CORRECTIONS_H

void initialiseCorrections();

uint16_t correctionsFuel();
uint8_t correctionWUE(); //Warmup enrichment
uint16_t correctionCranking(); //Cranking enrichment
uint8_t correctionASE(); //After Start Enrichment
uint16_t correctionAccel(); //Acceleration Enrichment
uint8_t correctionFloodClear(); //Check for flood clear on cranking
uint8_t correctionAFRClosedLoop(); //Closed loop AFR adjustment
uint8_t correctionFlex(); //Flex fuel adjustment
uint8_t correctionFuelTemp(); //Fuel temp correction
uint8_t correctionBatVoltage(); //Battery voltage correction
uint8_t correctionIATDensity(); //Inlet temp density correction
uint8_t correctionBaro(); //Barometric pressure correction
uint8_t correctionLaunch(); //Launch control correction
bool correctionDFCO(); //Decelleration fuel cutoff


int8_t correctionsIgn(int8_t advance);
int8_t correctionFixedTiming(int8_t);
int8_t correctionCrankingFixedTiming(int8_t);
int8_t correctionFlexTiming(int8_t);
int8_t correctionWMITiming(int8_t);
int8_t correctionIATretard(int8_t);
int8_t correctionCLTadvance(int8_t);
int8_t correctionIdleAdvance(int8_t);
int8_t correctionSoftRevLimit(int8_t);
int8_t correctionNitrous(int8_t);
int8_t correctionSoftLaunch(int8_t);
int8_t correctionSoftFlatShift(int8_t);
int8_t correctionKnock(int8_t);

uint16_t correctionsDwell(uint16_t dwell);

extern int16_t MAP_rateOfChange;
extern int16_t TPS_rateOfChange;
extern uint8_t activateMAPDOT; //The mapDOT value seen when the MAE was activated. 
extern uint8_t activateTPSDOT; //The tpsDOT value seen when the MAE was activated. 

extern uint16_t AFRnextCycle;
extern uint32_t knockStartTime;
extern uint8_t lastKnockCount;
extern int16_t knockWindowMin; //The current minimum crank angle for a knock pulse to be valid
extern int16_t knockWindowMax;//The current maximum crank angle for a knock pulse to be valid
extern uint16_t aseTaperStart;
extern uint16_t dfcoStart;

#endif // CORRECTIONS_H
