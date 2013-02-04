/*
These are some utility functions and variables used through the main code
*/

#define MS_IN_MINUTE 60000
#define US_IN_MINUTE 60000000


//The following functions help determine the required fuel constant. For more information about these calculations, please refer to http://www.megamanual.com/v22manual/mfuel.htm
int AIRDEN(int MAP, int temp)
  {
    
  }
  
/*
This functino retuns a pulsewidth time (in tenths of a ms) given the following:
REQ_FUEL
VE: Lookup from the main MAP vs RPM fuel table
MAP: In KPa, read from the sensor
GammaE: Sum of Enrichment factors (Cold start, acceleration). This is a multiplication factor (Eg to add 10%, this should be 110)
injOpen: Injector open time. The time the injector take to open in tenths of a ms
*/
int PW(int REQ_FUEL, int VE, int MAP, int GammaE, int injOpen)
  {
    return REQ_FUEL * (VE/100) * (MAP/100) * (GammaE/100) + injOpen;
  }
