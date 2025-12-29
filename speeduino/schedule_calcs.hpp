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

static inline __attribute__((always_inline)) uint16_t _adjustToTDC(int16_t angle, uint16_t angleOffset, uint16_t maxAngle) {
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

static inline uint32_t calculateInjectorTimeout(const FuelSchedule &schedule, int16_t openAngle, int16_t crankAngle)
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

static inline int16_t _calculateSparkAngle(const IgnitionSchedule &schedule, int8_t advance) {
  int16_t angle = (schedule.channelDegrees==0 ? CRANK_ANGLE_MAX_IGN : schedule.channelDegrees) - advance;
  if(angle > CRANK_ANGLE_MAX_IGN) {angle -= CRANK_ANGLE_MAX_IGN;}
  return angle;
}

static inline int16_t _calculateCoilChargeAngle(uint16_t dwellAngle, int16_t dischargeAngle) {
  if (dischargeAngle>(int16_t)dwellAngle) {
    return dischargeAngle - (int16_t)dwellAngle;
  }
  return dischargeAngle + CRANK_ANGLE_MAX_IGN - (int16_t)dwellAngle;
}

static inline void calculateIgnitionAngles(IgnitionSchedule &schedule, uint16_t dwellAngle, int8_t advance)
{
  schedule.dischargeAngle = _calculateSparkAngle(schedule,  advance);
  schedule.chargeAngle = _calculateCoilChargeAngle(dwellAngle, schedule.dischargeAngle);
}


static inline void calculateIgnitionTrailingRotary(IgnitionSchedule &leading, uint16_t dwellAngle, int16_t rotarySplitDegrees, IgnitionSchedule &trailing) 
{
  trailing.dischargeAngle = (int16_t)ignitionLimits(leading.dischargeAngle + rotarySplitDegrees);
  trailing.chargeAngle = (int16_t)ignitionLimits(trailing.dischargeAngle - (int16_t)dwellAngle); 
}

static inline uint32_t _calculateIgnitionTimeout(const IgnitionSchedule &schedule, int16_t crankAngle)
{
  return _calculateAngularTime(schedule, schedule.channelDegrees, schedule.chargeAngle, crankAngle, CRANK_ANGLE_MAX_IGN);
}

// The concept here is that we have a more accurate crank angle.
// Ignition timing is driven by target spark angle relative to crank position.
// So the timing to begin & end charging the coil is based on crank angle.
// With a more accurate crank angle, we can increase the precision of the
// spark timing.
static inline void adjustCrankAngle(IgnitionSchedule &schedule, int16_t crankAngle) {
  constexpr uint8_t MIN_CYCLES_FOR_CORRECTION = 6U;

  ATOMIC() { // Prevent race conditions with the timer interrupt.
    // We only want to adjust the crank angle if we are running and the coil is charging or we are waiting for the timer to fire.
    if( isRunning(schedule) ) {
      if  (schedule.dischargeAngle>crankAngle) { 
        // Coil is charging so change the charge time so the spark fires at
        // the requested crank angle (this could reduce dwell time & potentially
        // result in a weaker spark).
        SET_COMPARE(schedule._compare, schedule._counter + angleToTimerTicks( ignitionLimits(schedule.dischargeAngle-crankAngle) )); 
      } 
    }
    else if( (schedule.Status==PENDING) ) {
      if ((currentStatus.startRevolutions > MIN_CYCLES_FOR_CORRECTION) && (schedule.chargeAngle>crankAngle)) {
        // We are waiting for the timer to fire & start charging the coil.
        // Keep dwell (I.e. duration) constant (for better spark) - instead adjust the waiting period so 
        // the spark fires at the requested crank angle.
        SET_COMPARE(schedule._compare, schedule._counter + angleToTimerTicks( ignitionLimits(schedule.chargeAngle-crankAngle) )); 
      }
    } else {
      // Unknown state, so no adjustment possible
    }
  }
}

static inline  __attribute__((always_inline))void setIgnitionSchedule(IgnitionSchedule &schedule, int16_t crankAngle, uint32_t dwellDuration) {
  uint32_t delay = _calculateIgnitionTimeout(schedule, crankAngle);

  if (delay > 0U) {
    _setIgnitionScheduleDuration(schedule, delay, dwellDuration);
  }
}