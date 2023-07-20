#ifndef CRANKMATHS_H
#define CRANKMATHS_H

#include "maths.h"

#define CRANKMATH_METHOD_INTERVAL_DEFAULT  0
#define CRANKMATH_METHOD_INTERVAL_REV      1
#define CRANKMATH_METHOD_INTERVAL_TOOTH    2
#define CRANKMATH_METHOD_ALPHA_BETA        3
#define CRANKMATH_METHOD_2ND_DERIVATIVE    4

#define SECOND_DERIV_ENABLED                0          

//#define fastDegreesToUS(targetDegrees) ((targetDegrees) * (unsigned long)timePerDegree)
#define fastDegreesToUS(targetDegrees) (((targetDegrees) * (unsigned long)timePerDegreex16) >> 4)
#define fastTimeToAngle(time) (((unsigned long)(time) * degreesPeruSx32768) / 32768U) //Divide by 32768 will be converted at compile time to bitshift

#define ignitionLimits(angle) ( (((int16_t)(angle)) >= CRANK_ANGLE_MAX_IGN) ? ((angle) - CRANK_ANGLE_MAX_IGN) : ( ((int16_t)(angle) < 0) ? ((angle) + CRANK_ANGLE_MAX_IGN) : (angle)) )


unsigned long angleToTime(uint16_t angle, byte method);
inline unsigned long angleToTimeIntervalRev(uint16_t angle) {
    return div360(angle * revolutionTime);
}
uint16_t timeToAngle(unsigned long time, byte method);
void doCrankSpeedCalcs(void);

// At 1 RPM, each degree of angular rotation takes this many microseconds
#define US_PER_DEG_PER_RPM 166666UL

// uS per degree at current RPM in UQ12.4 fixed point
// Using 16 bits means there is a hard lower bound of 
// 41 RPM in the system:
//   41 RPM == 4065.04 us per degree == 650440 UQ12.4
//   (40 RPM==66666 UQ12.4)
extern volatile uint16_t timePerDegreex16;

#define MIN_RPM ((US_PER_DEG_PER_RPM/(UINT16_MAX/16U))+1)

// Note that this is less accurate than using timePerDegreex16
// but will be faster in some cases due to it's limited range.
// Ranges from 4065 (MIN_RPM) to 9 (MAX_RPM)
extern volatile uint16_t timePerDegree;

// Degrees per uS in UQ1.15 fixed point.
// Ranges from 8 (0.000246) at MIN_RPM to 3542 (0.108) at MAX_RPM
extern volatile uint16_t degreesPeruSx32768;

#endif