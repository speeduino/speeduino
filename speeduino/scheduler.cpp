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
#include "scheduledIO.h"
#include "timers.h"
#include "schedule_calcs.h"
#include "preprocessor.h"
#include "units.h"
#include "schedule_state_machine.h"
#include "unit_testing.h"
#include "decoders.h"

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

constexpr table2D_u8_u8_8 rotarySplitTable(&configPage10.rotarySplitBins, &configPage10.rotarySplitValues);

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

static void reset(Schedule &schedule)
{
    schedule.Status = OFF;
    setCallbacks(schedule, nullCallback, nullCallback);
}

static inline void reset(FuelSchedule &schedule) 
{
    reset((Schedule&)schedule);
}

static inline void reset(IgnitionSchedule &schedule) 
{
    reset((Schedule&)schedule);
    schedule.chargeAngle = 0;
    schedule.dischargeAngle = 0;
    schedule.channelDegrees = 0;
}

void initialiseFuelSchedulers(void)
{
    reset(fuelSchedule1);
    reset(fuelSchedule2);
    reset(fuelSchedule3);
    reset(fuelSchedule4);
#if INJ_CHANNELS >= 5
    reset(fuelSchedule5);
#endif
#if INJ_CHANNELS >= 6
    reset(fuelSchedule6);
#endif
#if INJ_CHANNELS >= 7
    reset(fuelSchedule7);
#endif
#if INJ_CHANNELS >= 8
    reset(fuelSchedule8);
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
    reset(ignitionSchedule1);
    reset(ignitionSchedule2);
    reset(ignitionSchedule3);
    reset(ignitionSchedule4);
#if (IGN_CHANNELS >= 5)
    reset(ignitionSchedule5);
#endif
#if IGN_CHANNELS >= 6
    reset(ignitionSchedule6);
#endif
#if IGN_CHANNELS >= 7
    reset(ignitionSchedule7);
#endif
#if IGN_CHANNELS >= 8
    reset(ignitionSchedule8);
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

void setCallbacks(Schedule &schedule, voidVoidCallback pStartCallback, voidVoidCallback pEndCallback)
{
  schedule.pStartCallback = pStartCallback;
  schedule.pEndCallback = pEndCallback;
}

// Event duration cannot be longer than the maximum timer period
static inline uint16_t clipDuration(uint16_t duration) {
#ifndef MAX_TIMER_PERIOD
  #error MAX_TIMER_PERIOD must be defined
#else
#if MAX_TIMER_PERIOD < UINT16_MAX //cppcheck-suppress misra-c2012-20.9
  return min((uint16_t)(MAX_TIMER_PERIOD - 1U), duration);
#else
  return duration;
#endif
#endif
}

static inline void setScheduleNext(Schedule &schedule, uint32_t delay, uint16_t duration)
{
  //The duration of the pulsewidth cannot be longer than the maximum timer period. This is unlikely as pulse widths should never get that long, but it's here for safety
  //Duration can safely be set here as the schedule is already running at the previous duration value already used
  schedule.duration = uS_TO_TIMER_COMPARE(clipDuration(duration));
  schedule.nextStartCompare = schedule._counter + uS_TO_TIMER_COMPARE(delay);
  schedule.Status = RUNNING_WITHNEXT;
}

static inline void setScheduleRunning(Schedule &schedule, uint32_t delay, uint16_t duration)
{
  //The following must be enclosed in the noInterupts block to avoid contention caused if the relevant interrupt fires before the state is fully set
  //The duration of the pulsewidth cannot be longer than the maximum timer period. This is unlikely as pulse widths should never get that long, but it's here for safety
  schedule.duration = uS_TO_TIMER_COMPARE(clipDuration(duration));
  SET_COMPARE(schedule._compare, schedule._counter + uS_TO_TIMER_COMPARE(delay));
  schedule.Status = PENDING; //Turn this schedule on
}

void setSchedule(Schedule &schedule, uint32_t delay, uint16_t duration, bool allowQueuedSchedule)
{
  if((delay>0U) && (delay < (uint32_t)MAX_TIMER_PERIOD) && (duration > 0U))
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

void moveToNextState(FuelSchedule &schedule)
{
  movetoNextState(schedule, defaultPendingToRunning, defaultRunningToOff, defaultRunningToPending);
} 

///@}

/**
 * @defgroup ignition-schedule-ISR Ignition schedule timer ISRs 
 *   
 * @{
 */

///@cond
// Dwell smoothing macros. They are split up like this for MISRA compliance.
#define DWELL_AVERAGE_ALPHA 30
#define DWELL_AVERAGE(input) LOW_PASS_FILTER((input), DWELL_AVERAGE_ALPHA, currentStatus.actualDwell)
//#define DWELL_AVERAGE(input) (currentStatus.dwell) //Can be use to disable the above for testing
///@endcond

/**
 * @brief Called when an ignition event ends. I.e. a spark fires
 * 
 * @param pSchedule Pointer to the schedule that fired the spark
 */
static inline void onEndIgnitionEvent(IgnitionSchedule *pSchedule) {
  ignitionCount = ignitionCount + 1U; //Increment the ignition counter
  int32_t elapsed = (int32_t)(micros() - pSchedule->_startTime);
  currentStatus.actualDwell = (uint16_t)DWELL_AVERAGE( elapsed );
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

void moveToNextState(IgnitionSchedule &schedule)
{
  movetoNextState(schedule, ignitionPendingToRunning, ignitionRunningToOff, ignitionRunningToPending);
}

void disableSchedule(Schedule &schedule)
{
  ATOMIC() {
    if(schedule.Status == PENDING) { 
      schedule.Status = OFF; 
    } else if(schedule.Status == RUNNING_WITHNEXT) { 
      schedule.Status = RUNNING; 
    } else {
      // Must be off already :-)
    }
  }
}
void disableFuelSchedule(uint8_t channel)
{
  switch(channel)
  {
    default:
    case 0: disableSchedule(fuelSchedule1); break;
    case 1: disableSchedule(fuelSchedule2); break;
    case 2: disableSchedule(fuelSchedule3); break;
    case 3: disableSchedule(fuelSchedule4); break;
#if (INJ_CHANNELS >= 5)
    case 4: disableSchedule(fuelSchedule5); break;
#endif
#if (INJ_CHANNELS >= 6)
    case 5: disableSchedule(fuelSchedule6); break;
#endif
#if (INJ_CHANNELS >= 7)
    case 6: disableSchedule(fuelSchedule7); break;
#endif
#if (INJ_CHANNELS >= 8)
    case 7: disableSchedule(fuelSchedule8); break;
#endif
  }
}
void disableIgnSchedule(uint8_t channel)
{
  switch(channel)
  {
    default:
    case 0: disableSchedule(ignitionSchedule1); break;
    case 1: disableSchedule(ignitionSchedule2); break;
    case 2: disableSchedule(ignitionSchedule3); break;
    case 3: disableSchedule(ignitionSchedule4); break;
    case 4: disableSchedule(ignitionSchedule5); break;
#if IGN_CHANNELS >= 6      
    case 5: disableSchedule(ignitionSchedule6); break;
#endif
#if IGN_CHANNELS >= 7      
    case 6: disableSchedule(ignitionSchedule7); break;
#endif
#if IGN_CHANNELS >= 8      
    case 7: disableSchedule(ignitionSchedule8); break;
#endif
  }
}

void disableAllFuelSchedules(void)
{
  for (uint8_t index=0; index<INJ_CHANNELS; ++index) {
    disableFuelSchedule(index);
  }
}
void disableAllIgnSchedules(void)
{
  for (uint8_t index=0; index<IGN_CHANNELS; ++index) {
    disableIgnSchedule(index);
  }
}

TESTABLE_INLINE_STATIC void applyChannelOverDwellProtection(IgnitionSchedule &schedule, uint32_t targetOverdwellTime) {
  //Check first whether each spark output is currently on. Only check it's dwell time if it is
  ATOMIC() {
    if (isRunning(schedule) && (schedule._startTime < targetOverdwellTime)) { 
      ignitionRunningToOff(&schedule); //Call the end function to disable the spark output
    }
  }
}

TESTABLE_INLINE_STATIC bool isOverDwellActive(const config4 &page4, const statuses &current){
  bool isCrankLocked = page4.ignCranklock && (current.RPM < current.crankRPM); //Dwell limiter is disabled during cranking on setups using the locked cranking timing. WE HAVE to do the RPM check here as relying on the engine cranking bit can be potentially too slow in updating
  return (page4.useDwellLim) && !isCrankLocked;
}

void applyOverDwellProtection(void)
{
  if (isOverDwellActive(configPage4, currentStatus)) {
    uint32_t targetOverdwellTime = micros() - (configPage4.dwellLimit * 1000U); //Convert to uS

    applyChannelOverDwellProtection(ignitionSchedule1, targetOverdwellTime);
#if IGN_CHANNELS >= 2
    applyChannelOverDwellProtection(ignitionSchedule2, targetOverdwellTime);
#endif
#if IGN_CHANNELS >= 3
    applyChannelOverDwellProtection(ignitionSchedule3, targetOverdwellTime);
#endif
#if IGN_CHANNELS >= 4
    applyChannelOverDwellProtection(ignitionSchedule4, targetOverdwellTime);
#endif
#if IGN_CHANNELS >= 5
    applyChannelOverDwellProtection(ignitionSchedule5, targetOverdwellTime);
#endif
#if IGN_CHANNELS >= 6
    applyChannelOverDwellProtection(ignitionSchedule6, targetOverdwellTime);
#endif
#if IGN_CHANNELS >= 7
    applyChannelOverDwellProtection(ignitionSchedule7, targetOverdwellTime);
#endif
#if IGN_CHANNELS >= 8
    applyChannelOverDwellProtection(ignitionSchedule8, targetOverdwellTime);
#endif
  }
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

static inline bool isAnyIgnScheduleRunning(void) {
  return isRunning(ignitionSchedule1)      
#if IGN_CHANNELS >= 2 
      || isRunning(ignitionSchedule2)
#endif      
#if IGN_CHANNELS >= 3 
      || isRunning(ignitionSchedule3)
#endif      
#if IGN_CHANNELS >= 4       
      || isRunning(ignitionSchedule4)
#endif      
#if IGN_CHANNELS >= 5      
      || isRunning(ignitionSchedule5)
#endif
#if IGN_CHANNELS >= 6
      || isRunning(ignitionSchedule6)
#endif
#if IGN_CHANNELS >= 7
      || isRunning(ignitionSchedule7)
#endif
#if IGN_CHANNELS >= 8
      || isRunning(ignitionSchedule8)
#endif
      ;
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


static inline bool isSwitchableCylinderCount(const config2 &page2)
{
  return (page2.nCylinders==4U)
      || (page2.nCylinders==6U)
      || (page2.nCylinders==8U)
      ;
}

static inline bool isSemiSequentialIgnition(const config2 &page2, const config4 &page4, const decoder_status_t &decoderStatus)
{
  return (page4.sparkMode == IGN_MODE_SEQUENTIAL) 
      && isSwitchableCylinderCount(page2)
      && decoderStatus.syncStatus==SyncStatus::Partial;
}

static inline bool isFullSequentialIgnition(const config4 &page4, const decoder_status_t &decoderStatus)
{
  return (page4.sparkMode == IGN_MODE_SEQUENTIAL) 
      && decoderStatus.syncStatus==SyncStatus::Full;
}

static inline void changeIgnitionToHalfSync(const config2 &page2, statuses &current)
{
  ATOMIC()
  {
    if (!isAnyIgnScheduleRunning()) {
      CRANK_ANGLE_MAX_IGN = 360;
      switch (page2.nCylinders)
      {
        case 4:
          setCallbacks(ignitionSchedule1, beginCoil1and3Charge, endCoil1and3Charge);
          setCallbacks(ignitionSchedule2, beginCoil2and4Charge, endCoil2and4Charge);
          current.maxIgnOutputs = 2U;
          break;
              
        case 6:
          setCallbacks(ignitionSchedule1, beginCoil1and4Charge, endCoil1and4Charge);
          setCallbacks(ignitionSchedule2, beginCoil2and5Charge, endCoil2and5Charge);
          setCallbacks(ignitionSchedule3, beginCoil3and6Charge, endCoil3and6Charge);
          current.maxIgnOutputs = 3U;
          break;

        case 8:
          setCallbacks(ignitionSchedule1, beginCoil1and5Charge, endCoil1and5Charge);
          setCallbacks(ignitionSchedule2, beginCoil2and6Charge, endCoil2and6Charge);
          setCallbacks(ignitionSchedule3, beginCoil3and7Charge, endCoil3and7Charge);
          setCallbacks(ignitionSchedule4, beginCoil4and8Charge, endCoil4and8Charge);
          current.maxIgnOutputs = 4U;
          break;
          
        default:
          break; //No actions required for other cylinder counts 
      }
    }
  }
}

static inline void changeIgnitionToFullSequential(const config2 &page2, statuses &current)
{
  ATOMIC()
  {
    if (!isAnyIgnScheduleRunning()) {
      CRANK_ANGLE_MAX_IGN = 720;
      current.maxIgnOutputs = min((uint8_t)IGN_CHANNELS, page2.nCylinders);
      switch (current.maxIgnOutputs)
      {
      case 4:
        setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge);
        break;

      case 6:
        setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge);
        setCallbacks(ignitionSchedule3, beginCoil3Charge, endCoil3Charge);
        break;

      case 8:
        setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge);
        setCallbacks(ignitionSchedule3, beginCoil3Charge, endCoil3Charge);
        setCallbacks(ignitionSchedule4, beginCoil4Charge, endCoil4Charge);
        break;

      default:
        break; //No actions required for other cylinder counts 
      }
    }
  }
}

TESTABLE_INLINE_STATIC void matchIgnitionSchedulersToSyncState(const config2 &page2, const config4 &page4, const decoder_status_t &decoderStatus, statuses &current)
{
  if (isFullSequentialIgnition(page4, decoderStatus) && ( CRANK_ANGLE_MAX_IGN != 720 )) {
    changeIgnitionToFullSequential(page2, current);
  } else if(isSemiSequentialIgnition(page2, page4, decoderStatus) && (CRANK_ANGLE_MAX_IGN != 360) ) { 
    changeIgnitionToHalfSync(page2, current);
  } else {
    // Ignition layout matches current sync - nothing to do but keep MISRA checker happy
  }
}


static inline void calculateRotaryIgnitionAngles(uint16_t dwellAngle, const statuses &current)
{
#if IGN_CHANNELS>=4
  calculateIgnitionAngles(ignitionSchedule1, dwellAngle, current.advance);
  calculateIgnitionAngles(ignitionSchedule2, dwellAngle, current.advance);
  uint8_t splitDegrees = table2D_getValue(&rotarySplitTable, (uint8_t)current.ignLoad);

  //The trailing angles are set relative to the leading ones
  calculateIgnitionTrailingRotary(ignitionSchedule1, dwellAngle, splitDegrees, ignitionSchedule3);
  calculateIgnitionTrailingRotary(ignitionSchedule2, dwellAngle, splitDegrees, ignitionSchedule4);
#endif
}

static inline void calculateNonRotaryIgnitionAngles(uint16_t dwellAngle, const statuses &current)
{
  switch (current.maxIgnOutputs)
  {
  case 8:
#if IGN_CHANNELS >= 8
    calculateIgnitionAngles(ignitionSchedule8, dwellAngle, current.advance);
#endif
    [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 7:
#if IGN_CHANNELS >= 7
    calculateIgnitionAngles(ignitionSchedule7, dwellAngle, current.advance);
#endif
    [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 6:
#if IGN_CHANNELS >= 6
    calculateIgnitionAngles(ignitionSchedule6, dwellAngle, current.advance);
#endif
    [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 5:
#if IGN_CHANNELS >= 5
    calculateIgnitionAngles(ignitionSchedule5, dwellAngle, current.advance);
#endif
    [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 4:
#if IGN_CHANNELS >= 4
    calculateIgnitionAngles(ignitionSchedule4, dwellAngle, current.advance);
#endif
    [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 3:
#if IGN_CHANNELS >= 3
    calculateIgnitionAngles(ignitionSchedule3, dwellAngle, current.advance);
#endif
    [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 2:
#if IGN_CHANNELS >= 2
    calculateIgnitionAngles(ignitionSchedule2, dwellAngle, current.advance);
#endif
    [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 1:
    calculateIgnitionAngles(ignitionSchedule1, dwellAngle, current.advance);
    break;
  default:
    // Do nothing
    break;
  }
}

/** Calculate the Ignition angles for all cylinders (based on @ref config2.nCylinders).
 * both start and end angles are calculated for each channel.
 * Also the mode of ignition firing - wasted spark vs. dedicated spark per cyl. - is considered here.
 */
void __attribute__((flatten)) calculateIgnitionAngles(const config2 &page2, const config4 &page4, const decoder_status_t &decoderStatus, statuses &current)
{
  matchIgnitionSchedulersToSyncState(page2, page4, decoderStatus, current);

  uint16_t dwellAngle = timeToAngleDegPerMicroSec(current.dwell);

  if((current.maxIgnOutputs==4U) && (page4.sparkMode == IGN_MODE_ROTARY))
  {
    calculateRotaryIgnitionAngles(dwellAngle, current);
  }
  else
  {
    calculateNonRotaryIgnitionAngles(dwellAngle, current);
  }
}
