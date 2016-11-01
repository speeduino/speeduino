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

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
   // Much help in this from http://arduinomega.blogspot.com.au/2011/05/timer2-and-overflow-interrupt-lets-get.html
    //Fuel Schedules, which uses timer 3
    TCCR3B = 0x00;          //Disable Timer3 while we set it up
    TCNT3  = 0;             //Reset Timer Count
    TIFR3  = 0x00;          //Timer3 INT Flag Reg: Clear Timer Overflow Flag
    TCCR3A = 0x00;          //Timer3 Control Reg A: Wave Gen Mode normal
    TCCR3B = (1 << CS12);   //Timer3 Control Reg B: Timer Prescaler set to 256. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg
    //TCCR3B = 0x03;   //Timer3 Control Reg B: Timer Prescaler set to 64. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg

    //Ignition Schedules, which uses timer 5
    TCCR5B = 0x00;          //Disable Timer5 while we set it up
    TCNT5  = 0;             //Reset Timer Count
    TIFR5  = 0x00;          //Timer5 INT Flag Reg: Clear Timer Overflow Flag
    TCCR5A = 0x00;          //Timer5 Control Reg A: Wave Gen Mode normal
    //TCCR5B = (1 << CS12);   //Timer5 Control Reg B: Timer Prescaler set to 256. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg
    TCCR5B = 0x03;         //aka Divisor = 64 = 490.1Hz

    //The remaining Schedules (Schedules 4 for fuel and ignition) use Timer4
    TCCR4B = 0x00;          //Disable Timer4 while we set it up
    TCNT4  = 0;             //Reset Timer Count
    TIFR4  = 0x00;          //Timer4 INT Flag Reg: Clear Timer Overflow Flag
    TCCR4A = 0x00;          //Timer4 Control Reg A: Wave Gen Mode normal
    TCCR4B = (1 << CS12);   //Timer4 Control Reg B: aka Divisor = 256 = 122.5HzTimer Prescaler set to 256. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg 
    
#elif defined (CORE_TEENSY) && defined (__MK20DX256__)

//Configure ARM timers here
    FTM0_MODE |= FTM_MODE_WPDIS;  // Write Protection Disable
    FTM0_MODE |= FTM_MODE_FTMEN;  // Unrestricted FTM mode
    FTM0_SC   |= FTM_SC_TOIE;     // enable Overflow Interrupt
  
    // enable the clock for FTM0
    FTM0_SC |= FTM_SC_CLKS(0b10);
    // 00 No clock selected. This in effect disables the FTM counter.
    // 01 System clock
    // 10 Fixed frequency clock
    // 11 External clock

    // set Prescaler 
    //FTM0_SC |= FTM_SC_PS(0b111);
    FTM0_SC |= 0b000;
    // 000 Divide by 1
    // 001 Divide by 2
    // 010 Divide by 4
    // 011 Divide by 8
    // 100 Divide by 16
    // 101 Divide by 32
    // 110 Divide by 64
    // 111 Divide by 128
  
    // set the counter initial value
    FTM0_CNT = 0;
  
    // enable the clock for FTM0
    SIM_SCGC6 |= SIM_SCGC6_FTM0;
  
    // enable IRQ Interrupt
    NVIC_ENABLE_IRQ(IRQ_FTM0);
  
    FTM0_FMS |= FTM0_WPEN;
