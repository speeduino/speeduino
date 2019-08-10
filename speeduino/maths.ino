#include "maths.h"
#include "globals.h"

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

/*
The following are all fast versions of specific divisions
Ref: www.hackersdelight.org/divcMore.pdf
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
  //return (n / 100);
  unsigned long q, r;
  q = (n >> 1) + (n >> 3) + (n >> 6) - (n >> 10) +
  (n >> 12) + (n >> 13) - (n >> 16);
  q = q + (q >> 20);
  q = q >> 6;
  r = n - (q * 100);
  return q + ((r + 28) >> 7);
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
