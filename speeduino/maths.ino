#include "maths.h"
#include "globals.h"

//Replace the standard arduino map() function to use the div function instead
int16_t fastMap(uint32_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max)
{
  uint32_t a = (x - (uint32_t)in_min);
  int16_t b = (out_max - out_min);
  int16_t c = (in_max - in_min);
  int16_t d = (ldiv( (a * (int32_t)b) , (int32_t)c ).quot);
  return d + out_min;
  //return ldiv( ((x - in_min) * (out_max - out_min)) , (in_max - in_min) ).quot + out_min;
  //return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*
The following are all fast versions of specific divisions
Ref: www.hackersdelight.org/divcMore.pdf
*/

//Unsigned divide by 10
uint16_t divu10(uint16_t n)
{
  uint32_t q, r;
  q = (n >> 1) + (n >> 2);
  q = q + (q >> 4);
  q = q + (q >> 8);
  q = q + (q >> 16);
  q = q >> 3;
  r = n - (q * 10);
  return q + ((r + 6) >> 4);
}

//Signed divide by 100
int16_t divs100(int32_t n)
{
  return (n / 100); // Amazingly, gcc is producing a better /divide by 100 function than this
  /*
  int32_t q, r;
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
uint32_t divu100(uint32_t n)
{
  //return (n / 100);
  uint32_t q, r;
  q = (n >> 1) + (n >> 3) + (n >> 6) - (n >> 10) +
  (n >> 12) + (n >> 13) - (n >> 16);
  q = q + (q >> 20);
  q = q >> 6;
  r = n - (q * 100);
  return q + ((r + 28) >> 7);
}

//Return x percent of y
//This is a relatively fast approximation of a percentage value.
uint32_t percentage(uint8_t x, uint32_t y)
{
  return (y * x) / 100; //For some reason this is faster
  //return divu100(y * x);
}

/*
 * Calculates integer power values. Same as pow() but with ints
 */
inline int32_t powint(int16_t factor, uint16_t exponent)
{
   int32_t product = 1;
   uint16_t counter = exponent;
   while ( (counter--) > 0) { product *= factor; }
   return product;
}
