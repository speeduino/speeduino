#ifndef MATH_H
#define MATH_H

int fastMap1023toX(int, int);
unsigned long percentage(byte, unsigned long);

#define degreesToUS(degrees) (decoderIsLowRes == true ) ? ((degrees * 166666UL) / currentStatus.RPM) : degrees * (unsigned long)timePerDegree
#define uSToDegrees(time) (((unsigned long)time * currentStatus.RPM) / 166666)

#endif
