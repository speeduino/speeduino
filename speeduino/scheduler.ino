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

    ignitionSchedule1.ignRptCnt = 0;
    ignitionSchedule2.ignRptCnt = 0;
    ignitionSchedule3.ignRptCnt = 0;
    ignitionSchedule4.ignRptCnt = 0;
    ignitionSchedule5.ignRptCnt = 0;
    ignitionSchedule6.ignRptCnt = 0;
    ignitionSchedule7.ignRptCnt = 0;
    ignitionSchedule8.ignRptCnt = 0;
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

//Experimental new generic function. This is NOT yet ready and functional
void setFuelSchedule(struct Schedule *targetSchedule, unsigned long timeout, unsigned long duration)
{
  if(targetSchedule->Status != RUNNING) //Check that we're not already part way through a schedule
  {
    targetSchedule->duration = duration;

    //Need to check that the timeout doesn't exceed the overflow
    COMPARE_TYPE timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >16x (Each tick represents 16uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when appliedcausing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

    //The following must be enclosed in the noInterupts block to avoid contention caused if the relevant interrupt fires before the state is fully set
    noInterrupts();
    //targetSchedule->startCompare = *targetSchedule->counter + timeout_timer_compare;
    targetSchedule->startCompare = FUEL1_COUNTER + timeout_timer_compare; //Insert correct counter HERE!
    targetSchedule->endCompare = targetSchedule->startCompare + uS_TO_TIMER_COMPARE(duration);
    targetSchedule->Status = PENDING; //Turn this schedule on
    targetSchedule->schedulesSet++; //Increment the number of times this schedule has been set

    //*targetSchedule->compare = targetSchedule->startCompare;
    SET_COMPARE(FUEL1_COMPARE, targetSchedule->startCompare); //Insert corrector compare HERE!
    interrupts();
    FUEL1_TIMER_ENABLE();
  }
  else
  {
    //If the schedule is already running, we can set the next schedule so it is ready to go
    //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
    //targetSchedule->nextStartCompare = *targetSchedule->counter + uS_TO_TIMER_COMPARE(timeout);
    targetSchedule->nextEndCompare = targetSchedule->nextStartCompare + uS_TO_TIMER_COMPARE(duration);
    BIT_SET(targetSchedule->scheduleFlags, BIT_SCHEDULE_NEXT);
  }
}


//void setFuelSchedule1(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
void setFuelSchedule1(unsigned long timeout, unsigned long duration) //Uses timer 3 compare A
{
  //Check whether timeout exceeds the maximum future time. This can potentially occur on sequential setups when below ~115rpm
  //if(timeout < MAX_TIMER_PERIOD)
  {
    if(fuelSchedule1.Status != RUNNING) //Check that we're not already part way through a schedule
    {
      //Need to check that the timeout doesn't exceed the overflow
      if ((timeout+duration) < MAX_TIMER_PERIOD)
      {
        fuelSchedule1.duration = duration;
        //The following must be enclosed in the noInterupts block to avoid contention caused if the relevant interrupt fires before the state is fully set
        noInterrupts();
        fuelSchedule1.startCompare = FUEL1_COUNTER + uS_TO_TIMER_COMPARE(timeout);
        fuelSchedule1.endCompare = fuelSchedule1.startCompare + uS_TO_TIMER_COMPARE(duration);
        fuelSchedule1.Status = PENDING; //Turn this schedule on
        fuelSchedule1.schedulesSet++; //Increment the number of times this schedule has been set
        //Schedule 1 shares a timer with schedule 5
        //if(channel5InjEnabled) { SET_COMPARE(FUEL1_COMPARE, setQueue(timer3Aqueue, &fuelSchedule1, &fuelSchedule5, FUEL1_COUNTER) ); }
        //else { timer3Aqueue[0] = &fuelSchedule1; timer3Aqueue[1] = &fuelSchedule1; timer3Aqueue[2] = &fuelSchedule1; timer3Aqueue[3] = &fuelSchedule1; SET_COMPARE(FUEL1_COMPARE, fuelSchedule1.startCompare); }
        //timer3Aqueue[0] = &fuelSchedule1; timer3Aqueue[1] = &fuelSchedule1; timer3Aqueue[2] = &fuelSchedule1; timer3Aqueue[3] = &fuelSchedule1;
        SET_COMPARE(FUEL1_COMPARE, fuelSchedule1.startCompare);
        interrupts();
        FUEL1_TIMER_ENABLE();
      }
    }
    else
    {
      //If the schedule is already running, we can set the next schedule so it is ready to go
      //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
      if( (timeout+duration) < MAX_TIMER_PERIOD )
      {
        noInterrupts();
        fuelSchedule1.nextStartCompare = FUEL1_COUNTER + uS_TO_TIMER_COMPARE(timeout);
        fuelSchedule1.nextEndCompare = fuelSchedule1.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
        fuelSchedule1.duration = duration;
        fuelSchedule1.hasNextSchedule = true;
        interrupts();
      }
    } //Schedule is RUNNING
  } //Timeout less than threshold
}

void setFuelSchedule2(unsigned long timeout, unsigned long duration) //Uses timer 3 compare B
{
  //Check whether timeout exceeds the maximum future time. This can potentially occur on sequential setups when below ~115rpm
  if(timeout < MAX_TIMER_PERIOD)
  {
    if(fuelSchedule2.Status != RUNNING) //Check that we're not already part way through a schedule
    {
      //Callbacks no longer used, but retained for now:
      //fuelSchedule2.StartCallback = startCallback;
      //fuelSchedule2.EndCallback = endCallback;
      fuelSchedule2.duration = duration;

      //Need to check that the timeout doesn't exceed the overflow
      COMPARE_TYPE timeout_timer_compare;
      if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
      else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

      //The following must be enclosed in the noInterupts block to avoid contention caused if the relevant interrupt fires before the state is fully set
      noInterrupts();
      fuelSchedule2.startCompare = FUEL2_COUNTER + timeout_timer_compare;
      fuelSchedule2.endCompare = fuelSchedule2.startCompare + uS_TO_TIMER_COMPARE(duration);
      SET_COMPARE(FUEL2_COMPARE, fuelSchedule2.startCompare); //Use the B compare unit of timer 3
      fuelSchedule2.Status = PENDING; //Turn this schedule on
      fuelSchedule2.schedulesSet++; //Increment the number of times this schedule has been set
      interrupts();
      FUEL2_TIMER_ENABLE();
    }
    else
    {
      //If the schedule is already running, we can set the next schedule so it is ready to go
      //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
      fuelSchedule2.nextStartCompare = FUEL2_COUNTER + uS_TO_TIMER_COMPARE(timeout);
      fuelSchedule2.nextEndCompare = fuelSchedule2.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
      fuelSchedule2.hasNextSchedule = true;
    }
  }
}
//void setFuelSchedule3(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
void setFuelSchedule3(unsigned long timeout, unsigned long duration) //Uses timer 3 compare C
{
  //Check whether timeout exceeds the maximum future time. This can potentially occur on sequential setups when below ~115rpm
  if(timeout < MAX_TIMER_PERIOD)
  {
    if(fuelSchedule3.Status != RUNNING) //Check that we're not already part way through a schedule
    {
      //Callbacks no longer used, but retained for now:
      //fuelSchedule3.StartCallback = startCallback;
      //fuelSchedule3.EndCallback = endCallback;
      fuelSchedule3.duration = duration;

      //Need to check that the timeout doesn't exceed the overflow
      COMPARE_TYPE timeout_timer_compare;
      if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
      else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

      //The following must be enclosed in the noInterupts block to avoid contention caused if the relevant interrupt fires before the state is fully set
      noInterrupts();
      fuelSchedule3.startCompare = FUEL3_COUNTER + timeout_timer_compare;
      fuelSchedule3.endCompare = fuelSchedule3.startCompare + uS_TO_TIMER_COMPARE(duration);
      SET_COMPARE(FUEL3_COMPARE, fuelSchedule3.startCompare); //Use the C compare unit of timer 3
      fuelSchedule3.Status = PENDING; //Turn this schedule on
      fuelSchedule3.schedulesSet++; //Increment the number of times this schedule has been set
      interrupts();
      FUEL3_TIMER_ENABLE();
    }
    else
    {
      //If the schedule is already running, we can set the next schedule so it is ready to go
      //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
      fuelSchedule3.nextStartCompare = FUEL3_COUNTER + uS_TO_TIMER_COMPARE(timeout);
      fuelSchedule3.nextEndCompare = fuelSchedule3.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
      fuelSchedule3.hasNextSchedule = true;
    }
  }
}
//void setFuelSchedule4(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
void setFuelSchedule4(unsigned long timeout, unsigned long duration) //Uses timer 4 compare B
{
  //Check whether timeout exceeds the maximum future time. This can potentially occur on sequential setups when below ~115rpm
  if(timeout < MAX_TIMER_PERIOD)
  {
    if(fuelSchedule4.Status != RUNNING) //Check that we're not already part way through a schedule
    {
      //Callbacks no longer used, but retained for now:
      //fuelSchedule4.StartCallback = startCallback;
      //fuelSchedule4.EndCallback = endCallback;
      fuelSchedule4.duration = duration;

      //Need to check that the timeout doesn't exceed the overflow
      COMPARE_TYPE timeout_timer_compare;
      if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
      else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

      //The following must be enclosed in the noInterupts block to avoid contention caused if the relevant interrupt fires before the state is fully set
      noInterrupts();
      fuelSchedule4.startCompare = FUEL4_COUNTER + timeout_timer_compare;
      fuelSchedule4.endCompare = fuelSchedule4.startCompare + uS_TO_TIMER_COMPARE(duration);
      SET_COMPARE(FUEL4_COMPARE, fuelSchedule4.startCompare); //Use the B compare unit of timer 4
      fuelSchedule4.Status = PENDING; //Turn this schedule on
      fuelSchedule4.schedulesSet++; //Increment the number of times this schedule has been set
      interrupts();
      FUEL4_TIMER_ENABLE();
    }
    else
    {
      //If the schedule is already running, we can set the next schedule so it is ready to go
      //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
      fuelSchedule4.nextStartCompare = FUEL4_COUNTER + uS_TO_TIMER_COMPARE(timeout);
      fuelSchedule4.nextEndCompare = fuelSchedule4.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
      fuelSchedule4.hasNextSchedule = true;
    }
  }
}

#if INJ_CHANNELS >= 5
void setFuelSchedule5(unsigned long timeout, unsigned long duration) //Uses timer 4 compare C
{
  //Check whether timeout exceeds the maximum future time. This can potentially occur on sequential setups when below ~115rpm
  if(timeout < MAX_TIMER_PERIOD)
  {
    if(fuelSchedule5.Status != RUNNING) //Check that we're not already part way through a schedule
    {
      fuelSchedule5.duration = duration;

      //Need to check that the timeout doesn't exceed the overflow
      COMPARE_TYPE timeout_timer_compare;
      if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
      else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

      //The following must be enclosed in the noInterupts block to avoid contention caused if the relevant interrupt fires before the state is fully set
      noInterrupts();
      fuelSchedule5.startCompare = FUEL5_COUNTER + timeout_timer_compare;
      fuelSchedule5.endCompare = fuelSchedule5.startCompare + uS_TO_TIMER_COMPARE(duration);
      SET_COMPARE(FUEL5_COMPARE, fuelSchedule5.startCompare); //Use the C compare unit of timer 4
      fuelSchedule5.Status = PENDING; //Turn this schedule on
      fuelSchedule5.schedulesSet++; //Increment the number of times this schedule has been set
      interrupts();
      FUEL5_TIMER_ENABLE();
    }
    else
    {
      //If the schedule is already running, we can set the next schedule so it is ready to go
      //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
      fuelSchedule5.nextStartCompare = FUEL5_COUNTER + uS_TO_TIMER_COMPARE(timeout);
      fuelSchedule5.nextEndCompare = fuelSchedule5.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
      fuelSchedule5.hasNextSchedule = true;
    }
  }
}
#endif

#if INJ_CHANNELS >= 6
void setFuelSchedule6(unsigned long timeout, unsigned long duration) //Uses timer 4 compare A
{
  //Check whether timeout exceeds the maximum future time. This can potentially occur on sequential setups when below ~115rpm
  if(timeout < MAX_TIMER_PERIOD)
  {
    if(fuelSchedule6.Status != RUNNING) //Check that we're not already part way through a schedule
    {
      fuelSchedule6.duration = duration;

      //Need to check that the timeout doesn't exceed the overflow
      COMPARE_TYPE timeout_timer_compare;
      if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
      else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

      //The following must be enclosed in the noInterupts block to avoid contention caused if the relevant interrupt fires before the state is fully set
      noInterrupts();
      fuelSchedule6.startCompare = FUEL6_COUNTER + timeout_timer_compare;
      fuelSchedule6.endCompare = fuelSchedule6.startCompare + uS_TO_TIMER_COMPARE(duration);
      SET_COMPARE(FUEL6_COMPARE, fuelSchedule6.startCompare); //Use the A compare unit of timer 4
      fuelSchedule6.Status = PENDING; //Turn this schedule on
      fuelSchedule6.schedulesSet++; //Increment the number of times this schedule has been set
      interrupts();
      FUEL6_TIMER_ENABLE();
    }
    else
    {
      //If the schedule is already running, we can set the next schedule so it is ready to go
      //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
      fuelSchedule6.nextStartCompare = FUEL6_COUNTER + uS_TO_TIMER_COMPARE(timeout);
      fuelSchedule6.nextEndCompare = fuelSchedule6.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
      fuelSchedule6.hasNextSchedule = true;
    }
  }
}
#endif

#if INJ_CHANNELS >= 7
void setFuelSchedule7(unsigned long timeout, unsigned long duration) //Uses timer 5 compare C
{
  //Check whether timeout exceeds the maximum future time. This can potentially occur on sequential setups when below ~115rpm
  if(timeout < MAX_TIMER_PERIOD)
  {
    if(fuelSchedule7.Status != RUNNING) //Check that we're not already part way through a schedule
    {
      fuelSchedule7.duration = duration;

      //Need to check that the timeout doesn't exceed the overflow
      COMPARE_TYPE timeout_timer_compare;
      if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
      else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

      //The following must be enclosed in the noInterupts block to avoid contention caused if the relevant interrupt fires before the state is fully set
      noInterrupts();
      fuelSchedule7.startCompare = FUEL7_COUNTER + timeout_timer_compare;
      fuelSchedule7.endCompare = fuelSchedule7.startCompare + uS_TO_TIMER_COMPARE(duration);
      SET_COMPARE(FUEL7_COMPARE, fuelSchedule7.startCompare); //Use the C compare unit of timer 5
      fuelSchedule7.Status = PENDING; //Turn this schedule on
      fuelSchedule7.schedulesSet++; //Increment the number of times this schedule has been set
      interrupts();
      FUEL7_TIMER_ENABLE();
    }
    else
    {
      //If the schedule is already running, we can set the next schedule so it is ready to go
      //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
      fuelSchedule7.nextStartCompare = FUEL7_COUNTER + uS_TO_TIMER_COMPARE(timeout);
      fuelSchedule7.nextEndCompare = fuelSchedule7.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
      fuelSchedule7.hasNextSchedule = true;
    }
  }
}
#endif

#if INJ_CHANNELS >= 8
void setFuelSchedule8(unsigned long timeout, unsigned long duration) //Uses timer 5 compare B
{
  //Check whether timeout exceeds the maximum future time. This can potentially occur on sequential setups when below ~115rpm
  if(timeout < MAX_TIMER_PERIOD)
  {
    if(fuelSchedule8.Status != RUNNING) //Check that we're not already part way through a schedule
    {
      fuelSchedule8.duration = duration;

      //Need to check that the timeout doesn't exceed the overflow
      COMPARE_TYPE timeout_timer_compare;
      if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
      else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

      //The following must be enclosed in the noInterupts block to avoid contention caused if the relevant interrupt fires before the state is fully set
      noInterrupts();
      fuelSchedule8.startCompare = FUEL8_COUNTER + timeout_timer_compare;
      fuelSchedule8.endCompare = fuelSchedule8.startCompare + uS_TO_TIMER_COMPARE(duration);
      SET_COMPARE(FUEL8_COMPARE, fuelSchedule8.startCompare); //Use the B compare unit of timer 5
      fuelSchedule8.Status = PENDING; //Turn this schedule on
      fuelSchedule8.schedulesSet++; //Increment the number of times this schedule has been set
      interrupts();
      FUEL8_TIMER_ENABLE();
    }
    else
    {
      //If the schedule is already running, we can set the next schedule so it is ready to go
      //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
      fuelSchedule8.nextStartCompare = FUEL8_COUNTER + uS_TO_TIMER_COMPARE(timeout);
      fuelSchedule8.nextEndCompare = fuelSchedule8.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
      fuelSchedule8.hasNextSchedule = true;
    }
  }
}
#endif

//Ignition schedulers use Timer 5
void setIgnitionSchedule1(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  if(ignitionSchedule1.Status != RUNNING) //Check that we're not already part way through a schedule
  {
    ignitionSchedule1.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule1.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule1.duration = duration;

    //Need to check that the timeout doesn't exceed the overflow
    COMPARE_TYPE timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case
    
    if (ignitionSchedule1.ignRptCnt == 0)
    {
      noInterrupts();
      ignitionSchedule1.startCompare = IGN1_COUNTER + timeout_timer_compare; //As there is a tick every 4uS, there are timeout/4 ticks until the interrupt should be triggered ( >>2 divides by 4)
      if(!BIT_CHECK(ignitionSchedule1.scheduleFlags, BIT_SCHEDULE_DECODER)) { ignitionSchedule1.endCompare = ignitionSchedule1.startCompare + uS_TO_TIMER_COMPARE(duration); } //The .endCompare value is also set by the per tooth timing in decoders.ino. The check here is so that it's not getting overriden.
      SET_COMPARE(IGN1_COMPARE, ignitionSchedule1.startCompare);
      ignitionSchedule1.Status = PENDING; //Turn this schedule on
      ignitionSchedule1.schedulesSet++;
      interrupts();
    }
    IGN1_TIMER_ENABLE();
  }
  else
  {
    //If the schedule is already running, we can set the next schedule so it is ready to go
    //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
    if (timeout < MAX_TIMER_PERIOD)
    {
      ignitionSchedule1.nextStartCompare = IGN1_COUNTER + uS_TO_TIMER_COMPARE(timeout);
      ignitionSchedule1.nextEndCompare = ignitionSchedule1.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
      BIT_SET(ignitionSchedule1.scheduleFlags, BIT_SCHEDULE_NEXT);
    }
  }
  if ((configPage9.crankIgnOutRpt == 1) && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && (ignitionSchedule1.ignRptCnt <= 3))
  {
    if (ignitionSchedule1.ignRptCnt >=1)
    { ignitionSchedule1.repeatStartCompare = IGN1_COUNTER + uS_TO_TIMER_COMPARE(((((uint32_t)configPage4.dwellCrank * 100) * configPage9.ignRptScale) / 100) + (configPage4.sparkDur * 100)); }
    else
    { ignitionSchedule1.repeatStartCompare = IGN1_COUNTER + uS_TO_TIMER_COMPARE(duration + (configPage4.sparkDur * 100)); }
    ignitionSchedule1.repeatEndCompare = ignitionSchedule1.repeatStartCompare + uS_TO_TIMER_COMPARE(((((uint32_t)configPage4.dwellCrank * 100) * configPage9.ignRptScale) / 100));
  }
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
  if(ignitionSchedule2.Status != RUNNING) //Check that we're not already part way through a schedule
  {
    ignitionSchedule2.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule2.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule2.duration = duration;

    //Need to check that the timeout doesn't exceed the overflow
    COMPARE_TYPE timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

    if (ignitionSchedule2.ignRptCnt == 0)
    {
      noInterrupts();
      ignitionSchedule2.startCompare = IGN2_COUNTER + timeout_timer_compare; //As there is a tick every 4uS, there are timeout/4 ticks until the interrupt should be triggered ( >>2 divides by 4)
      if(!BIT_CHECK(ignitionSchedule2.scheduleFlags, BIT_SCHEDULE_DECODER)) { ignitionSchedule2.endCompare = ignitionSchedule2.startCompare + uS_TO_TIMER_COMPARE(duration); } //The .endCompare value is also set by the per tooth timing in decoders.ino. The check here is so that it's not getting overriden.
      SET_COMPARE(IGN2_COMPARE, ignitionSchedule2.startCompare);
      ignitionSchedule2.Status = PENDING; //Turn this schedule on
      ignitionSchedule2.schedulesSet++;
      interrupts();
    }
    IGN2_TIMER_ENABLE();
  }
  else
  {
    //If the schedule is already running, we can set the next schedule so it is ready to go
    //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
    if (timeout < MAX_TIMER_PERIOD)
    {
      ignitionSchedule2.nextStartCompare = IGN2_COUNTER + uS_TO_TIMER_COMPARE(timeout);
      ignitionSchedule2.nextEndCompare = ignitionSchedule2.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
      BIT_SET(ignitionSchedule2.scheduleFlags, BIT_SCHEDULE_NEXT);
    }
  }
  if ((configPage9.crankIgnOutRpt == 1) && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && (ignitionSchedule2.ignRptCnt <= 3))
  {
    if (ignitionSchedule2.ignRptCnt >=1)
    { ignitionSchedule2.repeatStartCompare = IGN2_COUNTER + uS_TO_TIMER_COMPARE(((((uint32_t)configPage4.dwellCrank * 100) * configPage9.ignRptScale) / 100) + (configPage4.sparkDur * 100)); }
    else
    { ignitionSchedule2.repeatStartCompare = IGN2_COUNTER + uS_TO_TIMER_COMPARE(duration + (configPage4.sparkDur * 100)); }
    ignitionSchedule2.repeatEndCompare = ignitionSchedule2.repeatStartCompare + uS_TO_TIMER_COMPARE(((((uint32_t)configPage4.dwellCrank * 100) * configPage9.ignRptScale) / 100));
  }
}
void setIgnitionSchedule3(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  if(ignitionSchedule3.Status != RUNNING) //Check that we're not already part way through a schedule
  {

    ignitionSchedule3.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule3.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule3.duration = duration;

    //Need to check that the timeout doesn't exceed the overflow
    COMPARE_TYPE timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case
    
    if (ignitionSchedule3.ignRptCnt == 0)
    {
      noInterrupts();
      ignitionSchedule3.startCompare = IGN3_COUNTER + timeout_timer_compare; //As there is a tick every 4uS, there are timeout/4 ticks until the interrupt should be triggered ( >>2 divides by 4)
      if(!BIT_CHECK(ignitionSchedule3.scheduleFlags, BIT_SCHEDULE_DECODER)) { ignitionSchedule3.endCompare = ignitionSchedule3.startCompare + uS_TO_TIMER_COMPARE(duration); } //The .endCompare value is also set by the per tooth timing in decoders.ino. The check here is so that it's not getting overriden.
      SET_COMPARE(IGN3_COMPARE, ignitionSchedule3.startCompare);
      ignitionSchedule3.Status = PENDING; //Turn this schedule on
      ignitionSchedule3.schedulesSet++;
      interrupts();
    }
    IGN3_TIMER_ENABLE();
  }
  else
  {
    //If the schedule is already running, we can set the next schedule so it is ready to go
    //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
    if (timeout < MAX_TIMER_PERIOD)
    {
      ignitionSchedule3.nextStartCompare = IGN3_COUNTER + uS_TO_TIMER_COMPARE(timeout);
      ignitionSchedule3.nextEndCompare = ignitionSchedule3.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
      BIT_SET(ignitionSchedule3.scheduleFlags, BIT_SCHEDULE_NEXT);
    }
  }
  if ((configPage9.crankIgnOutRpt == 1) && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && (ignitionSchedule3.ignRptCnt <= 3))
  {
    if (ignitionSchedule3.ignRptCnt >=1)
    { ignitionSchedule3.repeatStartCompare = IGN3_COUNTER + uS_TO_TIMER_COMPARE(((((uint32_t)configPage4.dwellCrank * 100) * configPage9.ignRptScale) / 100) + (configPage4.sparkDur * 100)); }
    else
    { ignitionSchedule3.repeatStartCompare = IGN3_COUNTER + uS_TO_TIMER_COMPARE(duration + (configPage4.sparkDur * 100)); }
    ignitionSchedule3.repeatEndCompare = ignitionSchedule3.repeatStartCompare + uS_TO_TIMER_COMPARE(((((uint32_t)configPage4.dwellCrank * 100) * configPage9.ignRptScale) / 100));
  }
}
void setIgnitionSchedule4(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  if(ignitionSchedule4.Status != RUNNING) //Check that we're not already part way through a schedule
  {

    ignitionSchedule4.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule4.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule4.duration = duration;

    //Need to check that the timeout doesn't exceed the overflow
    COMPARE_TYPE timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

    if (ignitionSchedule4.ignRptCnt == 0)
    {
      noInterrupts();
      ignitionSchedule4.startCompare = IGN4_COUNTER + timeout_timer_compare;
      //if(ignitionSchedule4.endScheduleSetByDecoder == false) { ignitionSchedule4.endCompare = ignitionSchedule4.startCompare + uS_TO_TIMER_COMPARE(duration); } //The .endCompare value is also set by the per tooth timing in decoders.ino. The check here is so that it's not getting overridden. 
      if(!BIT_CHECK(ignitionSchedule4.scheduleFlags, BIT_SCHEDULE_DECODER)) { ignitionSchedule4.endCompare = ignitionSchedule4.startCompare + uS_TO_TIMER_COMPARE(duration); } //The .endCompare value is also set by the per tooth timing in decoders.ino. The check here is so that it's not getting overriden.
      SET_COMPARE(IGN4_COMPARE, ignitionSchedule4.startCompare);
      ignitionSchedule4.Status = PENDING; //Turn this schedule on
      ignitionSchedule4.schedulesSet++;
      interrupts();
    }
    IGN4_TIMER_ENABLE();
  }
  else
  {
    //If the schedule is already running, we can set the next schedule so it is ready to go
    //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
    if (timeout < MAX_TIMER_PERIOD)
    {
      ignitionSchedule4.nextStartCompare = IGN4_COUNTER + uS_TO_TIMER_COMPARE(timeout);
      ignitionSchedule4.nextEndCompare = ignitionSchedule4.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
      BIT_SET(ignitionSchedule4.scheduleFlags, BIT_SCHEDULE_NEXT);
    }
  }
  if ((configPage9.crankIgnOutRpt == 1) && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && (ignitionSchedule4.ignRptCnt <= 3))
  {
    if (ignitionSchedule4.ignRptCnt >=1)
    { ignitionSchedule4.repeatStartCompare = IGN4_COUNTER + uS_TO_TIMER_COMPARE(((((uint32_t)configPage4.dwellCrank * 100) * configPage9.ignRptScale) / 100) + (configPage4.sparkDur * 100)); }
    else
    { ignitionSchedule4.repeatStartCompare = IGN4_COUNTER + uS_TO_TIMER_COMPARE(duration + (configPage4.sparkDur * 100)); }
    ignitionSchedule4.repeatEndCompare = ignitionSchedule4.repeatStartCompare + uS_TO_TIMER_COMPARE(((((uint32_t)configPage4.dwellCrank * 100) * configPage9.ignRptScale) / 100));
  }
}
#if IGN_CHANNELS >= 5
void setIgnitionSchedule5(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  if(ignitionSchedule5.Status != RUNNING) //Check that we're not already part way through a schedule
  {

    ignitionSchedule5.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule5.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule5.duration = duration;

    //Need to check that the timeout doesn't exceed the overflow
    COMPARE_TYPE timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

    if (ignitionSchedule5.ignRptCnt == 0)
    {
      noInterrupts();
      ignitionSchedule5.startCompare = IGN5_COUNTER + timeout_timer_compare;
      if(!BIT_CHECK(ignitionSchedule5.scheduleFlags, BIT_SCHEDULE_DECODER)) { ignitionSchedule5.endCompare = ignitionSchedule5.startCompare + uS_TO_TIMER_COMPARE(duration); } //The .endCompare value is also set by the per tooth timing in decoders.ino. The check here is so that it's not getting overridden. 
      SET_COMPARE(IGN5_COMPARE, ignitionSchedule5.startCompare);
      ignitionSchedule5.Status = PENDING; //Turn this schedule on
      ignitionSchedule5.schedulesSet++;
      interrupts();
    }
    IGN5_TIMER_ENABLE();
  }
  else
  {
    //If the schedule is already running, we can set the next schedule so it is ready to go
    //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
    if (timeout < MAX_TIMER_PERIOD)
    {
      ignitionSchedule5.nextStartCompare = IGN5_COUNTER + uS_TO_TIMER_COMPARE(timeout);
      ignitionSchedule5.nextEndCompare = ignitionSchedule5.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
      BIT_SET(ignitionSchedule5.scheduleFlags, BIT_SCHEDULE_NEXT);
    }
  }
  if ((configPage9.crankIgnOutRpt == 1) && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && (ignitionSchedule5.ignRptCnt <= 3))
  {
    if (ignitionSchedule5.ignRptCnt >=1)
    { ignitionSchedule5.repeatStartCompare = IGN5_COUNTER + uS_TO_TIMER_COMPARE(((((uint32_t)configPage4.dwellCrank * 100) * configPage9.ignRptScale) / 100) + (configPage4.sparkDur * 100)); }
    else
    { ignitionSchedule5.repeatStartCompare = IGN5_COUNTER + uS_TO_TIMER_COMPARE(duration + (configPage4.sparkDur * 100)); }
    ignitionSchedule5.repeatEndCompare = ignitionSchedule5.repeatStartCompare + uS_TO_TIMER_COMPARE(((((uint32_t)configPage4.dwellCrank * 100) * configPage9.ignRptScale) / 100));
  }
}
#endif
#if IGN_CHANNELS >= 6
void setIgnitionSchedule6(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  if(ignitionSchedule6.Status != RUNNING) //Check that we're not already part way through a schedule
  {
    ignitionSchedule6.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule6.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule6.duration = duration;

    //Need to check that the timeout doesn't exceed the overflow
    COMPARE_TYPE timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

    if (ignitionSchedule6.ignRptCnt == 0)
    {
      noInterrupts();
      ignitionSchedule6.startCompare = IGN6_COUNTER + timeout_timer_compare;
      if(!BIT_CHECK(ignitionSchedule6.scheduleFlags, BIT_SCHEDULE_DECODER)) { ignitionSchedule6.endCompare = ignitionSchedule6.startCompare + uS_TO_TIMER_COMPARE(duration); } //The .endCompare value is also set by the per tooth timing in decoders.ino. The check here is so that it's not getting overridden. 
      SET_COMPARE(IGN6_COMPARE, ignitionSchedule6.startCompare);
      ignitionSchedule6.Status = PENDING; //Turn this schedule on
      ignitionSchedule6.schedulesSet++;
      interrupts();
    }
    IGN6_TIMER_ENABLE();
  }
  else
  {
    //If the schedule is already running, we can set the next schedule so it is ready to go
    //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
    if (timeout < MAX_TIMER_PERIOD)
    {
      ignitionSchedule6.nextStartCompare = IGN6_COUNTER + uS_TO_TIMER_COMPARE(timeout);
      ignitionSchedule6.nextEndCompare = ignitionSchedule6.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
      BIT_SET(ignitionSchedule6.scheduleFlags, BIT_SCHEDULE_NEXT);
    }
  }
  if ((configPage9.crankIgnOutRpt == 1) && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && (ignitionSchedule6.ignRptCnt <= 3))
  {
    if (ignitionSchedule6.ignRptCnt >=1)
    { ignitionSchedule6.repeatStartCompare = IGN6_COUNTER + uS_TO_TIMER_COMPARE(((((uint32_t)configPage4.dwellCrank * 100) * configPage9.ignRptScale) / 100) + (configPage4.sparkDur * 100)); }
    else
    { ignitionSchedule6.repeatStartCompare = IGN6_COUNTER + uS_TO_TIMER_COMPARE(duration + (configPage4.sparkDur * 100)); }
    ignitionSchedule6.repeatEndCompare = ignitionSchedule6.repeatStartCompare + uS_TO_TIMER_COMPARE(((((uint32_t)configPage4.dwellCrank * 100) * configPage9.ignRptScale) / 100));
  }
}
#endif
#if IGN_CHANNELS >= 7
void setIgnitionSchedule7(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  if(ignitionSchedule7.Status != RUNNING) //Check that we're not already part way through a schedule
  {
    ignitionSchedule7.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule7.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule7.duration = duration;

    //Need to check that the timeout doesn't exceed the overflow
    COMPARE_TYPE timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

    if (ignitionSchedule7.ignRptCnt == 0)
    {
      noInterrupts();
      ignitionSchedule7.startCompare = IGN7_COUNTER + timeout_timer_compare;
      if(!BIT_CHECK(ignitionSchedule7.scheduleFlags, BIT_SCHEDULE_DECODER)) { ignitionSchedule7.endCompare = ignitionSchedule7.startCompare + uS_TO_TIMER_COMPARE(duration); } //The .endCompare value is also set by the per tooth timing in decoders.ino. The check here is so that it's not getting overridden. 
      SET_COMPARE(IGN7_COMPARE, ignitionSchedule7.startCompare);
      ignitionSchedule7.Status = PENDING; //Turn this schedule on
      ignitionSchedule7.schedulesSet++;
      interrupts();
    }
    IGN7_TIMER_ENABLE();
  }
  else
  {
    //If the schedule is already running, we can set the next schedule so it is ready to go
    //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
    if (timeout < MAX_TIMER_PERIOD)
    {
      ignitionSchedule7.nextStartCompare = IGN7_COUNTER + uS_TO_TIMER_COMPARE(timeout);
      ignitionSchedule7.nextEndCompare = ignitionSchedule7.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
      BIT_SET(ignitionSchedule7.scheduleFlags, BIT_SCHEDULE_NEXT);
    }
  }
  if ((configPage9.crankIgnOutRpt == 1) && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && (ignitionSchedule7.ignRptCnt <= 3))
  {
    if (ignitionSchedule7.ignRptCnt >=1)
    { ignitionSchedule7.repeatStartCompare = IGN7_COUNTER + uS_TO_TIMER_COMPARE(((((uint32_t)configPage4.dwellCrank * 100) * configPage9.ignRptScale) / 100) + (configPage4.sparkDur * 100)); }
    else
    { ignitionSchedule7.repeatStartCompare = IGN7_COUNTER + uS_TO_TIMER_COMPARE(duration + (configPage4.sparkDur * 100)); }
    ignitionSchedule7.repeatEndCompare = ignitionSchedule7.repeatStartCompare + uS_TO_TIMER_COMPARE(((((uint32_t)configPage4.dwellCrank * 100) * configPage9.ignRptScale) / 100));
  }
}
#endif
#if IGN_CHANNELS >= 8
void setIgnitionSchedule8(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  if(ignitionSchedule8.Status != RUNNING) //Check that we're not already part way through a schedule
  {
    ignitionSchedule8.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule8.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule8.duration = duration;

    //Need to check that the timeout doesn't exceed the overflow
    COMPARE_TYPE timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

    if (ignitionSchedule8.ignRptCnt == 0)
    {
      noInterrupts();
      ignitionSchedule8.startCompare = IGN8_COUNTER + timeout_timer_compare;
      if(!BIT_CHECK(ignitionSchedule8.scheduleFlags, BIT_SCHEDULE_DECODER)) { ignitionSchedule8.endCompare = ignitionSchedule8.startCompare + uS_TO_TIMER_COMPARE(duration); } //The .endCompare value is also set by the per tooth timing in decoders.ino. The check here is so that it's not getting overridden. 
      SET_COMPARE(IGN8_COMPARE, ignitionSchedule8.startCompare);
      ignitionSchedule8.Status = PENDING; //Turn this schedule on
      ignitionSchedule8.schedulesSet++;
      interrupts();
    }
    IGN8_TIMER_ENABLE();
  }
  else
  {
    //If the schedule is already running, we can set the next schedule so it is ready to go
    //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
    if (timeout < MAX_TIMER_PERIOD)
    {
      ignitionSchedule8.nextStartCompare = IGN8_COUNTER + uS_TO_TIMER_COMPARE(timeout);
      ignitionSchedule8.nextEndCompare = ignitionSchedule8.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
      BIT_CLEAR(ignitionSchedule8.scheduleFlags, BIT_SCHEDULE_NEXT);
    }
  }
  if ((configPage9.crankIgnOutRpt == 1) && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && (ignitionSchedule8.ignRptCnt <= 3))
  {
    if (ignitionSchedule8.ignRptCnt >=1)
    { ignitionSchedule8.repeatStartCompare = IGN8_COUNTER + uS_TO_TIMER_COMPARE(((((uint32_t)configPage4.dwellCrank * 100) * configPage9.ignRptScale) / 100) + (configPage4.sparkDur * 100)); }
    else
    { ignitionSchedule8.repeatStartCompare = IGN8_COUNTER + uS_TO_TIMER_COMPARE(duration + (configPage4.sparkDur * 100)); }
    ignitionSchedule8.repeatEndCompare = ignitionSchedule8.repeatStartCompare + uS_TO_TIMER_COMPARE(((((uint32_t)configPage4.dwellCrank * 100) * configPage9.ignRptScale) / 100));
  }
}
#endif
/** Perform the injector priming pulses.
 * Set these to run at an arbitrary time in the future (100us).
 * The prime pulse value is in ms*10, so need to multiple by 100 to get to uS
 */
extern void beginInjectorPriming(void)
{
  unsigned long primingValue = table2D_getValue(&PrimingPulseTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
  if (configPage9.priminScaleEnbl == 1) { primingValue += (primingValue * configPage9.primingScaleValue) / 50; } //Adds x% to the pulse

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
      if(BIT_CHECK(ignitionSchedule1.scheduleFlags, BIT_SCHEDULE_DECODER)) { SET_COMPARE(IGN1_COMPARE, ignitionSchedule1.endCompare); }
      else { SET_COMPARE(IGN1_COMPARE, IGN1_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule1.duration) ); } //Doing this here prevents a potential overflow on restarts
    }
    else if (ignitionSchedule1.Status == RUNNING)
    {
      ignitionSchedule1.EndCallback();
      ignitionSchedule1.Status = OFF; //Turn off the schedule
      ignitionSchedule1.schedulesSet = 0;
      BIT_CLEAR(ignitionSchedule1.scheduleFlags, BIT_SCHEDULE_DECODER);
      ignitionCount += 1; //Increment the ignition counter
      if (ignitionSchedule1.ignRptCnt >= 3) { (BIT_SET(ignitionSchedule1.scheduleFlags, BIT_SCHEDULE_REPEATED)); ignitionSchedule1.ignRptCnt = 0; }
      
      if ((configPage9.crankIgnOutRpt == 1) && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && !BIT_CHECK(ignitionSchedule1.scheduleFlags, BIT_SCHEDULE_REPEATED))
      {
        SET_COMPARE(IGN1_COMPARE, ignitionSchedule1.repeatStartCompare);
        ignitionSchedule1.endCompare = ignitionSchedule1.repeatEndCompare;
        ignitionSchedule1.Status = PENDING;
        ignitionSchedule1.schedulesSet = 1;
        ignitionSchedule1.ignRptCnt += 1;
        BIT_CLEAR(ignitionSchedule1.scheduleFlags, BIT_SCHEDULE_NEXT);
        BIT_SET(ignitionSchedule1.scheduleFlags, BIT_SCHEDULE_DECODER);
      }
      //If there is a next schedule queued up, activate it
      else if (BIT_CHECK(ignitionSchedule1.scheduleFlags, BIT_SCHEDULE_NEXT))
      {
        SET_COMPARE(IGN1_COMPARE, ignitionSchedule1.nextStartCompare);
        ignitionSchedule1.Status = PENDING;
        ignitionSchedule1.schedulesSet = 1;
        ignitionSchedule1.ignRptCnt = 0;
        BIT_CLEAR(ignitionSchedule1.scheduleFlags, BIT_SCHEDULE_NEXT);
        BIT_CLEAR(ignitionSchedule1.scheduleFlags, BIT_SCHEDULE_REPEATED);
      }
      else{ IGN1_TIMER_DISABLE(); BIT_CLEAR(ignitionSchedule1.scheduleFlags, BIT_SCHEDULE_REPEATED); ignitionSchedule1.ignRptCnt = 0; }
    }
    else if (ignitionSchedule1.Status == OFF)
    {
      //Catch any spurious interrupts. This really shouldn't ever be called, but there as a safety
      IGN1_TIMER_DISABLE();
      ignitionSchedule1.scheduleFlags = 0;
      ignitionSchedule1.ignRptCnt = 0;
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
      if(BIT_CHECK(ignitionSchedule2.scheduleFlags, BIT_SCHEDULE_DECODER)) { SET_COMPARE(IGN2_COMPARE, ignitionSchedule2.endCompare); }
      else { SET_COMPARE(IGN2_COMPARE, IGN2_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule2.duration) ); } //Doing this here prevents a potential overflow on restarts
    }
    else if (ignitionSchedule2.Status == RUNNING)
    {
      ignitionSchedule2.EndCallback();
      ignitionSchedule2.Status = OFF; //Turn off the schedule
      ignitionSchedule2.schedulesSet = 0;
      BIT_CLEAR(ignitionSchedule2.scheduleFlags, BIT_SCHEDULE_DECODER);
      ignitionCount += 1; //Increment the ignition counter
      if (ignitionSchedule2.ignRptCnt >= 3) { (BIT_SET(ignitionSchedule2.scheduleFlags, BIT_SCHEDULE_REPEATED)); ignitionSchedule2.ignRptCnt = 0; }

      if ((configPage9.crankIgnOutRpt == 1) && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && !BIT_CHECK(ignitionSchedule2.scheduleFlags, BIT_SCHEDULE_REPEATED))
      {
        SET_COMPARE(IGN2_COMPARE, ignitionSchedule2.repeatStartCompare);
        ignitionSchedule2.endCompare = ignitionSchedule2.repeatEndCompare;
        ignitionSchedule2.Status = PENDING;
        ignitionSchedule2.schedulesSet = 1;
        ignitionSchedule2.ignRptCnt += 1;
        BIT_CLEAR(ignitionSchedule2.scheduleFlags, BIT_SCHEDULE_NEXT);
        BIT_SET(ignitionSchedule2.scheduleFlags, BIT_SCHEDULE_DECODER);
      }
      //If there is a next schedule queued up, activate it
      else if (BIT_CHECK(ignitionSchedule2.scheduleFlags, BIT_SCHEDULE_NEXT))
      {
        SET_COMPARE(IGN2_COMPARE, ignitionSchedule2.nextStartCompare);
        ignitionSchedule2.Status = PENDING;
        ignitionSchedule2.schedulesSet = 1;
        ignitionSchedule2.ignRptCnt = 0;
        BIT_CLEAR(ignitionSchedule2.scheduleFlags, BIT_SCHEDULE_NEXT);
        BIT_CLEAR(ignitionSchedule2.scheduleFlags, BIT_SCHEDULE_REPEATED);
      }
      else{ IGN2_TIMER_DISABLE(); BIT_CLEAR(ignitionSchedule2.scheduleFlags, BIT_SCHEDULE_REPEATED); ignitionSchedule2.ignRptCnt = 0; }
    }
    else if (ignitionSchedule2.Status == OFF)
    {
      //Catch any spurious interrupts. This really shouldn't ever be called, but there as a safety
      IGN2_TIMER_DISABLE();
      ignitionSchedule2.scheduleFlags = 0;
      ignitionSchedule2.ignRptCnt = 0;
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
      if(BIT_CHECK(ignitionSchedule3.scheduleFlags, BIT_SCHEDULE_DECODER)) { SET_COMPARE(IGN3_COMPARE, ignitionSchedule3.endCompare); }
      else { SET_COMPARE(IGN3_COMPARE, IGN3_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule3.duration) ); } //Doing this here prevents a potential overflow that can occur at low RPMs
    }
    else if (ignitionSchedule3.Status == RUNNING)
    {
      ignitionSchedule3.EndCallback();
      ignitionSchedule3.Status = OFF; //Turn off the schedule
      ignitionSchedule3.schedulesSet = 0;
      BIT_CLEAR(ignitionSchedule3.scheduleFlags, BIT_SCHEDULE_DECODER);
      ignitionCount += 1; //Increment the ignition counter
      if (ignitionSchedule3.ignRptCnt >= 3) { (BIT_SET(ignitionSchedule3.scheduleFlags, BIT_SCHEDULE_REPEATED)); ignitionSchedule3.ignRptCnt = 0; }

      if ((configPage9.crankIgnOutRpt == 1) && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && !BIT_CHECK(ignitionSchedule3.scheduleFlags, BIT_SCHEDULE_REPEATED))
      {
        SET_COMPARE(IGN3_COMPARE, ignitionSchedule3.repeatStartCompare);
        ignitionSchedule3.endCompare = ignitionSchedule3.repeatEndCompare;
        ignitionSchedule3.Status = PENDING;
        ignitionSchedule3.schedulesSet = 1;
        ignitionSchedule3.ignRptCnt += 1;
        BIT_CLEAR(ignitionSchedule3.scheduleFlags, BIT_SCHEDULE_NEXT);
        BIT_SET(ignitionSchedule3.scheduleFlags, BIT_SCHEDULE_DECODER);
      }
      //If there is a next schedule queued up, activate it
      else if (BIT_CHECK(ignitionSchedule3.scheduleFlags, BIT_SCHEDULE_NEXT))
      {
        SET_COMPARE(IGN3_COMPARE, ignitionSchedule3.nextStartCompare);
        ignitionSchedule3.Status = PENDING;
        ignitionSchedule3.schedulesSet = 1;
        ignitionSchedule3.ignRptCnt = 0;
        BIT_CLEAR(ignitionSchedule3.scheduleFlags, BIT_SCHEDULE_NEXT);
        BIT_CLEAR(ignitionSchedule3.scheduleFlags, BIT_SCHEDULE_REPEATED);
      }
      else { IGN3_TIMER_DISABLE(); BIT_CLEAR(ignitionSchedule3.scheduleFlags, BIT_SCHEDULE_REPEATED); ignitionSchedule3.ignRptCnt = 0; }
    }
    else if (ignitionSchedule3.Status == OFF)
    {
      //Catch any spurious interrupts. This really shouldn't ever be called, but there as a safety
      IGN3_TIMER_DISABLE();
      ignitionSchedule3.scheduleFlags = 0;
      ignitionSchedule3.ignRptCnt = 0;
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
      if(BIT_CHECK(ignitionSchedule4.scheduleFlags, BIT_SCHEDULE_DECODER)) { SET_COMPARE(IGN4_COMPARE, ignitionSchedule4.endCompare); }
      else { SET_COMPARE(IGN4_COMPARE, IGN4_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule4.duration) ); } //Doing this here prevents a potential overflow that can occur at low RPMs
    }
    else if (ignitionSchedule4.Status == RUNNING)
    {
      ignitionSchedule4.EndCallback();
      ignitionSchedule4.Status = OFF; //Turn off the schedule
      ignitionSchedule4.schedulesSet = 0;
      BIT_CLEAR(ignitionSchedule4.scheduleFlags, BIT_SCHEDULE_DECODER);
      ignitionCount += 1; //Increment the ignition counter
      if (ignitionSchedule4.ignRptCnt >= 3) { (BIT_SET(ignitionSchedule4.scheduleFlags, BIT_SCHEDULE_REPEATED)); ignitionSchedule4.ignRptCnt = 0; }

      if ((configPage9.crankIgnOutRpt == 1) && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && !BIT_CHECK(ignitionSchedule4.scheduleFlags, BIT_SCHEDULE_REPEATED))
      {
        SET_COMPARE(IGN4_COMPARE, ignitionSchedule4.repeatStartCompare);
        ignitionSchedule4.endCompare = ignitionSchedule4.repeatEndCompare;
        ignitionSchedule4.Status = PENDING;
        ignitionSchedule4.schedulesSet = 1;
        ignitionSchedule4.ignRptCnt += 1;
        BIT_CLEAR(ignitionSchedule4.scheduleFlags, BIT_SCHEDULE_NEXT);
        BIT_SET(ignitionSchedule4.scheduleFlags, BIT_SCHEDULE_DECODER);
      }
      //If there is a next schedule queued up, activate it
      else if (BIT_CHECK(ignitionSchedule4.scheduleFlags, BIT_SCHEDULE_NEXT))
      {
        SET_COMPARE(IGN4_COMPARE, ignitionSchedule4.nextStartCompare);
        ignitionSchedule4.Status = PENDING;
        ignitionSchedule4.schedulesSet = 1;
        ignitionSchedule4.ignRptCnt = 0;
        BIT_CLEAR(ignitionSchedule4.scheduleFlags, BIT_SCHEDULE_NEXT);
        BIT_CLEAR(ignitionSchedule4.scheduleFlags, BIT_SCHEDULE_REPEATED);
      }
      else { IGN4_TIMER_DISABLE(); BIT_CLEAR(ignitionSchedule4.scheduleFlags, BIT_SCHEDULE_REPEATED); ignitionSchedule4.ignRptCnt = 0; }
    }
    else if (ignitionSchedule4.Status == OFF)
    {
      //Catch any spurious interrupts. This really shouldn't ever be called, but there as a safety
      IGN4_TIMER_DISABLE();
      ignitionSchedule4.scheduleFlags = 0;
      ignitionSchedule4.ignRptCnt = 0;
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
      if(BIT_CHECK(ignitionSchedule5.scheduleFlags, BIT_SCHEDULE_DECODER)) { SET_COMPARE(IGN5_COMPARE, ignitionSchedule5.endCompare); }
      else { SET_COMPARE(IGN5_COMPARE, IGN5_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule5.duration) ); } //If the decoder based timing isn't set, doing this here prevents a potential overflow that can occur at low RPMs
    }
    else if (ignitionSchedule5.Status == RUNNING)
    {
      ignitionSchedule5.EndCallback();
      ignitionSchedule5.Status = OFF; //Turn off the schedule
      ignitionSchedule5.schedulesSet = 0;
      BIT_CLEAR(ignitionSchedule5.scheduleFlags, BIT_SCHEDULE_DECODER);
      ignitionCount += 1; //Increment the ignition counter
      if (ignitionSchedule5.ignRptCnt >= 3) { (BIT_SET(ignitionSchedule5.scheduleFlags, BIT_SCHEDULE_REPEATED)); ignitionSchedule5.ignRptCnt = 0; }

      if ((configPage9.crankIgnOutRpt == 1) && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && !BIT_CHECK(ignitionSchedule5.scheduleFlags, BIT_SCHEDULE_REPEATED))
      {
        SET_COMPARE(IGN5_COMPARE, ignitionSchedule5.repeatStartCompare);
        ignitionSchedule5.endCompare = ignitionSchedule5.repeatEndCompare;
        ignitionSchedule5.Status = PENDING;
        ignitionSchedule5.schedulesSet = 1;
        ignitionSchedule5.ignRptCnt += 1;
        BIT_CLEAR(ignitionSchedule5.scheduleFlags, BIT_SCHEDULE_NEXT);
        BIT_SET(ignitionSchedule5.scheduleFlags, BIT_SCHEDULE_DECODER);
      }
      //If there is a next schedule queued up, activate it
      else if (BIT_CHECK(ignitionSchedule5.scheduleFlags, BIT_SCHEDULE_NEXT))
      {
        SET_COMPARE(IGN5_COMPARE, ignitionSchedule5.nextStartCompare);
        ignitionSchedule5.Status = PENDING;
        ignitionSchedule5.schedulesSet = 1;
        ignitionSchedule5.ignRptCnt = 0;
        BIT_CLEAR(ignitionSchedule5.scheduleFlags, BIT_SCHEDULE_NEXT);
        BIT_CLEAR(ignitionSchedule5.scheduleFlags, BIT_SCHEDULE_REPEATED);
      }
      else{ IGN5_TIMER_DISABLE(); BIT_CLEAR(ignitionSchedule5.scheduleFlags, BIT_SCHEDULE_REPEATED); ignitionSchedule5.ignRptCnt = 0; }
    }
    else if (ignitionSchedule5.Status == OFF)
    {
      //Catch any spurious interrupts. This really shouldn't ever be called, but there as a safety
      IGN5_TIMER_DISABLE();
      ignitionSchedule5.scheduleFlags = 0;
      ignitionSchedule5.ignRptCnt = 0;
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
      if(BIT_CHECK(ignitionSchedule6.scheduleFlags, BIT_SCHEDULE_DECODER)) { SET_COMPARE(IGN6_COMPARE, ignitionSchedule6.endCompare); }
      else { SET_COMPARE(IGN6_COMPARE, IGN6_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule6.duration) ); } //If the decoder based timing isn't set, doing this here prevents a potential overflow that can occur at low RPMs
    }
    else if (ignitionSchedule6.Status == RUNNING)
    {
      ignitionSchedule6.EndCallback();
      ignitionSchedule6.Status = OFF; //Turn off the schedule
      ignitionSchedule6.schedulesSet = 0;
      BIT_CLEAR(ignitionSchedule6.scheduleFlags, BIT_SCHEDULE_DECODER);
      ignitionCount += 1; //Increment the ignition counter
      if (ignitionSchedule6.ignRptCnt >= 3) { (BIT_SET(ignitionSchedule6.scheduleFlags, BIT_SCHEDULE_REPEATED)); ignitionSchedule6.ignRptCnt = 0; }

      if ((configPage9.crankIgnOutRpt == 1) && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && !BIT_CHECK(ignitionSchedule6.scheduleFlags, BIT_SCHEDULE_REPEATED))
      {
        SET_COMPARE(IGN6_COMPARE, ignitionSchedule6.repeatStartCompare);
        ignitionSchedule6.endCompare = ignitionSchedule6.repeatEndCompare;
        ignitionSchedule6.Status = PENDING;
        ignitionSchedule6.schedulesSet = 1;
        ignitionSchedule6.ignRptCnt += 1;
        BIT_CLEAR(ignitionSchedule6.scheduleFlags, BIT_SCHEDULE_NEXT);
        BIT_SET(ignitionSchedule6.scheduleFlags, BIT_SCHEDULE_DECODER);
      }
      //If there is a next schedule queued up, activate it
      else if(BIT_CHECK(ignitionSchedule6.scheduleFlags, BIT_SCHEDULE_NEXT))
      {
        SET_COMPARE(IGN6_COMPARE, ignitionSchedule6.nextStartCompare);
        ignitionSchedule6.Status = PENDING;
        ignitionSchedule6.schedulesSet = 1;
        ignitionSchedule6.ignRptCnt = 0;
        BIT_CLEAR(ignitionSchedule6.scheduleFlags, BIT_SCHEDULE_NEXT);
        BIT_CLEAR(ignitionSchedule6.scheduleFlags, BIT_SCHEDULE_REPEATED);
      }
      else{ IGN6_TIMER_DISABLE(); BIT_CLEAR(ignitionSchedule6.scheduleFlags, BIT_SCHEDULE_REPEATED); ignitionSchedule6.ignRptCnt = 0; }
    }
    else if (ignitionSchedule6.Status == OFF)
    {
      //Catch any spurious interrupts. This really shouldn't ever be called, but there as a safety
      IGN6_TIMER_DISABLE();
      ignitionSchedule6.scheduleFlags = 0;
      ignitionSchedule6.ignRptCnt = 0;
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
      if(BIT_CHECK(ignitionSchedule7.scheduleFlags, BIT_SCHEDULE_DECODER)) { SET_COMPARE(IGN7_COMPARE, ignitionSchedule7.endCompare); }
      else { SET_COMPARE(IGN7_COMPARE, IGN7_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule7.duration) ); } //If the decoder based timing isn't set, doing this here prevents a potential overflow that can occur at low RPMs
    }
    else if (ignitionSchedule7.Status == RUNNING)
    {
      ignitionSchedule7.EndCallback();
      ignitionSchedule7.Status = OFF; //Turn off the schedule
      ignitionSchedule7.schedulesSet = 0;
      BIT_CLEAR(ignitionSchedule7.scheduleFlags, BIT_SCHEDULE_DECODER);
      ignitionCount += 1; //Increment the ignition counter
      if (ignitionSchedule7.ignRptCnt >= 3) { (BIT_SET(ignitionSchedule7.scheduleFlags, BIT_SCHEDULE_REPEATED)); ignitionSchedule7.ignRptCnt = 0; }

      if ((configPage9.crankIgnOutRpt == 1) && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && !BIT_CHECK(ignitionSchedule7.scheduleFlags, BIT_SCHEDULE_REPEATED))
      {
        SET_COMPARE(IGN7_COMPARE, ignitionSchedule7.repeatStartCompare);
        ignitionSchedule7.endCompare = ignitionSchedule7.repeatEndCompare;
        ignitionSchedule7.Status = PENDING;
        ignitionSchedule7.schedulesSet = 1;
        ignitionSchedule7.ignRptCnt += 1;
        BIT_CLEAR(ignitionSchedule7.scheduleFlags, BIT_SCHEDULE_NEXT);
        BIT_SET(ignitionSchedule7.scheduleFlags, BIT_SCHEDULE_DECODER);
      }
      //If there is a next schedule queued up, activate it
      else if(BIT_CHECK(ignitionSchedule7.scheduleFlags, BIT_SCHEDULE_NEXT))
      {
        SET_COMPARE(IGN7_COMPARE, ignitionSchedule7.nextStartCompare);
        ignitionSchedule7.Status = PENDING;
        ignitionSchedule7.schedulesSet = 1;
        ignitionSchedule7.ignRptCnt = 0;
        BIT_CLEAR(ignitionSchedule7.scheduleFlags, BIT_SCHEDULE_NEXT);
        BIT_CLEAR(ignitionSchedule7.scheduleFlags, BIT_SCHEDULE_REPEATED);
      }
      else{ IGN7_TIMER_DISABLE(); BIT_CLEAR(ignitionSchedule7.scheduleFlags, BIT_SCHEDULE_REPEATED); ignitionSchedule7.ignRptCnt = 0; }
    }
    else if (ignitionSchedule7.Status == OFF)
    {
      //Catch any spurious interrupts. This really shouldn't ever be called, but there as a safety
      IGN7_TIMER_DISABLE();
      ignitionSchedule7.scheduleFlags = 0;
      ignitionSchedule7.ignRptCnt = 0;
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
      if(BIT_CHECK(ignitionSchedule8.scheduleFlags, BIT_SCHEDULE_DECODER)) { SET_COMPARE(IGN8_COMPARE, ignitionSchedule8.endCompare); }
      else { SET_COMPARE(IGN8_COMPARE, IGN8_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule8.duration) ); } //If the decoder based timing isn't set, doing this here prevents a potential overflow that can occur at low RPMs
    }
    else if (ignitionSchedule8.Status == RUNNING)
    {
      ignitionSchedule8.EndCallback();
      ignitionSchedule8.Status = OFF; //Turn off the schedule
      ignitionSchedule8.schedulesSet = 0;
      BIT_CLEAR(ignitionSchedule8.scheduleFlags, BIT_SCHEDULE_DECODER);
      ignitionCount += 1; //Increment the ignition counter
      if (ignitionSchedule8.ignRptCnt >= 3) { (BIT_SET(ignitionSchedule8.scheduleFlags, BIT_SCHEDULE_REPEATED)); ignitionSchedule8.ignRptCnt = 0; }

      if ((configPage9.crankIgnOutRpt == 1) && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && !BIT_CHECK(ignitionSchedule8.scheduleFlags, BIT_SCHEDULE_REPEATED))
      {
        SET_COMPARE(IGN8_COMPARE, ignitionSchedule8.repeatStartCompare);
        ignitionSchedule8.endCompare = ignitionSchedule8.repeatEndCompare;
        ignitionSchedule8.Status = PENDING;
        ignitionSchedule8.schedulesSet = 1;
        ignitionSchedule8.ignRptCnt += 1;
        BIT_CLEAR(ignitionSchedule8.scheduleFlags, BIT_SCHEDULE_NEXT);
        BIT_SET(ignitionSchedule8.scheduleFlags, BIT_SCHEDULE_DECODER);
      }
      //If there is a next schedule queued up, activate it
      else if(BIT_CHECK(ignitionSchedule8.scheduleFlags, BIT_SCHEDULE_NEXT))
      {
        SET_COMPARE(IGN8_COMPARE, ignitionSchedule8.nextStartCompare);
        ignitionSchedule8.Status = PENDING;
        ignitionSchedule8.schedulesSet = 1;
        ignitionSchedule8.ignRptCnt = 0;
        BIT_CLEAR(ignitionSchedule8.scheduleFlags, BIT_SCHEDULE_NEXT);
        BIT_CLEAR(ignitionSchedule8.scheduleFlags, BIT_SCHEDULE_REPEATED);
      }
      else{ IGN8_TIMER_DISABLE(); BIT_CLEAR(ignitionSchedule8.scheduleFlags, BIT_SCHEDULE_REPEATED); ignitionSchedule8.ignRptCnt = 0; }
    }
    else if (ignitionSchedule8.Status == OFF)
    {
      //Catch any spurious interrupts. This really shouldn't ever be called, but there as a safety
      IGN8_TIMER_DISABLE();
      ignitionSchedule8.scheduleFlags = 0;
      ignitionSchedule8.ignRptCnt = 0;
    }
  }
#endif
