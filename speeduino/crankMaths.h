#ifndef CRANKMATHS_H
#define CRANKMATHS_H

#include "maths.h"
#include "globals.h"

void doCrankSpeedCalcs(void);

#define ignitionLimits(angle) ( (((int16_t)(angle)) >= CRANK_ANGLE_MAX_IGN) ? ((angle) - CRANK_ANGLE_MAX_IGN) : ( ((int16_t)(angle) < 0) ? ((angle) + CRANK_ANGLE_MAX_IGN) : (angle)) )

/** @brief At 1 RPM, each degree of angular rotation takes this many microseconds */
#define US_PER_DEG_PER_RPM 166666UL

/** @brief The maximum rpm that the ECU will attempt to run at. 
 * 
 * It is NOT related to the rev limiter, but is instead dictates how fast certain operations will be
 * allowed to run. Lower number gives better performance 
 **/
#define MAX_RPM 18000U

/** @brief Absolute minimum RPM that the crank math (& therefore all of Speeduino) can be used with 
 * 
 * This is dictated by the use of uint16_t as the base type for storing
 * angle to time conversion factor (timePerDegreex16)
*/
#define MIN_RPM ((US_PER_DEG_PER_RPM/(UINT16_MAX/16UL))+1UL)

/**
 * @name Converts angular degrees to the time interval that amount of rotation
 * will take at current RPM.
 * 
 * Based on angle of [0,720] and min/max RPM, result ranges from
 * 9 (MAX_RPM, 1 deg) to 2926828 (MIN_RPM, 720 deg)
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

/** @brief Converts based on the time the last full crank revolution took 
 * 
 * Much slower than angleToTimeMicroSecPerDegree but also more accurate
*/
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