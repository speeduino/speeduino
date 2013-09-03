


void initialiseScheduler()
  {
    
    // Much help in this from http://arduinomega.blogspot.com.au/2011/05/timer2-and-overflow-interrupt-lets-get.html
    //Schedule 1, which is uses timer 3
    TCCR3B = 0x00;          //Disbale Timer2 while we set it up
    TCNT3  = 0;             //Reset Timer Count
    TIFR3  = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
    TIMSK3 = 0x01;          //Timer2 INT Reg: Timer2 Overflow Interrupt Enable
    TCCR3A = 0x00;          //Timer2 Control Reg A: Wave Gen Mode normal
    TCCR3B = (1 << CS12);   //Timer2 Control Reg B: Timer Prescaler set to 256. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg
    
    //Schedule 2, which is uses timer 4
    TCCR4B = 0x00;          //Disbale Timer2 while we set it up
    TCNT4  = 0;             //Reset Timer Count
    TIFR4  = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
    TIMSK4 = 0x01;          //Timer2 INT Reg: Timer2 Overflow Interrupt Enable
    TCCR4A = 0x00;          //Timer2 Control Reg A: Wave Gen Mode normal
    TCCR4B = (1 << CS12);   //Timer2 Control Reg B: Timer Prescaler set to 256. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg
   
    schedule1Status = 0; 
    schedule2Status = 0;
  }
  
/*
This turns schedule 1 on, gives it callback functions and resets the relevant timer based on the time in the future that this should be triggered
Args:
startCallback: The function to be called once the timeout1 is reached
timeout1: The number of uS in the future that the callback should be triggered
duration: The number of uS before endCallback is called
endCallback: This function is called once the duration time has been reached
*/
void setSchedule1(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    //We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
    //As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
    //TODO: Need to add check for timeout > 1048576 ????
    if(schedule1Status == 2) { return; } //Check that we're note already part way through a schedule
    TCNT3 = 65536 - (timeout / 16); //Each tick occurs every 16uS with a 256 prescaler so divide the timeout by 16 to get ther required number of ticks. Subtract this from the total number of tick (65536 for 16-bit timer)
    schedule1Duration = duration;
    schedule1StartCallback = startCallback; //Name the start callback function
    schedule1EndCallback = endCallback; //Name the start callback function
    schedule1Status = 1; //Turn this schedule on and set it
  }
  
//As above, but for schedule2
void setSchedule2(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
  {
    //TODO: Need to add check for timeout > 1048576 ????
    if(schedule2Status == 2) { return; } //Check that we're note already part way through a schedule
    TCNT4 = 65536 - (timeout / 16); //Each tick occurs every 16uS with a 256 prescaler so divide the timeout by 16 to get ther required number of ticks. Subtract this from the total number of tick (65536 for 16-bit timer)
    schedule2Duration = duration;
    schedule2StartCallback = startCallback; //Name the callback function
    schedule2EndCallback = endCallback; //Name the callback function
    schedule2Status = 1; //Turn this schedule on
  }
  
//Timer3 (schedule 1) Overflow Interrupt Vector
//This needs to call the callback function if one has been provided and rest the timer
ISR(TIMER3_OVF_vect)
  {
    if (schedule1Status == 1) //Check to see if this schedule is turn on
    {
      schedule1StartCallback(); //Replace with user provided callback
      schedule1Status = 2; //Turn off the callback 
      TCNT3 = 65536 - (schedule1Duration / 16);
    }
    else if (schedule1Status == 2)
    {
       schedule1EndCallback();
       schedule1Status = 0; //Turn off the callback
       TCNT3 = 0;           //Reset Timer to 0 out of 255
    }
    
  TIFR3 = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
  }

//AS above for schedule2
ISR(TIMER4_OVF_vect)
  {
    if (schedule2Status == 1) //A value of 1 means call the start callback
    {
      schedule2StartCallback();
      schedule2Status = 2; //Set to call the end callback on the next run
      TCNT4 = 65536 - (schedule2Duration / 16); 
    }
    else if (schedule2Status == 2)
    {
       schedule2EndCallback();
       schedule2Status = 0; //Turn off the callback
       TCNT4 = 0;           //Reset Timer to 0 out of 255
    }
    
  TIFR4 = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
  }
