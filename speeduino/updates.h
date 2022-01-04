#ifndef UPDATES_H
#define UPDATES_H

#include "table3d.h"

void doUpdates();
void multiplyTableLoadTableLoad(const void*, table_type_t, uint8_t); //Added 202201 - to update the table Y axis as TPS now works at 0.5% increments. Multiplies the load axis values by 4 (most tables) or by 2 (VVT table)
void divideTableLoadTableLoad(const void*, table_type_t, uint8_t); //Added 202201 - to update the table Y axis as TPS now works at 0.5% increments. This should only be needed by the VVT tables when using MAP as load. 

#endif