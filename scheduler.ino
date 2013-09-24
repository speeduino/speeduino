


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

    //Ignition Schedules, which uses timer 5
    TCCR5B = 0x00;          //Disbale Timer3 while we set it up
    TCNT5  = 0;             //Reset Timer Count
    TIFR5  = 0x00;          //Timer3 INT Flag Reg: Clear Timer Overflow Flag
    TCCR5A = 0x00;          //Timer3 Control Reg A: Wave Gen Mode normal
    TCCR5B = (1 << CS12);   //Timer3 Control Reg B: Timer Prescaler set to 256. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg
    ignitionSchedule1.Status = OFF;
    ignitionSchedule2.Status = OFF;
  }
  
/*
These 4 function turn a schedule on, provides the time to start and the duration and gives it callback functions.
All 4 functions operate the same, just on different schedules
Args:
startCallback: The function to be called once the timeout is reached
timeout: The number of uS in the future that the callback should be triggered
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
    fuelSchedule1.EndCallback = endCallback; //Name the start callback function
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
    fuelSchedule2.EndCallback = endCallback; //Name the start callback function
    fuelSchedule2.Status = PENDING; //Turn this schedule on
    TIMSK3 |= (1 << OCIE3B); //Turn on the B compare unit (ie turn on the interrupt)
  }
//Ignition schedulers use Timer 5
void setIgnitionSchedule1(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(ignitionSchedule1.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    if(ignitionSchedule1.Status == PENDING) { return; } //Check that we're not already part way through a schedule
    
    //We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
    //As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
    //unsigned int absoluteTimeout = TCNT5 + (timeout / 16); //Each tick occurs every 16uS with the 256 prescaler, so divide the timeout by 16 to get ther required number of ticks. Add this to the current tick count to get the target time. This will automatically overflow as required
    unsigned int absoluteTimeout = TCNT5 + (timeout >> 4); //As above, but with bit shift instead of / 16
    OCR5A = absoluteTimeout;
    ignitionSchedule1.duration = duration;
    ignitionSchedule1.StartCallback = startCallback; //Name the start callback function
    ignitionSchedule1.EndCallback = endCallback; //Name the start callback function
    ignitionSchedule1.Status = PENDING; //Turn this schedule on
    TIMSK5 |= (1 << OCIE5A); //Turn on the A compare unit (ie turn on the interrupt)
  }
void setIgnitionSchedule2(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    if(ignitionSchedule2.Status == RUNNING) { return; } //Check that we're not already part way through a schedule
    
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
  
  
  

//This function (All 4 ISR functions that are below) gets called when either the start time or the duration time are reached
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
  //TIFR3 = 0x00;          //Timer3 INT Flag Reg: Clear Timer Overflow Flag. I'm not 100% sure this is necessary, but better to be safe
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
  //TIFR3 = 0x00;          //Timer3 INT Flag Reg: Clear Timer Overflow Flag. I'm not 100% sure this is necessary, but better to be safe
  }
ISR(TIMER5_COMPA_vect) //ignitionSchedule1
  {
    if (ignitionSchedule1.Status == PENDING) //Check to see if this schedule is turn on
    {
      ignitionSchedule1.StartCallback();
      ignitionSchedule1.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      //unsigned int absoluteTimeout = TCNT5 + (ignitionSchedule1.duration / 16);
      unsigned int absoluteTimeout = TCNT5 + (ignitionSchedule1.duration >> 4); //Divide by 16
      OCR5A = absoluteTimeout;
    }
    else if (ignitionSchedule1.Status == RUNNING)
    {
       ignitionSchedule1.EndCallback();
       ignitionSchedule1.Status = OFF; //Turn off the schedule
       TIMSK5 &= ~(1 << OCIE5A); //Turn off this output compare unit (This simply writes 0 to the OCIE3A bit of TIMSK3)
    }
  //TIFR3 = 0x00;          //Timer3 INT Flag Reg: Clear Timer Overflow Flag. I'm not 100% sure this is necessary, but better to be safe
  }
ISR(TIMER5_COMPB_vect) //ignitionSchedule2
  {
    if (ignitionSchedule2.Status == PENDING) //Check to see if this schedule is turn on
    {
      ignitionSchedule2.StartCallback();
      ignitionSchedule2.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      //unsigned int absoluteTimeout = TCNT5 + (ignitionSchedule2.duration / 16);
      unsigned int absoluteTimeout = TCNT5 + (ignitionSchedule2.duration >> 4); //Divide by 16
      OCR5B = absoluteTimeout;
    }
    else if (ignitionSchedule2.Status == RUNNING)
    {
       ignitionSchedule2.EndCallback();
       ignitionSchedule2.Status = OFF; //Turn off the schedule
       TIMSK5 &= ~(1 << OCIE5B); //Turn off this output compare unit (This simply writes 0 to the OCIE3A bit of TIMSK3)
    }
  //TIFR3 = 0x00;          //Timer3 INT Flag Reg: Clear Timer Overflow Flag. I'm not 100% sure this is necessary, but better to be safe
  }
