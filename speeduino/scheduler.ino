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

FuelSchedule fuelSchedule1;
FuelSchedule fuelSchedule2;
FuelSchedule fuelSchedule3;
FuelSchedule fuelSchedule4;
FuelSchedule fuelSchedule5;
FuelSchedule fuelSchedule6;
FuelSchedule fuelSchedule7;
FuelSchedule fuelSchedule8;

Schedule ignitionSchedule1;
Schedule ignitionSchedule2;
Schedule ignitionSchedule3;
Schedule ignitionSchedule4;
Schedule ignitionSchedule5;
Schedule ignitionSchedule6;
Schedule ignitionSchedule7;
Schedule ignitionSchedule8;

void (*inj1StartFunction)(void);
void (*inj1EndFunction)(void);
void (*inj2StartFunction)(void);
void (*inj2EndFunction)(void);
void (*inj3StartFunction)(void);
void (*inj3EndFunction)(void);
void (*inj4StartFunction)(void);
void (*inj4EndFunction)(void);
void (*inj5StartFunction)(void);
void (*inj5EndFunction)(void);
void (*inj6StartFunction)(void);
void (*inj6EndFunction)(void);
void (*inj7StartFunction)(void);
void (*inj7EndFunction)(void);
void (*inj8StartFunction)(void);
void (*inj8EndFunction)(void);

void (*ign1StartFunction)(void);
void (*ign1EndFunction)(void);
void (*ign2StartFunction)(void);
void (*ign2EndFunction)(void);
void (*ign3StartFunction)(void);
void (*ign3EndFunction)(void);
void (*ign4StartFunction)(void);
void (*ign4EndFunction)(void);
void (*ign5StartFunction)(void);
void (*ign5EndFunction)(void);
void (*ign6StartFunction)(void);
void (*ign6EndFunction)(void);
void (*ign7StartFunction)(void);
void (*ign7EndFunction)(void);
void (*ign8StartFunction)(void);
void (*ign8EndFunction)(void);

void initialiseSchedulers(void)
{
    //nullSchedule.Status = OFF;

    fuelSchedule1.Status = OFF;
    fuelSchedule2.Status = OFF;
    fuelSchedule3.Status = OFF;
    fuelSchedule4.Status = OFF;
    fuelSchedule5.Status = OFF;
    fuelSchedule6.Status = OFF;
    fuelSchedule7.Status = OFF;
    fuelSchedule8.Status = OFF;

    fuelSchedule1.schedulesSet = 0;
    fuelSchedule2.schedulesSet = 0;
    fuelSchedule3.schedulesSet = 0;
    fuelSchedule4.schedulesSet = 0;
    fuelSchedule5.schedulesSet = 0;
    fuelSchedule6.schedulesSet = 0;
    fuelSchedule7.schedulesSet = 0;
    fuelSchedule8.schedulesSet = 0;

    ignitionSchedule1.Status = OFF;
    ignitionSchedule2.Status = OFF;
    ignitionSchedule3.Status = OFF;
    ignitionSchedule4.Status = OFF;
    ignitionSchedule5.Status = OFF;
    ignitionSchedule6.Status = OFF;
    ignitionSchedule7.Status = OFF;
    ignitionSchedule8.Status = OFF;

    IGN1_TIMER_ENABLE();
    IGN2_TIMER_ENABLE();
    IGN3_TIMER_ENABLE();
    IGN4_TIMER_ENABLE();
#if (IGN_CHANNELS >= 5)
    IGN5_TIMER_ENABLE();
    IGN6_TIMER_ENABLE();
    IGN7_TIMER_ENABLE();
    IGN8_TIMER_ENABLE();
#endif

    FUEL1_TIMER_ENABLE();
    FUEL2_TIMER_ENABLE();
    FUEL3_TIMER_ENABLE();
    FUEL4_TIMER_ENABLE();
#if (INJ_CHANNELS >= 5)
    FUEL5_TIMER_ENABLE();
    FUEL6_TIMER_ENABLE();
    FUEL7_TIMER_ENABLE();
    FUEL8_TIMER_ENABLE();
#endif

    ignitionSchedule1.schedulesSet = 0;
    ignitionSchedule2.schedulesSet = 0;
    ignitionSchedule3.schedulesSet = 0;
    ignitionSchedule4.schedulesSet = 0;
    ignitionSchedule5.schedulesSet = 0;
    ignitionSchedule6.schedulesSet = 0;
    ignitionSchedule7.schedulesSet = 0;
    ignitionSchedule8.schedulesSet = 0;
}

/*
These 8 function turn a schedule on, provides the time to start and the duration and gives it callback functions.
All 8 functions operate the same, just on different schedules
Args:
startCallback: The function to be called once the timeout is reached
timeout: The number of uS in the future that the startCallback should be triggered
duration: The number of uS after startCallback is called before endCallback is called
endCallback: This function is called once the duration time has been reached
*/

static void SET_FUEL1(COMPARE_TYPE compare) { FUEL1_COMPARE = compare; }
static void SET_FUEL2(COMPARE_TYPE compare) { FUEL2_COMPARE = compare; }
static void SET_FUEL3(COMPARE_TYPE compare) { FUEL3_COMPARE = compare; }
static void SET_FUEL4(COMPARE_TYPE compare) { FUEL4_COMPARE = compare; }
static void SET_FUEL5(COMPARE_TYPE compare) { FUEL5_COMPARE = compare; }
static void SET_FUEL6(COMPARE_TYPE compare) { FUEL6_COMPARE = compare; }
static void SET_FUEL7(COMPARE_TYPE compare) { FUEL7_COMPARE = compare; }
static void SET_FUEL8(COMPARE_TYPE compare) { FUEL8_COMPARE = compare; }

