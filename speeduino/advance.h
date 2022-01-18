/** \file advance.h
 * @brief Functions for calculating degrees of ignition advance
 * 
 */

#ifndef ADVANCE_H
#define ADVANCE_H

int8_t getAdvance();
int16_t getAdvance1();
int16_t getAdvance2();
bool shouldWeUseSparkTable2();

#endif // ADVANCE_H