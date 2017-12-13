#ifndef MATH_H
#define MATH_H

int fastMap1023toX(int, int);
unsigned long percentage(byte, unsigned long);

//#define degreesToUS(degrees) (decoderIsLowRes == true ) ? ((degrees * 166666UL) / currentStatus.RPM) : (degrees * (unsigned long)timePerDegree)
#define degreesToUS(degrees) ((degrees * revolutionTime) / 360)
#define fastDegreesToUS(degrees) (degrees * (unsigned long)timePerDegree)
//#define degreesToUS(degrees) ((degrees * revolutionTime * 3054198967ULL) >> 40) //Fast version of divide by 360
//#define degreesToUS(degrees) (degrees * (unsigned long)timePerDegree)

#define uSToDegrees(time) (((unsigned long)time * currentStatus.RPM) / 166666)
//#define uSToDegrees(time) ( (((uint64_t)time * currentStatus.RPM * 211107077ULL) >> 45) ) //Crazy magic numbers method from Hackers delight (www.hackersdelight.org/magic.htm)
#define DIV_ROUND_CLOSEST(n, d) ((((n) < 0) ^ ((d) < 0)) ? (((n) - (d)/2)/(d)) : (((n) + (d)/2)/(d)))

//This is a dedicated function that specifically handles the case of mapping 0-1023 values into a 0 to X range
//This is a common case because it means converting from a standard 10-bit analog input to a byte or 10-bit analog into 0-511 (Eg the temperature readings)
#if defined(_VARIANT_ARDUINO_STM32_) //libmaple
  #define fastMap1023toX(x, out_max) ( ((unsigned long)x * out_max) >> 12)
  //This is a new version that allows for out_min
  #define fastMap10Bit(x, out_min, out_max) ( ( ((unsigned long)x * (out_max-out_min)) >> 12 ) + out_min)
#else
  #define fastMap1023toX(x, out_max) ( ((unsigned long)x * out_max) >> 10)
  //This is a new version that allows for out_min
  #define fastMap10Bit(x, out_min, out_max) ( ( ((unsigned long)x * (out_max-out_min)) >> 10 ) + out_min)
#endif

#endif
