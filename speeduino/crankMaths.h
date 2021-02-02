#ifndef CRANKMATHS_H
#define CRANKMATHS_H

#define CRANKMATH_METHOD_INTERVAL_DEFAULT  0
#define CRANKMATH_METHOD_INTERVAL_REV      1
#define CRANKMATH_METHOD_INTERVAL_TOOTH    2
#define CRANKMATH_METHOD_ALPHA_BETA        3
#define CRANKMATH_METHOD_2ND_DERIVATIVE    4

//#define fastDegreesToUS(targetDegrees) ((targetDegrees) * (uint32_t)timePerDegree)
#define fastDegreesToUS(targetDegrees) (((targetDegrees) * (uint32_t)timePerDegreex16) >> 4)
/*#define fastTimeToAngle(time) (((uint32_t)time * degreesPeruSx2048) / 2048) */ //Divide by 2048 will be converted at compile time to bitshift
#define fastTimeToAngle(time) (((uint32_t)(time) * degreesPeruSx32768) / 32768) //Divide by 32768 will be converted at compile time to bitshift

#define ignitionLimits(angle) ( (((int16_t)(angle)) >= CRANK_ANGLE_MAX_IGN) ? ((angle) - CRANK_ANGLE_MAX_IGN) : ( ((int16_t)(angle) < 0) ? ((angle) + CRANK_ANGLE_MAX_IGN) : (angle)) )


uint32_t angleToTime(int16_t, uint8_t);
uint16_t timeToAngle(uint32_t, uint8_t);
void doCrankSpeedCalcs();

volatile uint16_t timePerDegree;
volatile uint16_t timePerDegreex16;
volatile uint16_t degreesPeruSx2048;
volatile uint32_t degreesPeruSx32768;

//These are only part of the experimental 2nd deriv calcs
uint8_t deltaToothCount = 0; //The last tooth that was used with the deltaV calc
int16_t rpmDelta;

#endif
