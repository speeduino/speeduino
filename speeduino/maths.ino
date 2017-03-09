#include "maths.h"

#define DIV_ROUND_CLOSEST(n, d) ((((n) < 0) ^ ((d) < 0)) ? (((n) - (d)/2)/(d)) : (((n) + (d)/2)/(d)))

//Replace the standard arduino map() function to use the div function instead
int fastMap(unsigned long x, int in_min, int in_max, int out_min, int out_max)
{
  return ldiv( ((x - in_min) * (out_max - out_min)) , (in_max - in_min) ).quot + out_min;
  //return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//This is a dedicated function that specifically handles the case of mapping 0-1023 values into a 0 to X range 
//This is a common case because it means converting from a standard 10-bit analog input to a byte or 10-bit analog into 0-511 (Eg the temperature readings)
//int fastMap1023toX(unsigned long x, int in_min, int in_max, int out_min, int out_max)
//removed ununsed variables, in_min and out_min is aways 0, in_max is aways 1023
int fastMap1023toX(unsigned long x, int out_max)
{
 return (x * out_max) >> 10;
}

/*
The following are all fast versions of specific divisions
Ref: http://www.hackersdelight.org/divcMore.pdf
*/

//Unsigned divide by 10
unsigned int divu10(unsigned int n) {
 unsigned long q, r;
 q = (n >> 1) + (n >> 2);
 q = q + (q >> 4);
 q = q + (q >> 8);
 q = q + (q >> 16);
 q = q >> 3;
 r = n - q*10;
 return q + ((r + 6) >> 4);
// return q + (r > 9);
}

//Signed divide by 10
int divs10(long n) {
 long q, r;
 n = n + (n>>31 & 9);
 q = (n >> 1) + (n >> 2);
 q = q + (q >> 4);
 q = q + (q >> 8);
 q = q + (q >> 16);
 q = q >> 3;
 r = n - q*10;
 return q + ((r + 6) >> 4);
// return q + (r > 9);
}

//Signed divide by 100
int divs100(long n) {
  return (n / 100); // Amazingly, gcc is producing a better /divide by 100 function than this
 long q, r;
 n = n + (n>>31 & 99);
 q = (n >> 1) + (n >> 3) + (n >> 6) - (n >> 10) +
 (n >> 12) + (n >> 13) - (n >> 16);
 q = q + (q >> 20);
 q = q >> 6;
 r = n - q*100;
 return q + ((r + 28) >> 7);
// return q + (r > 99);
}

//Unsigned divide by 100
unsigned long divu100(unsigned long n) {
  //return (n / 100); // No difference with this on/off
 unsigned long q, r;
 q = (n >> 1) + (n >> 3) + (n >> 6) - (n >> 10) +
 (n >> 12) + (n >> 13) - (n >> 16);
 q = q + (q >> 20);
 q = q >> 6;
 r = n - q*100;
 return q + ((r + 28) >> 7);
// return q + (r > 99);
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
   while (exponent--)
      product *= factor;
   return product;
}