static COUNTER_TYPE GET_FUEL1() { return FUEL1_COUNTER; }
static COUNTER_TYPE GET_FUEL2() { return FUEL2_COUNTER; }
static COUNTER_TYPE GET_FUEL3() { return FUEL3_COUNTER; }
static COUNTER_TYPE GET_FUEL4() { return FUEL4_COUNTER; }
static COUNTER_TYPE GET_FUEL5() { return FUEL5_COUNTER; }
static COUNTER_TYPE GET_FUEL6() { return FUEL6_COUNTER; }
static COUNTER_TYPE GET_FUEL7() { return FUEL7_COUNTER; }
static COUNTER_TYPE GET_FUEL8() { return FUEL8_COUNTER; }

//Experimental new generic function. This is NOT yet ready and functional
static inline __attribute__((always_inline)) // <-- this is critical for performance
void setFuelScheduleA(unsigned long timeout,
  unsigned long duration,
  void (*timer_enable)(),
  FuelSchedule& schedule,
  void (*set_compare)(COMPARE_TYPE),
  COMPARE_TYPE (*get_counter)())
{
  //Check whether timeout exceeds the maximum future time. This can potentially occur on sequential setups when below ~115rpm
  //if(timeout < MAX_TIMER_PERIOD)
  {
    if(schedule.Status != RUNNING) //Check that we're not already part way through a schedule
    {
      //Need to check that the timeout doesn't exceed the overflow
      if ((timeout+duration) < MAX_TIMER_PERIOD)
      {
        schedule.duration = duration;
        //The following must be enclosed in the noInterupts block to avoid contention caused if the relevant interrupt fires before the state is fully set
        noInterrupts();
        schedule.startCompare = get_counter() + uS_TO_TIMER_COMPARE(timeout);
        schedule.endCompare = schedule.startCompare + uS_TO_TIMER_COMPARE(duration);
        schedule.Status = PENDING; //Turn this schedule on
        schedule.schedulesSet++; //Increment the number of times this schedule has been set
        //Schedule 1 shares a timer with schedule 5
        //if(channel5InjEnabled) { SET_COMPARE(compare, setQueue(timer3Aqueue, &schedule, &fuelSchedule5, FUEL1_COUNTER) ); }
        //else { timer3Aqueue[0] = &schedule; timer3Aqueue[1] = &schedule; timer3Aqueue[2] = &schedule; timer3Aqueue[3] = &schedule; SET_COMPARE(compare, fuelSchedule1.startCompare); }
        //timer3Aqueue[0] = &schedule; timer3Aqueue[1] = &schedule; timer3Aqueue[2] = &schedule; timer3Aqueue[3] = &schedule;
        set_compare(schedule.startCompare);
        interrupts();
        timer_enable();
      }
    }
    else
    {
      //If the schedule is already running, we can set the next schedule so it is ready to go
      //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
      if( (timeout+duration) < MAX_TIMER_PERIOD )
      {
        noInterrupts();
        schedule.nextStartCompare = get_counter() + uS_TO_TIMER_COMPARE(timeout);
        schedule.nextEndCompare = schedule.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
        schedule.duration = duration;
        schedule.hasNextSchedule = true;
        interrupts();
      }
    } //Schedule is RUNNING
  } //Timeout less than threshold
}

//void setFuelSchedule1(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
void setFuelSchedule1(unsigned long timeout, unsigned long duration) //Uses timer 3 compare A
{
  setFuelScheduleA(timeout, duration, FUEL1_TIMER_ENABLE, fuelSchedule1, SET_FUEL1, GET_FUEL1);
}

//Experimental new generic function. This is NOT yet ready and functional
static inline __attribute__((always_inline)) // <-- this is critical for performance
void setFuelScheduleB(unsigned long timeout,
  unsigned long duration,
  void (*timer_enable)(),
  FuelSchedule& schedule,
  void (*set_compare)(COMPARE_TYPE),
  COMPARE_TYPE (*get_counter)())
{
  //Check whether timeout exceeds the maximum future time. This can potentially occur on sequential setups when below ~115rpm
  if(timeout < MAX_TIMER_PERIOD)
  {
    if(schedule.Status != RUNNING) //Check that we're not already part way through a schedule
    {
      //Callbacks no longer used, but retained for now:
      //schedule.StartCallback = startCallback;
      //schedule.EndCallback = endCallback;
      schedule.duration = duration;

      //Need to check that the timeout doesn't exceed the overflow
      COMPARE_TYPE timeout_timer_compare;
      if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
      else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

      //The following must be enclosed in the noInterupts block to avoid contention caused if the relevant interrupt fires before the state is fully set
      noInterrupts();
      schedule.startCompare = get_counter() + timeout_timer_compare;
      schedule.endCompare = schedule.startCompare + uS_TO_TIMER_COMPARE(duration);
      set_compare(schedule.startCompare); //Use the B compare unit of timer 3
      schedule.Status = PENDING; //Turn this schedule on
      schedule.schedulesSet++; //Increment the number of times this schedule has been set
      interrupts();
      timer_enable();
    }
    else
    {
      //If the schedule is already running, we can set the next schedule so it is ready to go
      //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
      schedule.nextStartCompare = get_counter() + uS_TO_TIMER_COMPARE(timeout);
      schedule.nextEndCompare = schedule.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
      schedule.hasNextSchedule = true;
    }
  }
}

void setFuelSchedule2(unsigned long timeout, unsigned long duration) //Uses timer 3 compare B
{
  setFuelScheduleB(timeout, duration, FUEL2_TIMER_ENABLE, fuelSchedule2, SET_FUEL2, GET_FUEL2);
}

void setFuelSchedule3(unsigned long timeout, unsigned long duration) //Uses timer 3 compare C
{
  setFuelScheduleB(timeout, duration, FUEL3_TIMER_ENABLE, fuelSchedule3, SET_FUEL3, GET_FUEL3);
}

void setFuelSchedule4(unsigned long timeout, unsigned long duration) //Uses timer 4 compare B
{
  setFuelScheduleB(timeout, duration, FUEL4_TIMER_ENABLE, fuelSchedule4, SET_FUEL4, GET_FUEL4);
}

#if INJ_CHANNELS >= 5
void setFuelSchedule5(unsigned long timeout, unsigned long duration) //Uses timer 4 compare C
{
  setFuelScheduleB(timeout, duration, FUEL5_TIMER_ENABLE, fuelSchedule5, SET_FUEL5, GET_FUEL5);
}
#endif

