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

FuelSchedule fuelSchedule1(FUEL1_COUNTER, FUEL1_COMPARE, FUEL1_TIMER_DISABLE, FUEL1_TIMER_ENABLE);
FuelSchedule fuelSchedule2(FUEL2_COUNTER, FUEL2_COMPARE, FUEL2_TIMER_DISABLE, FUEL2_TIMER_ENABLE);
FuelSchedule fuelSchedule3(FUEL3_COUNTER, FUEL3_COMPARE, FUEL3_TIMER_DISABLE, FUEL3_TIMER_ENABLE);
FuelSchedule fuelSchedule4(FUEL4_COUNTER, FUEL4_COMPARE, FUEL4_TIMER_DISABLE, FUEL4_TIMER_ENABLE);

#if (INJ_CHANNELS >= 5)
FuelSchedule fuelSchedule5(FUEL5_COUNTER, FUEL5_COMPARE, FUEL5_TIMER_DISABLE, FUEL5_TIMER_ENABLE);
#endif
#if (INJ_CHANNELS >= 6)
FuelSchedule fuelSchedule6(FUEL6_COUNTER, FUEL6_COMPARE, FUEL6_TIMER_DISABLE, FUEL6_TIMER_ENABLE);
#endif
#if (INJ_CHANNELS >= 7)
FuelSchedule fuelSchedule7(FUEL7_COUNTER, FUEL7_COMPARE, FUEL7_TIMER_DISABLE, FUEL7_TIMER_ENABLE);
#endif
#if (INJ_CHANNELS >= 8)
FuelSchedule fuelSchedule8(FUEL8_COUNTER, FUEL8_COMPARE, FUEL8_TIMER_DISABLE, FUEL8_TIMER_ENABLE);
#endif

IgnitionSchedule ignitionSchedule1(IGN1_COUNTER, IGN1_COMPARE, IGN1_TIMER_DISABLE, IGN1_TIMER_ENABLE);
IgnitionSchedule ignitionSchedule2(IGN2_COUNTER, IGN2_COMPARE, IGN2_TIMER_DISABLE, IGN2_TIMER_ENABLE);
IgnitionSchedule ignitionSchedule3(IGN3_COUNTER, IGN3_COMPARE, IGN3_TIMER_DISABLE, IGN3_TIMER_ENABLE);
IgnitionSchedule ignitionSchedule4(IGN4_COUNTER, IGN4_COMPARE, IGN4_TIMER_DISABLE, IGN4_TIMER_ENABLE);
IgnitionSchedule ignitionSchedule5(IGN5_COUNTER, IGN5_COMPARE, IGN5_TIMER_DISABLE, IGN5_TIMER_ENABLE);

#if IGN_CHANNELS >= 6
IgnitionSchedule ignitionSchedule6(IGN6_COUNTER, IGN6_COMPARE, IGN6_TIMER_DISABLE, IGN6_TIMER_ENABLE);
#endif
#if IGN_CHANNELS >= 7
IgnitionSchedule ignitionSchedule7(IGN7_COUNTER, IGN7_COMPARE, IGN7_TIMER_DISABLE, IGN7_TIMER_ENABLE);
#endif
#if IGN_CHANNELS >= 8
IgnitionSchedule ignitionSchedule8(IGN8_COUNTER, IGN8_COMPARE, IGN8_TIMER_DISABLE, IGN8_TIMER_ENABLE);
#endif

