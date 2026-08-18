/**
 * @file 
 * @brief Crank calculations. To be reused by individual decoders
 */
#pragma once
#include "src/utils/nominmax.h"
#include <tuple>
#include "config_pages.h"

 struct crank_angle_calculator_t
 {
    uint16_t toothCurrentCount = 0;   ///< 1-based tooth number
    uint32_t toothLastToothTime = 0;  ///< Time in µS that the current tooth was last detected
    bool revZeroOrOne = false;        ///< When calculating over 720°, tells the calculator if we are in the 1st revolution (false) or the 2nd (true)

    using tuple_type = std::tuple<uint16_t, uint32_t, bool>;

    crank_angle_calculator_t() = default;
    explicit crank_angle_calculator_t(const tuple_type &data);

    int16_t calculateFromInitial(int16_t initialCrankAngle, uint32_t currMicros, const config4 &page4) const;

private:
    int16_t calculateAdjustmentSinceLastTooth(uint32_t currMicros) const;
    uint16_t getSecondRevolutionOffset(const config4 &page4) const;
};

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

/** @brief A crank angle calculator that computes the initial crank angle from a tooth angle
 * 
 * Useful when the teeth are evenly spaced and the same width
 */
struct trigger_angle_crank_angle_calculator_t : public crank_angle_calculator_t
{
  uint16_t toothAngle;

  using tuple_type = std::tuple<uint16_t, uint32_t, bool, uint16_t>;

  trigger_angle_crank_angle_calculator_t() = default;
  explicit trigger_angle_crank_angle_calculator_t(const tuple_type &data);

  /**
   * @brief Calculate the crank angle
   * 
   * @param currMicros Current time in µS. Usually the result of a call to micros()
   * @param toothAngle The tooth angle
   * @param page4 The tunr
   */
  int16_t calculate(uint32_t currMicros, const config4 &page4) const;

private:
  int16_t calculateInitialAngle(void) const;
};
