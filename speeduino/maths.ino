#include "maths.h"

#define DIV_ROUND_CLOSEST(n, d) ((((n) < 0) ^ ((d) < 0)) ? (((n) - (d)/2)/(d)) : (((n) + (d)/2)/(d)))

//Replace the standard arduino map() function to use the div function instead
int fastMap(unsigned long x, int in_min, int in_max, int out_min, int out_max)
{
  unsigned long a = (x - (unsigned long)in_min);
  int b = (out_max - out_min);
  int c = (in_max - in_min);
  int d = (ldiv( (a * (long)b) , (long)c ).quot);
  return d + out_min;
  //return ldiv( ((x - in_min) * (out_max - out_min)) , (in_max - in_min) ).quot + out_min;
  //return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//This is a dedicated function that specifically handles the case of mapping 0-1023 values into a 0 to X range
//This is a common case because it means converting from a standard 10-bit analog input to a byte or 10-bit analog into 0-511 (Eg the temperature readings)
//int fastMap1023toX(unsigned long x, int in_min, int in_max, int out_min, int out_max)
//removed ununsed variables, in_min and out_min is aways 0, in_max is aways 1023
#if defined(CORE_STM32)
  #define fastResize(x, out_max) ( ((unsigned long)x * out_max) >> 12)
#else  
  #define fastResize(x, out_max) ( ((unsigned long)x * out_max) >> 10)
#endif
/*
The following are all fast versions of specific divisions
Ref: http://www.hackersdelight.org/divcMore.pdf
*/

//Unsigned divide by 10
unsigned int divu10(unsigned int n)
{
  unsigned long q, r;
  q = (n >> 1) + (n >> 2);
  q = q + (q >> 4);
  q = q + (q >> 8);
  q = q + (q >> 16);
  q = q >> 3;
  r = n - (q * 10);
  return q + ((r + 6) >> 4);
}

//Signed divide by 10
int divs10(long n)
{
  long q, r, p;
  p = n + ( (n>>31) & 9);
  q = (p >> 1) + (p >> 2);
  q = q + (q >> 4);
  q = q + (q >> 8);
  q = q + (q >> 16);
  q = q >> 3;
  r = p - (q * 10);
  return q + ((r + 6) >> 4);
}

//Signed divide by 100
int divs100(long n)
{
  return (n / 100); // Amazingly, gcc is producing a better /divide by 100 function than this
  /*
  long q, r;
  n = n + (n>>31 & 99);
  q = (n >> 1) + (n >> 3) + (n >> 6) - (n >> 10) +
  (n >> 12) + (n >> 13) - (n >> 16);
  q = q + (q >> 20);
  q = q >> 6;
  r = n - q*100;
  return q + ((r + 28) >> 7);
  */
}

//Unsigned divide by 100
unsigned long divu100(unsigned long n)
{
  #if defined(CORE_STM32)
    return (n / 100); // No difference with this on/off
  #else
    unsigned long q, r;
    q = (n >> 1) + (n >> 3) + (n >> 6) - (n >> 10) +
    (n >> 12) + (n >> 13) - (n >> 16);
    q = q + (q >> 20);
    q = q >> 6;
    r = n - q*100;
    return q + ((r + 28) >> 7);
  #endif  
}

//Return x percent of y
//This is a relatively fast approximation of a percentage value.
unsigned long percentage(byte x, unsigned long y)
{
  return (y * x) / 100; //For some reason this is faster
  //return divu100(y * x);
}

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
