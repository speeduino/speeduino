


void initialiseSchedulers()
  {
   // Much help in this from http://arduinomega.blogspot.com.au/2011/05/timer2-and-overflow-interrupt-lets-get.html
    //Fuel Schedules, which uses timer 3
    TCCR3B = 0x00;          //Disbale Timer3 while we set it up
    TCNT3  = 0;             //Reset Timer Count
    TIFR3  = 0x00;          //Timer3 INT Flag Reg: Clear Timer Overflow Flag
    TCCR3A = 0x00;          //Timer3 Control Reg A: Wave Gen Mode normal
    TCCR3B = (1 << CS12);   //Timer3 Control Reg B: Timer Prescaler set to 256. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg
    fuelSchedule1.Status = OFF;
    fuelSchedule2.Status = OFF;
    fuelSchedule3.Status = OFF;

    //Ignition Schedules, which uses timer 5
    TCCR5B = 0x00;          //Disbale Timer3 while we set it up
    TCNT5  = 0;             //Reset Timer Count
    TIFR5  = 0x00;          //Timer5 INT Flag Reg: Clear Timer Overflow Flag
    TCCR5A = 0x00;          //Timer5 Control Reg A: Wave Gen Mode normal
    TCCR5B = (1 << CS12);   //Timer5 Control Reg B: Timer Prescaler set to 256. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg
    ignitionSchedule1.Status = OFF;
    ignitionSchedule2.Status = OFF;
    ignitionSchedule3.Status = OFF;
    
    //The remaining Schedules (Schedules 4 for fuel and ignition) use Timer4
    TCCR4B = 0x00;          //Disbale Timer4 while we set it up
    TCNT4  = 0;             //Reset Timer Count
    TIFR4  = 0x00;          //Timer4 INT Flag Reg: Clear Timer Overflow Flag
    TCCR4A = 0x00;          //Timer4 Control Reg A: Wave Gen Mode normal
    TCCR4B = (1 << CS12);   //Timer4 Control Reg B: Timer Prescaler set to 256. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg
    ignitionSchedule4.Status = OFF;
    fuelSchedule4.Status = OFF;
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
void setFuelSchedule1(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(fuelSchedule1.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    
    //We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
    //As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
    //unsigned int absoluteTimeout = TCNT3 + (timeout / 16); //Each tick occurs every 16uS with the 256 prescaler, so divide the timeout by 16 to get ther required number of ticks. Add this to the current tick count to get the target time. This will automatically overflow as required
    unsigned int absoluteTimeout = TCNT3 + (timeout >> 4); //As above, but with bit shift instead of / 16
    OCR3A = absoluteTimeout;
    fuelSchedule1.duration = duration;
    fuelSchedule1.StartCallback = startCallback; //Name the start callback function
    fuelSchedule1.EndCallback = endCallback; //Name the end callback function
    fuelSchedule1.Status = PENDING; //Turn this schedule on
    TIMSK3 |= (1 << OCIE3A); //Turn on the A compare unit (ie turn on the interrupt)
  }
void setFuelSchedule2(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(fuelSchedule2.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    
    //We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
    //As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
    //unsigned int absoluteTimeout = TCNT3 + (timeout / 16); //Each tick occurs every 16uS with the 256 prescaler, so divide the timeout by 16 to get ther required number of ticks. Add this to the current tick count to get the target time. This will automatically overflow as required
    unsigned int absoluteTimeout = TCNT3 + (timeout >> 4); //As above, but with bit shift instead of / 16
    OCR3B = absoluteTimeout; //Use the B copmare unit of timer 3
    fuelSchedule2.duration = duration;
    fuelSchedule2.StartCallback = startCallback; //Name the start callback function
    fuelSchedule2.EndCallback = endCallback; //Name the end callback function
    fuelSchedule2.Status = PENDING; //Turn this schedule on
    TIMSK3 |= (1 << OCIE3B); //Turn on the B compare unit (ie turn on the interrupt)
  }
void setFuelSchedule3(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(fuelSchedule3.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    
    //We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
    //As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
    //unsigned int absoluteTimeout = TCNT3 + (timeout / 16); //Each tick occurs every 16uS with the 256 prescaler, so divide the timeout by 16 to get ther required number of ticks. Add this to the current tick count to get the target time. This will automatically overflow as required
    unsigned int absoluteTimeout = TCNT3 + (timeout >> 4); //As above, but with bit shift instead of / 16
    OCR3C = absoluteTimeout; //Use the C compare unit of timer 3
    fuelSchedule3.duration = duration;
    fuelSchedule3.StartCallback = startCallback; //Name the start callback function
    fuelSchedule3.EndCallback = endCallback; //Name the end callback function
    fuelSchedule3.Status = PENDING; //Turn this schedule on
    TIMSK3 |= (1 << OCIE3C); //Turn on the C compare unit (ie turn on the interrupt)
  }
void setFuelSchedule4(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)()) //Uses timer 4 compare B
  {
    if(fuelSchedule4.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    
    //We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
    //As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
    //unsigned int absoluteTimeout = TCNT3 + (timeout / 16); //Each tick occurs every 16uS with the 256 prescaler, so divide the timeout by 16 to get ther required number of ticks. Add this to the current tick count to get the target time. This will automatically overflow as required
    unsigned int absoluteTimeout = TCNT4 + (timeout >> 4); //As above, but with bit shift instead of / 16
    OCR4B = absoluteTimeout; //Use the B compare unit of timer 4
    fuelSchedule4.duration = duration;
    fuelSchedule4.StartCallback = startCallback; //Name the start callback function
    fuelSchedule4.EndCallback = endCallback; //Name the end callback function
    fuelSchedule4.Status = PENDING; //Turn this schedule on
    TIMSK4 |= (1 << OCIE4B); //Turn on the B compare unit (ie turn on the interrupt)
  }
//Ignition schedulers use Timer 5
void setIgnitionSchedule1(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(ignitionSchedule1.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    if(ignitionSchedule1.Status == PENDING) { TIMSK5 &= ~(1 << OCIE5A); } //Check that we're not already part way through a schedule
    
    //We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
    //As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
    //unsigned int absoluteTimeout = TCNT5 + (timeout / 16); //Each tick occurs every 16uS with the 256 prescaler, so divide the timeout by 16 to get ther required number of ticks. Add this to the current tick count to get the target time. This will automatically overflow as required
    //unsigned char sreg;
    //sreg = SREG;
    //noInterrupts();
    unsigned int absoluteTimeout = TCNT5 + (timeout >> 4); //As above, but with bit shift instead of / 16
    OCR5A = absoluteTimeout;
    //SREG = sreg;
    ignitionSchedule1.duration = duration;
    ignitionSchedule1.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule1.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule1.Status = PENDING; //Turn this schedule on
    TIMSK5 |= (1 << OCIE5A); //Turn on the A compare unit (ie turn on the interrupt)
  }
void setIgnitionSchedule2(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(ignitionSchedule2.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    if(ignitionSchedule2.Status == PENDING) { TIMSK5 &= ~(1 << OCIE5B); } //Check that we're not already part way through a schedule
    
    //We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
    //As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
    //unsigned int absoluteTimeout = TCNT5 + (timeout / 16); //Each tick occurs every 16uS with the 256 prescaler, so divide the timeout by 16 to get ther required number of ticks. Add this to the current tick count to get the target time. This will automatically overflow as required
    unsigned int absoluteTimeout = TCNT5 + (timeout >> 4); //As above, but with bit shift instead of / 16
    OCR5B = absoluteTimeout;
    ignitionSchedule2.duration = duration;
    ignitionSchedule2.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule2.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule2.Status = PENDING; //Turn this schedule on
    TIMSK5 |= (1 << OCIE5B); //Turn on the B compare unit (ie turn on the interrupt)
  }
void setIgnitionSchedule3(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(ignitionSchedule3.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    
    //We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
    //As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
    //unsigned int absoluteTimeout = TCNT5 + (timeout / 16); //Each tick occurs every 16uS with the 256 prescaler, so divide the timeout by 16 to get ther required number of ticks. Add this to the current tick count to get the target time. This will automatically overflow as required
    unsigned int absoluteTimeout = TCNT5 + (timeout >> 4); //As above, but with bit shift instead of / 16
    OCR5C = absoluteTimeout;
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
    //As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
    //unsigned int absoluteTimeout = TCNT5 + (timeout / 16); //Each tick occurs every 16uS with the 256 prescaler, so divide the timeout by 16 to get ther required number of ticks. Add this to the current tick count to get the target time. This will automatically overflow as required
    unsigned int absoluteTimeout = TCNT4 + (timeout >> 4); //As above, but with bit shift instead of / 16
    OCR4A = absoluteTimeout;
    ignitionSchedule4.duration = duration;
    ignitionSchedule4.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule4.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule4.Status = PENDING; //Turn this schedule on
    TIMSK4 |= (1 << OCIE4A); //Turn on the C compare unit (ie turn on the interrupt)
  }
  
  

//This function (All 8 ISR functions that are below) gets called when either the start time or the duration time are reached
//This calls the relevant callback function (startCallback or endCallback) depending on the status of the schedule.
//If the startCallback function is called, we put the scheduler into RUNNING state
//Timer3A (fuel schedule 1) Compare Vector
ISR(TIMER3_COMPA_vect) //fuelSchedule1
  {
    if (fuelSchedule1.Status == PENDING) //Check to see if this schedule is turn on
    {
      fuelSchedule1.StartCallback();
      fuelSchedule1.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      //unsigned int absoluteTimeout = TCNT3 + (fuelSchedule1.duration / 16);
      unsigned int absoluteTimeout = TCNT3 + (fuelSchedule1.duration >> 4); //Divide by 16
      OCR3A = absoluteTimeout;
    }
    else if (fuelSchedule1.Status == RUNNING)
    {
       fuelSchedule1.EndCallback();
       fuelSchedule1.Status = OFF; //Turn off the schedule
       TIMSK3 &= ~(1 << OCIE3A); //Turn off this output compare unit (This simply writes 0 to the OCIE3A bit of TIMSK3)
    }
  }
ISR(TIMER3_COMPB_vect) //fuelSchedule2
  {
    if (fuelSchedule2.Status == PENDING) //Check to see if this schedule is turn on
    {
      fuelSchedule2.StartCallback();
      fuelSchedule2.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      //unsigned int absoluteTimeout = TCNT3 + (fuelSchedule2.duration / 16);
      unsigned int absoluteTimeout = TCNT3 + (fuelSchedule2.duration >> 4); //Divide by 16
      OCR3B = absoluteTimeout;
    }
    else if (fuelSchedule2.Status == RUNNING)
    {
       fuelSchedule2.EndCallback();
       fuelSchedule2.Status = OFF; //Turn off the schedule
       TIMSK3 &= ~(1 << OCIE3B); //Turn off this output compare unit (This simply writes 0 to the OCIE3A bit of TIMSK3)
    }
  }
ISR(TIMER3_COMPC_vect) //fuelSchedule3
  {
    noInterrupts();
    if (fuelSchedule3.Status == PENDING) //Check to see if this schedule is turn on
    {
      fuelSchedule3.StartCallback();
      fuelSchedule3.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      //unsigned int absoluteTimeout = TCNT3 + (fuelSchedule2.duration / 16);
      unsigned int absoluteTimeout = TCNT3 + (fuelSchedule3.duration >> 4); //Divide by 16
      OCR3C = absoluteTimeout;
    }
    else if (fuelSchedule3.Status == RUNNING)
    {
       fuelSchedule3.EndCallback();
       fuelSchedule3.Status = OFF; //Turn off the schedule
       TIMSK3 &= ~(1 << OCIE3C); //Turn off this output compare unit (This simply writes 0 to the OCIE3A bit of TIMSK3)
    }
    interrupts();
  }
ISR(TIMER4_COMPB_vect) //fuelSchedule4
  {
    noInterrupts();
    if (fuelSchedule4.Status == PENDING) //Check to see if this schedule is turn on
    {
      fuelSchedule4.StartCallback();
      fuelSchedule4.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      //unsigned int absoluteTimeout = TCNT3 + (fuelSchedule2.duration / 16);
      unsigned int absoluteTimeout = TCNT4 + (fuelSchedule4.duration >> 4); //Divide by 16
      OCR4B = absoluteTimeout;
    }
    else if (fuelSchedule4.Status == RUNNING)
    {
       fuelSchedule4.EndCallback();
       fuelSchedule4.Status = OFF; //Turn off the schedule
       TIMSK4 &= ~(1 << OCIE4B); //Turn off this output compare unit (This simply writes 0 to the OCIE3A bit of TIMSK3)
    }
    interrupts();
  }
ISR(TIMER5_COMPA_vect) //ignitionSchedule1
  {
    if (ignitionSchedule1.Status == PENDING) //Check to see if this schedule is turn on
    {
      ignitionSchedule1.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      ignitionSchedule1.StartCallback();
      //unsigned int absoluteTimeout = TCNT5 + (ignitionSchedule1.duration / 16);
      unsigned int absoluteTimeout = TCNT5 + (ignitionSchedule1.duration >> 4); //Divide by 16
      OCR5A = absoluteTimeout;
    }
    else if (ignitionSchedule1.Status == RUNNING)
    {
      ignitionSchedule1.Status = OFF; //Turn off the schedule
      ignitionSchedule1.EndCallback();
      TIMSK5 &= ~(1 << OCIE5A); //Turn off this output compare unit (This simply writes 0 to the OCIE3A bit of TIMSK3)
    }
  }
ISR(TIMER5_COMPB_vect) //ignitionSchedule2
  {
    if (ignitionSchedule2.Status == PENDING) //Check to see if this schedule is turn on
    {
      ignitionSchedule2.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      ignitionSchedule2.StartCallback();
      //unsigned int absoluteTimeout = TCNT5 + (ignitionSchedule2.duration / 16);
      unsigned int absoluteTimeout = TCNT5 + (ignitionSchedule2.duration >> 4); //Divide by 16
      OCR5B = absoluteTimeout;
    }
    else if (ignitionSchedule2.Status == RUNNING)
    {
      ignitionSchedule2.Status = OFF; //Turn off the schedule
      ignitionSchedule2.EndCallback();
      TIMSK5 &= ~(1 << OCIE5B); //Turn off this output compare unit (This simply writes 0 to the OCIE3A bit of TIMSK3)
    }
  }
ISR(TIMER5_COMPC_vect) //ignitionSchedule3
  {
    noInterrupts();
    if (ignitionSchedule3.Status == PENDING) //Check to see if this schedule is turn on
    {
      ignitionSchedule3.StartCallback();
      ignitionSchedule3.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      //unsigned int absoluteTimeout = TCNT5 + (ignitionSchedule2.duration / 16);
      unsigned int absoluteTimeout = TCNT5 + (ignitionSchedule3.duration >> 4); //Divide by 16
      OCR5C = absoluteTimeout;
    }
    else if (ignitionSchedule3.Status == RUNNING)
    {
       ignitionSchedule3.EndCallback();
       ignitionSchedule3.Status = OFF; //Turn off the schedule
       TIMSK5 &= ~(1 << OCIE5C); //Turn off this output compare unit (This simply writes 0 to the OCIE3A bit of TIMSK3)
    }
    interrupts();
  }
ISR(TIMER4_COMPA_vect) //ignitionSchedule4
  {
    noInterrupts();
    if (ignitionSchedule4.Status == PENDING) //Check to see if this schedule is turn on
    {
      ignitionSchedule4.StartCallback();
      ignitionSchedule4.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      //unsigned int absoluteTimeout = TCNT5 + (ignitionSchedule2.duration / 16);
      unsigned int absoluteTimeout = TCNT4 + (ignitionSchedule4.duration >> 4); //Divide by 16
      OCR4A = absoluteTimeout;
    }
    else if (ignitionSchedule4.Status == RUNNING)
    {
       ignitionSchedule4.EndCallback();
       ignitionSchedule4.Status = OFF; //Turn off the schedule
       TIMSK5 &= ~(1 << OCIE4A); //Turn off this output compare unit (This simply writes 0 to the OCIE3A bit of TIMSK3)
    }
    interrupts();
  }
