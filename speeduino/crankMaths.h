#ifndef CRANKMATHS_H
#define CRANKMATHS_H

#define CRANKMATH_METHOD_INTERVAL_DEFAULT  0
#define CRANKMATH_METHOD_INTERVAL_REV      1
#define CRANKMATH_METHOD_INTERVAL_TOOTH    2
#define CRANKMATH_METHOD_ALPHA_BETA        3
#define CRANKMATH_METHOD_2ND_DERIVATIVE    4

//#define fastDegreesToUS(targetDegrees) ((targetDegrees) * (unsigned long)timePerDegree)
#define fastDegreesToUS(targetDegrees) (((targetDegrees) * (unsigned long)timePerDegreex16) >> 4)
/*#define fastTimeToAngle(time) (((unsigned long)time * degreesPeruSx2048) / 2048) */ //Divide by 2048 will be converted at compile time to bitshift
#define fastTimeToAngle(time) (((unsigned long)(time) * degreesPeruSx32768) / 32768) //Divide by 32768 will be converted at compile time to bitshift

#define ignitionLimits(angle) ( (((int16_t)(angle)) >= CRANK_ANGLE_MAX_IGN) ? ((angle) - CRANK_ANGLE_MAX_IGN) : ( ((int16_t)(angle) < 0) ? ((angle) + CRANK_ANGLE_MAX_IGN) : (angle)) )


unsigned long angleToTime(int16_t, byte);
uint16_t timeToAngle(unsigned long, byte);
void doCrankSpeedCalcs();

volatile uint16_t timePerDegree;
volatile uint16_t timePerDegreex16;
volatile uint16_t degreesPeruSx2048;
volatile unsigned long degreesPeruSx32768;

//These are only part of the experimental 2nd deriv calcs
byte deltaToothCount = 0; //The last tooth that was used with the deltaV calc
int rpmDelta;

#endif