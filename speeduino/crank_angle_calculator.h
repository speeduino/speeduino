/**
 * @file 
 * @brief Crank calculations. To be reused by individual decoders
 */
#pragma once
#include "config_pages.h"
#include "atomic.h"

 struct crank_angle_calculator_t
 {
    uint16_t toothCurrentCount = 0;   ///< 1-based tooth number
    uint32_t toothLastToothTime = 0;  ///< Time in µS that the current tooth was last detected
    bool revZeroOrOne = false;        ///< When calculating over 720°, tells the calculator if we are in the 1st revolution (false) or the 2nd (true)

    crank_angle_calculator_t() = default;
    crank_angle_calculator_t(const crank_angle_calculator_t&) = default;
    crank_angle_calculator_t(crank_angle_calculator_t&&) = default;    
    explicit crank_angle_calculator_t(uint16_t toothCurrentCount, uint32_t toothLastToothTime, bool revZeroOrOne);

    int16_t calculateFromInitial(int16_t initialCrankAngle, uint32_t currMicros, const config4 &page4) const;

private:
    int16_t calculateAdjustmentSinceLastTooth(uint32_t currMicros) const;
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

/** @brief A crank angle calculator that looks up the initial crank angle from an array.
 * 
 * Useful when the teeth are unevenly spaced or of varying widths
 */
struct lookup_crank_angle_calculator_t : public crank_angle_calculator_t
{
  using crank_angle_calculator_t::crank_angle_calculator_t;

  /**
   * @brief Calculate the crank angle
   * 
   * @param currMicros Current time in µS. Usually the result of a call to micros()
   * @param toothAngles Array to lookup. Note that teeth numbers are 1-based, but arrays are 0-based
   * @param page4 The tune
   */
  int16_t calculate(uint32_t currMicros, const int16_t toothAngles[], const config4 &page4) const;
private:
  int16_t calculateInitialAngle(const int16_t toothAngles[]) const;
};

template <typename TCount, typename TTime, typename TRev>
static inline lookup_crank_angle_calculator_t atomic_make_lookup_caa(TCount &toothCurrentCount, TTime &toothLastToothTime, TRev &revZeroOrOne)
{
  ATOMIC()
  {
    return lookup_crank_angle_calculator_t(toothCurrentCount, toothLastToothTime, revZeroOrOne);
  }
  __builtin_unreachable(); 
}

/** @brief A crank angle calculator that computes the initial crank angle from a tooth angle
 * 
 * Useful when the teeth are evenly spaced and the same width
 */
struct trigger_angle_crank_angle_calculator_t : public crank_angle_calculator_t
{
  using crank_angle_calculator_t::crank_angle_calculator_t;

  /**
   * @brief Calculate the crank angle
   * 
   * @param currMicros Current time in µS. Usually the result of a call to micros()
   * @param toothAngle The tooth angle
   * @param page4 The tunr
   */
  int16_t calculate(uint32_t currMicros, uint16_t toothAngle, const config4 &page4) const;

private:
  int16_t calculateInitialAngle(uint16_t toothAngle) const;
};

template <typename TCount, typename TTime, typename TRev>
static inline trigger_angle_crank_angle_calculator_t atomic_make_angle_caa(TCount &toothCurrentCount, TTime &toothLastToothTime, TRev &revZeroOrOne)
{
  ATOMIC()
  {
    return trigger_angle_crank_angle_calculator_t(toothCurrentCount, toothLastToothTime, revZeroOrOne);
  }
  __builtin_unreachable(); 
}