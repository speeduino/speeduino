/*
All functions in the gamma file return

*/
#ifndef CORRECTIONS_H
#define CORRECTIONS_H

#include "statuses.h"

void initialiseCorrections(void);

/** @brief Returns true if deceleration fuel cutoff should be on, false if its off */
bool isDFCOActive(void);

fuel_corrections_t getCorrectionsFuel(void);
uint8_t calculateAfrTarget(table3d16RpmLoad &afrLookUpTable, const statuses &current, const config2 &page2, const config6 &page6);

int8_t correctionsIgn(int8_t advance);
int8_t correctionFixedTiming(int8_t advance);
int8_t correctionCrankingFixedTiming(int8_t advance);

uint16_t correctionsDwell(uint16_t dwell);


#endif // CORRECTIONS_H
