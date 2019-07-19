/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

#include "globals.h"
#include "scheduler.h"
#include "scheduledIO.h"


void initialiseSchedulers()
{
    // Are the four lanes below needed?
    ignitionSchedule1.enable();
    ignitionSchedule2.enable();
    ignitionSchedule3.enable();
    ignitionSchedule4.enable();

    if (configPage2.injLayout == INJ_SEMISEQUENTIAL)
    {
      fuelSchedule1.StartCallback = openInjector1and4;
      fuelSchedule1.EndCallback   = closeInjector1and4;
      fuelSchedule2.StartCallback = openInjector2and3;
      fuelSchedule2.EndCallback   = closeInjector2and3;
    }
    else
    {
      fuelSchedule1.StartCallback = openInjector1;
      fuelSchedule1.EndCallback   = closeInjector1;
      fuelSchedule2.StartCallback = openInjector2;
      fuelSchedule2.EndCallback   = closeInjector2;
    }

    fuelSchedule3.StartCallback = openInjector3;
    fuelSchedule3.EndCallback   = closeInjector3;
    fuelSchedule4.StartCallback = openInjector4;
    fuelSchedule4.EndCallback   = closeInjector4;

    //Note the hacky use of fuel schedule 3 below
    fuelSchedule5.StartCallback = openInjector3and5;
    fuelSchedule5.EndCallback   = closeInjector3and5;

    fuelSchedule6.StartCallback = openInjector6;
    fuelSchedule6.EndCallback   = closeInjector6;
    fuelSchedule7.StartCallback = openInjector7;
    fuelSchedule7.EndCallback   = closeInjector7;
    fuelSchedule8.StartCallback = openInjector8;
    fuelSchedule8.EndCallback   = closeInjector8;
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

//Experimental new generic function
void Schedule::setSchedule(uint32_t _timeout, uint32_t _duration)
{
  if(Status != Schedule::RUNNING) //Check that we're not already part way through a schedule
  {
    duration = uS_TO_TIMER_COMPARE(_duration);

    //Need to check that the timeout doesn't exceed the overflow
    uint16_t timeout_timer_compare;
    if (_timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); } // If the timeout is >16x (Each tick represents 16uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when appliedcausing erratic behaviour such as erroneous sparking.
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(_timeout); } //Normal case

    //The following must be enclosed in the noInterupts block to avoid contention caused if the relevant interrupt fires before the state is fully set
    noInterrupts();
    startCompare = timer.counter + timeout_timer_compare;
    Status = Schedule::PENDING; //Turn this schedule on
    schedulesSet++; //Increment the number of times this schedule has been set
    timer.compare = startCompare;
    interrupts();
    enable();
  }
  else
  {
    //If the schedule is already running, we can set the next schedule so it is ready to go
    //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
    nextStartCompare = timer.counter + uS_TO_TIMER_COMPARE(_timeout);
    nextDuration = uS_TO_TIMER_COMPARE(_duration);
    hasNextSchedule = true;
  }
}


static inline void refreshIgnitionSchedule1(unsigned long timeToEnd)
{
  if( (ignitionSchedule1.Status == Schedule::RUNNING) && (timeToEnd < ignitionSchedule1.duration) )
  //Must have the threshold check here otherwise it can cause a condition where the compare fires twice, once after the other, both for the end
  //if( (timeToEnd < ignitionSchedule1.duration) && (timeToEnd > IGNITION_REFRESH_THRESHOLD) )
  {
    noInterrupts();
    ignitionSchedule1.endCompare = ignitionSchedule1.timer.counter + uS_TO_TIMER_COMPARE(timeToEnd);
    ignitionSchedule1.timer.compare = ignitionSchedule1.endCompare;
    interrupts();
  }
}