#endif

    
    fuelSchedule1.Status = OFF; 
    fuelSchedule2.Status = OFF;
    fuelSchedule3.Status = OFF;
    fuelSchedule4.Status = OFF;
    fuelSchedule5.Status = OFF;

    fuelSchedule1.schedulesSet = 0;
    fuelSchedule2.schedulesSet = 0;
    fuelSchedule3.schedulesSet = 0;
    fuelSchedule4.schedulesSet = 0;
    fuelSchedule5.schedulesSet = 0;

    ignitionSchedule1.Status = OFF;
    ignitionSchedule2.Status = OFF;
    ignitionSchedule3.Status = OFF;
    ignitionSchedule4.Status = OFF;
    
    ignitionSchedule1.schedulesSet = 0;
    ignitionSchedule2.schedulesSet = 0;
    ignitionSchedule3.schedulesSet = 0;
    ignitionSchedule4.schedulesSet = 0;
    
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
     fuelSchedule1.startCompare = FUEL1_COUNTER + (timeout >> 4); //As above, but with bit shift instead of / 16
     fuelSchedule1.endCompare = fuelSchedule1.startCompare + (duration >> 4);
     fuelSchedule1.Status = PENDING; //Turn this schedule on
     fuelSchedule1.schedulesSet++; //Increment the number of times this schedule has been set
     if(channel5InjEnabled) { FUEL1_COMPARE = setQueue(timer3Aqueue, &fuelSchedule1, &fuelSchedule5, FUEL1_COUNTER); } //Schedule 1 shares a timer with schedule 5
     else { timer3Aqueue[0] = &fuelSchedule1; timer3Aqueue[1] = &fuelSchedule1; timer3Aqueue[2] = &fuelSchedule1; timer3Aqueue[3] = &fuelSchedule1; FUEL1_COMPARE = fuelSchedule1.startCompare; }
     interrupts();
     FUEL1_TIMER_ENABLE();
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
     fuelSchedule2.startCompare = FUEL2_COUNTER + (timeout >> 4); //As above, but with bit shift instead of / 16
     fuelSchedule2.endCompare = fuelSchedule2.startCompare + (duration >> 4);
     FUEL2_COMPARE = fuelSchedule2.startCompare; //Use the B copmare unit of timer 3
     fuelSchedule2.Status = PENDING; //Turn this schedule on
     fuelSchedule2.schedulesSet++; //Increment the number of times this schedule has been set
     interrupts();
     FUEL2_TIMER_ENABLE();
  }
void setFuelSchedule3(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(fuelSchedule3.Status == RUNNING) { return; } //Check that we're not already part way through a schedule

    fuelSchedule3.StartCallback = startCallback; //Name the start callback function
    fuelSchedule3.EndCallback = endCallback; //Name the end callback function
    fuelSchedule3.duration = duration;
    
    /*
     * The following must be enclosed in the noIntterupts block to avoid contention caused if the relevant interrupts fires before the state is fully set
     * We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
     * As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
     * unsigned int absoluteTimeout = TCNT3 + (timeout / 16); //Each tick occurs every 16uS with the 256 prescaler, so divide the timeout by 16 to get ther required number of ticks. Add this to the current tick count to get the target time. This will automatically overflow as required
     */
    noInterrupts();
    fuelSchedule3.startCompare = FUEL3_COUNTER + (timeout >> 4); //As above, but with bit shift instead of / 16
    fuelSchedule3.endCompare = fuelSchedule3.startCompare + (duration >> 4);
    FUEL3_COMPARE = fuelSchedule3.startCompare; //Use the C copmare unit of timer 3
    fuelSchedule3.Status = PENDING; //Turn this schedule on
    fuelSchedule3.schedulesSet++; //Increment the number of times this schedule has been set
    interrupts();
    FUEL3_TIMER_ENABLE();
  }
