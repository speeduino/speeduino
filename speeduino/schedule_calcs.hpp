// Note that all functions with an underscore prefix are NOT part 
// of the public API. They are only here so we can inline them.

#include "scheduler.h"
#include "crankMaths.h"

inline uint16_t calculateInjectorStartAngle(uint16_t pwDegrees, int16_t injChannelDegrees, uint16_t injAngle)
{
  uint16_t startAngle = injAngle + injChannelDegrees;
  if (startAngle>pwDegrees) {
    return startAngle - pwDegrees;
  } 
  return (startAngle + CRANK_ANGLE_MAX_INJ) - pwDegrees;
}

inline uint32_t _calculateInjectorTimeout(const FuelSchedule &schedule, uint16_t openAngle, uint16_t crankAngle) {
  int16_t delta = openAngle - crankAngle;
  if (delta<0)
  {
    if ((schedule.Status == RUNNING) && (delta>-CRANK_ANGLE_MAX_INJ)) 
    { 
      // Guarenteed to be >0
      delta = delta + CRANK_ANGLE_MAX_INJ; 
    }
    else
    {
      return 0;
    }
  }

  return ((uint32_t)(delta) * (uint32_t)timePerDegree);
}

static inline int _adjustToInjChannel(int angle, int channelInjDegrees) {
  angle = angle - channelInjDegrees;
  if( angle < 0) { return angle + CRANK_ANGLE_MAX_INJ; }
  return angle;
}

inline uint32_t calculateInjectorTimeout(const FuelSchedule &schedule, int channelInjDegrees, int openAngle, int crankAngle)
{
  if (channelInjDegrees==0) {
    return _calculateInjectorTimeout(schedule, openAngle, crankAngle);
  }
  return _calculateInjectorTimeout(schedule, _adjustToInjChannel(openAngle, channelInjDegrees), _adjustToInjChannel(crankAngle, channelInjDegrees));
}

inline void calculateIgnitionAngle(const int dwellAngle, const uint16_t channelIgnDegrees, int8_t advance, int *pEndAngle, int *pStartAngle)
{
  *pEndAngle = (channelIgnDegrees==0 ? CRANK_ANGLE_MAX_IGN : channelIgnDegrees) - advance;
  if(*pEndAngle > CRANK_ANGLE_MAX_IGN) {*pEndAngle -= CRANK_ANGLE_MAX_IGN;}
  *pStartAngle = *pEndAngle - dwellAngle;
  if(*pStartAngle < 0) {*pStartAngle += CRANK_ANGLE_MAX_IGN;}
}

inline void calculateIgnitionTrailingRotary(int dwellAngle, int rotarySplitDegrees, int leadIgnitionAngle, int *pEndAngle, int *pStartAngle)
{
  *pEndAngle = leadIgnitionAngle + rotarySplitDegrees;
  *pStartAngle = *pEndAngle - dwellAngle;
  if(*pStartAngle > CRANK_ANGLE_MAX_IGN) {*pStartAngle -= CRANK_ANGLE_MAX_IGN;}
  if(*pStartAngle < 0) {*pStartAngle += CRANK_ANGLE_MAX_IGN;}
}

inline uint32_t _calculateIgnitionTimeout(const Schedule &schedule, int16_t startAngle, int16_t crankAngle) {
  int16_t delta = startAngle - crankAngle;
  if (delta<0)
  {
    if ((schedule.Status == RUNNING) && (delta>-CRANK_ANGLE_MAX_IGN)) 
    { 
      // Msut be >0
      delta = delta + CRANK_ANGLE_MAX_IGN; 
    }
    else
    {
      return 0;
    }
  }
  return angleToTimeIntervalRev(delta);
}

static inline uint16_t _adjustToIgnChannel(int angle, int channelInjDegrees) {
  angle = angle - channelInjDegrees;
  if( angle < 0) { return angle + CRANK_ANGLE_MAX_IGN; }
  return angle;
}

inline uint32_t calculateIgnitionTimeout(const Schedule &schedule, int startAngle, int channelIgnDegrees, int crankAngle)
{
  if (channelIgnDegrees==0) {
      return _calculateIgnitionTimeout(schedule, startAngle, crankAngle);
  }
  return _calculateIgnitionTimeout(schedule, _adjustToIgnChannel(startAngle, channelIgnDegrees), _adjustToIgnChannel(crankAngle, channelIgnDegrees));
}