static inline void fuelScheduleInterrupt(Schedule& fuelSchedule)
{
  switch(fuelSchedule.Status)
  {
  case Schedule::PENDING: //Check to see if this schedule is turn on
    if (fuelSchedule.StartCallback) { fuelSchedule.StartCallback(); }
    fuelSchedule.Status = Schedule::RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
    fuelSchedule.timer.compare = fuelSchedule.timer.counter + fuelSchedule.duration; //Doing this here prevents a potential overflow on restarts
    break;

  case Schedule::RUNNING:
    if (fuelSchedule.EndCallback) { fuelSchedule.EndCallback(); }
    fuelSchedule.Status = Schedule::OFF; //Turn off the schedule
    fuelSchedule.schedulesSet = 0;

    //If there is a next schedule queued up, activate it
    if(fuelSchedule.hasNextSchedule == true)
    {
      fuelSchedule.timer.compare = fuelSchedule.nextStartCompare;
      fuelSchedule.duration = fuelSchedule.nextDuration;
      fuelSchedule.Status = Schedule::PENDING;
      fuelSchedule.schedulesSet = 1;
      fuelSchedule.hasNextSchedule = false;
    }
    else { fuelSchedule.disable(); }
    break;

  default:
    //Safety check. Turn off this output compare unit and return without performing any action
    fuelSchedule.disable();
    break;
  }
}

static inline void ignitionScheduleInterrupt(Schedule& ignitionSchedule)
{
  switch (ignitionSchedule.Status)
  {
  case Schedule::PENDING: //Check to see if this schedule is turn on
    if (ignitionSchedule.StartCallback) { ignitionSchedule.StartCallback(); }
    ignitionSchedule.Status = Schedule::RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
    ignitionSchedule.startTime = micros();
    if(ignitionSchedule.endScheduleSetByDecoder == true) { ignitionSchedule.timer.compare = ignitionSchedule.endCompare; }
    else { ignitionSchedule.timer.compare = ignitionSchedule.timer.counter + ignitionSchedule.duration; } //Doing this here prevents a potential overflow on restarts
    break;

  case Schedule::RUNNING:
    if (ignitionSchedule.EndCallback) { ignitionSchedule.EndCallback(); }
    ignitionSchedule.Status = Schedule::OFF; //Turn off the schedule
    ignitionSchedule.schedulesSet = 0;
    ignitionSchedule.hasNextSchedule = false;
    ignitionSchedule.endScheduleSetByDecoder = false;
    ignitionCount += 1; //Increment the igintion counter
    ignitionSchedule.disable();
    break;

  default:
    //Catch any spurious interrupts. This really shouldn't ever be called, but there as a safety
    ignitionSchedule.disable();
    break;
  }
}

/*******************************************************************************************************************************************************************************************************/
//This function (All 8 ISR functions that are below) gets called when either the start time or the duration time are reached
//This calls the relevant callback function (startCallback or endCallback) depending on the status of the schedule.
//If the startCallback function is called, we put the scheduler into RUNNING state
//Timer3A (fuel schedule 1) Compare Vector
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
ISR(TIMER3_COMPA_vect) //fuelSchedules 1 and 5
#else
static inline void fuelSchedule1Interrupt() //Most ARM chips can simply call a function
#endif
  {
    fuelScheduleInterrupt(fuelSchedule1);
  }

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
ISR(TIMER3_COMPB_vect) //fuelSchedule2
#else
static inline void fuelSchedule2Interrupt() //Most ARM chips can simply call a function
#endif
  {
    fuelScheduleInterrupt(fuelSchedule2);
  }

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
ISR(TIMER3_COMPC_vect) //fuelSchedule3
#else
static inline void fuelSchedule3Interrupt() //Most ARM chips can simply call a function
#endif
  {
    fuelScheduleInterrupt(fuelSchedule3);
  }

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
ISR(TIMER4_COMPB_vect) //fuelSchedule4
#else
static inline void fuelSchedule4Interrupt() //Most ARM chips can simply call a function
#endif
  {
    fuelScheduleInterrupt(fuelSchedule4);
  }

