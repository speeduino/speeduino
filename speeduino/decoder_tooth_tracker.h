/**
 * @file 
 * @brief Crank calculations. To be reused by individual decoders
 */
#pragma once
#include "config_pages.h"

struct tooth_tracker_t
{
    uint16_t toothCurrentCount = 0;
    uint32_t toothLastToothTime = 0;

    int16_t calculateCrankAngle(uint32_t currMicros, uint16_t triggerToothAngle, const config4 &page4) const;
    int16_t calculateCrankAngle(uint32_t currMicros, const int16_t toothAngles[], const config4 &page4) const;
};

 struct seq_tooth_tracker_t : public tooth_tracker_t
 {
    bool revZeroOrOne = false;

    int16_t calculateCrankAngle(uint32_t currMicros, uint16_t triggerToothAngle, const config4 &page4) const;
    int16_t calculateCrankAngle(uint32_t currMicros, const int16_t toothAngles[], const config4 &page4) const;
 };