#if INJ_CHANNELS >= 6
void setFuelSchedule6(unsigned long timeout, unsigned long duration) //Uses timer 4 compare A
{
  setFuelScheduleB(timeout, duration, FUEL6_TIMER_ENABLE, fuelSchedule6, SET_FUEL6, GET_FUEL6);
}
#endif

#if INJ_CHANNELS >= 7
void setFuelSchedule7(unsigned long timeout, unsigned long duration) //Uses timer 5 compare C
{
  setFuelScheduleB(timeout, duration, FUEL7_TIMER_ENABLE, fuelSchedule7, SET_FUEL7, GET_FUEL7);
}
#endif

#if INJ_CHANNELS >= 8
void setFuelSchedule8(unsigned long timeout, unsigned long duration) //Uses timer 5 compare B
{
  setFuelScheduleB(timeout, duration, FUEL8_TIMER_ENABLE, fuelSchedule8, SET_FUEL8, GET_FUEL8);
}
#endif

static void SET_IGN1(COMPARE_TYPE compare) { IGN1_COMPARE = compare; }
static void SET_IGN2(COMPARE_TYPE compare) { IGN2_COMPARE = compare; }
static void SET_IGN3(COMPARE_TYPE compare) { IGN3_COMPARE = compare; }
static void SET_IGN4(COMPARE_TYPE compare) { IGN4_COMPARE = compare; }
static void SET_IGN5(COMPARE_TYPE compare) { IGN5_COMPARE = compare; }
static void SET_IGN6(COMPARE_TYPE compare) { IGN6_COMPARE = compare; }
static void SET_IGN7(COMPARE_TYPE compare) { IGN7_COMPARE = compare; }
static void SET_IGN8(COMPARE_TYPE compare) { IGN8_COMPARE = compare; }

static COUNTER_TYPE GET_IGN1() { return IGN1_COUNTER; }
static COUNTER_TYPE GET_IGN2() { return IGN2_COUNTER; }
static COUNTER_TYPE GET_IGN3() { return IGN3_COUNTER; }
static COUNTER_TYPE GET_IGN4() { return IGN4_COUNTER; }
static COUNTER_TYPE GET_IGN5() { return IGN5_COUNTER; }
static COUNTER_TYPE GET_IGN6() { return IGN6_COUNTER; }
static COUNTER_TYPE GET_IGN7() { return IGN7_COUNTER; }
static COUNTER_TYPE GET_IGN8() { return IGN8_COUNTER; }

static inline __attribute__((always_inline)) // <-- this is critical for performance
void setIgnitionScheduleA(
  void (*startCallback)(),
  unsigned long timeout,
  unsigned long duration,
  void(*endCallback)(),
  void (*timer_enable)(),
  Schedule& schedule,
  void (*set_compare)(COMPARE_TYPE),
  COMPARE_TYPE (*get_counter)())
{
  if(schedule.Status != RUNNING) //Check that we're not already part way through a schedule
  {
    schedule.StartCallback = startCallback; //Name the start callback function
    schedule.EndCallback = endCallback; //Name the start callback function
    schedule.duration = duration;

    //Need to check that the timeout doesn't exceed the overflow
    COMPARE_TYPE timeout_timer_compare;
    //timeout -= (micros() - lastCrankAngleCalc);
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

    noInterrupts();
    schedule.startCompare = get_counter() + timeout_timer_compare; //As there is a tick every 4uS, there are timeout/4 ticks until the interrupt should be triggered ( >>2 divides by 4)
    if(schedule.endScheduleSetByDecoder == false) { schedule.endCompare = schedule.startCompare + uS_TO_TIMER_COMPARE(duration); } //The .endCompare value is also set by the per tooth timing in decoders.ino. The check here is so that it's not getting overridden. 
    set_compare(schedule.startCompare);
    schedule.Status = PENDING; //Turn this schedule on
    schedule.schedulesSet++;
    interrupts();
    timer_enable();
  }
  else
  {
    //If the schedule is already running, we can set the next schedule so it is ready to go
    //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
    if (timeout < MAX_TIMER_PERIOD)
    {
      schedule.nextStartCompare = get_counter() + uS_TO_TIMER_COMPARE(timeout);
      schedule.nextEndCompare = schedule.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
      schedule.hasNextSchedule = true;
    }
  }
}

void setIgnitionSchedule1(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  setIgnitionScheduleA(startCallback, timeout, duration, endCallback, IGN1_TIMER_ENABLE, ignitionSchedule1, SET_IGN1, GET_IGN1);
}

inline void refreshIgnitionSchedule1(unsigned long timeToEnd)
{
  if( (ignitionSchedule1.Status == RUNNING) && (timeToEnd < ignitionSchedule1.duration) )
  //Must have the threshold check here otherwise it can cause a condition where the compare fires twice, once after the other, both for the end
  //if( (timeToEnd < ignitionSchedule1.duration) && (timeToEnd > IGNITION_REFRESH_THRESHOLD) )
  {
    noInterrupts();
    ignitionSchedule1.endCompare = IGN1_COUNTER + uS_TO_TIMER_COMPARE(timeToEnd);
    SET_COMPARE(IGN1_COMPARE, ignitionSchedule1.endCompare);
    interrupts();
  }
}

