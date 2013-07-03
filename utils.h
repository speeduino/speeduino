/*
These are some utility functions and variables used through the main code
*/ 

#define MS_IN_MINUTE 60000
#define US_IN_MINUTE 60000000


/* The following functions help determine the required fuel constant. For more information about these calculations, please refer to http://www.megamanual.com/v22manual/mfuel.htm
  Calc below are for metric inputs of temp (degrees C) and MAP (kPa) to produce kg/m3.
*/
int AIRDEN(int MAP, int temp)
  {
	return (1.2929 * 273.13/(temp+273.13) * MAP/101.325);
  }

/*
This functino retuns a pulsewidth time (in us) given the following:
REQ_FUEL
VE: Lookup from the main MAP vs RPM fuel table
MAP: In KPa, read from the sensor
GammaE: Sum of Enrichment factors (Cold start, acceleration). This is a multiplication factor (Eg to add 10%, this should be 110)
injOpen: Injector open time. The time the injector take to open in uS
*/
int PW(int REQ_FUEL, int VE, int MAP, int GammaE, int injOpen)
  {
    return (REQ_FUEL * (float)(VE/100.0) * (float)(MAP/100.0) * (float)(GammaE/100.0) + injOpen);
  }

/* Determine the Gamma Enrichment number. Forumla borrowed from MS2 manual... may be skipped/simplified for arduino!
WARMUP: Warmup Correction 
O2_CLOSED: Feedback from Closed Loop Operation
AIRTEMP: Air Temp correction <-- Skip?
BARO: Barometric Correction <-- Skip?
*/

int GammaE( int WARMUP, int O2_CLOSED, int AIRTEMP, int BARO)
  {
    return (WARMUP/100) * (O2_CLOSED/100) * (AIRTEMP/100) * (BARO/100);
  }
