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

    tooth_tracker_t() = default;
    explicit tooth_tracker_t(uint16_t toothCurrentCount, uint32_t toothLastToothTime);

    int16_t calculateCrankAngle(uint32_t currMicros, uint16_t triggerToothAngle, const config4 &page4) const;
    int16_t calculateCrankAngle(uint32_t currMicros, const int16_t toothAngles[], const config4 &page4) const;
};

 struct seq_tooth_tracker_t : public tooth_tracker_t
 {
    bool revZeroOrOne = false;

    seq_tooth_tracker_t() = default;
    explicit seq_tooth_tracker_t(uint16_t toothCurrentCount, uint32_t toothLastToothTime, bool revZeroOrOne);

    int16_t calculateCrankAngle(uint32_t currMicros, uint16_t triggerToothAngle, const config4 &page4) const;
    int16_t calculateCrankAngle(uint32_t currMicros, const int16_t toothAngles[], const config4 &page4) const;
 };

 seq_tooth_tracker_t atomic_make_stt(const uint16_t &toothCurrentCount, const volatile uint32_t &toothLastToothTime, const volatile bool &revZeroOrOne);