void initialiseSchedulers()
{
    fuelSchedule1.Status = OFF;
    fuelSchedule2.Status = OFF;
    fuelSchedule3.Status = OFF;
    fuelSchedule4.Status = OFF;
#if INJ_CHANNELS >= 5
    fuelSchedule5.Status = OFF;
#endif
#if INJ_CHANNELS >= 6
    fuelSchedule6.Status = OFF;
#endif
#if INJ_CHANNELS >= 7
    fuelSchedule7.Status = OFF;
#endif
#if INJ_CHANNELS >= 8
    fuelSchedule8.Status = OFF;
#endif

    ignitionSchedule1.Status = OFF;
    IGN1_TIMER_ENABLE();
    ignitionSchedule2.Status = OFF;
    IGN2_TIMER_ENABLE();
    ignitionSchedule3.Status = OFF;
    IGN3_TIMER_ENABLE();
    ignitionSchedule4.Status = OFF;
    IGN4_TIMER_ENABLE();
#if (IGN_CHANNELS >= 5)
    ignitionSchedule5.Status = OFF;
    IGN5_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 6
    ignitionSchedule6.Status = OFF;
    IGN6_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 7
    ignitionSchedule7.Status = OFF;
    IGN7_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 8
    ignitionSchedule8.Status = OFF;
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

  fuelSchedule1.pStartFunction = nullCallback;
  fuelSchedule1.pEndFunction = nullCallback;
  fuelSchedule2.pStartFunction = nullCallback;
  fuelSchedule2.pEndFunction = nullCallback;
  fuelSchedule3.pStartFunction = nullCallback;
  fuelSchedule3.pEndFunction = nullCallback;
  fuelSchedule4.pStartFunction = nullCallback;
  fuelSchedule4.pEndFunction = nullCallback;
#if (INJ_CHANNELS >= 5)  
  fuelSchedule5.pStartFunction = nullCallback;
  fuelSchedule5.pEndFunction = nullCallback;
#endif
#if (INJ_CHANNELS >= 6)
  fuelSchedule6.pStartFunction = nullCallback;
  fuelSchedule6.pEndFunction = nullCallback;
#endif
#if (INJ_CHANNELS >= 7)
  fuelSchedule7.pStartFunction = nullCallback;
  fuelSchedule7.pEndFunction = nullCallback;
#endif
#if (INJ_CHANNELS >= 8)
  fuelSchedule8.pStartFunction = nullCallback;
  fuelSchedule8.pEndFunction = nullCallback;
#endif

  ignitionSchedule1.pStartCallback = nullCallback;
  ignitionSchedule1.pEndCallback = nullCallback;
  ignition1StartAngle=0;
  ignition1EndAngle=0;
  channel1IgnDegrees=0; /**< The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones) */

  ignitionSchedule2.pStartCallback = nullCallback;
  ignitionSchedule2.pEndCallback = nullCallback;
  ignition2StartAngle=0;
  ignition2EndAngle=0;
  channel2IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */

  ignitionSchedule3.pStartCallback = nullCallback;
  ignitionSchedule3.pEndCallback = nullCallback;
  ignition3StartAngle=0;
  ignition3EndAngle=0;
  channel3IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */

  ignitionSchedule4.pStartCallback = nullCallback;
  ignitionSchedule4.pEndCallback = nullCallback;
  ignition4StartAngle=0;
  ignition4EndAngle=0;
  channel4IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */

#if (IGN_CHANNELS >= 5)
  ignitionSchedule5.pStartCallback = nullCallback;
  ignitionSchedule5.pEndCallback = nullCallback;
  ignition5StartAngle=0;
  ignition5EndAngle=0;
  channel5IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
#endif
#if (IGN_CHANNELS >= 6)
  ignitionSchedule6.pStartCallback = nullCallback;
  ignitionSchedule6.pEndCallback = nullCallback;
  ignition6StartAngle=0;
  ignition6EndAngle=0;
  channel6IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
#endif
#if (IGN_CHANNELS >= 7)
  ignitionSchedule7.pStartCallback = nullCallback;
  ignitionSchedule7.pEndCallback = nullCallback;
  ignition7StartAngle=0;
  ignition7EndAngle=0;
  channel7IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
#endif
#if (IGN_CHANNELS >= 8)
  ignitionSchedule8.pStartCallback = nullCallback;
  ignitionSchedule8.pEndCallback = nullCallback;
  ignition8StartAngle=0;
  ignition8EndAngle=0;
  channel8IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
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

/*
These 8 function turn a schedule on, provides the time to start and the duration and gives it callback functions.
All 8 functions operate the same, just on different schedules
Args:
startCallback: The function to be called once the timeout is reached
timeout: The number of uS in the future that the startCallback should be triggered
duration: The number of uS after startCallback is called before endCallback is called
endCallback: This function is called once the duration time has been reached
*/

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

void setFuelSchedule3(unsigned long timeout, unsigned long duration) //Uses timer 3 compare C
{
  //Check whether timeout exceeds the maximum future time. This can potentially occur on sequential setups when below ~115rpm
  if(timeout < MAX_TIMER_PERIOD)
  {
    if(fuelSchedule3.Status != RUNNING) //Check that we're not already part way through a schedule
    {
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

void setFuelSchedule4(unsigned long timeout, unsigned long duration) //Uses timer 4 compare B
{
  //Check whether timeout exceeds the maximum future time. This can potentially occur on sequential setups when below ~115rpm
  if(timeout < MAX_TIMER_PERIOD)
  {
    if(fuelSchedule4.Status != RUNNING) //Check that we're not already part way through a schedule
    {
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
void setIgnitionSchedule1(unsigned long timeout, unsigned long duration)
{
  if(ignitionSchedule1.Status != RUNNING) //Check that we're not already part way through a schedule
  {
    ignitionSchedule1.duration = duration;

    //Need to check that the timeout doesn't exceed the overflow
    COMPARE_TYPE timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

    noInterrupts();
    ignitionSchedule1.startCompare = IGN1_COUNTER + timeout_timer_compare; //As there is a tick every 4uS, there are timeout/4 ticks until the interrupt should be triggered ( >>2 divides by 4)
    if(ignitionSchedule1.endScheduleSetByDecoder == false) { ignitionSchedule1.endCompare = ignitionSchedule1.startCompare + uS_TO_TIMER_COMPARE(duration); } //The .endCompare value is also set by the per tooth timing in decoders.ino. The check here is so that it's not getting overridden. 
    SET_COMPARE(IGN1_COMPARE, ignitionSchedule1.startCompare);
    ignitionSchedule1.Status = PENDING; //Turn this schedule on
    interrupts();
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
      ignitionSchedule1.hasNextSchedule = true;
    }

  }
}

void refreshIgnitionSchedule1(unsigned long timeToEnd)
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

void setIgnitionSchedule2(unsigned long timeout, unsigned long duration)
{
  if(ignitionSchedule2.Status != RUNNING) //Check that we're not already part way through a schedule
  {
    ignitionSchedule2.duration = duration;

    //Need to check that the timeout doesn't exceed the overflow
    COMPARE_TYPE timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

    noInterrupts();
    ignitionSchedule2.startCompare = IGN2_COUNTER + timeout_timer_compare; //As there is a tick every 4uS, there are timeout/4 ticks until the interrupt should be triggered ( >>2 divides by 4)
    if(ignitionSchedule2.endScheduleSetByDecoder == false) { ignitionSchedule2.endCompare = ignitionSchedule2.startCompare + uS_TO_TIMER_COMPARE(duration); } //The .endCompare value is also set by the per tooth timing in decoders.ino. The check here is so that it's not getting overridden. 
    SET_COMPARE(IGN2_COMPARE, ignitionSchedule2.startCompare);
    ignitionSchedule2.Status = PENDING; //Turn this schedule on
    interrupts();
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
      ignitionSchedule2.hasNextSchedule = true;
    }
  }
}
void setIgnitionSchedule3(unsigned long timeout, unsigned long duration)
{
  if(ignitionSchedule3.Status != RUNNING) //Check that we're not already part way through a schedule
  {
    ignitionSchedule3.duration = duration;

    //Need to check that the timeout doesn't exceed the overflow
    COMPARE_TYPE timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

    noInterrupts();
    ignitionSchedule3.startCompare = IGN3_COUNTER + timeout_timer_compare; //As there is a tick every 4uS, there are timeout/4 ticks until the interrupt should be triggered ( >>2 divides by 4)
    if(ignitionSchedule3.endScheduleSetByDecoder == false) { ignitionSchedule3.endCompare = ignitionSchedule3.startCompare + uS_TO_TIMER_COMPARE(duration); } //The .endCompare value is also set by the per tooth timing in decoders.ino. The check here is so that it's not getting overridden. 
    SET_COMPARE(IGN3_COMPARE, ignitionSchedule3.startCompare);
    ignitionSchedule3.Status = PENDING; //Turn this schedule on
    interrupts();
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
      ignitionSchedule3.hasNextSchedule = true;
    }
  }
}
void setIgnitionSchedule4(unsigned long timeout, unsigned long duration)
{
  if(ignitionSchedule4.Status != RUNNING) //Check that we're not already part way through a schedule
  {
    ignitionSchedule4.duration = duration;

    //Need to check that the timeout doesn't exceed the overflow
    COMPARE_TYPE timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

    noInterrupts();
    ignitionSchedule4.startCompare = IGN4_COUNTER + timeout_timer_compare;
    if(ignitionSchedule4.endScheduleSetByDecoder == false) { ignitionSchedule4.endCompare = ignitionSchedule4.startCompare + uS_TO_TIMER_COMPARE(duration); } //The .endCompare value is also set by the per tooth timing in decoders.ino. The check here is so that it's not getting overridden. 
    SET_COMPARE(IGN4_COMPARE, ignitionSchedule4.startCompare);
    ignitionSchedule4.Status = PENDING; //Turn this schedule on
    interrupts();
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
      ignitionSchedule4.hasNextSchedule = true;
    }
  }
}
void setIgnitionSchedule5(unsigned long timeout, unsigned long duration)
{
  if(ignitionSchedule5.Status != RUNNING) //Check that we're not already part way through a schedule
  {
    ignitionSchedule5.duration = duration;

    //Need to check that the timeout doesn't exceed the overflow
    COMPARE_TYPE timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

    noInterrupts();
    ignitionSchedule5.startCompare = IGN5_COUNTER + timeout_timer_compare;
    if(ignitionSchedule5.endScheduleSetByDecoder == false) { ignitionSchedule5.endCompare = ignitionSchedule5.startCompare + uS_TO_TIMER_COMPARE(duration); } //The .endCompare value is also set by the per tooth timing in decoders.ino. The check here is so that it's not getting overridden. 
    SET_COMPARE(IGN5_COMPARE, ignitionSchedule5.startCompare);
    ignitionSchedule5.Status = PENDING; //Turn this schedule on
    interrupts();
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
      ignitionSchedule5.hasNextSchedule = true;
    }
  }
}

#if IGN_CHANNELS >= 6
void setIgnitionSchedule6(unsigned long timeout, unsigned long duration)
{
  if(ignitionSchedule6.Status != RUNNING) //Check that we're not already part way through a schedule
  {
    ignitionSchedule6.duration = duration;

    //Need to check that the timeout doesn't exceed the overflow
    COMPARE_TYPE timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

    noInterrupts();
    ignitionSchedule6.startCompare = IGN6_COUNTER + timeout_timer_compare;
    if(ignitionSchedule6.endScheduleSetByDecoder == false) { ignitionSchedule6.endCompare = ignitionSchedule6.startCompare + uS_TO_TIMER_COMPARE(duration); } //The .endCompare value is also set by the per tooth timing in decoders.ino. The check here is so that it's not getting overridden. 
    SET_COMPARE(IGN6_COMPARE, ignitionSchedule6.startCompare);
    ignitionSchedule6.Status = PENDING; //Turn this schedule on
    interrupts();
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
      ignitionSchedule6.hasNextSchedule = true;
    }
  }
}
#endif

#if IGN_CHANNELS >= 7
void setIgnitionSchedule7(unsigned long timeout, unsigned long duration)
{
  if(ignitionSchedule7.Status != RUNNING) //Check that we're not already part way through a schedule
  {
    ignitionSchedule7.duration = duration;

    //Need to check that the timeout doesn't exceed the overflow
    COMPARE_TYPE timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

    noInterrupts();
    ignitionSchedule7.startCompare = IGN7_COUNTER + timeout_timer_compare;
    if(ignitionSchedule7.endScheduleSetByDecoder == false) { ignitionSchedule7.endCompare = ignitionSchedule7.startCompare + uS_TO_TIMER_COMPARE(duration); } //The .endCompare value is also set by the per tooth timing in decoders.ino. The check here is so that it's not getting overridden. 
    SET_COMPARE(IGN7_COMPARE, ignitionSchedule7.startCompare);
    ignitionSchedule7.Status = PENDING; //Turn this schedule on
    interrupts();
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
      ignitionSchedule7.hasNextSchedule = true;
    }
  }
}
#endif

