/**
 * @file 
 * @brief Crank calculations. To be reused by individual decoders
 * 
 * Crank angle calculations (usually) have 4 components:
 * 1. The angle of the last detected tooth. 
 * This can be computed from a fixed tooth angle or looked up from an array.
 * This only gives accuracy to the nearest tooth.
 * 2. A temporal adjustment to account for rotation since the last tooth was detected.
 * This can be a simple delta or based on the interval between teeth
 * 3. An adjustment for sequential operation to account for the 1st (0-359°) or 2nd (360-719°) revolution
 * 4. A user defined offset from TDC (this accounts for sensor position on the wheel)
 */
#pragma once
#include "src/utils/nominmax.h"
#include <tuple>
#include "config_pages.h"

/**
 * @addtogroup CrankCalcMixins Crank calculation mixins
 * @brief Mix-in classes, each capturing one component of a crank angle computation.
 * 
 * Do not use directly, instead use one of the concrete classes.
 * @{
 */

/** @brief A crank angle calculator that computes a temporal adjustment based on when the tooth was last detected */
struct last_tooth_rev_calculator_t
{
  uint32_t _toothLastToothTime = 0;  ///< Time in µS that the current tooth was last detected

#if !defined(UNIT_TEST)
protected:
#endif
  last_tooth_rev_calculator_t() = default;
  explicit last_tooth_rev_calculator_t(uint32_t toothLastToothTime);
  
  int16_t calculate(uint32_t currMicros) const;
};

/** @brief A crank angle calculator that computes a temporal adjustment based on  the gap between the 2 most recent 
 * teeth rather than the last full revolution */
struct tooth_interval_calculator_t
{
  uint32_t _toothLastToothTime = 0;         ///< Time in µS that the current tooth was last detected
  uint32_t _toothLastMinusOneToothTime = 0; ///< Time in µS that the tooth before the last tooth was detected
  uint16_t _toothAngle = 0;                 ///< Tooth angle (1-359°)
  bool _toothAngleCorrect;                  ///< Is @ref _toothAngle valid

#if !defined(UNIT_TEST)
protected:
#endif
  tooth_interval_calculator_t() = default;
  explicit tooth_interval_calculator_t(uint32_t toothLastToothTime, uint32_t toothLastMinusOneToothTime, uint16_t toothAngle, bool toothAngleCorrect);
  
  int16_t calculate(uint32_t currMicros) const;
};

/** @brief A crank angle calculator that looks up the initial crank angle from an array.
 * 
 * Useful when the teeth are unevenly spaced or of varying widths */
struct lookup_initial_calculator_t
{
  uint16_t _toothCurrentCount = 0;   ///< 1-based tooth number

#if !defined(UNIT_TEST)
protected:
#endif
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
 * Useful when the teeth are evenly spaced and the same width */
struct compute_initial_calculator_t
{
  uint16_t _toothCurrentCount = 0;  ///< 1-based tooth number
  uint16_t _toothAngle = 0;         ///< Tooth angle (1-359°)

#if !defined(UNIT_TEST)
protected:
#endif
  compute_initial_calculator_t() = default;
  explicit compute_initial_calculator_t(uint16_t toothCurrentCount, uint16_t toothAngle);
  
  int16_t calculate(void) const;
};

/** @brief A crank angle calculator that computes a revolution adjustment */
struct sequential_correction_calculator_t
{
  bool _revZeroOrOne = false; ///< When calculating over 720°, tells the calculator if we are in the 1st revolution (false) or the 2nd (true)

#if !defined(UNIT_TEST)
protected:
#endif
  sequential_correction_calculator_t() = default;
  explicit sequential_correction_calculator_t(bool revZeroOrOne);
  
  int16_t calculate(const config4 &page4) const;
};

/// @}

/**
 * @addtogroup CrankCalcs Crank calculation classes
 * @brief Each of these is a composite of various mix-in classes, so pay close attention to the
 * base classes - they define the calculation.
 * @{
 */
struct simple_crank_angle_calculator_t : last_tooth_rev_calculator_t, sequential_correction_calculator_t
 {
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
  trigger_angle_crank_angle_calculator_t() = default;
  explicit trigger_angle_crank_angle_calculator_t(const std::tuple<uint32_t, bool, uint16_t, uint16_t> &data);

  /**
   * @brief Calculate the crank angle
   * 
   * @param currMicros Current time in µS. Usually the result of a call to micros()
   * @param page4 The tune
   */
  int16_t calculate(uint32_t currMicros, const config4 &page4) const;
};

struct lookup_crank_angle_calculator_tooth_interval_t : public tooth_interval_calculator_t, sequential_correction_calculator_t, lookup_initial_calculator_t
{
  lookup_crank_angle_calculator_tooth_interval_t() = default;
  explicit lookup_crank_angle_calculator_tooth_interval_t(const std::tuple<uint32_t, uint32_t, bool, uint16_t, uint16_t, bool> &data);

  /**
   * @brief Calculate the crank angle
   * 
   * @param currMicros Current time in µS. Usually the result of a call to micros()
   * @param toothAngles Array to lookup. Note that teeth numbers are 1-based, but arrays are 0-based
   * @param page4 The tune
   */
  int16_t calculate(uint32_t currMicros, const int16_t toothAngles[], const config4 &page4) const;
};

struct compute_crank_angle_calculator_tooth_interval_t : public tooth_interval_calculator_t, sequential_correction_calculator_t, compute_initial_calculator_t
{
  compute_crank_angle_calculator_tooth_interval_t() = default;
  explicit compute_crank_angle_calculator_tooth_interval_t(const std::tuple<uint32_t, uint32_t, bool, uint16_t, uint16_t, bool> &data);

  /**
   * @brief Calculate the crank angle
   * 
   * @param currMicros Current time in µS. Usually the result of a call to micros()
   * @param page4 The tune
   */
  int16_t calculate(uint32_t currMicros, const config4 &page4) const;
};

/// @}