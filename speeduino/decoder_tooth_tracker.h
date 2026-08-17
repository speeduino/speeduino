/**
 * @file 
 * @brief Crank calculations. To be reused by individual decoders
 */
#pragma once
#include "config_pages.h"

 struct seq_tooth_tracker_t
 {
    uint16_t toothCurrentCount = 0;
    uint32_t toothLastToothTime = 0;
    bool revZeroOrOne = false;

    seq_tooth_tracker_t() = default;
    seq_tooth_tracker_t(const seq_tooth_tracker_t&) = default;
    seq_tooth_tracker_t(seq_tooth_tracker_t&&) = default;    
    explicit seq_tooth_tracker_t(uint16_t toothCurrentCount, uint32_t toothLastToothTime, bool revZeroOrOne);

    int16_t calculateCrankAngle(uint32_t currMicros, uint16_t triggerToothAngle, const config4 &page4) const;
    int16_t calculateCrankAngle(uint32_t currMicros, const int16_t toothAngles[], const config4 &page4) const;

private:
    int16_t calculateCrankAngleInner(int16_t initialCrankAngle, uint32_t currMicros, const config4 &page4) const;
    int16_t calculateAdjustmentSinceLastTooth(uint32_t currMicros) const;
    int16_t calculateInitialAngle(uint16_t triggerToothAngle) const;
    int16_t calculateInitialAngle(const int16_t toothAngles[]) const;
    uint16_t getSecondRevolutionOffset(const config4 &page4) const;
};

 seq_tooth_tracker_t atomic_make_stt(const uint16_t &toothCurrentCount, const volatile uint32_t &toothLastToothTime, const volatile bool &revZeroOrOne);