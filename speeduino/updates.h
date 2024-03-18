#ifndef UPDATES_H
#define UPDATES_H

#include "table3d.h"

void doUpdates(void);
void multiplyTableLoad(void *pTable, table_type_t key, uint8_t multiplier); //Added 202201 - to update the table Y axis as TPS now works at 0.5% increments. Multiplies the load axis values by 4 (most tables) or by 2 (VVT table)
void divideTableLoad(void *pTable, table_type_t key, uint8_t divisor); //Added 202201 - to update the table Y axis as TPS now works at 0.5% increments. This should only be needed by the VVT tables when using MAP as load. 
void multiplyTableValue(uint8_t pageNum, uint8_t multiplier); //Added to update the table values. Multiplies the value by the multiplier
void divideTableValue(uint8_t pageNum, uint8_t divisor); //Added to update the table values. Divide the value by divisor

#endif