void setIgnitionSchedule2(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  setIgnitionScheduleA(startCallback, timeout, duration, endCallback, IGN2_TIMER_ENABLE, ignitionSchedule2, SET_IGN2, GET_IGN2);
}
void setIgnitionSchedule3(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  setIgnitionScheduleA(startCallback, timeout, duration, endCallback, IGN3_TIMER_ENABLE, ignitionSchedule3, SET_IGN3, GET_IGN3);
}
void setIgnitionSchedule4(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  setIgnitionScheduleA(startCallback, timeout, duration, endCallback, IGN4_TIMER_ENABLE, ignitionSchedule4, SET_IGN4, GET_IGN4);
}
void setIgnitionSchedule5(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  setIgnitionScheduleA(startCallback, timeout, duration, endCallback, IGN5_TIMER_ENABLE, ignitionSchedule5, SET_IGN5, GET_IGN5);
}
void setIgnitionSchedule6(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  setIgnitionScheduleA(startCallback, timeout, duration, endCallback, IGN6_TIMER_ENABLE, ignitionSchedule6, SET_IGN6, GET_IGN6);
}
void setIgnitionSchedule7(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  setIgnitionScheduleA(startCallback, timeout, duration, endCallback, IGN7_TIMER_ENABLE, ignitionSchedule7, SET_IGN7, GET_IGN7);
}
void setIgnitionSchedule8(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  setIgnitionScheduleA(startCallback, timeout, duration, endCallback, IGN8_TIMER_ENABLE, ignitionSchedule8, SET_IGN8, GET_IGN8);
}
/** Perform the injector priming pulses.
 * Set these to run at an arbitrary time in the future (100us).
 * The prime pulse value is in ms*10, so need to multiple by 100 to get to uS
 */
