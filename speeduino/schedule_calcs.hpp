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

static inline __attribute__((always_inline)) uint32_t _calculateAngularTime(const Schedule &schedule, uint16_t eventAngle, uint16_t crankAngle, uint16_t maxAngle) {
  int16_t delta = eventAngle - crankAngle;
  if ( (isRunning(schedule)) || (schedule.Status == OFF)) {
    while(delta < 0) { delta += (int16_t)maxAngle; }
  } 

  return delta > 0 ? angleToTimeMicroSecPerDegree((uint16_t)delta) : 0U;
}

static inline __attribute__((always_inline)) uint16_t _adjustToTDC(int angle, uint16_t angleOffset, uint16_t maxAngle) {
  angle = angle - (int)angleOffset;
  if( angle < 0) { return angle + (int)maxAngle; }
  return angle;
}

static inline __attribute__((always_inline)) uint32_t _calculateAngularTime(const Schedule &schedule, uint16_t angleOffset, uint16_t eventAngle, uint16_t crankAngle, uint16_t maxAngle) {
  if (angleOffset==0U) { // Optimize for zero channel angle - no need to adjust start & crank angles
    return _calculateAngularTime(schedule, eventAngle, crankAngle, maxAngle);
  }
  // Realign the current crank angle and the desired start angle around 0 degrees for the given cylinder/output
  // Eg: If cylinder 2 TDC is 180 degrees after cylinder 1 (E.g. a standard 4 cylinder engine), then
  // adjusted crank angle is 180* less than the current crank angle and adjusted start angle is the desired open angle less 180*. 
  // Thus the cylinder is being treated relative to its own TDC, regardless of its offset
  //
  // This is done to avoid very small or very large deltas between crank angle and start angle.
  return _calculateAngularTime(schedule, 
            _adjustToTDC(eventAngle, angleOffset, maxAngle),
            _adjustToTDC(crankAngle, angleOffset, maxAngle),
            maxAngle);
}

static inline uint32_t calculateInjectorTimeout(const FuelSchedule &schedule, int openAngle, int crankAngle)
{
  int16_t delta = openAngle - crankAngle;

  if (delta<0)
  {
    if (schedule.Status != PENDING)
    {
      while(delta < 0) { delta += CRANK_ANGLE_MAX_INJ; }
    }
    else
    {
      delta = 0;
    }
  }
  return angleToTimeMicroSecPerDegree((uint16_t)delta);
}

static inline void calculateIgnitionAngle(IgnitionSchedule &schedule, const uint16_t dwellAngle, int8_t advance)
{
  schedule.endAngle = (schedule.channelIgnDegrees==0 ? CRANK_ANGLE_MAX_IGN : schedule.channelIgnDegrees) - advance;
  if(schedule.endAngle > CRANK_ANGLE_MAX_IGN) {schedule.endAngle -= CRANK_ANGLE_MAX_IGN;}
  schedule.startAngle = schedule.endAngle - dwellAngle;
  if(schedule.startAngle < 0) {schedule.startAngle += CRANK_ANGLE_MAX_IGN;}
}

static inline void calculateIgnitionTrailingRotary(IgnitionSchedule &leading, uint16_t dwellAngle, int rotarySplitDegrees, IgnitionSchedule &trailing) {
  trailing.endAngle = leading.endAngle + rotarySplitDegrees;
  trailing.startAngle = trailing.endAngle - dwellAngle;
  if(trailing.startAngle > CRANK_ANGLE_MAX_IGN) {trailing.startAngle -= CRANK_ANGLE_MAX_IGN;}
  if(trailing.startAngle < 0) {trailing.startAngle += CRANK_ANGLE_MAX_IGN;}
}

static inline __attribute__((always_inline)) uint32_t _calculateIgnitionTimeout(const IgnitionSchedule &schedule, int crankAngle) 
{
  int16_t delta = schedule.startAngle - crankAngle;
  if (delta < 0)
  {
    if ((isRunning(schedule)) && (delta>-CRANK_ANGLE_MAX_IGN)) 
    { 
      // Must be >0
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

static inline uint32_t calculateIgnitionTimeout(const IgnitionSchedule &schedule, int crankAngle)
{
  return _calculateAngularTime(schedule, schedule.channelIgnDegrees, schedule.startAngle, crankAngle, CRANK_ANGLE_MAX_IGN);
}

// The concept here is that we have a more accurate crank angle.
// Ignition timing is driven by target spark angle relative to crank position.
static inline void adjustCrankAngle(IgnitionSchedule &schedule, int crankAngle) {
  constexpr uint8_t MIN_CYCLES_FOR_CORRECTION = 6U;

  if( isRunning(schedule) ) { 
    // Coil is charging so change the charge time so the spark fires at
    // the requested crank angle (this could reduce dwell time & potentially
    // result in a weaker spark).
    uint32_t timeToSpark = angleToTimeMicroSecPerDegree( ignitionLimits(schedule.endAngle-crankAngle) );
    COMPARE_TYPE ticksToSpark = (COMPARE_TYPE)uS_TO_TIMER_COMPARE( timeToSpark );
    schedule._compare = schedule._counter + ticksToSpark; 
  }
  else if((schedule.Status==PENDING) && (currentStatus.startRevolutions > MIN_CYCLES_FOR_CORRECTION) ) { 
    // We are waiting for the timer to fire & start charging the coil.
    // Keep dwell (I.e. duration) constant (for better spark) - instead adjust the waiting period so 
    // the spark fires at the requested crank angle.
    uint32_t timeToRun = angleToTimeMicroSecPerDegree( ignitionLimits(schedule.startAngle-crankAngle) );
    COMPARE_TYPE ticksToRun = (COMPARE_TYPE)uS_TO_TIMER_COMPARE( timeToRun );
    schedule._compare = schedule._counter + ticksToRun; 
  } else {
    // Schedule isn't on, so no adjustment possible
    // But keep the MISRA police happy.
  }
}