#if (INJ_CHANNELS >= 5)
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER1_COMPC_vect) //fuelSchedule5
#else
static inline void fuelSchedule5Interrupt() //Most ARM chips can simply call a function
#endif
{
    fuelScheduleInterrupt(fuelSchedule5);
}
#endif

#if (INJ_CHANNELS >= 6)
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER4_COMPA_vect) //fuelSchedule6
#else
static inline void fuelSchedule6Interrupt() //Most ARM chips can simply call a function
#endif
{
    fuelScheduleInterrupt(fuelSchedule6);
}
#endif

#if (INJ_CHANNELS >= 7)
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER5_COMPC_vect) //fuelSchedule7
#else
static inline void fuelSchedule7Interrupt() //Most ARM chips can simply call a function
#endif
{
    fuelScheduleInterrupt(fuelSchedule7);
}
#endif

#if (INJ_CHANNELS >= 8)
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER5_COMPB_vect) //fuelSchedule8
#else
static inline void fuelSchedule8Interrupt() //Most ARM chips can simply call a function
#endif
{
    fuelScheduleInterrupt(fuelSchedule8);
}
#endif

#if IGN_CHANNELS >= 1
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER5_COMPA_vect) //ignitionSchedule1
#else
static inline void ignitionSchedule1Interrupt() //Most ARM chips can simply call a function
#endif
  {
    ignitionScheduleInterrupt(ignitionSchedule1);
  }
#endif

#if IGN_CHANNELS >= 2
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER5_COMPB_vect) //ignitionSchedule2
#else
static inline void ignitionSchedule2Interrupt() //Most ARM chips can simply call a function
#endif
  {
    ignitionScheduleInterrupt(ignitionSchedule2);
  }
#endif

#if IGN_CHANNELS >= 3
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER5_COMPC_vect) //ignitionSchedule3
#else
static inline void ignitionSchedule3Interrupt() //Most ARM chips can simply call a function
#endif
  {
    ignitionScheduleInterrupt(ignitionSchedule3);
  }
#endif

#if IGN_CHANNELS >= 4
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER4_COMPA_vect) //ignitionSchedule4
#else
static inline void ignitionSchedule4Interrupt() //Most ARM chips can simply call a function
#endif
  {
    ignitionScheduleInterrupt(ignitionSchedule4);
  }
#endif

#if IGN_CHANNELS >= 5
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER1_COMPC_vect) //ignitionSchedule5
#else
static inline void ignitionSchedule5Interrupt() //Most ARM chips can simply call a function
#endif
  {
    ignitionScheduleInterrupt(ignitionSchedule5);
  }
#endif

#if IGN_CHANNELS >= 6
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER1_COMPC_vect) //ignitionSchedule6  NOT CORRECT!!!
#else
static inline void ignitionSchedule6Interrupt() //Most ARM chips can simply call a function
#endif
  {
    ignitionScheduleInterrupt(ignitionSchedule6);
  }
#endif

#if IGN_CHANNELS >= 7
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER1_COMPC_vect) //ignitionSchedule6  NOT CORRECT!!!
#else
static inline void ignitionSchedule7Interrupt() //Most ARM chips can simply call a function
#endif
  {
    ignitionScheduleInterrupt(ignitionSchedule7);
  }
#endif

#if IGN_CHANNELS >= 8
#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER1_COMPC_vect) //ignitionSchedule8  NOT CORRECT!!!
#else
static inline void ignitionSchedule8Interrupt() //Most ARM chips can simply call a function
#endif
  {
    ignitionScheduleInterrupt(ignitionSchedule8);
  }
#endif


