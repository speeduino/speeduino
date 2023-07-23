#ifndef CRANKMATHS_H
#define CRANKMATHS_H

#include "maths.h"

void doCrankSpeedCalcs(void);

#define ignitionLimits(angle) ( (((int16_t)(angle)) >= CRANK_ANGLE_MAX_IGN) ? ((angle) - CRANK_ANGLE_MAX_IGN) : ( ((int16_t)(angle) < 0) ? ((angle) + CRANK_ANGLE_MAX_IGN) : (angle)) )

/** @brief At 1 RPM, each degree of angular rotation takes this many microseconds */
#define US_PER_DEG_PER_RPM 166666UL

/** @brief Absolute minimum RPM that the crank math (& therefore all of Speeduino) can be used with */
#define MIN_RPM ((US_PER_DEG_PER_RPM/(UINT16_MAX/16UL))+1UL)

/**
 * @name Converts angular degrees to the time interval that amount of rotation
 * will take at current RPM.
 *
 * @param angle Angle in degrees
 * @return Time interval in uS
 */
///@{
/** @brief Converts based on the time one degree of rotation takes 
 * 
 * Inverse of timeToAngleDegPerMicroSec
*/
uint32_t angleToTimeMicroSecPerDegree(uint16_t angle);

/** @brief Converts based on the time the last full crank revolution took */
uint32_t angleToTimeIntervalRev(uint16_t angle);

/** @brief Converts based on the time interval between the 2 most recently detected decoder teeth 
 * 
 * Inverse of timeToAngleIntervalTooth
*/
uint32_t angleToTimeIntervalTooth(uint16_t angle);
///@}

/**
 * @name Converts a time interval in microsecods to the equivalent degrees of angular (crank)
 * rotation at current RPM.
 *
 * @param time Time interval in uS
 * @return Angle in degrees
 */
///@{
/** @brief Converts based on the the interval on time one degree of rotation takes 
 * 
 * Inverse of angleToTimeMicroSecPerDegree
*/
uint16_t timeToAngleDegPerMicroSec(uint32_t time);

/** @brief Converts based on the time interval between the 2 most recently detected decoder teeth 
 * 
 * Inverse of angleToTimeIntervalTooth
*/
uint16_t timeToAngleIntervalTooth(uint32_t time);
///@}

#endif