#ifndef CRANKMATHS_H
#define CRANKMATHS_H

#include "maths.h"
#include "board_definition.h"

/**
 * @file
 * 
 * @brief Crank revolution based mathematical functions. 
 * 
 */

/** @brief At 1 RPM, each degree of angular rotation takes this many microseconds */
static constexpr uint32_t MICROS_PER_DEG_1_RPM = UDIV_ROUND_CLOSEST(MICROS_PER_MIN, 360UL, uint32_t);

/** @brief The maximum rpm that the ECU will attempt to run at. 
 * 
 * It is NOT related to the rev limiter, but is instead dictates how fast certain operations will be
 * allowed to run. Lower number gives better performance 
 **/
static constexpr uint16_t MAX_RPM = 18000U;

/** @brief Absolute minimum RPM that the crank math (& therefore all of Speeduino) can be used with.
 * 
 * This is dictated by the use of uint16_t as the base type for storing
 * time --> angle conversion factor (degreesPerMicro)
*/
static constexpr uint16_t MIN_RPM = (uint16_t)UDIV_ROUND_UP(MICROS_PER_DEG_1_RPM, (uint32_t)UINT16_MAX/16UL, uint32_t);

/**
 * @brief Minimum time in µS that one crank revolution can take.
 * 
 * @note: many calculations are done over 2 revolutions (cycles), in which case this would be doubled 
 */
static constexpr uint16_t MIN_REVOLUTION_TIME = MICROS_PER_MIN/MAX_RPM;

/**
 * @brief Maximum time in µS that one crank revolution can take.
 * 
 * @note: many calculations are done over 2 revolutions (cycles), in which case this would be doubled 
 */
static constexpr uint32_t MAX_REVOLUTION_TIME = MICROS_PER_MIN/MIN_RPM;

/**
 * @brief Converts a crank revolution time to engine speed
 * 
 * @param revolutionTime The time in µS that one crank revolution takes. Zero if the engine speed is unknown
 * @return The engine speed in RPM, rounded to the closest integer & limited to MAX_RPM. Zero if revolutionTime is zero
 */
static inline uint16_t RpmFromRevolutionTimeUs(uint32_t revolutionTime) {
    if (revolutionTime==0U) { return 0U; }
    return (uint16_t)clamp(fast_div_closest(MICROS_PER_MIN, revolutionTime), (uint32_t)0UL, (uint32_t)MAX_RPM);
}

extern int16_t CRANK_ANGLE_MAX_IGN; ///< The number of crank degrees that the system tracks ignition over.
extern int16_t CRANK_ANGLE_MAX_INJ; ///< The number of crank degrees that the system tracks fuel injection over.

/**
 * @brief Makes one pass at nudging the angle to within [0,CRANK_ANGLE_MAX_IGN]
 * 
 * @param angle A crank angle in degrees
 * @return int16_t 
 */
static inline int16_t ignitionLimits(int16_t angle) {
    return nudge((int16_t)0, (int16_t)CRANK_ANGLE_MAX_IGN, angle);
}

/** @brief The factors used to convert between crank angle & time. These are derived from the crank revolution time. */
struct angle_converter_factors_t {
  /** @brief uS per degree in UQ24.8 fixed point */
  uint32_t microsPerDegree;
  /** @brief Degrees per uS in UQ1.15 fixed point.
   * 
   * Ranges from 8 (0.000246) at MIN_RPM to 3542 (0.108) at MAX_RPM
   */
  uint16_t degreesPerMicro;
};

/**
 * @brief Calculate the angle<-->time conversion factors for a crank revolution time
 * 
 * This does not change the factors in use (see applyAngleConverterFactors()), so the
 * (relatively slow) divisions can be done outside of a critical section.
 * 
 * @param revolutionTime The crank revolution time in uS. Zero results in zero factors.
 */
angle_converter_factors_t calculateAngleConverterFactors(uint32_t revolutionTime) noexcept;

/**
 * @brief Set the factors used by the degree<-->angle conversions
 * 
 * @note If an ISR could be converting angles, call this from within an ATOMIC() block.
 * 
 * @param factors The factors to use. See calculateAngleConverterFactors()
 */
void applyAngleConverterFactors(const angle_converter_factors_t &factors) noexcept;

/**
 * @brief Set the revolution time, from which some of the degree<-->angle conversions are derived
 * 
 * Shorthand for applyAngleConverterFactors(calculateAngleConverterFactors(revolutionTime))
 * 
 * @param revolutionTime The crank revolution time.
 */
void setAngleConverterRevolutionTime(uint32_t revolutionTime) noexcept;

/**
 * @brief Converts angular degrees to the time interval that amount of rotation
 * will take at current RPM.
 * 
 * Based on angle of [0,720] and min/max RPM, result ranges from
 * 9 (MAX_RPM, 1 deg) to 2926828 (MIN_RPM, 720 deg)
 *
 * @param angle Angle in degrees
 * @return Time interval in uS
 */
uint32_t angleToTime(uint16_t angle) noexcept;

/**
 * @brief Converts angular degrees to the equivalent timer ticks at current RPM.
 * 
 * @param angle Angle in degrees
 * @return Number of timer ticks 
 */
COMPARE_TYPE angleToTimerTicks(uint16_t angle) noexcept;

/**
 * @brief Converts a time interval in microsecods to the equivalent degrees of angular (crank)
 * rotation at current RPM.
 *
 * Inverse of angleToTime
 *
 * @param time Time interval in uS
 * @return Angle in degrees
 */
uint16_t timeToAngle(uint32_t time) noexcept;

#endif