#include "globals.h"
#include "scheduler.h"
#include "crankMaths.h"

inline uint16_t calculateInjectorStartAngle(uint16_t PWdivTimerPerDegree, int16_t injChannelDegrees)
{
  uint16_t tempInjectorStartAngle = (currentStatus.injAngle + injChannelDegrees);
  if(tempInjectorStartAngle < PWdivTimerPerDegree) { tempInjectorStartAngle += CRANK_ANGLE_MAX_INJ; }
  tempInjectorStartAngle -= PWdivTimerPerDegree;
  while(tempInjectorStartAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { tempInjectorStartAngle -= CRANK_ANGLE_MAX_INJ; }

  return tempInjectorStartAngle;
}

inline uint32_t calculateInjectorTimeout(const FuelSchedule &schedule, int channelInjDegrees, int injectorStartAngle, int crankAngle)
{
    int tempCrankAngle = crankAngle - channelInjDegrees;
    if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
    int tempStartAngle = injectorStartAngle - channelInjDegrees;
    if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
    if ( (tempStartAngle <= tempCrankAngle) && (schedule.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
    if ( tempStartAngle > tempCrankAngle )
    {
        return (((uint32_t)tempStartAngle - (uint32_t)tempCrankAngle) * (uint32_t)timePerDegree);
    }
    return 0U;
}

inline void calculateIgnitionAngle(const int dwellAngle, const uint16_t channelIgnDegrees, int *pEndAngle, int *pStartAngle)
{
  *pEndAngle = (channelIgnDegrees==0 ? CRANK_ANGLE_MAX_IGN : channelIgnDegrees) - currentStatus.advance;
  if(*pEndAngle > CRANK_ANGLE_MAX_IGN) {*pEndAngle -= CRANK_ANGLE_MAX_IGN;}
  *pStartAngle = *pEndAngle - dwellAngle;
  if(*pStartAngle < 0) {*pStartAngle += CRANK_ANGLE_MAX_IGN;}
}

inline void calculateIgnitionAngle1(int dwellAngle)
{
  calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, &ignition1EndAngle, &ignition1StartAngle);
}

inline void calculateIgnitionAngle2(int dwellAngle)
{
  calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, &ignition2EndAngle, &ignition2StartAngle);
}

inline void calculateIgnitionAngle3(int dwellAngle)
{
  calculateIgnitionAngle(dwellAngle, channel3IgnDegrees, &ignition3EndAngle, &ignition3StartAngle);
}

// ignition 3 for rotary
inline void calculateIgnitionAngle3(int dwellAngle, int rotarySplitDegrees)
{
  ignition3EndAngle = ignition1EndAngle + rotarySplitDegrees;
  ignition3StartAngle = ignition3EndAngle - dwellAngle;
  if(ignition3StartAngle > CRANK_ANGLE_MAX_IGN) {ignition3StartAngle -= CRANK_ANGLE_MAX_IGN;}
  if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}
}

inline void calculateIgnitionAngle4(int dwellAngle)
{
  calculateIgnitionAngle(dwellAngle, channel4IgnDegrees, &ignition4EndAngle, &ignition4StartAngle);
}

// ignition 4 for rotary
inline void calculateIgnitionAngle4(int dwellAngle, int rotarySplitDegrees)
{
  ignition4EndAngle = ignition2EndAngle + rotarySplitDegrees;
  ignition4StartAngle = ignition4EndAngle - dwellAngle;
  if(ignition4StartAngle > CRANK_ANGLE_MAX_IGN) {ignition4StartAngle -= CRANK_ANGLE_MAX_IGN;}
  if(ignition4StartAngle < 0) {ignition4StartAngle += CRANK_ANGLE_MAX_IGN;}
}

inline void calculateIgnitionAngle5(int dwellAngle)
{
  calculateIgnitionAngle(dwellAngle, channel5IgnDegrees, &ignition5EndAngle, &ignition5StartAngle);
}

inline void calculateIgnitionAngle6(int dwellAngle)
{
  calculateIgnitionAngle(dwellAngle, channel6IgnDegrees, &ignition6EndAngle, &ignition6StartAngle);
}

inline void calculateIgnitionAngle7(int dwellAngle)
{
  calculateIgnitionAngle(dwellAngle, channel7IgnDegrees, &ignition7EndAngle, &ignition7StartAngle);
}

inline void calculateIgnitionAngle8(int dwellAngle)
{
  calculateIgnitionAngle(dwellAngle, channel8IgnDegrees, &ignition8EndAngle, &ignition8StartAngle);
}

inline uint32_t calculateIgnitionTimeout(const Schedule &schedule, int startAngle, int channelIgnDegrees, int crankAngle)
{
    int tempCrankAngle = crankAngle - channelIgnDegrees;
    if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_IGN; }
    int tempStartAngle = startAngle - channelIgnDegrees;
    if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }

    if ( (tempStartAngle <= tempCrankAngle) && (schedule.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
    if(tempStartAngle > tempCrankAngle)
    { 
        return angleToTime((tempStartAngle - tempCrankAngle), CRANKMATH_METHOD_INTERVAL_REV); 
    }
    
    return 0U;

}