extern void beginInjectorPriming(void)
{
  unsigned long primingValue = table2D_getValue(&PrimingPulseTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
  if( (primingValue > 0) && (currentStatus.TPS < configPage4.floodClear) )
  {
    primingValue = primingValue * 100 * 5; //to achieve long enough priming pulses, the values in tuner studio are divided by 0.5 instead of 0.1, so multiplier of 5 is required.
    if ( BIT_CHECK(channelInjEnabled, INJ1_CMD_BIT) == true ) { setFuelSchedule1(100, primingValue); }
#if (INJ_CHANNELS >= 2)
    if ( BIT_CHECK(channelInjEnabled, INJ2_CMD_BIT) == true ) { setFuelSchedule2(100, primingValue); }
#endif
#if (INJ_CHANNELS >= 3)
    if ( BIT_CHECK(channelInjEnabled, INJ3_CMD_BIT) == true ) { setFuelSchedule3(100, primingValue); }
#endif
#if (INJ_CHANNELS >= 4)
    if ( BIT_CHECK(channelInjEnabled, INJ4_CMD_BIT) == true ) { setFuelSchedule4(100, primingValue); }
#endif
#if (INJ_CHANNELS >= 5)
    if ( BIT_CHECK(channelInjEnabled, INJ5_CMD_BIT) == true ) { setFuelSchedule5(100, primingValue); }
#endif
#if (INJ_CHANNELS >= 6)
    if ( BIT_CHECK(channelInjEnabled, INJ6_CMD_BIT) == true ) { setFuelSchedule6(100, primingValue); }
#endif
#if (INJ_CHANNELS >= 7)
    if ( BIT_CHECK(channelInjEnabled, INJ7_CMD_BIT) == true) { setFuelSchedule7(100, primingValue); }
#endif
#if (INJ_CHANNELS >= 8)
    if ( BIT_CHECK(channelInjEnabled, INJ8_CMD_BIT) == true ) { setFuelSchedule8(100, primingValue); }
#endif
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
#if (INJ_CHANNELS >= 1)
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
//fuelSchedules 1 and 5
ISR(TIMER3_COMPA_vect) //cppcheck-suppress misra-c2012-8.2
#else
static inline void fuelSchedule1Interrupt() //Most ARM chips can simply call a function
#endif
  {
    if (fuelSchedule1.Status == PENDING) //Check to see if this schedule is turn on
    {
      //To use timer queue, change fuelShedule1 to timer3Aqueue[0];
      inj1StartFunction();
      fuelSchedule1.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      SET_COMPARE(FUEL1_COMPARE, FUEL1_COUNTER + uS_TO_TIMER_COMPARE(fuelSchedule1.duration) ); //Doing this here prevents a potential overflow on restarts
    }
    else if (fuelSchedule1.Status == RUNNING)
    {
       //timer3Aqueue[0]->EndCallback();
       inj1EndFunction();
       fuelSchedule1.Status = OFF; //Turn off the schedule
       fuelSchedule1.schedulesSet = 0;
       //SET_COMPARE(FUEL1_COMPARE, fuelSchedule1.endCompare);

       //If there is a next schedule queued up, activate it
       if(fuelSchedule1.hasNextSchedule == true)
       {
         SET_COMPARE(FUEL1_COMPARE, fuelSchedule1.nextStartCompare);
         fuelSchedule1.endCompare = fuelSchedule1.nextEndCompare;
         fuelSchedule1.Status = PENDING;
         fuelSchedule1.schedulesSet = 1;
         fuelSchedule1.hasNextSchedule = false;
       }
       else { FUEL1_TIMER_DISABLE(); }
    }
    else if (fuelSchedule1.Status == OFF) { FUEL1_TIMER_DISABLE(); } //Safety check. Turn off this output compare unit and return without performing any action
  }
#endif

#if (INJ_CHANNELS >= 2)
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
ISR(TIMER3_COMPB_vect) //cppcheck-suppress misra-c2012-8.2
#else
static inline void fuelSchedule2Interrupt() //Most ARM chips can simply call a function
#endif
  {
    if (fuelSchedule2.Status == PENDING) //Check to see if this schedule is turn on
    {
      //fuelSchedule2.StartCallback();
      inj2StartFunction();
      fuelSchedule2.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      SET_COMPARE(FUEL2_COMPARE, FUEL2_COUNTER + uS_TO_TIMER_COMPARE(fuelSchedule2.duration) ); //Doing this here prevents a potential overflow on restarts
    }
    else if (fuelSchedule2.Status == RUNNING)
    {
       //fuelSchedule2.EndCallback();
       inj2EndFunction();
       fuelSchedule2.Status = OFF; //Turn off the schedule
       fuelSchedule2.schedulesSet = 0;

       //If there is a next schedule queued up, activate it
       if(fuelSchedule2.hasNextSchedule == true)
       {
         SET_COMPARE(FUEL2_COMPARE, fuelSchedule2.nextStartCompare);
         fuelSchedule2.endCompare = fuelSchedule2.nextEndCompare;
         fuelSchedule2.Status = PENDING;
         fuelSchedule2.schedulesSet = 1;
         fuelSchedule2.hasNextSchedule = false;
       }
       else { FUEL2_TIMER_DISABLE(); }
    }
  }
#endif

#if (INJ_CHANNELS >= 3)
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
ISR(TIMER3_COMPC_vect) //cppcheck-suppress misra-c2012-8.2
#else
static inline void fuelSchedule3Interrupt() //Most ARM chips can simply call a function
#endif
  {
    if (fuelSchedule3.Status == PENDING) //Check to see if this schedule is turn on
    {
      //fuelSchedule3.StartCallback();
      inj3StartFunction();
      fuelSchedule3.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      SET_COMPARE(FUEL3_COMPARE, FUEL3_COUNTER + uS_TO_TIMER_COMPARE(fuelSchedule3.duration) ); //Doing this here prevents a potential overflow on restarts
    }
    else if (fuelSchedule3.Status == RUNNING)
    {
       //fuelSchedule3.EndCallback();
       inj3EndFunction();
       fuelSchedule3.Status = OFF; //Turn off the schedule
       fuelSchedule3.schedulesSet = 0;

       //If there is a next schedule queued up, activate it
       if(fuelSchedule3.hasNextSchedule == true)
       {
         SET_COMPARE(FUEL3_COMPARE, fuelSchedule3.nextStartCompare);
         fuelSchedule3.endCompare = fuelSchedule3.nextEndCompare;
         fuelSchedule3.Status = PENDING;
         fuelSchedule3.schedulesSet = 1;
         fuelSchedule3.hasNextSchedule = false;
       }
       else { FUEL3_TIMER_DISABLE(); }
    }
  }
#endif

#if (INJ_CHANNELS >= 4)
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
ISR(TIMER4_COMPB_vect) //cppcheck-suppress misra-c2012-8.2
#else
static inline void fuelSchedule4Interrupt() //Most ARM chips can simply call a function
#endif
  {
    if (fuelSchedule4.Status == PENDING) //Check to see if this schedule is turn on
    {
      //fuelSchedule4.StartCallback();
      inj4StartFunction();
      fuelSchedule4.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      SET_COMPARE(FUEL4_COMPARE, FUEL4_COUNTER + uS_TO_TIMER_COMPARE(fuelSchedule4.duration) ); //Doing this here prevents a potential overflow on restarts
    }
    else if (fuelSchedule4.Status == RUNNING)
    {
       //fuelSchedule4.EndCallback();
       inj4EndFunction();
       fuelSchedule4.Status = OFF; //Turn off the schedule
       fuelSchedule4.schedulesSet = 0;

       //If there is a next schedule queued up, activate it
       if(fuelSchedule4.hasNextSchedule == true)
       {
         SET_COMPARE(FUEL4_COMPARE, fuelSchedule4.nextStartCompare);
         fuelSchedule4.endCompare = fuelSchedule4.nextEndCompare;
         fuelSchedule4.Status = PENDING;
         fuelSchedule4.schedulesSet = 1;
         fuelSchedule4.hasNextSchedule = false;
       }
       else { FUEL4_TIMER_DISABLE(); }
    }
  }
#endif

#if (INJ_CHANNELS >= 5)
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER4_COMPC_vect) //cppcheck-suppress misra-c2012-8.2
#else
static inline void fuelSchedule5Interrupt() //Most ARM chips can simply call a function
#endif
{
  if (fuelSchedule5.Status == PENDING) //Check to see if this schedule is turn on
  {
    inj5StartFunction();
    fuelSchedule5.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
    SET_COMPARE(FUEL5_COMPARE, FUEL5_COUNTER + uS_TO_TIMER_COMPARE(fuelSchedule5.duration) ); //Doing this here prevents a potential overflow on restarts
  }
  else if (fuelSchedule5.Status == RUNNING)
  {
     inj5EndFunction();
     fuelSchedule5.Status = OFF; //Turn off the schedule
     fuelSchedule5.schedulesSet = 0;

     //If there is a next schedule queued up, activate it
     if(fuelSchedule5.hasNextSchedule == true)
     {
       SET_COMPARE(FUEL5_COMPARE, fuelSchedule5.nextStartCompare);
       fuelSchedule5.endCompare = fuelSchedule5.nextEndCompare;
       fuelSchedule5.Status = PENDING;
       fuelSchedule5.schedulesSet = 1;
       fuelSchedule5.hasNextSchedule = false;
     }
     else { FUEL5_TIMER_DISABLE(); }
  }
}
#endif

#if (INJ_CHANNELS >= 6)
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER4_COMPA_vect) //cppcheck-suppress misra-c2012-8.2
#else
static inline void fuelSchedule6Interrupt() //Most ARM chips can simply call a function
#endif
{
  if (fuelSchedule6.Status == PENDING) //Check to see if this schedule is turn on
  {
    //fuelSchedule6.StartCallback();
    inj6StartFunction();
    fuelSchedule6.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
    SET_COMPARE(FUEL6_COMPARE, FUEL6_COUNTER + uS_TO_TIMER_COMPARE(fuelSchedule6.duration) ); //Doing this here prevents a potential overflow on restarts
  }
  else if (fuelSchedule6.Status == RUNNING)
  {
     //fuelSchedule6.EndCallback();
     inj6EndFunction();
     fuelSchedule6.Status = OFF; //Turn off the schedule
     fuelSchedule6.schedulesSet = 0;

     //If there is a next schedule queued up, activate it
     if(fuelSchedule6.hasNextSchedule == true)
     {
       SET_COMPARE(FUEL6_COMPARE, fuelSchedule6.nextStartCompare);
       fuelSchedule6.endCompare = fuelSchedule6.nextEndCompare;
       fuelSchedule6.Status = PENDING;
       fuelSchedule6.schedulesSet = 1;
       fuelSchedule6.hasNextSchedule = false;
     }
     else { FUEL6_TIMER_DISABLE(); }
  }
}
#endif

#if (INJ_CHANNELS >= 7)
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER5_COMPC_vect) //cppcheck-suppress misra-c2012-8.2
#else
static inline void fuelSchedule7Interrupt() //Most ARM chips can simply call a function
#endif
{
  if (fuelSchedule7.Status == PENDING) //Check to see if this schedule is turn on
  {
    //fuelSchedule7.StartCallback();
    inj7StartFunction();
    fuelSchedule7.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
    SET_COMPARE(FUEL7_COMPARE, FUEL7_COUNTER + uS_TO_TIMER_COMPARE(fuelSchedule7.duration) ); //Doing this here prevents a potential overflow on restarts
  }
  else if (fuelSchedule7.Status == RUNNING)
  {
     //fuelSchedule7.EndCallback();
     inj7EndFunction();
     fuelSchedule7.Status = OFF; //Turn off the schedule
     fuelSchedule7.schedulesSet = 0;

     //If there is a next schedule queued up, activate it
     if(fuelSchedule7.hasNextSchedule == true)
     {
       SET_COMPARE(FUEL7_COMPARE, fuelSchedule7.nextStartCompare);
       fuelSchedule7.endCompare = fuelSchedule7.nextEndCompare;
       fuelSchedule7.Status = PENDING;
       fuelSchedule7.schedulesSet = 1;
       fuelSchedule7.hasNextSchedule = false;
     }
     else { FUEL7_TIMER_DISABLE(); }
  }
}
#endif

