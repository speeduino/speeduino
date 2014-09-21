/*
These are some utility functions and variables used through the main code
*/ 
#include <Arduino.h>
#define MS_IN_MINUTE 60000
#define US_IN_MINUTE 60000000

/*
Returns how much free dynamic memory exists (between heap and stack)
*/
int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


/* The following functions help determine the required fuel constant. For more information about these calculations, please refer to http://www.megamanual.com/v22manual/mfuel.htm
  Calc below are for metric inputs of temp (degrees C) and MAP (kPa) to produce kg/m3.
*/
int AIRDEN(int MAP, int temp)
  {
	return (1.2929 * 273.13/(temp+273.13) * MAP/101.325);
  }

/*
This function retuns a pulsewidth time (in us) using a either Alpha-N or Speed Density algorithms, given the following:
REQ_FUEL
VE: Lookup from the main MAP vs RPM fuel table
MAP: In KPa, read from the sensor
GammaE: Sum of Enrichment factors (Cold start, acceleration). This is a multiplication factor (Eg to add 10%, this should be 110)
injDT: Injector dead time. The time the injector take to open minus the time it takes to close (Both in uS)
TPS: Throttle position (0% to 100%)

This function is called by PW_SD and PW_AN for speed0density and pure Alpha-N calculations respectively. 
*/
unsigned int PW(int REQ_FUEL, byte VE, byte MAP, int corrections, int injOpen, byte TPS)
  {
    //Standard float version of the calculation
    //return (REQ_FUEL * (float)(VE/100.0) * (float)(MAP/100.0) * (float)(TPS/100.0) * (float)(corrections/100.0) + injOpen);
    
    //100% float free version, does sacrifice a little bit of accuracy. Accuracy loss is in the order of 0.1ms (100uS)
    int iVE = ((int)VE << 7) / 100;
    int iMAP = ((int)MAP << 7) / 100;
    int iCorrections = (corrections << 7) / 100;
    int iTPS = ((int)TPS << 7) / 100;

    unsigned long intermediate = ((long)REQ_FUEL * (long)iVE) >>7; //Need to use an intermediate value to avoid overflowing the long
    intermediate = (intermediate * iMAP) >> 7;
    intermediate = (intermediate * iCorrections) >> 7;
    intermediate = (intermediate * iTPS) >> 7;
    unsigned int final = (unsigned int)(intermediate + injOpen);
    //if (final > 65535) { final = 65535; } //Make sure this won't overflow. This means the maximum pulsewidth possible is 65.535mS
    return final;

  }
 
//Convenience functions for Speed Density and Alpha-N
unsigned int PW_SD(int REQ_FUEL, byte VE, byte MAP, int corrections, int injOpen)
{
  return PW(REQ_FUEL, VE, MAP, corrections, injOpen, 100); //Just use 1 in place of the TPS
}

unsigned int PW_AN(int REQ_FUEL, byte VE, byte TPS, int corrections, int injOpen)
{
  return PW(REQ_FUEL, VE, 100, corrections, injOpen, TPS); //Just use 1 in place of the MAP
}
