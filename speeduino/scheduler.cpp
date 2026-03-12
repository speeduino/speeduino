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
#include "schedule_calcs.h"
#include "preprocessor.h"
#include "units.h"
#include "schedule_state_machine.h"
#include "unit_testing.h"
#include "decoders.h"
#include "scheduledIO_inj.h"

FuelSchedule fuelSchedule1(FUEL1_COUNTER, FUEL1_COMPARE); //cppcheck-suppress misra-c2012-8.4
FuelSchedule fuelSchedule2(FUEL2_COUNTER, FUEL2_COMPARE); //cppcheck-suppress misra-c2012-8.4
FuelSchedule fuelSchedule3(FUEL3_COUNTER, FUEL3_COMPARE); //cppcheck-suppress misra-c2012-8.4
FuelSchedule fuelSchedule4(FUEL4_COUNTER, FUEL4_COMPARE); //cppcheck-suppress misra-c2012-8.4
#if (INJ_CHANNELS >= 5)
FuelSchedule fuelSchedule5(FUEL5_COUNTER, FUEL5_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if (INJ_CHANNELS >= 6)
FuelSchedule fuelSchedule6(FUEL6_COUNTER, FUEL6_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if (INJ_CHANNELS >= 7)
FuelSchedule fuelSchedule7(FUEL7_COUNTER, FUEL7_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if (INJ_CHANNELS >= 8)
FuelSchedule fuelSchedule8(FUEL8_COUNTER, FUEL8_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif

IgnitionSchedule ignitionSchedule1(IGN1_COUNTER, IGN1_COMPARE); //cppcheck-suppress misra-c2012-8.4
IgnitionSchedule ignitionSchedule2(IGN2_COUNTER, IGN2_COMPARE); //cppcheck-suppress misra-c2012-8.4
IgnitionSchedule ignitionSchedule3(IGN3_COUNTER, IGN3_COMPARE); //cppcheck-suppress misra-c2012-8.4
IgnitionSchedule ignitionSchedule4(IGN4_COUNTER, IGN4_COMPARE); //cppcheck-suppress misra-c2012-8.4
IgnitionSchedule ignitionSchedule5(IGN5_COUNTER, IGN5_COMPARE); //cppcheck-suppress misra-c2012-8.4
#if IGN_CHANNELS >= 6
IgnitionSchedule ignitionSchedule6(IGN6_COUNTER, IGN6_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if IGN_CHANNELS >= 7
IgnitionSchedule ignitionSchedule7(IGN7_COUNTER, IGN7_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if IGN_CHANNELS >= 8
IgnitionSchedule ignitionSchedule8(IGN8_COUNTER, IGN8_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif

void Schedule::reset(void)
{
    Status = OFF;
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
}

void initialiseFuelSchedulers(void)
{
    fuelSchedule1.reset();
    fuelSchedule2.reset();
    fuelSchedule3.reset();
    fuelSchedule4.reset();
#if INJ_CHANNELS >= 5
    fuelSchedule5.reset();
#endif
#if INJ_CHANNELS >= 6
    fuelSchedule6.reset();
#endif
#if INJ_CHANNELS >= 7
    fuelSchedule7.reset();
#endif
#if INJ_CHANNELS >= 8
    fuelSchedule8.reset();
#endif

	channel1InjDegrees = 0; /**< The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones) */
	channel2InjDegrees = 0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
	channel3InjDegrees = 0; /**< The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC */
	channel4InjDegrees = 0; /**< The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC */
#if (INJ_CHANNELS >= 5)
	channel5InjDegrees = 0; /**< The number of crank degrees until cylinder 5 is at TDC */
#endif
#if (INJ_CHANNELS >= 6)
	channel6InjDegrees = 0; /**< The number of crank degrees until cylinder 6 is at TDC */
#endif
#if (INJ_CHANNELS >= 7)
	channel7InjDegrees = 0; /**< The number of crank degrees until cylinder 7 is at TDC */
#endif
#if (INJ_CHANNELS >= 8)
	channel8InjDegrees = 0; /**< The number of crank degrees until cylinder 8 is at TDC */
#endif
}

void initialiseIgnitionSchedulers(void)
{
    ignitionSchedule1.reset();
    ignitionSchedule2.reset();
    ignitionSchedule3.reset();
    ignitionSchedule4.reset();
#if (IGN_CHANNELS >= 5)
    ignitionSchedule5.reset();
#endif
#if IGN_CHANNELS >= 6
    ignitionSchedule6.reset();
#endif
#if IGN_CHANNELS >= 7
    ignitionSchedule7.reset();
#endif
#if IGN_CHANNELS >= 8
    ignitionSchedule8.reset();
#endif
}

void startIgnitionSchedulers(void)
{
  IGN1_TIMER_ENABLE();
#if IGN_CHANNELS >= 2
  IGN2_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 3
  IGN3_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 4
  IGN4_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 5
  IGN5_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 6
  IGN6_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 7
  IGN7_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 8
  IGN8_TIMER_ENABLE();
#endif  
}

void stopIgnitionSchedulers(void)
{
  IGN1_TIMER_DISABLE();
#if IGN_CHANNELS >= 2
  IGN2_TIMER_DISABLE();
#endif
#if IGN_CHANNELS >= 3
  IGN3_TIMER_DISABLE();
#endif
#if IGN_CHANNELS >= 4
  IGN4_TIMER_DISABLE();
#endif
#if IGN_CHANNELS >= 5
  IGN5_TIMER_DISABLE();
#endif
#if IGN_CHANNELS >= 6
  IGN6_TIMER_DISABLE();
#endif
#if IGN_CHANNELS >= 7
  IGN7_TIMER_DISABLE();
#endif
#if IGN_CHANNELS >= 8
  IGN8_TIMER_DISABLE();
#endif  

}

void startFuelSchedulers(void)
{
  FUEL1_TIMER_ENABLE();
  FUEL2_TIMER_ENABLE();
  FUEL3_TIMER_ENABLE();
  FUEL4_TIMER_ENABLE();
#if INJ_CHANNELS >= 5
  FUEL5_TIMER_ENABLE();
#endif
#if INJ_CHANNELS >= 6
  FUEL6_TIMER_ENABLE();
#endif
#if INJ_CHANNELS >= 7
  FUEL7_TIMER_ENABLE();
#endif
#if INJ_CHANNELS >= 8
  FUEL8_TIMER_ENABLE();
#endif
}

void stopFuelSchedulers(void)
{
  FUEL1_TIMER_DISABLE();
  FUEL2_TIMER_DISABLE();
  FUEL3_TIMER_DISABLE();
  FUEL4_TIMER_DISABLE();
#if INJ_CHANNELS >= 5
  FUEL5_TIMER_DISABLE();
#endif
#if INJ_CHANNELS >= 6
  FUEL6_TIMER_DISABLE();
#endif
#if INJ_CHANNELS >= 7
  FUEL7_TIMER_DISABLE();
#endif
#if INJ_CHANNELS >= 8
  FUEL8_TIMER_DISABLE();
#endif  
}

void setCallbacks(Schedule &schedule, Schedule::callback pStartCallback, Schedule::callback pEndCallback) noexcept
{
  schedule.pStartCallback = pStartCallback;
  schedule.pEndCallback = pEndCallback;
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
  schedule.duration = uS_TO_TIMER_COMPARE(clipDuration(duration));
  schedule.nextStartCompare = schedule._counter + uS_TO_TIMER_COMPARE(delay);
  schedule.Status = RUNNING_WITHNEXT;
}

static inline void setScheduleRunning(Schedule &schedule, uint32_t delay, uint16_t duration) noexcept
{
  //The following must be enclosed in the noInterupts block to avoid contention caused if the relevant interrupt fires before the state is fully set
  //The duration of the pulsewidth cannot be longer than the maximum timer period. This is unlikely as pulse widths should never get that long, but it's here for safety
  schedule.duration = uS_TO_TIMER_COMPARE(clipDuration(duration));
  SET_COMPARE(schedule._compare, schedule._counter + uS_TO_TIMER_COMPARE(delay));
  schedule.Status = PENDING; //Turn this schedule on
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

constexpr table2D_u8_u8_4 PrimingPulseTable(&configPage2.primeBins, &configPage2.primePulse);

/** Perform the injector priming pulses.
 * Set these to run at an arbitrary time in the future (100us).
 * The prime pulse value is in ms*10, so need to multiple by 100 to get to uS
 */
void beginInjectorPriming(void)
{
  uint16_t primingValue = (uint16_t)table2D_getValue(&PrimingPulseTable, temperatureAddOffset(currentStatus.coolant));
  if( (primingValue > 0U) && (currentStatus.TPS <= configPage4.floodClear) )
  {
    constexpr uint32_t PRIMING_DELAY = 100U; // 100us
    // The prime pulse value is in ms*2, so need to multiply by 500 to get to µS
    constexpr uint16_t PULSE_TS_SCALE_FACTOR = 100U * 5U; 

    primingValue = primingValue * PULSE_TS_SCALE_FACTOR; 
    if ( currentStatus.maxInjOutputs >= 1U ) { setFuelSchedule(fuelSchedule1, PRIMING_DELAY, primingValue); }
#if (INJ_CHANNELS >= 2)
    if ( currentStatus.maxInjOutputs >= 2U ) { setFuelSchedule(fuelSchedule2, PRIMING_DELAY, primingValue); }
#endif
#if (INJ_CHANNELS >= 3)
    if ( currentStatus.maxInjOutputs >= 3U ) { setFuelSchedule(fuelSchedule3, PRIMING_DELAY, primingValue); }
#endif
#if (INJ_CHANNELS >= 4)
    if ( currentStatus.maxInjOutputs >= 4U ) { setFuelSchedule(fuelSchedule4, PRIMING_DELAY, primingValue); }
#endif
#if (INJ_CHANNELS >= 5)
    if ( currentStatus.maxInjOutputs >= 5U ) { setFuelSchedule(fuelSchedule5, PRIMING_DELAY, primingValue); }
#endif
#if (INJ_CHANNELS >= 6)
    if ( currentStatus.maxInjOutputs >= 6U ) { setFuelSchedule(fuelSchedule6, PRIMING_DELAY, primingValue); }
#endif
#if (INJ_CHANNELS >= 7)
    if ( currentStatus.maxInjOutputs >= 7U) { setFuelSchedule(fuelSchedule7, PRIMING_DELAY, primingValue); }
#endif
#if (INJ_CHANNELS >= 8)
    if ( currentStatus.maxInjOutputs >= 8U ) { setFuelSchedule(fuelSchedule8, PRIMING_DELAY, primingValue); }
#endif
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

static inline bool isAnyFuelScheduleRunning(void) {
  ATOMIC() {
    return isRunning(fuelSchedule1)
        || isRunning(fuelSchedule2)
        || isRunning(fuelSchedule3)
        || isRunning(fuelSchedule4)
  #if INJ_CHANNELS >= 5      
        || isRunning(fuelSchedule5)
  #endif
  #if INJ_CHANNELS >= 6
        || isRunning(fuelSchedule6)
  #endif
  #if INJ_CHANNELS >= 7
        || isRunning(fuelSchedule7)
  #endif
  #if INJ_CHANNELS >= 8
        || isRunning(fuelSchedule8)
  #endif
        ;
  }
  return false;// Avoid compiler warning, but optimized out
}

static inline void changeFuellingToFullSequential(const config2 &page2, statuses &current)
{
  if( (page2.injLayout == INJ_SEQUENTIAL) && (CRANK_ANGLE_MAX_INJ != 720) && (!isAnyFuelScheduleRunning()))
  {
    ATOMIC() {
      CRANK_ANGLE_MAX_INJ = 720;
      current.maxInjOutputs = page2.nCylinders;
      
      setCallbacks(fuelSchedule1, openInjector1, closeInjector1);
      setCallbacks(fuelSchedule2, openInjector2, closeInjector2);
      setCallbacks(fuelSchedule3, openInjector3, closeInjector3);
      setCallbacks(fuelSchedule4, openInjector4, closeInjector4);
  #if INJ_CHANNELS >= 5
      setCallbacks(fuelSchedule5, openInjector5, closeInjector5);
  #endif
  #if INJ_CHANNELS >= 6
      setCallbacks(fuelSchedule6, openInjector6, closeInjector6);
  #endif
  #if INJ_CHANNELS >= 7
      setCallbacks(fuelSchedule7, openInjector7, closeInjector7);
  #endif
  #if INJ_CHANNELS >= 8
      setCallbacks(fuelSchedule8, openInjector8, closeInjector8);
  #endif
    }
  }
}

/** Change injectors or/and ignition angles to 720deg.
 * Roll back req_fuel size and set number of outputs equal to cylinder count.
* */
void changeHalfToFullSync(const config2 &page2, statuses &current)
{
  changeFuellingToFullSequential(page2, current);
}


static inline void changeFuellingtoHalfSync(const config2 &page2, const config4 &page4, statuses &current)
{
  if((page2.injLayout == INJ_SEQUENTIAL) && (CRANK_ANGLE_MAX_INJ != 360))
  {
    ATOMIC()
    {
      CRANK_ANGLE_MAX_INJ = 360;
      switch (page2.nCylinders)
      {
        case 4:
          if(page4.inj4cylPairing == INJ_PAIR_13_24)
          {
            setCallbacks(fuelSchedule1, openInjector1and3, closeInjector1and3);
            setCallbacks(fuelSchedule2, openInjector2and4, closeInjector2and4);
          }
          else
          {
            setCallbacks(fuelSchedule1, openInjector1and4, closeInjector1and4);
            setCallbacks(fuelSchedule2, openInjector2and3, closeInjector2and3);
          }
          current.maxInjOutputs = 2U;
          break;
              
        case 6:
          setCallbacks(fuelSchedule1, openInjector1and4, closeInjector1and4);
          setCallbacks(fuelSchedule2, openInjector2and5, closeInjector2and5);
          setCallbacks(fuelSchedule3, openInjector3and6, closeInjector3and6);
          current.maxInjOutputs = 3U;
          break;

        case 8:
          setCallbacks(fuelSchedule1, openInjector1and5, closeInjector1and5);
          setCallbacks(fuelSchedule2, openInjector2and6, closeInjector2and6);
          setCallbacks(fuelSchedule3, openInjector3and7, closeInjector3and7);
          setCallbacks(fuelSchedule4, openInjector4and8, closeInjector4and8);
          current.maxInjOutputs = 4U;
          break;

        default:
          break; //No actions required for other cylinder counts 
      }
    }
  }
}

/** Change injectors or/and ignition angles to 360deg.
 * In semi sequentiol mode req_fuel size is half.
 * Set number of outputs equal to half cylinder count.
* */
void changeFullToHalfSync(const config2 &page2, const config4 &page4, statuses &current)
{
  changeFuellingtoHalfSync(page2, page4, current);
}
