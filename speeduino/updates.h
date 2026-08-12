#ifndef UPDATES_H
#define UPDATES_H

#include "table3d.h"

void doUpdates(void);
void multiplyTableValue(uint8_t pageNum, uint8_t multiplier); //Added to update the table values. Multiplies the value by the multiplier
void divideTableValue(uint8_t pageNum, uint8_t divisor); //Added to update the table values. Divide the value by divisor

#endif