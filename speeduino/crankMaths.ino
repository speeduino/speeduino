#include "globals.h"
#include "crankMaths.h"
#include "decoders.h"

/*
* Converts a crank angle into a time from or since that angle occurred.
* Positive angles are assumed to be in the future, negative angles in the past:
*   * Future angle calculations will use a predicted speed/acceleration
*   * Past angle calculations will use the known speed
* 
* Currently 4 methods are planned and/or available:
* 1) Last interval based on a full revolution
* 2) Last interval based on the time between the last 2 teeth (Crank Pattern dependant)
* 3) Closed loop error correction (Alpha-beta filter) 
* 4) 2nd derivative prediction (Speed + acceleration)
*/
unsigned long angleToTime(int16_t angle, byte method)
{
    unsigned long returnTime = 0;

    if( (method == CRANKMATH_METHOD_INTERVAL_REV) || (method == CRANKMATH_METHOD_INTERVAL_DEFAULT) )
    {
        returnTime = ((angle * revolutionTime) / 360);
        //returnTime = angle * (unsigned long)timePerDegree;
    }
    else if (method == CRANKMATH_METHOD_INTERVAL_TOOTH)
    {
        //Still uses a last interval method (ie retrospective), but bases the interval on the gap between the 2 most recent teeth rather than the last full revolution
        if(triggerToothAngleIsCorrect == true)
        {
            returnTime = ( ((toothLastToothTime - toothLastMinusOneToothTime) / triggerToothAngle) * angle );
        }
        else { returnTime = angleToTime(angle, CRANKMATH_METHOD_INTERVAL_REV); } //Safety check. This can occur if the last tooth seen was outside the normal pattern etc
    }

    return returnTime;
}

/*
* Convert a time (uS) into an angle at current speed
* Currently 4 methods are planned and/or available:
* 1) Last interval based on a full revolution
* 2) Last interval based on the time between the last 2 teeth (Crank Pattern dependant)
* 3) Closed loop error correction (Alpha-beta filter) 
* 4) 2nd derivative prediction (Speed + acceleration)
*/
uint16_t timeToAngle(unsigned long time, byte method)
{
    uint16_t returnAngle = 0;

    if( (method == CRANKMATH_METHOD_INTERVAL_REV) || (method == CRANKMATH_METHOD_INTERVAL_DEFAULT) )
    {
        //A last interval method of calculating angle that does not take into account any acceleration. The interval used is the time taken to complete the last full revolution
        //degreesPeruSx2048 is the number of degrees the crank moves per uS. This value is almost always <1uS, so it is multiplied by 2048. This allows an angle calcuation with only a multiply and a bitshift without any appreciable drop in accuracy
        returnAngle = (time * degreesPeruSx2048) / 2048; //Divide by 2048 will be converted at compile time to bitshift
    }
    else if (method == CRANKMATH_METHOD_INTERVAL_TOOTH)
    {
        //Still uses a last interval method (ie retrospective), but bases the interval on the gap between the 2 most recent teeth rather than the last full revolution
        if(triggerToothAngleIsCorrect == true)
        {
            returnAngle = ( (unsigned long)(time * triggerToothAngle) / (toothLastToothTime - toothLastMinusOneToothTime) );
        }
        else { returnAngle = timeToAngle(time, CRANKMATH_METHOD_INTERVAL_REV); } //Safety check. This can occur if the last tooth seen was outside the normal pattern etc
    }
    else if (method == CRANKMATH_METHOD_ALPHA_BETA)
    {
        //Not yet implemented. Default to Rev
        returnAngle = timeToAngle(time, CRANKMATH_METHOD_INTERVAL_REV);
    }
    else if (method == CRANKMATH_METHOD_2ND_DERIVATIVE)
    {
        //Not yet implemented. Default to Rev
        returnAngle = timeToAngle(time, CRANKMATH_METHOD_INTERVAL_REV);
    }

   return returnAngle;
    
}