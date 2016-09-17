/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

#include "scheduler.h"
#include "globals.h"

void initialiseSchedulers()
  {
    nullSchedule.Status = OFF;
    
   // Much help in this from http://arduinomega.blogspot.com.au/2011/05/timer2-and-overflow-interrupt-lets-get.html
    //Fuel Schedules, which uses timer 3
    TCCR3B = 0x00;          //Disable Timer3 while we set it up
    TCNT3  = 0;             //Reset Timer Count
    TIFR3  = 0x00;          //Timer3 INT Flag Reg: Clear Timer Overflow Flag
    TCCR3A = 0x00;          //Timer3 Control Reg A: Wave Gen Mode normal
    TCCR3B = (1 << CS12);   //Timer3 Control Reg B: Timer Prescaler set to 256. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg
    //TCCR3B = 0x03;   //Timer3 Control Reg B: Timer Prescaler set to 64. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg
    fuelSchedule1.Status = OFF; 
    fuelSchedule2.Status = OFF;
    fuelSchedule3.Status = OFF;

    fuelSchedule1.schedulesSet = 0;
    fuelSchedule2.schedulesSet = 0;
    fuelSchedule3.schedulesSet = 0;

    //Ignition Schedules, which uses timer 5
    TCCR5B = 0x00;          //Disable Timer3 while we set it up
    TCNT5  = 0;             //Reset Timer Count
    TIFR5  = 0x00;          //Timer5 INT Flag Reg: Clear Timer Overflow Flag
    TCCR5A = 0x00;          //Timer5 Control Reg A: Wave Gen Mode normal
    //TCCR5B = (1 << CS12);   //Timer5 Control Reg B: Timer Prescaler set to 256. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg
    TCCR5B = 0x03;         //aka Divisor = 64 = 490.1Hz
    ignitionSchedule1.Status = OFF;
    ignitionSchedule2.Status = OFF;
    ignitionSchedule3.Status = OFF;

    ignitionSchedule1.schedulesSet = 0;
    ignitionSchedule2.schedulesSet = 0;
    ignitionSchedule3.schedulesSet = 0;
    
    //The remaining Schedules (Schedules 4 for fuel and ignition) use Timer4
    TCCR4B = 0x00;          //Disable Timer4 while we set it up
    TCNT4  = 0;             //Reset Timer Count
    TIFR4  = 0x00;          //Timer4 INT Flag Reg: Clear Timer Overflow Flag
    TCCR4A = 0x00;          //Timer4 Control Reg A: Wave Gen Mode normal
    TCCR4B = (1 << CS12);   //Timer4 Control Reg B: aka Divisor = 256 = 122.5HzTimer Prescaler set to 256. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg 
    ignitionSchedule4.Status = OFF;
    fuelSchedule4.Status = OFF;

    ignitionSchedule4.schedulesSet = 0;
    fuelSchedule4.schedulesSet = 0;
    //Note that timer4 compare channel C is used by the idle control
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
volatile bool flip = 0;
void setFuelSchedule1(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(fuelSchedule1.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    
    fuelSchedule1.StartCallback = startCallback; //Name the start callback function
    fuelSchedule1.EndCallback = endCallback; //Name the end callback function
    fuelSchedule1.duration = duration;

    /*
     * The following must be enclosed in the noIntterupts block to avoid contention caused if the relevant interrupts fires before the state is fully set
     * We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
     * As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
     * unsigned int absoluteTimeout = TCNT3 + (timeout / 16); //Each tick occurs every 16uS with the 256 prescaler, so divide the timeout by 16 to get ther required number of ticks. Add this to the current tick count to get the target time. This will automatically overflow as required
     */
     noInterrupts();
     fuelSchedule1.startCompare = TCNT3 + (timeout >> 4); //As above, but with bit shift instead of / 16
     fuelSchedule1.endCompare = fuelSchedule1.startCompare + (duration >> 4);
     fuelSchedule1.Status = PENDING; //Turn this schedule on
     fuelSchedule1.schedulesSet++; //Increment the number of times this schedule has been set
     if(channel5InjEnabled) { OCR3A = setQueue(timer3Aqueue, &fuelSchedule1, &fuelSchedule5, TCNT3); } //Schedule 1 shares a timer with schedule 5
     else { timer3Aqueue[0] = &fuelSchedule1; timer3Aqueue[1] = &fuelSchedule1; timer3Aqueue[2] = &fuelSchedule1; timer3Aqueue[3] = &fuelSchedule1; OCR3A = fuelSchedule1.startCompare; }
     interrupts();
     TIMSK3 |= (1 << OCIE3A); //Turn on the A compare unit (ie turn on the interrupt)
  }
void setFuelSchedule2(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(fuelSchedule2.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    
    fuelSchedule2.StartCallback = startCallback; //Name the start callback function
    fuelSchedule2.EndCallback = endCallback; //Name the end callback function
    fuelSchedule2.duration = duration;

    /*
     * The following must be enclosed in the noIntterupts block to avoid contention caused if the relevant interrupts fires before the state is fully set
     * We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
     * As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
     * unsigned int absoluteTimeout = TCNT3 + (timeout / 16); //Each tick occurs every 16uS with the 256 prescaler, so divide the timeout by 16 to get ther required number of ticks. Add this to the current tick count to get the target time. This will automatically overflow as required
     */
     noInterrupts();
     fuelSchedule2.startCompare = TCNT3 + (timeout >> 4); //As above, but with bit shift instead of / 16
     fuelSchedule2.endCompare = fuelSchedule2.startCompare + (duration >> 4);
     OCR3B = fuelSchedule2.startCompare; //Use the B copmare unit of timer 3
     fuelSchedule2.Status = PENDING; //Turn this schedule on
     fuelSchedule2.schedulesSet++; //Increment the number of times this schedule has been set
     interrupts();
     TIMSK3 |= (1 << OCIE3B); //Turn on the B compare unit (ie turn on the interrupt)
  }
void setFuelSchedule3(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(fuelSchedule3.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    
    //We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
    //As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
    //unsigned int absoluteTimeout = TCNT3 + (timeout / 16); //Each tick occurs every 16uS with the 256 prescaler, so divide the timeout by 16 to get ther required number of ticks. Add this to the current tick count to get the target time. This will automatically overflow as require
    fuelSchedule3.startCompare = TCNT3 + (timeout >> 4); //As above, but with bit shift instead of / 16
    fuelSchedule3.endCompare = fuelSchedule3.startCompare + (duration >> 4);
    OCR3C = fuelSchedule3.startCompare; //Use the C copmare unit of timer 3
    fuelSchedule3.duration = duration;
    fuelSchedule3.StartCallback = startCallback; //Name the start callback function
    fuelSchedule3.EndCallback = endCallback; //Name the end callback function
    fuelSchedule3.Status = PENDING; //Turn this schedule on
    fuelSchedule3.schedulesSet++; //Increment the number of times this schedule has been set
    TIMSK3 |= (1 << OCIE3C); //Turn on the C compare unit (ie turn on the interrupt)
  }
void setFuelSchedule4(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)()) //Uses timer 4 compare B
  {
    if(fuelSchedule4.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    
    //We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
    //As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
    //unsigned int absoluteTimeout = TCNT4 + (timeout / 4); //Each tick occurs every 4uS with the 128 prescaler, so divide the timeout by 4 to get ther required number of ticks. Add this to the current tick count to get the target time. This will automatically overflow as required
    fuelSchedule4.startCompare = TCNT4 + (timeout >> 4);
    fuelSchedule4.endCompare = fuelSchedule4.startCompare + (duration >> 4);
    OCR4B = fuelSchedule4.startCompare; //Use the C copmare unit of timer 3
    fuelSchedule4.duration = duration;
    fuelSchedule4.StartCallback = startCallback; //Name the start callback function
    fuelSchedule4.EndCallback = endCallback; //Name the end callback function
    fuelSchedule4.Status = PENDING; //Turn this schedule on
    fuelSchedule4.schedulesSet++; //Increment the number of times this schedule has been set
    TIMSK4 |= (1 << OCIE4B); //Turn on the B compare unit (ie turn on the interrupt)
  }
void setFuelSchedule5(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(fuelSchedule5.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    
    //We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
    //As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
    //unsigned int absoluteTimeout = TCNT3 + (timeout / 16); //Each tick occurs every 16uS with the 256 prescaler, so divide the timeout by 16 to get ther required number of ticks. Add this to the current tick count to get the target time. This will automatically overflow as required
    fuelSchedule5.StartCallback = startCallback; //Name the start callback function
    fuelSchedule5.EndCallback = endCallback; //Name the end callback function
    fuelSchedule5.duration = duration;

    /*
     * The following must be enclosed in the noIntterupts block to avoid contention caused if the relevant interrupts fires before the state is fully set
     */
    noInterrupts();
    fuelSchedule5.startCompare = TCNT3 + (timeout >> 4); //As above, but with bit shift instead of / 16
    fuelSchedule5.endCompare = fuelSchedule5.startCompare + (duration >> 4);
    fuelSchedule5.Status = PENDING; //Turn this schedule on
    fuelSchedule5.schedulesSet++; //Increment the number of times this schedule has been set
    OCR3A = setQueue(timer3Aqueue, &fuelSchedule1, &fuelSchedule5, TCNT3); //Schedule 1 shares a timer with schedule 5
    interrupts();
    TIMSK3 |= (1 << OCIE3A); //Turn on the A compare unit (ie turn on the interrupt)
  }
//Ignition schedulers use Timer 5
void setIgnitionSchedule1(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(ignitionSchedule1.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    
    //As the timer is ticking every 4uS (Time per Tick = (Prescale)*(1/Frequency)) 
    if (timeout > 262140) { timeout = 262100; } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when appliedcausing erratic behaviour such as erroneous sparking. 
    OCR5A = TCNT5 + (timeout >> 2); //As there is a tick every 4uS, there are timeout/4 ticks until the interrupt should be triggered ( >>2 divides by 4)
    
    ignitionSchedule1.duration = duration;
    ignitionSchedule1.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule1.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule1.Status = PENDING; //Turn this schedule on
    TIMSK5 |= (1 << OCIE5A); //Turn on the A compare unit (ie turn on the interrupt)
  }
void setIgnitionSchedule2(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(ignitionSchedule2.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    
    //As the timer is ticking every 4uS (Time per Tick = (Prescale)*(1/Frequency)) 
    if (timeout > 262140) { timeout = 262100; } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking. This must be set slightly lower than the max of 262140 to avoid strangeness
    OCR5B = TCNT5 + (timeout >> 2); //As there is a tick every 4uS, there are timeout/4 ticks until the interrupt should be triggered ( >>2 divides by 4)
    
    ignitionSchedule2.duration = duration;
    ignitionSchedule2.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule2.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule2.Status = PENDING; //Turn this schedule on
    TIMSK5 |= (1 << OCIE5B); //Turn on the B compare unit (ie turn on the interrupt)
  }
void setIgnitionSchedule3(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(ignitionSchedule3.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    
    //The timer is ticking every 4uS (Time per Tick = (Prescale)*(1/Frequency)) 
    if (timeout > 262140) { timeout = 262100; } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking. This must be set slightly lower than the max of 262140 to avoid strangeness
    OCR5C = TCNT5 + (timeout >> 2); //As there is a tick every 4uS, there are timeout/4 ticks until the interrupt should be triggered ( >>2 divides by 4)
    
    ignitionSchedule3.duration = duration;
    ignitionSchedule3.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule3.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule3.Status = PENDING; //Turn this schedule on
    TIMSK5 |= (1 << OCIE5C); //Turn on the C compare unit (ie turn on the interrupt)
  }
void setIgnitionSchedule4(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(ignitionSchedule4.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    
    //We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
    //The timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency))
    //Note this is different to the other ignition timers
    unsigned int absoluteTimeout = TCNT4 + (timeout >> 4); //As above, but with bit shift instead of / 16
    
    OCR4A = absoluteTimeout;
    ignitionSchedule4.duration = duration;
    ignitionSchedule4.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule4.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule4.Status = PENDING; //Turn this schedule on
    TIMSK4 |= (1 << OCIE4A); //Turn on the A compare unit (ie turn on the interrupt)
  }
void setIgnitionSchedule5(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    return;
    if(ignitionSchedule1.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    
    //As the timer is ticking every 4uS (Time per Tick = (Prescale)*(1/Frequency)) 
    if (timeout > 262140) { timeout = 262100; } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking. This must be set slightly lower than the max of 262140 to avoid strangeness
    OCR5A = TCNT5 + (timeout >> 2); //As there is a tick every 4uS, there are timeout/4 ticks until the interrupt should be triggered ( >>2 divides by 4)
    
    ignitionSchedule5.duration = duration;
    ignitionSchedule5.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule5.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule5.Status = PENDING; //Turn this schedule on
    TIMSK5 |= (1 << OCIE5A); //Turn on the A compare unit (ie turn on the interrupt)
  }
  
/*******************************************************************************************************************************************************************************************************/
//This function (All 8 ISR functions that are below) gets called when either the start time or the duration time are reached
//This calls the relevant callback function (startCallback or endCallback) depending on the status of the schedule.
//If the startCallback function is called, we put the scheduler into RUNNING state
//Timer3A (fuel schedule 1) Compare Vector
ISR(TIMER3_COMPA_vect, ISR_NOBLOCK) //fuelSchedules 1 and 5
  {
    if (timer3Aqueue[0]->Status == OFF) { TIMSK3 &= ~(1 << OCIE3A); return; } //Safety check. Turn off this output compare unit and return without performing any action
    if (timer3Aqueue[0]->Status == PENDING) //Check to see if this schedule is turn on
    {
      timer3Aqueue[0]->StartCallback();
      timer3Aqueue[0]->Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      OCR3A = popQueue(timer3Aqueue);
    }
    else if (timer3Aqueue[0]->Status == RUNNING)
    {
       timer3Aqueue[0]->EndCallback();
       timer3Aqueue[0]->Status = OFF; //Turn off the schedule
       timer3Aqueue[0]->schedulesSet = 0;
       OCR3A = popQueue(timer3Aqueue);
    }
  }

ISR(TIMER3_COMPB_vect, ISR_NOBLOCK) //fuelSchedule2
  {
    if (fuelSchedule2.Status == PENDING) //Check to see if this schedule is turn on
    {
      fuelSchedule2.StartCallback();
      fuelSchedule2.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      OCR3B = fuelSchedule2.endCompare;
    }
    else if (fuelSchedule2.Status == RUNNING)
    {
       fuelSchedule2.EndCallback();
       fuelSchedule2.Status = OFF; //Turn off the schedule
       fuelSchedule2.schedulesSet = 0;
       TIMSK3 &= ~(1 << OCIE3B); //Turn off this output compare unit (This simply writes 0 to the OCIE3A bit of TIMSK3)
    }
  }

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //AVR chips use the ISR for this
ISR(TIMER3_COMPC_vect, ISR_NOBLOCK) //fuelSchedule3
#elif defined (CORE_TEENSY) && defined (__MK20DX256__)
void timer3compareCinterrupt() //Most ARM chips can simply call a function
#endif
  {
    if (fuelSchedule3.Status == PENDING) //Check to see if this schedule is turn on
    {
      fuelSchedule3.StartCallback();
      fuelSchedule3.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      OCR3C = fuelSchedule3.endCompare;
    }
    else if (fuelSchedule3.Status == RUNNING)
    {
       fuelSchedule3.EndCallback();
       fuelSchedule3.Status = OFF; //Turn off the schedule
       fuelSchedule3.schedulesSet = 0;
       TIMSK3 &= ~(1 << OCIE3C); //Turn off this output compare unit (This simply writes 0 to the OCIE3A bit of TIMSK3)
    }
  }
  
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //AVR chips use the ISR for this
ISR(TIMER4_COMPB_vect, ISR_NOBLOCK) //fuelSchedule4
#elif defined (CORE_TEENSY) && defined (__MK20DX256__)
void timer4compareBinterrupt() //Most ARM chips can simply call a function
#endif
  {
    if (fuelSchedule4.Status == PENDING) //Check to see if this schedule is turn on
    {
      fuelSchedule4.StartCallback();
      fuelSchedule4.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      OCR4B = fuelSchedule4.endCompare;
    }
    else if (fuelSchedule4.Status == RUNNING)
    {
       fuelSchedule4.EndCallback();
       fuelSchedule4.Status = OFF; //Turn off the schedule
       fuelSchedule4.schedulesSet = 0;
       TIMSK4 &= ~(1 << OCIE4B); //Turn off this output compare unit (This simply writes 0 to the OCIE3A bit of TIMSK3)
    }
  }
  
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //AVR chips use the ISR for this
ISR(TIMER5_COMPA_vect, ISR_NOBLOCK) //ignitionSchedule1
#elif defined (CORE_TEENSY) && defined (__MK20DX256__)
void timer5compareAinterrupt() //Most ARM chips can simply call a function
#endif
  {
    if (ignitionSchedule1.Status == PENDING) //Check to see if this schedule is turn on
    {
      //if ( ign1LastRev == startRevolutions ) { return; }
      ignitionSchedule1.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      ignitionSchedule1.startTime = micros();
      ignitionSchedule1.StartCallback();
      ign1LastRev = startRevolutions;
      OCR5A = TCNT5 + (ignitionSchedule1.duration >> 2); //Divide by 4
    }
    else if (ignitionSchedule1.Status == RUNNING)
    {
      ignitionSchedule1.Status = OFF; //Turn off the schedule
      ignitionSchedule1.EndCallback();
      ignitionCount += 1; //Increment the igintion counter
      TIMSK5 &= ~(1 << OCIE5A); //Turn off this output compare unit
    }
  }
  
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //AVR chips use the ISR for this
ISR(TIMER5_COMPB_vect, ISR_NOBLOCK) //ignitionSchedule2
#elif defined (CORE_TEENSY) && defined (__MK20DX256__)
void timer5compareBinterrupt() //Most ARM chips can simply call a function
#endif
  {
    if (ignitionSchedule2.Status == PENDING) //Check to see if this schedule is turn on
    {
      //if ( ign2LastRev == startRevolutions ) { return; }
      ignitionSchedule2.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      ignitionSchedule2.startTime = micros();
      ignitionSchedule2.StartCallback();
      ign2LastRev = startRevolutions;
      OCR5B = TCNT5 + (ignitionSchedule2.duration >> 2);
    }
    else if (ignitionSchedule2.Status == RUNNING)
    {
      ignitionSchedule2.Status = OFF; //Turn off the schedule
      ignitionSchedule2.EndCallback();
      ignitionCount += 1; //Increment the igintion counter
      TIMSK5 &= ~(1 << OCIE5B); //Turn off this output compare unit (This simply writes 0 to the OCIE3A bit of TIMSK3)
    }
  }
  
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //AVR chips use the ISR for this
ISR(TIMER5_COMPC_vect, ISR_NOBLOCK) //ignitionSchedule3
#elif defined (CORE_TEENSY) && defined (__MK20DX256__)
void timer5compareCinterrupt() //Most ARM chips can simply call a function
#endif
  {
    if (ignitionSchedule3.Status == PENDING) //Check to see if this schedule is turn on
    {
      //if ( ign3LastRev == startRevolutions ) { return; }
      ignitionSchedule3.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      ignitionSchedule3.startTime = micros();
      ignitionSchedule3.StartCallback();
      ign3LastRev = startRevolutions;
      OCR5C = TCNT5 + (ignitionSchedule3.duration >> 2);
    }
    else if (ignitionSchedule3.Status == RUNNING)
    {
       ignitionSchedule3.Status = OFF; //Turn off the schedule
       ignitionSchedule3.EndCallback();
       ignitionCount += 1; //Increment the igintion counter
       TIMSK5 &= ~(1 << OCIE5C); //Turn off this output compare unit (This simply writes 0 to the OCIE3A bit of TIMSK3)
    }
  }
  
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //AVR chips use the ISR for this
ISR(TIMER4_COMPA_vect, ISR_NOBLOCK) //ignitionSchedule4
#elif defined (CORE_TEENSY) && defined (__MK20DX256__)
void timer4compareAinterrupt() //Most ARM chips can simply call a function
#endif
  {
    if (ignitionSchedule4.Status == PENDING) //Check to see if this schedule is turn on
    {
      //if ( ign4LastRev == startRevolutions ) { return; }
      ignitionSchedule4.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      ignitionSchedule4.startTime = micros();
      ignitionSchedule4.StartCallback();
      ign4LastRev = startRevolutions;
      OCR4A = TCNT4 + (ignitionSchedule4.duration >> 4); //Divide by 16
    }
    else if (ignitionSchedule4.Status == RUNNING)
    {
       ignitionSchedule4.Status = OFF; //Turn off the schedule
       ignitionSchedule4.EndCallback();
       ignitionCount += 1; //Increment the igintion counter
       TIMSK4 &= ~(1 << OCIE4A); //Turn off this output compare unit (This simply writes 0 to the OCIE4A bit of TIMSK4)
    }
  }
