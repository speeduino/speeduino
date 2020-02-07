#ifndef MATH_H
#define MATH_H

#include <Arduino.h>

int fastMap1023toX(int, int);
unsigned long percentage(byte, unsigned long);
int divs100(long);
unsigned long divu100(unsigned long);

/*
 * Calculates integer power values. Same as pow() but with ints
 */
inline long powint(int factor, unsigned int exponent)
{
   long product = 1;
   unsigned int counter = exponent;
   while ( (counter--) > 0) { product *= factor; }
   return product;
}

#define DIV_ROUND_CLOSEST(n, d) ((((n) < 0) ^ ((d) < 0)) ? (((n) - (d)/2)/(d)) : (((n) + (d)/2)/(d)))

//This is a dedicated function that specifically handles the case of mapping 0-1023 values into a 0 to X range
//This is a common case because it means converting from a standard 10-bit analog input to a byte or 10-bit analog into 0-511 (Eg the temperature readings)
#if defined(_VARIANT_ARDUINO_STM32_) /*libmaple */ //ST stm32duino core returns 0 - 1023 for analog read first use analogReadResolution(12); to make it 12 bit
  #define fastMap1023toX(x, out_max) ( ((unsigned long)x * out_max) >> 12)
  //This is a new version that allows for out_min
  #define fastMap10Bit(x, out_min, out_max) ( ( ((unsigned long)x * (out_max-out_min)) >> 12 ) + out_min)
#else
  #define fastMap1023toX(x, out_max) ( ((unsigned long)x * out_max) >> 10)
  //This is a new version that allows for out_min
  #define fastMap10Bit(x, out_min, out_max) ( ( ((unsigned long)x * (out_max-out_min)) >> 10 ) + out_min)
#endif

#endif
