/**
 * @file 
 * @brief Crank calculations. To be reused by individual decoders
 */
#pragma once
#include "src/utils/nominmax.h"
#include <tuple>
#include "config_pages.h"

/** @brief A crank angle calculator that computes a temporal adjustment based on when the tooth was last detected */
struct last_tooth_rev_calculator_t
{
  uint32_t _toothLastToothTime = 0;  ///< Time in µS that the current tooth was last detected

  last_tooth_rev_calculator_t() = default;
  explicit last_tooth_rev_calculator_t(uint32_t toothLastToothTime);
  
  int16_t calculate(uint32_t currMicros) const;
};

/** @brief A crank angle calculator that looks up the initial crank angle from an array.
 * 
 * Useful when the teeth are unevenly spaced or of varying widths
 */
struct lookup_initial_calculator_t
{
  uint16_t _toothCurrentCount = 0;   ///< 1-based tooth number

  lookup_initial_calculator_t() = default;
  explicit lookup_initial_calculator_t(uint16_t toothCurrentCount);
  
  /**
   * @brief Calculate the crank angle
   * 
   * @param toothAngles Array to lookup. Note that teeth numbers are 1-based, but arrays are 0-based
   */
  int16_t calculate(const int16_t toothAngles[]) const;
};

/** @brief A crank angle calculator that computes the initial crank angle from a tooth angle
 * 
 * Useful when the teeth are evenly spaced and the same width
 */
struct compute_initial_calculator_t
{
  uint16_t _toothCurrentCount = 0;  ///< 1-based tooth number
  uint16_t _toothAngle = 0;         ///< Tooth angle (1-359°)

  compute_initial_calculator_t() = default;
  explicit compute_initial_calculator_t(uint16_t toothCurrentCount, uint16_t toothAngle);
  
  /** @brief Calculate the crank angle */
  int16_t calculate(void) const;
};

/** @brief A crank angle calculator that computes a revolution adjustment */
struct sequential_correction_calculator_t
{
  bool _revZeroOrOne = false; ///< When calculating over 720°, tells the calculator if we are in the 1st revolution (false) or the 2nd (true)

  sequential_correction_calculator_t() = default;
  explicit sequential_correction_calculator_t(bool revZeroOrOne);
  
  /**
   * @brief Calculate the crank angle
   * 
   * @param page4 The tune
   */
  int16_t calculate(const config4 &page4) const;
};

struct simple_crank_angle_calculator_t : last_tooth_rev_calculator_t, sequential_correction_calculator_t
 {
    simple_crank_angle_calculator_t() = default;
    explicit simple_crank_angle_calculator_t(const std::tuple<uint32_t, bool> &data);

    int16_t calculate(int16_t initialCrankAngle, uint32_t currMicros, const config4 &page4) const;
};

struct lookup_crank_angle_calculator_t : public last_tooth_rev_calculator_t, sequential_correction_calculator_t, lookup_initial_calculator_t
{
  lookup_crank_angle_calculator_t() = default;
  explicit lookup_crank_angle_calculator_t(const std::tuple<uint32_t, bool, uint16_t> &data);

  /**
   * @brief Calculate the crank angle
   * 
   * @param currMicros Current time in µS. Usually the result of a call to micros()
   * @param toothAngles Array to lookup. Note that teeth numbers are 1-based, but arrays are 0-based
   * @param page4 The tune
   */
  int16_t calculate(uint32_t currMicros, const int16_t toothAngles[], const config4 &page4) const;
};

struct trigger_angle_crank_angle_calculator_t : public last_tooth_rev_calculator_t, sequential_correction_calculator_t, compute_initial_calculator_t
{
  using data_type = std::tuple<uint32_t, bool, uint16_t, uint16_t>;

  trigger_angle_crank_angle_calculator_t() = default;
  explicit trigger_angle_crank_angle_calculator_t(const data_type &data);

  /**
   * @brief Calculate the crank angle
   * 
   * @param currMicros Current time in µS. Usually the result of a call to micros()
   * @param page4 The tune
   */
  int16_t calculate(uint32_t currMicros, const config4 &page4) const;
};
