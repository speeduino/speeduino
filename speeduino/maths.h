#ifndef MATH_H
#define MATH_H

int fastMap1023toX(int, int);
unsigned long percentage(byte, unsigned long);
inline long powint(int, unsigned int);
int divs100(long);
unsigned long divu100(unsigned long);

#define DIV_ROUND_CLOSEST(n, d) ((((n) < 0) ^ ((d) < 0)) ? (((n) - (d)/2)/(d)) : (((n) + (d)/2)/(d)))

//This is a dedicated function that specifically handles the case of mapping 0-1023 values into a 0 to X range
//This is a common case because it means converting from a standard 10-bit analog input to a byte or 10-bit analog into 0-511 (Eg the temperature readings)
#define fastMap1023toX(x, out_max) ( ((unsigned long)x * out_max) >> 10)
//This is a new version that allows for out_min
#define fastMap10Bit(x, out_min, out_max) ( ( ((unsigned long)x * (out_max-out_min)) >> 10 ) + out_min)

#endif