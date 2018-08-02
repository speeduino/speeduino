#ifndef CRANKMATHS_H
#define CRANKMATHS_H

#define CRANKMATH_METHOD_INTERVAL_DEFAULT  0
#define CRANKMATH_METHOD_INTERVAL_REV      1
#define CRANKMATH_METHOD_INTERVAL_TOOTH    2
#define CRANKMATH_METHOD_ALPHA_BETA        3
#define CRANKMATH_METHOD_2ND_DERIVATIVE    4

#define fastDegreesToUS(degrees) (degrees * (unsigned long)timePerDegree)

unsigned long angleToTime(int16_t, byte);
uint16_t timeToAngle(unsigned long, byte);

volatile int timePerDegree;
volatile uint16_t degreesPeruSx2048;

#endif