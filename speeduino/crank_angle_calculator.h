/**
 * @file 
 * @brief Crank calculations. To be reused by individual decoders
 */
#pragma once
#include "config_pages.h"
#include "atomic.h"

 struct crank_angle_calculator_t
 {
    uint16_t toothCurrentCount = 0;
    uint32_t toothLastToothTime = 0;
    bool revZeroOrOne = false;

    crank_angle_calculator_t() = default;
    crank_angle_calculator_t(const crank_angle_calculator_t&) = default;
    crank_angle_calculator_t(crank_angle_calculator_t&&) = default;    
    explicit crank_angle_calculator_t(uint16_t toothCurrentCount, uint32_t toothLastToothTime, bool revZeroOrOne);

    int16_t calculate(uint32_t currMicros, uint16_t triggerToothAngle, const config4 &page4) const;
    int16_t calculate(uint32_t currMicros, const int16_t toothAngles[], const config4 &page4) const;

private:
    int16_t calculateInner(int16_t initialCrankAngle, uint32_t currMicros, const config4 &page4) const;
    int16_t calculateAdjustmentSinceLastTooth(uint32_t currMicros) const;
    int16_t calculateInitialAngle(uint16_t triggerToothAngle) const;
    int16_t calculateInitialAngle(const int16_t toothAngles[]) const;
    uint16_t getSecondRevolutionOffset(const config4 &page4) const;
};

template <typename TCount, typename TTime, typename TRev>
static inline crank_angle_calculator_t atomic_make_caa(TCount &toothCurrentCount, TTime &toothLastToothTime, TRev &revZeroOrOne)
{
  ATOMIC()
  {
    return crank_angle_calculator_t(toothCurrentCount, toothLastToothTime, revZeroOrOne);
  }
  __builtin_unreachable(); 
}