void setFuelSchedule4(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)()) //Uses timer 4 compare B
  {
    if(fuelSchedule4.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
        
    fuelSchedule4.StartCallback = startCallback; //Name the start callback function
    fuelSchedule4.EndCallback = endCallback; //Name the end callback function
    fuelSchedule4.duration = duration;
    
    /*
     * The following must be enclosed in the noIntterupts block to avoid contention caused if the relevant interrupts fires before the state is fully set
     * We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
     * As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
     * unsigned int absoluteTimeout = TCNT3 + (timeout / 16); //Each tick occurs every 16uS with the 256 prescaler, so divide the timeout by 16 to get ther required number of ticks. Add this to the current tick count to get the target time. This will automatically overflow as required
     */
    noInterrupts();
    fuelSchedule4.startCompare = FUEL4_COUNTER + (timeout >> 4);
    fuelSchedule4.endCompare = fuelSchedule4.startCompare + (duration >> 4);
    FUEL4_COMPARE = fuelSchedule4.startCompare; //Use the C copmare unit of timer 3
    fuelSchedule4.Status = PENDING; //Turn this schedule on
    fuelSchedule4.schedulesSet++; //Increment the number of times this schedule has been set
    interrupts();
    FUEL4_TIMER_ENABLE();
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
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
    noInterrupts();
    fuelSchedule5.startCompare = TCNT3 + (timeout >> 4); //As above, but with bit shift instead of / 16
    fuelSchedule5.endCompare = fuelSchedule5.startCompare + (duration >> 4);
    fuelSchedule5.Status = PENDING; //Turn this schedule on
    fuelSchedule5.schedulesSet++; //Increment the number of times this schedule has been set
    OCR3A = setQueue(timer3Aqueue, &fuelSchedule1, &fuelSchedule5, TCNT3); //Schedule 1 shares a timer with schedule 5
    interrupts();
    TIMSK3 |= (1 << OCIE3A); //Turn on the A compare unit (ie turn on the interrupt)
#endif
  }
//Ignition schedulers use Timer 5
void setIgnitionSchedule1(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(ignitionSchedule1.Status == RUNNING) { return; } //Check that we're not already part way through a schedule

    ignitionSchedule1.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule1.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule1.duration = duration;
    
    //As the timer is ticking every 4uS (Time per Tick = (Prescale)*(1/Frequency)) 
    if (timeout > 262140) { timeout = 262100; } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when appliedcausing erratic behaviour such as erroneous sparking. 
    
    noInterrupts();
    ignitionSchedule1.startCompare = IGN1_COUNTER + (timeout >> 2); //As there is a tick every 4uS, there are timeout/4 ticks until the interrupt should be triggered ( >>2 divides by 4)
    ignitionSchedule1.endCompare = ignitionSchedule1.startCompare + (duration >> 2);
    IGN1_COMPARE = ignitionSchedule1.startCompare;
    ignitionSchedule1.Status = PENDING; //Turn this schedule on
    interrupts();
    IGN1_TIMER_ENABLE();
  }
void setIgnitionSchedule2(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(ignitionSchedule2.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    
    ignitionSchedule2.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule2.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule2.duration = duration;
    
    //As the timer is ticking every 4uS (Time per Tick = (Prescale)*(1/Frequency)) 
    if (timeout > 262140) { timeout = 262100; } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking. This must be set slightly lower than the max of 262140 to avoid strangeness
    
    noInterrupts();
    ignitionSchedule2.startCompare = IGN2_COUNTER + (timeout >> 2); //As there is a tick every 4uS, there are timeout/4 ticks until the interrupt should be triggered ( >>2 divides by 4)
    ignitionSchedule2.endCompare = ignitionSchedule2.startCompare + (duration >> 2);
    IGN2_COMPARE = ignitionSchedule2.startCompare;
    ignitionSchedule2.Status = PENDING; //Turn this schedule on
    interrupts();
    IGN2_TIMER_ENABLE();
  }
void setIgnitionSchedule3(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(ignitionSchedule3.Status == RUNNING) { return; } //Check that we're not already part way through a schedule

    ignitionSchedule3.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule3.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule3.duration = duration;
    
    //The timer is ticking every 4uS (Time per Tick = (Prescale)*(1/Frequency)) 
    if (timeout > 262140) { timeout = 262100; } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking. This must be set slightly lower than the max of 262140 to avoid strangeness
    
    noInterrupts();
    ignitionSchedule3.startCompare = IGN3_COUNTER + (timeout >> 2); //As there is a tick every 4uS, there are timeout/4 ticks until the interrupt should be triggered ( >>2 divides by 4)
    ignitionSchedule3.endCompare = ignitionSchedule3.startCompare + (duration >> 2);
    IGN3_COMPARE = ignitionSchedule3.startCompare;
    ignitionSchedule3.Status = PENDING; //Turn this schedule on
    interrupts();
    IGN3_TIMER_ENABLE(); 
  }
void setIgnitionSchedule4(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(ignitionSchedule4.Status == RUNNING) { return; } //Check that we're not already part way through a schedule

    ignitionSchedule4.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule4.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule4.duration = duration;
    
    //We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
    //The timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency))
    //Note this is different to the other ignition timers

    noInterrupts();
    ignitionSchedule4.startCompare = IGN4_COUNTER + (timeout >> 4); //As there is a tick every 4uS, there are timeout/4 ticks until the interrupt should be triggered ( >>2 divides by 4)
    ignitionSchedule4.endCompare = ignitionSchedule4.startCompare + (duration >> 4);
    IGN4_COMPARE = ignitionSchedule4.startCompare;
    ignitionSchedule4.Status = PENDING; //Turn this schedule on
    interrupts();
    IGN4_TIMER_ENABLE(); 
  }
void setIgnitionSchedule5(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    return;
    if(ignitionSchedule1.Status == RUNNING) { return; } //Check that we're not already part way through a schedule

    ignitionSchedule5.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule5.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule5.duration = duration;
    
    //As the timer is ticking every 4uS (Time per Tick = (Prescale)*(1/Frequency)) 
    if (timeout > 262140) { timeout = 262100; } // If the timeout is >4x (Each tick represents 4uS) the maximum allowed value of unsigned int (65535), the timer compare value will overflow when applied causing erratic behaviour such as erroneous sparking. This must be set slightly lower than the max of 262140 to avoid strangeness
    
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
    OCR5A = TCNT5 + (timeout >> 2); //As there is a tick every 4uS, there are timeout/4 ticks until the interrupt should be triggered ( >>2 divides by 4)
    ignitionSchedule5.Status = PENDING; //Turn this schedule on
    TIMSK5 |= (1 << OCIE5A); //Turn on the A compare unit (ie turn on the interrupt)
#endif
  }
  
/*******************************************************************************************************************************************************************************************************/
//This function (All 8 ISR functions that are below) gets called when either the start time or the duration time are reached
//This calls the relevant callback function (startCallback or endCallback) depending on the status of the schedule.
//If the startCallback function is called, we put the scheduler into RUNNING state
//Timer3A (fuel schedule 1) Compare Vector
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
ISR(TIMER3_COMPA_vect, ISR_NOBLOCK) //fuelSchedules 1 and 5
#elif defined (CORE_TEENSY)
void timer3compareAinterrupt() //Most ARM chips can simply call a function
#endif
  {
    if (timer3Aqueue[0]->Status == OFF) { FUEL1_TIMER_DISABLE(); return; } //Safety check. Turn off this output compare unit and return without performing any action
    if (timer3Aqueue[0]->Status == PENDING) //Check to see if this schedule is turn on
    {
      timer3Aqueue[0]->StartCallback();
      timer3Aqueue[0]->Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      FUEL1_COMPARE = popQueue(timer3Aqueue);
    }
    else if (timer3Aqueue[0]->Status == RUNNING)
    {
       timer3Aqueue[0]->EndCallback();
       timer3Aqueue[0]->Status = OFF; //Turn off the schedule
       timer3Aqueue[0]->schedulesSet = 0;
       FUEL1_COMPARE = popQueue(timer3Aqueue);
    }
  }

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
ISR(TIMER3_COMPB_vect, ISR_NOBLOCK) //fuelSchedule2
#elif defined (CORE_TEENSY)
void timer3compareBinterrupt() //Most ARM chips can simply call a function
#endif
  {
    if (fuelSchedule2.Status == PENDING) //Check to see if this schedule is turn on
    {
      fuelSchedule2.StartCallback();
      fuelSchedule2.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      FUEL2_COMPARE = fuelSchedule2.endCompare;
    }
    else if (fuelSchedule2.Status == RUNNING)
    {
       fuelSchedule2.EndCallback();
       fuelSchedule2.Status = OFF; //Turn off the schedule
       fuelSchedule2.schedulesSet = 0;
       FUEL2_TIMER_DISABLE();
    }
  }

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
ISR(TIMER3_COMPC_vect, ISR_NOBLOCK) //fuelSchedule3
#elif defined (CORE_TEENSY)
void timer3compareCinterrupt() //Most ARM chips can simply call a function
#endif
  {
    if (fuelSchedule3.Status == PENDING) //Check to see if this schedule is turn on
    {
      fuelSchedule3.StartCallback();
      fuelSchedule3.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      FUEL3_COMPARE = fuelSchedule3.endCompare;
    }
    else if (fuelSchedule3.Status == RUNNING)
    {
       fuelSchedule3.EndCallback();
       fuelSchedule3.Status = OFF; //Turn off the schedule
       fuelSchedule3.schedulesSet = 0;
       FUEL3_TIMER_DISABLE();
    }
  }
  
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
ISR(TIMER4_COMPB_vect, ISR_NOBLOCK) //fuelSchedule4
#elif defined (CORE_TEENSY)
void timer4compareBinterrupt() //Most ARM chips can simply call a function
#endif
  {
    if (fuelSchedule4.Status == PENDING) //Check to see if this schedule is turn on
    {
      fuelSchedule4.StartCallback();
      fuelSchedule4.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      FUEL4_COMPARE = fuelSchedule4.endCompare;
    }
    else if (fuelSchedule4.Status == RUNNING)
    {
       fuelSchedule4.EndCallback();
       fuelSchedule4.Status = OFF; //Turn off the schedule
       fuelSchedule4.schedulesSet = 0;
       FUEL4_TIMER_DISABLE(); 
    }
  }
  
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
ISR(TIMER5_COMPA_vect, ISR_NOBLOCK) //ignitionSchedule1
#elif defined (CORE_TEENSY)
void timer5compareAinterrupt() //Most ARM chips can simply call a function
#endif
  {
    if (ignitionSchedule1.Status == PENDING) //Check to see if this schedule is turn on
    {
      ignitionSchedule1.StartCallback();
      ignitionSchedule1.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      ignitionSchedule1.startTime = micros();
      //ign1LastRev = startRevolutions;
      IGN1_COMPARE = ignitionSchedule1.endCompare; //OCR5A = TCNT5 + (ignitionSchedule1.duration >> 2); //Divide by 4
    }
    else if (ignitionSchedule1.Status == RUNNING)
    {
      ignitionSchedule1.Status = OFF; //Turn off the schedule
      ignitionSchedule1.EndCallback();
      ignitionCount += 1; //Increment the igintion counter
      IGN1_TIMER_DISABLE();
    }
  }
  
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
ISR(TIMER5_COMPB_vect, ISR_NOBLOCK) //ignitionSchedule2
#elif defined (CORE_TEENSY)
void timer5compareBinterrupt() //Most ARM chips can simply call a function
#endif
  {
    if (ignitionSchedule2.Status == PENDING) //Check to see if this schedule is turn on
    {
      ignitionSchedule2.StartCallback();
      ignitionSchedule2.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      ignitionSchedule2.startTime = micros();
      //ign2LastRev = startRevolutions;
      IGN2_COMPARE = ignitionSchedule2.endCompare; //OCR5B = TCNT5 + (ignitionSchedule2.duration >> 2);
    }
    else if (ignitionSchedule2.Status == RUNNING)
    {
      ignitionSchedule2.Status = OFF; //Turn off the schedule
      ignitionSchedule2.EndCallback();
      ignitionCount += 1; //Increment the igintion counter
      IGN2_TIMER_DISABLE();
    }
  }
  
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
ISR(TIMER5_COMPC_vect, ISR_NOBLOCK) //ignitionSchedule3
#elif defined (CORE_TEENSY)
void timer5compareCinterrupt() //Most ARM chips can simply call a function
#endif
  {
    if (ignitionSchedule3.Status == PENDING) //Check to see if this schedule is turn on
    {
      ignitionSchedule3.StartCallback();
      ignitionSchedule3.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      ignitionSchedule3.startTime = micros();
      //ign3LastRev = startRevolutions;
      IGN3_COMPARE = ignitionSchedule3.endCompare; //OCR5C = TCNT5 + (ignitionSchedule3.duration >> 2);
    }
    else if (ignitionSchedule3.Status == RUNNING)
    {
       ignitionSchedule3.Status = OFF; //Turn off the schedule
       ignitionSchedule3.EndCallback();
       ignitionCount += 1; //Increment the igintion counter
       IGN3_TIMER_DISABLE();
    }
  }
  
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) //AVR chips use the ISR for this
ISR(TIMER4_COMPA_vect, ISR_NOBLOCK) //ignitionSchedule4
#elif defined (CORE_TEENSY)
void timer4compareAinterrupt() //Most ARM chips can simply call a function
#endif
  {
    if (ignitionSchedule4.Status == PENDING) //Check to see if this schedule is turn on
    {
      ignitionSchedule4.StartCallback();
      ignitionSchedule4.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      ignitionSchedule4.startTime = micros();
      //ign4LastRev = startRevolutions;
      IGN4_COMPARE = ignitionSchedule4.endCompare; //OCR4A = TCNT4 + (ignitionSchedule4.duration >> 4); //Divide by 16
    }
    else if (ignitionSchedule4.Status == RUNNING)
    {
       ignitionSchedule4.Status = OFF; //Turn off the schedule
       ignitionSchedule4.EndCallback();
       ignitionCount += 1; //Increment the igintion counter
       IGN4_TIMER_DISABLE();
    }
  }
