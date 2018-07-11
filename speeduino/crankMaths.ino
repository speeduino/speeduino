/*
* Converts a crank angle into a time from or since that angle occurred.
* Positive angles are assumed to be in the future, negative angles in the past:
*   * Future angle calculations will use a predicted speed/acceleration
*   * Past angle calculations will use the known speed
* 
* Multiple prediction methods will be implemented here for testing:
*   * Last interval using both last full revolution and gap between last teeth
*   * 2nd derivative prediction (Speed + acceleration)
*   * Closed loop error correction (Alpha-beta filter). 
*/

#define fastDegreesToUS(degrees) (degrees * (unsigned long)timePerDegree)
unsigned long angleToTime(int16_t angle)
{
//#define degreesToUS(degrees) (decoderIsLowRes == true ) ? ((degrees * 166666UL) / currentStatus.RPM) : (degrees * (unsigned long)timePerDegree)
//#define degreesToUS(degrees) ((degrees * revolutionTime) / 360)
//Fast version of divide by 360:
//#define degreesToUS(degrees) ((degrees * revolutionTime * 3054198967ULL) >> 40)
//#define degreesToUS(degrees) (degrees * (unsigned long)timePerDegree)
    return ((angle * revolutionTime) / 360);
}

/*
* Convert a time (uS) into an angle at current speed
*/
uint16_t timeToAngle(unsigned long time)
{

//#define uSToDegrees(time) (((unsigned long)time * currentStatus.RPM) / 166666)
//Crazy magic numbers method from Hackers delight (www.hackersdelight.org/magic.htm):
//#define uSToDegrees(time) ( (((uint64_t)time * currentStatus.RPM * 211107077ULL) >> 45) ) 
    return (((unsigned long)time * currentStatus.RPM) / 166666);

/*
    switch(calculationAlgorithm)
    {
        case CRANKMATH_METHOD_INTERVAL_TOOTH: 
            returnValue = ((elapsedTime * triggerToothAngle) / toothTime);
    }
    */
    
}