#if (INJ_CHANNELS >= 8)
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER5_COMPB_vect) //cppcheck-suppress misra-c2012-8.2
#else
static inline void fuelSchedule8Interrupt() //Most ARM chips can simply call a function
#endif
{
  if (fuelSchedule8.Status == PENDING) //Check to see if this schedule is turn on
  {
    //fuelSchedule8.StartCallback();
    inj8StartFunction();
    fuelSchedule8.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
    SET_COMPARE(FUEL8_COMPARE, FUEL8_COUNTER + uS_TO_TIMER_COMPARE(fuelSchedule8.duration) ); //Doing this here prevents a potential overflow on restarts
  }
  else if (fuelSchedule8.Status == RUNNING)
  {
     //fuelSchedule8.EndCallback();
     inj8EndFunction();
     fuelSchedule8.Status = OFF; //Turn off the schedule
     fuelSchedule8.schedulesSet = 0;

     //If there is a next schedule queued up, activate it
     if(fuelSchedule8.hasNextSchedule == true)
     {
       SET_COMPARE(FUEL8_COMPARE, fuelSchedule8.nextStartCompare);
       fuelSchedule8.endCompare = fuelSchedule8.nextEndCompare;
       fuelSchedule8.Status = PENDING;
       fuelSchedule8.schedulesSet = 1;
       fuelSchedule8.hasNextSchedule = false;
     }
     else { FUEL8_TIMER_DISABLE(); }
  }
}
#endif

#if IGN_CHANNELS >= 1
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER5_COMPA_vect) //cppcheck-suppress misra-c2012-8.2
#else
static inline void ignitionSchedule1Interrupt(void) //Most ARM chips can simply call a function
#endif
  {
    if (ignitionSchedule1.Status == PENDING) //Check to see if this schedule is turn on
    {
      ignitionSchedule1.StartCallback();
      ignitionSchedule1.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      ignitionSchedule1.startTime = micros();
      if(ignitionSchedule1.endScheduleSetByDecoder == true) { SET_COMPARE(IGN1_COMPARE, ignitionSchedule1.endCompare); }
      else { SET_COMPARE(IGN1_COMPARE, IGN1_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule1.duration) ); } //Doing this here prevents a potential overflow on restarts
    }
    else if (ignitionSchedule1.Status == RUNNING)
    {
      ignitionSchedule1.EndCallback();
      ignitionSchedule1.Status = OFF; //Turn off the schedule
      ignitionSchedule1.schedulesSet = 0;
      ignitionSchedule1.endScheduleSetByDecoder = false;
      ignitionCount += 1; //Increment the ignition counter

      //If there is a next schedule queued up, activate it
      if(ignitionSchedule1.hasNextSchedule == true)
      {
        SET_COMPARE(IGN1_COMPARE, ignitionSchedule1.nextStartCompare);
        ignitionSchedule1.Status = PENDING;
        ignitionSchedule1.schedulesSet = 1;
        ignitionSchedule1.hasNextSchedule = false;
      }
      else{ IGN1_TIMER_DISABLE(); }
    }
    else if (ignitionSchedule1.Status == OFF)
    {
      //Catch any spurious interrupts. This really shouldn't ever be called, but there as a safety
      IGN1_TIMER_DISABLE();
    }
  }
#endif

#if IGN_CHANNELS >= 2
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER5_COMPB_vect) //cppcheck-suppress misra-c2012-8.2
#else
static inline void ignitionSchedule2Interrupt(void) //Most ARM chips can simply call a function
#endif
  {
    if (ignitionSchedule2.Status == PENDING) //Check to see if this schedule is turn on
    {
      ignitionSchedule2.StartCallback();
      ignitionSchedule2.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      ignitionSchedule2.startTime = micros();
      if(ignitionSchedule2.endScheduleSetByDecoder == true) { SET_COMPARE(IGN2_COMPARE, ignitionSchedule2.endCompare); } //If the decoder has set the end compare value, assign it to the next compare
      else { SET_COMPARE(IGN2_COMPARE, IGN2_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule2.duration) ); } //If the decoder based timing isn't set, doing this here prevents a potential overflow that can occur at low RPMs
    }
    else if (ignitionSchedule2.Status == RUNNING)
    {
      ignitionSchedule2.Status = OFF; //Turn off the schedule
      ignitionSchedule2.EndCallback();
      ignitionSchedule2.schedulesSet = 0;
      ignitionSchedule2.endScheduleSetByDecoder = false;
      ignitionCount += 1; //Increment the ignition counter
      
      //If there is a next schedule queued up, activate it
      if(ignitionSchedule2.hasNextSchedule == true)
      {
        SET_COMPARE(IGN2_COMPARE, ignitionSchedule2.nextStartCompare);
        ignitionSchedule2.Status = PENDING;
        ignitionSchedule2.schedulesSet = 1;
        ignitionSchedule2.hasNextSchedule = false;
      }
      else{ IGN2_TIMER_DISABLE(); }
    }
    else if (ignitionSchedule2.Status == OFF)
    {
      //Catch any spurious interrupts. This really shouldn't ever be called, but there as a safety
      IGN2_TIMER_DISABLE();
    }
  }
