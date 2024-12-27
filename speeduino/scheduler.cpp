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

  ignition1StartAngle=0;
  ignition1EndAngle=0;
  channel1IgnDegrees=0; /**< The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones) */

  ignition2StartAngle=0;
  ignition2EndAngle=0;
  channel2IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */

  ignition3StartAngle=0;
  ignition3EndAngle=0;
  channel3IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */

  ignition4StartAngle=0;
  ignition4EndAngle=0;
  channel4IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */

#if (IGN_CHANNELS >= 5)
  ignition5StartAngle=0;
  ignition5EndAngle=0;
  channel5IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
#endif
#if (IGN_CHANNELS >= 6)
  ignition6StartAngle=0;
  ignition6EndAngle=0;
  channel6IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
#endif
#if (IGN_CHANNELS >= 7)
  ignition7StartAngle=0;
  ignition7EndAngle=0;
  channel7IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
#endif
#if (IGN_CHANNELS >= 8)
  ignition8StartAngle=0;
  ignition8EndAngle=0;
  channel8IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
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

void refreshIgnitionSchedule1(unsigned long timeToEnd)
{
  if( (isRunning(ignitionSchedule1)) && ((COMPARE_TYPE)uS_TO_TIMER_COMPARE(timeToEnd) < ignitionSchedule1.duration) )
  //Must have the threshold check here otherwise it can cause a condition where the compare fires twice, once after the other, both for the end
  {
    ATOMIC() {
      ignitionSchedule1.endCompare = IGN1_COUNTER + uS_TO_TIMER_COMPARE(timeToEnd);
      SET_COMPARE(IGN1_COMPARE, ignitionSchedule1.endCompare);
    }
  }
}

static table2D_u8_u8_4 PrimingPulseTable(&configPage2.primeBins, &configPage2.primePulse);

/** Perform the injector priming pulses.
 * Set these to run at an arbitrary time in the future (100us).
 * The prime pulse value is in ms*10, so need to multiple by 100 to get to uS
 */
void beginInjectorPriming(void)
{
  uint16_t primingValue = (uint16_t)table2D_getValue(&PrimingPulseTable, temperatureAddOffset(currentStatus.coolant));
  if( (primingValue > 0) && (currentStatus.TPS <= configPage4.floodClear) )
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

// Shared ISR function for all fuel timers.
// This is completely inlined into the ISR - there is no function call
// overhead.
static inline __attribute__((always_inline)) void fuelScheduleISR(FuelSchedule &schedule)
{
  if (schedule.Status == PENDING) //Check to see if this schedule is turn on
  {
    schedule.pStartCallback();
    schedule.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
    SET_COMPARE(schedule._compare, schedule._counter + schedule.duration); //Doing this here prevents a potential overflow on restarts
  }
  else if (isRunning(schedule))
  {
    schedule.pEndCallback();

    //If there is a next schedule queued up, activate it
    if(schedule.Status==RUNNING_WITHNEXT)
    {
      SET_COMPARE(schedule._compare, schedule.nextStartCompare); //Flip the next start compare time to be the current one. The duration of this next pulse will already have been set in _setFuelScheduleNext()
      schedule.Status = PENDING;
    } else {
      schedule.Status = OFF; //Turn off the schedule
    }
  }
  else
  {
    // Schedule is off, so do nothing but keep the MISRA checker happy
  }
} 

/*******************************************************************************************************************************************************************************************************/
/** fuelSchedule*Interrupt (All 8 ISR functions below) get called (as timed interrupts) when either the start time or the duration time are reached.
* This calls the relevant callback function (startCallback or endCallback) depending on the status (PENDING => Needs to run, RUNNING => Needs to stop) of the schedule.
* The status of schedule is managed here based on startCallback /endCallback function called:
* - startCallback - change scheduler into RUNNING state
* - endCallback - change scheduler into OFF state (or PENDING if schedule.hasNextSchedule is set)
*/
//Timer3A (fuel schedule 1) Compare Vector
#ifdef CORE_AVR //AVR chips use the ISR for this
//fuelSchedules 1 and 5
ISR(TIMER3_COMPA_vect) //cppcheck-suppress misra-c2012-8.2
#else
void fuelSchedule1Interrupt() //Most ARM chips can simply call a function
#endif
  {
    fuelScheduleISR(fuelSchedule1);
  }


#ifdef CORE_AVR
ISR(TIMER3_COMPB_vect) //cppcheck-suppress misra-c2012-8.2
#else
void fuelSchedule2Interrupt() //Most ARM chips can simply call a function
#endif
  {
    fuelScheduleISR(fuelSchedule2);
  }


#ifdef CORE_AVR
ISR(TIMER3_COMPC_vect) //cppcheck-suppress misra-c2012-8.2
#else
void fuelSchedule3Interrupt() //Most ARM chips can simply call a function
#endif
  {
    fuelScheduleISR(fuelSchedule3);
  }


#ifdef CORE_AVR
ISR(TIMER4_COMPB_vect) //cppcheck-suppress misra-c2012-8.2
#else
void fuelSchedule4Interrupt() //Most ARM chips can simply call a function
#endif
  {
    fuelScheduleISR(fuelSchedule4);
  }

#if INJ_CHANNELS >= 5
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER4_COMPC_vect) //cppcheck-suppress misra-c2012-8.2
#else
void fuelSchedule5Interrupt() //Most ARM chips can simply call a function
#endif
  {
    fuelScheduleISR(fuelSchedule5);
  }
