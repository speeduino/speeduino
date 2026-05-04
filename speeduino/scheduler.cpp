/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
/** @file
 * Injector and Ignition (on/off) scheduling (functions).
 * There is usually 8 functions for cylinders 1-8 with same naming pattern.
 * 
 * ## Scheduling structures
 * 
 * Structures @ref FuelSchedule and @ref Schedule describe (from scheduler.h) describe the scheduling info for Fuel and Ignition respectively.
 * They contain duration, current activity status, start timing, end timing, callbacks to carry out action, etc.
 * 
 * ## Scheduling Functions
 * 
 * For Injection:
 * - setFuelSchedule*(tout,dur) - **Setup** schedule for (next) injection on the channel
 * - inj*StartFunction() - Execute **start** of injection (Interrupt handler)
 * - inj*EndFunction() - Execute **end** of injection (interrupt handler)
 * 
 * For Ignition (has more complex schedule setup):
 * - setIgnitionSchedule*(cb_st,tout,dur,cb_end) - **Setup** schedule for (next) ignition on the channel
 * - ign*StartFunction() - Execute **start** of ignition (Interrupt handler)
 * - ign*EndFunction() - Execute **end** of ignition (Interrupt handler)
 */
#include "globals.h"
#include "scheduler.h"
#include "timers.h"
#include "preprocessor.h"
#include "units.h"
#include "schedule_state_machine.h"
#include "unit_testing.h"
#include "decoders.h"
#include "scheduledIO_inj.h"

void nullCallback(void) { return; }

void Schedule::reset(void)
{
    _status = OFF;
    setCallbacks(*this, nullCallback, nullCallback);
}

void IgnitionSchedule::reset(void) 
{
    Schedule::reset();
    chargeAngle = 0;
    dischargeAngle = 0;
    channelDegrees = 0;
}

void FuelSchedule::reset(void) 
{
    Schedule::reset();
    channelDegrees = 0;
}

void __attribute__((optimize("Os"))) setCallbacks(Schedule &schedule, Schedule::callback pStartCallback, Schedule::callback pEndCallback) noexcept
{
  schedule._pStartCallback = pStartCallback;
  schedule._pEndCallback = pEndCallback;
}

// Event duration cannot be longer than the maximum timer period
static inline uint16_t clipDuration(uint16_t duration) {
  if (MAX_TIMER_PERIOD < (uint32_t)UINT16_MAX)
  {
    return min((uint16_t)(MAX_TIMER_PERIOD - 1U), duration);
  }
  return duration;
}

static inline void setScheduleNext(Schedule &schedule, uint32_t delay, uint16_t duration) noexcept
{
  //The duration of the pulsewidth cannot be longer than the maximum timer period. This is unlikely as pulse widths should never get that long, but it's here for safety
  //Duration can safely be set here as the schedule is already running at the previous duration value already used
  schedule._duration = uS_TO_TIMER_COMPARE(clipDuration(duration));
  schedule._nextStartCompare = schedule._counter + uS_TO_TIMER_COMPARE(delay);
  schedule._status = RUNNING_WITHNEXT;
}

static inline void setScheduleRunning(Schedule &schedule, uint32_t delay, uint16_t duration) noexcept
{
  //The following must be enclosed in the noInterupts block to avoid contention caused if the relevant interrupt fires before the state is fully set
  //The duration of the pulsewidth cannot be longer than the maximum timer period. This is unlikely as pulse widths should never get that long, but it's here for safety
  schedule._duration = uS_TO_TIMER_COMPARE(clipDuration(duration));
  SET_COMPARE(schedule._compare, schedule._counter + uS_TO_TIMER_COMPARE(delay));
  schedule._status = PENDING; //Turn this schedule on
}

void setSchedule(Schedule &schedule, uint32_t delay, uint16_t duration, bool allowQueuedSchedule)
{
  if((delay>0U) && (delay < MAX_TIMER_PERIOD) && (duration > 0U))
  {
    ATOMIC() 
    {
      //Check that we're not already part way through a schedule
      if(!isRunning(schedule)) 
      { 
        setScheduleRunning(schedule, delay, duration);
      }
      // If the schedule is already running, we can queue up the next event.
      else if(allowQueuedSchedule)
      {
        setScheduleNext(schedule, delay, duration);
      } else {
        // Cannot schedule next event, as it would exceed the maximum future time
      }
    }
  }  
}

/**
 * @defgroup fuel-schedule-ISR Fuel schedule timer ISRs 
 *   
 * @{
 */

void moveToNextState(FuelSchedule &schedule) noexcept
{
  movetoNextState(schedule, defaultPendingToRunning, defaultRunningToOff, defaultRunningToPending);
} 

///@}

/**
 * @defgroup ignition-schedule-ISR Ignition schedule timer ISRs 
 *   
 * @{
 */

 /** @brief Increment a volatile variable correctly */
 template <typename T>
 static inline void increment_volatile(volatile T& value) {
  T next = value;
  ++next;
  value = next;
 }

/**
 * @brief Called when an ignition event ends. I.e. a spark fires
 * 
 * @param pSchedule Pointer to the schedule that fired the spark
 */
static inline void onEndIgnitionEvent(const IgnitionSchedule *pSchedule) {
  increment_volatile(ignitionCount); //Increment the ignition counter

  int32_t elapsed = (int32_t)(micros() - pSchedule->_startTime);
  constexpr uint8_t DWELL_AVERAGE_ALPHA = 30U; // ~10% smoothing (30/255)
  currentStatus.actualDwell = LOW_PASS_FILTER(elapsed, DWELL_AVERAGE_ALPHA, currentStatus.actualDwell);
}

/** @brief Called when the supplied schedule transitions from a PENDING state to RUNNING */
BEGIN_LTO_ALWAYS_INLINE(void) static ignitionPendingToRunning(Schedule *pSchedule) {
  defaultPendingToRunning(pSchedule);

  // cppcheck-suppress misra-c2012-11.3 ; A cast from pointer to base to pointer to derived must point to the same location
  IgnitionSchedule *pIgnition = (IgnitionSchedule *)pSchedule;
  pIgnition->_startTime = micros();
}
END_LTO_INLINE()

/** @brief Called when the supplied schedule transitions from a RUNNING state to OFF */
BEGIN_LTO_ALWAYS_INLINE(void) static ignitionRunningToOff(Schedule *pSchedule) {
  defaultRunningToOff(pSchedule);
  // cppcheck-suppress misra-c2012-11.3 ; A cast from pointer to base to pointer to derived must point to the same location
  onEndIgnitionEvent((IgnitionSchedule *)pSchedule);
}
END_LTO_INLINE()

/** @brief Called when the supplied schedule transitions from a RUNNING state to PENDING */
BEGIN_LTO_ALWAYS_INLINE(void) static ignitionRunningToPending(Schedule *pSchedule) {
  defaultRunningToPending(pSchedule);
  // cppcheck-suppress misra-c2012-11.3 ; A cast from pointer to base to pointer to derived must point to the same location
  onEndIgnitionEvent((IgnitionSchedule *)pSchedule);
}
END_LTO_INLINE()

void moveToNextState(IgnitionSchedule &schedule)  noexcept
{
  movetoNextState(schedule, ignitionPendingToRunning, ignitionRunningToOff, ignitionRunningToPending);
}