#endif

#if IGN_CHANNELS >= 3
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER5_COMPC_vect) //cppcheck-suppress misra-c2012-8.2
#else
static inline void ignitionSchedule3Interrupt(void) //Most ARM chips can simply call a function
#endif
  {
    if (ignitionSchedule3.Status == PENDING) //Check to see if this schedule is turn on
    {
      ignitionSchedule3.StartCallback();
      ignitionSchedule3.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      ignitionSchedule3.startTime = micros();
      if(ignitionSchedule3.endScheduleSetByDecoder == true) { SET_COMPARE(IGN3_COMPARE, ignitionSchedule3.endCompare ); } //If the decoder has set the end compare value, assign it to the next compare
      else { SET_COMPARE(IGN3_COMPARE, IGN3_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule3.duration)); } //If the decoder based timing isn't set, doing this here prevents a potential overflow that can occur at low RPMs
    }
    else if (ignitionSchedule3.Status == RUNNING)
    {
       ignitionSchedule3.Status = OFF; //Turn off the schedule
       ignitionSchedule3.EndCallback();
       ignitionSchedule3.schedulesSet = 0;
       ignitionSchedule3.endScheduleSetByDecoder = false;
       ignitionCount += 1; //Increment the ignition counter

       //If there is a next schedule queued up, activate it
       if(ignitionSchedule3.hasNextSchedule == true)
       {
         SET_COMPARE(IGN3_COMPARE, ignitionSchedule3.nextStartCompare);
         ignitionSchedule3.Status = PENDING;
         ignitionSchedule3.schedulesSet = 1;
         ignitionSchedule3.hasNextSchedule = false;
       }
       else { IGN3_TIMER_DISABLE(); }
    }
    else if (ignitionSchedule3.Status == OFF)
    {
      //Catch any spurious interrupts. This really shouldn't ever be called, but there as a safety
      IGN3_TIMER_DISABLE();
    }
  }
#endif

#if IGN_CHANNELS >= 4
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER4_COMPA_vect) //cppcheck-suppress misra-c2012-8.2
#else
static inline void ignitionSchedule4Interrupt(void) //Most ARM chips can simply call a function
#endif
  {
    if (ignitionSchedule4.Status == PENDING) //Check to see if this schedule is turn on
    {
      ignitionSchedule4.StartCallback();
      ignitionSchedule4.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      ignitionSchedule4.startTime = micros();
      if(ignitionSchedule4.endScheduleSetByDecoder == true) { SET_COMPARE(IGN4_COMPARE, ignitionSchedule4.endCompare); } //If the decoder has set the end compare value, assign it to the next compare
      else { SET_COMPARE(IGN4_COMPARE, IGN4_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule4.duration) ); } //If the decoder based timing isn't set, doing this here prevents a potential overflow that can occur at low RPMs
    }
    else if (ignitionSchedule4.Status == RUNNING)
    {
       ignitionSchedule4.Status = OFF; //Turn off the schedule
       ignitionSchedule4.EndCallback();
       ignitionSchedule4.schedulesSet = 0;
       ignitionSchedule4.endScheduleSetByDecoder = false;
       ignitionCount += 1; //Increment the ignition counter

       //If there is a next schedule queued up, activate it
       if(ignitionSchedule4.hasNextSchedule == true)
       {
         SET_COMPARE(IGN4_COMPARE, ignitionSchedule4.nextStartCompare);
         ignitionSchedule4.Status = PENDING;
         ignitionSchedule4.schedulesSet = 1;
         ignitionSchedule4.hasNextSchedule = false;
       }
       else { IGN4_TIMER_DISABLE(); }
    }
    else if (ignitionSchedule4.Status == OFF)
    {
      //Catch any spurious interrupts. This really shouldn't ever be called, but there as a safety
      IGN4_TIMER_DISABLE();
    }
  }
#endif

#if IGN_CHANNELS >= 5
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER4_COMPC_vect) //cppcheck-suppress misra-c2012-8.2
#else
static inline void ignitionSchedule5Interrupt(void) //Most ARM chips can simply call a function
#endif
  {
    if (ignitionSchedule5.Status == PENDING) //Check to see if this schedule is turn on
    {
      ignitionSchedule5.StartCallback();
      ignitionSchedule5.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      ignitionSchedule5.startTime = micros();
      if(ignitionSchedule5.endScheduleSetByDecoder == true) { SET_COMPARE(IGN5_COMPARE, ignitionSchedule5.endCompare); } //If the decoder has set the end compare value, assign it to the next compare
      else { SET_COMPARE(IGN5_COMPARE, IGN5_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule5.duration) ); } //If the decoder based timing isn't set, doing this here prevents a potential overflow that can occur at low RPMs
    }
    else if (ignitionSchedule5.Status == RUNNING)
    {
      ignitionSchedule5.Status = OFF; //Turn off the schedule
      ignitionSchedule5.EndCallback();
      ignitionSchedule5.schedulesSet = 0;
      ignitionSchedule5.endScheduleSetByDecoder = false;
      ignitionCount += 1; //Increment the ignition counter

      //If there is a next schedule queued up, activate it
      if(ignitionSchedule5.hasNextSchedule == true)
      {
        SET_COMPARE(IGN5_COMPARE, ignitionSchedule5.nextStartCompare);
        ignitionSchedule5.Status = PENDING;
        ignitionSchedule5.schedulesSet = 1;
        ignitionSchedule5.hasNextSchedule = false;
      }
      else{ IGN5_TIMER_DISABLE(); }
    }
    else if (ignitionSchedule5.Status == OFF)
    {
      //Catch any spurious interrupts. This really shouldn't ever be called, but there as a safety
      IGN5_TIMER_DISABLE();
    }
  }