#endif

#if INJ_CHANNELS >= 6
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER4_COMPA_vect) //cppcheck-suppress misra-c2012-8.2
#else
void fuelSchedule6Interrupt() //Most ARM chips can simply call a function
#endif
  {
    fuelScheduleISR(fuelSchedule6);
  }
#endif

#if INJ_CHANNELS >= 7
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER5_COMPC_vect) //cppcheck-suppress misra-c2012-8.2
#else
void fuelSchedule7Interrupt() //Most ARM chips can simply call a function
#endif
  {
    fuelScheduleISR(fuelSchedule7);
  }
#endif

#if INJ_CHANNELS >= 8
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER5_COMPB_vect) //cppcheck-suppress misra-c2012-8.2
#else
void fuelSchedule8Interrupt() //Most ARM chips can simply call a function
#endif
  {
    fuelScheduleISR(fuelSchedule8);
  }
#endif

// Shared ISR function for all ignition timers.
// This is completely inlined into the ISR - there is no function call
// overhead.
static inline __attribute__((always_inline)) void ignitionScheduleISR(IgnitionSchedule &schedule)
{
  if (schedule.Status == PENDING) //Check to see if this schedule is turn on
  {
    schedule.pStartCallback();
    schedule.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
    schedule.startTime = micros();
    if(schedule.endScheduleSetByDecoder == true) { SET_COMPARE(schedule._compare, schedule.endCompare); }
    else { SET_COMPARE(schedule._compare, schedule._counter + schedule.duration); } //Doing this here prevents a potential overflow on restarts
  }
  else if (isRunning(schedule))
  {
    schedule.pEndCallback();
    schedule.endScheduleSetByDecoder = false;
    ignitionCount = ignitionCount + 1U; //Increment the ignition counter
    currentStatus.actualDwell = DWELL_AVERAGE( (micros() - schedule.startTime) );

    //If there is a next schedule queued up, activate it
    if(schedule.Status==RUNNING_WITHNEXT)
    {
      SET_COMPARE(schedule._compare, schedule.nextStartCompare);
      schedule.Status = PENDING;
    } else {
      schedule.Status = OFF; //Turn off the schedule
    }
  }
  else
  {
    // Schedule is off, so do nothing but keep the MISRA checker happy
  }  
}

#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER5_COMPA_vect) //cppcheck-suppress misra-c2012-8.2
#else
void ignitionSchedule1Interrupt(void) //Most ARM chips can simply call a function
#endif
  {
    ignitionScheduleISR(ignitionSchedule1);
  }

#if IGN_CHANNELS >= 2
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER5_COMPB_vect) //cppcheck-suppress misra-c2012-8.2
#else
void ignitionSchedule2Interrupt(void) //Most ARM chips can simply call a function
#endif
  {
    ignitionScheduleISR(ignitionSchedule2);
  }
#endif

#if IGN_CHANNELS >= 3
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER5_COMPC_vect) //cppcheck-suppress misra-c2012-8.2
#else
void ignitionSchedule3Interrupt(void) //Most ARM chips can simply call a function
#endif
  {
    ignitionScheduleISR(ignitionSchedule3);
  }
#endif

#if IGN_CHANNELS >= 4
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER4_COMPA_vect) //cppcheck-suppress misra-c2012-8.2
#else
void ignitionSchedule4Interrupt(void) //Most ARM chips can simply call a function
#endif
  {
    ignitionScheduleISR(ignitionSchedule4);
  }
#endif

#if IGN_CHANNELS >= 5
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER4_COMPC_vect) //cppcheck-suppress misra-c2012-8.2
#else
void ignitionSchedule5Interrupt(void) //Most ARM chips can simply call a function
#endif
  {
    ignitionScheduleISR(ignitionSchedule5);
  }
#endif

#if IGN_CHANNELS >= 6
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER4_COMPB_vect) //cppcheck-suppress misra-c2012-8.2
#else
void ignitionSchedule6Interrupt(void) //Most ARM chips can simply call a function
#endif
  {
    ignitionScheduleISR(ignitionSchedule6);
  }
#endif

#if IGN_CHANNELS >= 7
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER3_COMPC_vect) //cppcheck-suppress misra-c2012-8.2
#else
void ignitionSchedule7Interrupt(void) //Most ARM chips can simply call a function
#endif
  {
    ignitionScheduleISR(ignitionSchedule7);
  }
#endif

#if IGN_CHANNELS >= 8
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER3_COMPB_vect) //cppcheck-suppress misra-c2012-8.2
#else
void ignitionSchedule8Interrupt(void) //Most ARM chips can simply call a function
#endif
  {
    ignitionScheduleISR(ignitionSchedule8);
  }
#endif

static void disableSchedule(Schedule &schedule)
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

void disableAllFuelSchedules()
{
  disableFuelSchedule(0);
  disableFuelSchedule(1);
  disableFuelSchedule(2);
  disableFuelSchedule(3);
  disableFuelSchedule(4);
  disableFuelSchedule(5);
  disableFuelSchedule(6);
  disableFuelSchedule(7);
}
void disableAllIgnSchedules()
{
  disableIgnSchedule(0);
  disableIgnSchedule(1);
  disableIgnSchedule(2);
  disableIgnSchedule(3);
  disableIgnSchedule(4);
  disableIgnSchedule(5);
  disableIgnSchedule(6);
  disableIgnSchedule(7);
}
