// Note that all functions with an underscore prefix are NOT part 
// of the public API. They are only here so we can inline them.

#include "scheduler.h"
#include "crankMaths.h"
#include "maths.h"
#include "timers.h"

static inline uint16_t calculateInjectorStartAngle(uint16_t pwDegrees, int16_t injChannelDegrees, uint16_t injAngle)
{
  // 0<=injAngle<=720°
  // 0<=injChannelDegrees<=720°
  // 0<pwDegrees<=??? (could be many crank rotations in the worst case!)
  // 45<=CRANK_ANGLE_MAX_INJ<=720
  // (CRANK_ANGLE_MAX_INJ can be as small as 360/nCylinders. E.g. 45° for 8 cylinder)

  uint16_t startAngle = (uint16_t)injAngle + (uint16_t)injChannelDegrees;
  
  while (startAngle<pwDegrees) { startAngle = startAngle + (uint16_t)CRANK_ANGLE_MAX_INJ; } // Avoid underflow
  startAngle = startAngle - pwDegrees; // startAngle guaranteed to be >=0.
  while (startAngle>(uint16_t)CRANK_ANGLE_MAX_INJ) { startAngle = startAngle - (uint16_t)CRANK_ANGLE_MAX_INJ; } // Clamp to 0<=startAngle<=CRANK_ANGLE_MAX_INJ

  return startAngle;
}

static inline uint32_t calculateInjectorTimeout(const FuelSchedule &schedule, int openAngle, int crankAngle)
{
  uint32_t tempTimeout = 0;
  int16_t delta = openAngle - crankAngle;

  if(delta > 0) { tempTimeout = angleToTimeMicroSecPerDegree((uint16_t)delta); }
  else if ( (schedule.Status == RUNNING) || (schedule.Status == OFF)) 
  {
    while(delta < 0) { delta += CRANK_ANGLE_MAX_INJ; }
    tempTimeout = angleToTimeMicroSecPerDegree((uint16_t)delta);
  }
  
  return tempTimeout;
}

static inline void calculateIgnitionAngle(const uint16_t dwellAngle, const uint16_t channelIgnDegrees, int8_t advance, int *pEndAngle, int *pStartAngle)
{
  *pEndAngle = (int16_t)(channelIgnDegrees==0U ? (uint16_t)CRANK_ANGLE_MAX_IGN : channelIgnDegrees) - (int16_t)advance;
  if(*pEndAngle > CRANK_ANGLE_MAX_IGN) {*pEndAngle -= CRANK_ANGLE_MAX_IGN;}
  *pStartAngle = *pEndAngle - dwellAngle;
  if(*pStartAngle < 0) {*pStartAngle += CRANK_ANGLE_MAX_IGN;}
}

static inline void calculateIgnitionTrailingRotary(uint16_t dwellAngle, int rotarySplitDegrees, int leadIgnitionAngle, int *pEndAngle, int *pStartAngle)
{
  *pEndAngle = leadIgnitionAngle + rotarySplitDegrees;
  *pStartAngle = *pEndAngle - dwellAngle;
  if(*pStartAngle > CRANK_ANGLE_MAX_IGN) {*pStartAngle -= CRANK_ANGLE_MAX_IGN;}
  if(*pStartAngle < 0) {*pStartAngle += CRANK_ANGLE_MAX_IGN;}
}

static inline uint32_t _calculateIgnitionTimeout(const IgnitionSchedule &schedule, int16_t startAngle, int16_t crankAngle) 
{
  int16_t delta = startAngle - crankAngle;
  if (delta < 0)
  {
    if ((schedule.Status == RUNNING) && (delta>-CRANK_ANGLE_MAX_IGN)) 
    { 
      // Msut be >0
      delta = delta + CRANK_ANGLE_MAX_IGN; 
    }
    else
    {
      return 0U;
    }
  }

  return angleToTimeMicroSecPerDegree(delta);
}

static inline uint16_t _adjustToIgnChannel(int angle, int channelInjDegrees) 
{
  angle = angle - channelInjDegrees;
  if( angle < 0) { return angle + CRANK_ANGLE_MAX_IGN; }
  return angle;
}

static inline uint32_t calculateIgnitionTimeout(const IgnitionSchedule &schedule, int startAngle, int channelIgnDegrees, int crankAngle)
{
  if (channelIgnDegrees == 0) 
  {
      return _calculateIgnitionTimeout(schedule, startAngle, crankAngle);
  }
  return _calculateIgnitionTimeout(schedule, _adjustToIgnChannel(startAngle, channelIgnDegrees), _adjustToIgnChannel(crankAngle, channelIgnDegrees));
}

#define MIN_CYCLES_FOR_ENDCOMPARE 6

inline void adjustCrankAngle(IgnitionSchedule &schedule, int endAngle, int crankAngle) 
{
  if( (schedule.Status == RUNNING) ) { 
    SET_COMPARE(schedule.compare, schedule.counter + uS_TO_TIMER_COMPARE( angleToTimeMicroSecPerDegree( ignitionLimits( (endAngle - crankAngle) ) ) ) ); 
  }
  else if(currentStatus.startRevolutions > MIN_CYCLES_FOR_ENDCOMPARE) { 
    schedule.endCompare = schedule.counter + uS_TO_TIMER_COMPARE( angleToTimeMicroSecPerDegree( ignitionLimits( (endAngle - crankAngle) ) ) ); 
    schedule.endScheduleSetByDecoder = true; 
  }
}