#if defined(CORE_TEENSY)
void ftm0_isr(void)
{
  //Use separate variables for each test to ensure conversion to bool
  bool interrupt1 = (FTM0_C0SC & FTM_CSC_CHF);
  bool interrupt2 = (FTM0_C1SC & FTM_CSC_CHF);
  bool interrupt3 = (FTM0_C2SC & FTM_CSC_CHF);
  bool interrupt4 = (FTM0_C3SC & FTM_CSC_CHF);
  bool interrupt5 = (FTM0_C4SC & FTM_CSC_CHF);
  bool interrupt6 = (FTM0_C5SC & FTM_CSC_CHF);
  bool interrupt7 = (FTM0_C6SC & FTM_CSC_CHF);
  bool interrupt8 = (FTM0_C7SC & FTM_CSC_CHF);

  if(interrupt1) { FTM0_C0SC &= ~FTM_CSC_CHF; fuelSchedule1Interrupt(); }
  else if(interrupt2) { FTM0_C1SC &= ~FTM_CSC_CHF; fuelSchedule2Interrupt(); }
  else if(interrupt3) { FTM0_C2SC &= ~FTM_CSC_CHF; fuelSchedule3Interrupt(); }
  else if(interrupt4) { FTM0_C3SC &= ~FTM_CSC_CHF; fuelSchedule4Interrupt(); }
  else if(interrupt5) { FTM0_C4SC &= ~FTM_CSC_CHF; ignitionSchedule1Interrupt(); }
  else if(interrupt6) { FTM0_C5SC &= ~FTM_CSC_CHF; ignitionSchedule2Interrupt(); }
  else if(interrupt7) { FTM0_C6SC &= ~FTM_CSC_CHF; ignitionSchedule3Interrupt(); }
  else if(interrupt8) { FTM0_C7SC &= ~FTM_CSC_CHF; ignitionSchedule4Interrupt(); }

}
void ftm3_isr(void)
{

#if (INJ_CHANNELS >= 5)
  bool interrupt1 = (FTM3_C0SC & FTM_CSC_CHF);
  if(interrupt1) { FTM3_C0SC &= ~FTM_CSC_CHF; fuelSchedule5Interrupt(); }
#endif
#if (INJ_CHANNELS >= 6)
  bool interrupt2 = (FTM3_C1SC & FTM_CSC_CHF);
  if(interrupt2) { FTM3_C1SC &= ~FTM_CSC_CHF; fuelSchedule6Interrupt(); }
#endif
#if (INJ_CHANNELS >= 7)
  bool interrupt3 = (FTM3_C2SC & FTM_CSC_CHF);
  if(interrupt3) { FTM3_C2SC &= ~FTM_CSC_CHF; fuelSchedule7Interrupt(); }
#endif
#if (INJ_CHANNELS >= 8)
  bool interrupt4 = (FTM3_C3SC & FTM_CSC_CHF);
  if(interrupt4) { FTM3_C3SC &= ~FTM_CSC_CHF; fuelSchedule8Interrupt(); }
#endif
#if (IGN_CHANNELS >= 5)
  bool interrupt5 = (FTM3_C4SC & FTM_CSC_CHF);
  if(interrupt5) { FTM3_C4SC &= ~FTM_CSC_CHF; ignitionSchedule5Interrupt(); }
#endif
#if (IGN_CHANNELS >= 6)
  bool interrupt6 = (FTM3_C5SC & FTM_CSC_CHF);
  if(interrupt6) { FTM3_C5SC &= ~FTM_CSC_CHF; ignitionSchedule6Interrupt(); }
#endif
#if (IGN_CHANNELS >= 7)
  bool interrupt7 = (FTM3_C6SC & FTM_CSC_CHF);
  if(interrupt7) { FTM3_C6SC &= ~FTM_CSC_CHF; ignitionSchedule7Interrupt(); }
#endif
#if (IGN_CHANNELS >= 8)
  bool interrupt8 = (FTM3_C7SC & FTM_CSC_CHF);
  if(interrupt8) { FTM3_C7SC &= ~FTM_CSC_CHF; ignitionSchedule8Interrupt(); }
#endif

}
#endif