#endif

#if IGN_CHANNELS >= 6
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER4_COMPB_vect) //cppcheck-suppress misra-c2012-8.2
#else
static inline void ignitionSchedule6Interrupt(void) //Most ARM chips can simply call a function
#endif
  {
    if (ignitionSchedule6.Status == PENDING) //Check to see if this schedule is turn on
    {
      ignitionSchedule6.StartCallback();
      ignitionSchedule6.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      ignitionSchedule6.startTime = micros();
      if(ignitionSchedule6.endScheduleSetByDecoder == true) { SET_COMPARE(IGN6_COMPARE, ignitionSchedule6.endCompare); } //If the decoder has set the end compare value, assign it to the next compare
      else { SET_COMPARE(IGN6_COMPARE, IGN6_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule6.duration) ); } //If the decoder based timing isn't set, doing this here prevents a potential overflow that can occur at low RPMs
    }
    else if (ignitionSchedule6.Status == RUNNING)
    {
      ignitionSchedule6.Status = OFF; //Turn off the schedule
      ignitionSchedule6.EndCallback();
      ignitionSchedule6.schedulesSet = 0;
      ignitionSchedule6.endScheduleSetByDecoder = false;
      ignitionCount += 1; //Increment the ignition counter

      //If there is a next schedule queued up, activate it
      if(ignitionSchedule6.hasNextSchedule == true)
      {
        SET_COMPARE(IGN6_COMPARE, ignitionSchedule6.nextStartCompare);
        ignitionSchedule6.Status = PENDING;
        ignitionSchedule6.schedulesSet = 1;
        ignitionSchedule6.hasNextSchedule = false;
      }
      else{ IGN6_TIMER_DISABLE(); }
    }
    else if (ignitionSchedule6.Status == OFF)
    {
      //Catch any spurious interrupts. This really shouldn't ever be called, but there as a safety
      IGN6_TIMER_DISABLE();
    }
  }
#endif

#if IGN_CHANNELS >= 7
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER3_COMPC_vect) //cppcheck-suppress misra-c2012-8.2
#else
static inline void ignitionSchedule7Interrupt(void) //Most ARM chips can simply call a function
#endif
  {
    if (ignitionSchedule7.Status == PENDING) //Check to see if this schedule is turn on
    {
      ignitionSchedule7.StartCallback();
      ignitionSchedule7.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      ignitionSchedule7.startTime = micros();
      if(ignitionSchedule7.endScheduleSetByDecoder == true) { SET_COMPARE(IGN7_COMPARE, ignitionSchedule7.endCompare); } //If the decoder has set the end compare value, assign it to the next compare
      else { SET_COMPARE(IGN7_COMPARE, IGN7_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule7.duration) ); } //If the decoder based timing isn't set, doing this here prevents a potential overflow that can occur at low RPMs
    }
    else if (ignitionSchedule7.Status == RUNNING)
    {
      ignitionSchedule7.Status = OFF; //Turn off the schedule
      ignitionSchedule7.EndCallback();
      ignitionSchedule7.schedulesSet = 0;
      ignitionSchedule7.endScheduleSetByDecoder = false;
      ignitionCount += 1; //Increment the ignition counter

      //If there is a next schedule queued up, activate it
      if(ignitionSchedule7.hasNextSchedule == true)
      {
        SET_COMPARE(IGN7_COMPARE, ignitionSchedule7.nextStartCompare);
        ignitionSchedule7.Status = PENDING;
        ignitionSchedule7.schedulesSet = 1;
        ignitionSchedule7.hasNextSchedule = false;
      }
      else{ IGN7_TIMER_DISABLE(); }
    }
    else if (ignitionSchedule7.Status == OFF)
    {
      //Catch any spurious interrupts. This really shouldn't ever be called, but there as a safety
      IGN7_TIMER_DISABLE();
    }
  }
#endif

#if IGN_CHANNELS >= 8
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER3_COMPB_vect) //cppcheck-suppress misra-c2012-8.2
#else
static inline void ignitionSchedule8Interrupt(void) //Most ARM chips can simply call a function
#endif
  {
    if (ignitionSchedule8.Status == PENDING) //Check to see if this schedule is turn on
    {
      ignitionSchedule8.StartCallback();
      ignitionSchedule8.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      ignitionSchedule8.startTime = micros();
      if(ignitionSchedule8.endScheduleSetByDecoder == true) { SET_COMPARE(IGN8_COMPARE, ignitionSchedule8.endCompare); } //If the decoder has set the end compare value, assign it to the next compare
      else { SET_COMPARE(IGN8_COMPARE, IGN8_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule8.duration) ); } //If the decoder based timing isn't set, doing this here prevents a potential overflow that can occur at low RPMs
    }
    else if (ignitionSchedule8.Status == RUNNING)
    {
      ignitionSchedule8.Status = OFF; //Turn off the schedule
      ignitionSchedule8.EndCallback();
      ignitionSchedule8.schedulesSet = 0;
      ignitionSchedule8.endScheduleSetByDecoder = false;
      ignitionCount += 1; //Increment the ignition counter

      //If there is a next schedule queued up, activate it
      if(ignitionSchedule8.hasNextSchedule == true)
      {
        SET_COMPARE(IGN8_COMPARE, ignitionSchedule8.nextStartCompare);
        ignitionSchedule8.Status = PENDING;
        ignitionSchedule8.schedulesSet = 1;
        ignitionSchedule8.hasNextSchedule = false;
      }
      else{ IGN8_TIMER_DISABLE(); }
    }
    else if (ignitionSchedule8.Status == OFF)
    {
      //Catch any spurious interrupts. This really shouldn't ever be called, but there as a safety
      IGN8_TIMER_DISABLE();
    }
  }
#endif