#if IGN_CHANNELS >= 8
void setIgnitionSchedule8(unsigned long timeout, unsigned long duration)
{
  if(ignitionSchedule8.Status != RUNNING) //Check that we're not already part way through a schedule
  {
    ignitionSchedule8.duration = duration;

    //Need to check that the timeout doesn't exceed the overflow
    COMPARE_TYPE timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); } //Normal case

    noInterrupts();
    ignitionSchedule8.startCompare = IGN8_COUNTER + timeout_timer_compare;
    if(ignitionSchedule8.endScheduleSetByDecoder == false) { ignitionSchedule8.endCompare = ignitionSchedule8.startCompare + uS_TO_TIMER_COMPARE(duration); } //The .endCompare value is also set by the per tooth timing in decoders.ino. The check here is so that it's not getting overridden. 
    SET_COMPARE(IGN8_COMPARE, ignitionSchedule8.startCompare);
    ignitionSchedule8.Status = PENDING; //Turn this schedule on
    interrupts();
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
      ignitionSchedule8.hasNextSchedule = true;
    }
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
  if( (primingValue > 0) && (currentStatus.TPS < configPage4.floodClear) )
  {
    primingValue = primingValue * 100 * 5; //to achieve long enough priming pulses, the values in tuner studio are divided by 0.5 instead of 0.1, so multiplier of 5 is required.
    if ( maxInjOutputs >= 1 ) { setFuelSchedule1(100, primingValue); }
#if (INJ_CHANNELS >= 2)
    if ( maxInjOutputs >= 2 ) { setFuelSchedule2(100, primingValue); }
#endif
#if (INJ_CHANNELS >= 3)
    if ( maxInjOutputs >= 3 ) { setFuelSchedule3(100, primingValue); }
#endif
#if (INJ_CHANNELS >= 4)
    if ( maxInjOutputs >= 4 ) { setFuelSchedule4(100, primingValue); }
#endif
#if (INJ_CHANNELS >= 5)
    if ( maxInjOutputs >= 5 ) { setFuelSchedule5(100, primingValue); }
#endif
#if (INJ_CHANNELS >= 6)
    if ( maxInjOutputs >= 6 ) { setFuelSchedule6(100, primingValue); }
#endif
#if (INJ_CHANNELS >= 7)
    if ( maxInjOutputs >= 7 ) { setFuelSchedule7(100, primingValue); }
#endif
#if (INJ_CHANNELS >= 8)
    if ( maxInjOutputs >= 8 ) { setFuelSchedule8(100, primingValue); }
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
    schedule.pStartFunction();
    schedule.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
    SET_COMPARE(schedule.compare, schedule.counter + uS_TO_TIMER_COMPARE(schedule.duration) ); //Doing this here prevents a potential overflow on restarts
  }
  else if (schedule.Status == RUNNING)
  {
      schedule.pEndFunction();
      schedule.Status = OFF; //Turn off the schedule

      //If there is a next schedule queued up, activate it
      if(schedule.hasNextSchedule == true)
      {
        SET_COMPARE(schedule.compare, schedule.nextStartCompare);
        SET_COMPARE(schedule.endCompare, schedule.nextEndCompare);
        schedule.Status = PENDING;
        schedule.hasNextSchedule = false;
      }
      else 
      { 
        schedule.pTimerDisable(); 
      }
  }
  else if (schedule.Status == OFF) 
  { 
    schedule.pTimerDisable(); //Safety check. Turn off this output compare unit and return without performing any action
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
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
//fuelSchedules 1 and 5
ISR(TIMER3_COMPA_vect) //cppcheck-suppress misra-c2012-8.2
#else
void fuelSchedule1Interrupt() //Most ARM chips can simply call a function
#endif
  {
    fuelScheduleISR(fuelSchedule1);
  }


#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
ISR(TIMER3_COMPB_vect) //cppcheck-suppress misra-c2012-8.2
#else
void fuelSchedule2Interrupt() //Most ARM chips can simply call a function
#endif
  {
    fuelScheduleISR(fuelSchedule2);
  }


#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
ISR(TIMER3_COMPC_vect) //cppcheck-suppress misra-c2012-8.2
#else
void fuelSchedule3Interrupt() //Most ARM chips can simply call a function
#endif
  {
    fuelScheduleISR(fuelSchedule3);
  }


#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
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
    if(schedule.endScheduleSetByDecoder == true) { SET_COMPARE(schedule.compare, schedule.endCompare); }
    else { SET_COMPARE(schedule.compare, schedule.counter + uS_TO_TIMER_COMPARE(schedule.duration) ); } //Doing this here prevents a potential overflow on restarts
  }
  else if (schedule.Status == RUNNING)
  {
    schedule.pEndCallback();
    schedule.Status = OFF; //Turn off the schedule
    schedule.endScheduleSetByDecoder = false;
    ignitionCount = ignitionCount + 1; //Increment the ignition counter
    currentStatus.actualDwell = DWELL_AVERAGE( (micros() - schedule.startTime) );

    //If there is a next schedule queued up, activate it
    if(schedule.hasNextSchedule == true)
    {
      SET_COMPARE(schedule.compare, schedule.nextStartCompare);
      schedule.Status = PENDING;
      schedule.hasNextSchedule = false;
    }
    else
    { 
      schedule.pTimerDisable(); 
    }
  }
  else if (schedule.Status == OFF)
  {
    //Catch any spurious interrupts. This really shouldn't ever be called, but there as a safety
    schedule.pTimerDisable(); 
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

void disablePendingFuelSchedule(byte channel)
{
  noInterrupts();
  switch(channel)
  {
    case 0:
      if(fuelSchedule1.Status == PENDING) { fuelSchedule1.Status = OFF; }
      break;
    case 1:
      if(fuelSchedule2.Status == PENDING) { fuelSchedule2.Status = OFF; }
      break;
    case 2: 
      if(fuelSchedule3.Status == PENDING) { fuelSchedule3.Status = OFF; }
      break;
    case 3:
      if(fuelSchedule4.Status == PENDING) { fuelSchedule4.Status = OFF; }
      break;
    case 4:
#if (INJ_CHANNELS >= 5)
      if(fuelSchedule5.Status == PENDING) { fuelSchedule5.Status = OFF; }
#endif
      break;
    case 5:
#if (INJ_CHANNELS >= 6)
      if(fuelSchedule6.Status == PENDING) { fuelSchedule6.Status = OFF; }
#endif
      break;
    case 6:
#if (INJ_CHANNELS >= 7)
      if(fuelSchedule7.Status == PENDING) { fuelSchedule7.Status = OFF; }
#endif
      break;
    case 7:
#if (INJ_CHANNELS >= 8)
      if(fuelSchedule8.Status == PENDING) { fuelSchedule8.Status = OFF; }
#endif
      break;
  }
  interrupts();
}
void disablePendingIgnSchedule(byte channel)
{
  noInterrupts();
  switch(channel)
  {
    case 0:
      if(ignitionSchedule1.Status == PENDING) { ignitionSchedule1.Status = OFF; }
      break;
    case 1:
      if(ignitionSchedule2.Status == PENDING) { ignitionSchedule2.Status = OFF; }
      break;
    case 2: 
      if(ignitionSchedule3.Status == PENDING) { ignitionSchedule3.Status = OFF; }
      break;
    case 3:
      if(ignitionSchedule4.Status == PENDING) { ignitionSchedule4.Status = OFF; }
      break;
    case 4:
      if(ignitionSchedule5.Status == PENDING) { ignitionSchedule5.Status = OFF; }
      break;
#if IGN_CHANNELS >= 6      
    case 6:
      if(ignitionSchedule6.Status == PENDING) { ignitionSchedule6.Status = OFF; }
      break;
#endif
#if IGN_CHANNELS >= 7      
    case 7:
      if(ignitionSchedule7.Status == PENDING) { ignitionSchedule7.Status = OFF; }
      break;
#endif
#if IGN_CHANNELS >= 8      
    case 8:
      if(ignitionSchedule8.Status == PENDING) { ignitionSchedule8.Status = OFF; }
      break;
#endif
  }
  interrupts();
}
