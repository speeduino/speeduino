/*
All functions in the gamma file return

*/
#ifndef CORRECTIONS_H
#define CORRECTIONS_H

void initialiseCorrections(void);
uint16_t correctionsFuel(void);

/** @brief Whether the flood clear fuel cut is currently latched. Only released by updateFloodClear() or an ECU restart */
bool isFloodClearActive(void);
/** @brief Latch or release the flood clear state. Must be called at the TPS read rate, even when the engine is not rotating */
void updateFloodClear(void);
uint8_t calculateAfrTarget(table3d16RpmLoad &afrLookUpTable, const statuses &current, const config2 &page2, const config6 &page6);

int8_t correctionsIgn(int8_t advance);
int8_t correctionFixedTiming(int8_t advance);
int8_t correctionCrankingFixedTiming(int8_t advance);

uint16_t correctionsDwell(uint16_t dwell);


#endif // CORRECTIONS_H
