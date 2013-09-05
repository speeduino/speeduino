#include <avr/interrupt.h> 
#include <avr/io.h>

//#define clockspeed 16000000

int schedule1Status; //Value=0 means do nothing, value=1 means call the startCallback, value=2 means call the endCallback
int schedule2Status; //As above, for 2nd scheduler
unsigned long schedule1Duration; //How long (uS) after calling the start callback to we call the end callback
unsigned long schedule2Duration;
void (*schedule1StartCallback)(); //Start Callback function for schedule1
void (*schedule2StartCallback)();
void (*schedule1EndCallback)(); //End Callback function for schedule1
void (*schedule2EndCallback)();


void initialiseSchedulers();
void setSchedule1(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
void setSchedule2(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());

void setup()
{
  Serial.begin(9600);
  initialiseSchedulers();
}

void loop()
{
  unsigned long curTime = micros();
  unsigned long uSInFuture = 50000; //50mS in the future
  unsigned long expectedTime = curTime + uSInFuture;
  
  setSchedule1(callback, uSInFuture, uSInFuture, callback);
  
  Serial.print("Expected time: ");
  Serial.println(expectedTime);
  
  delay(1000);
}

void callback()
{
   unsigned long arrivedTime = micros();
   
   Serial.print("Arrived time: ");
   Serial.println(arrivedTime);
}

void initialiseSchedulers()
  {
    
    // Much help in this from http://arduinomega.blogspot.com.au/2011/05/timer2-and-overflow-interrupt-lets-get.html
    //Schedule 1, which is uses timer 3
    
    TCCR3B = 0x00;          //Disbale Timer2 while we set it up
    TCNT3  = 0;             //Reset Timer Count
    TIFR3  = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
    //TIMSK3 = 0x01;          //Timer2 INT Reg: Timer2 Overflow Interrupt Enable
    TCCR3A = 0x00;          //Timer2 Control Reg A: Wave Gen Mode normal
    TCCR3B = (1 << CS12);   //Timer2 Control Reg B: Timer Prescaler set to 256. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg

    /*
    //Schedule 2, which is uses timer 4
    TCCR4B = 0x00;          //Disbale Timer2 while we set it up
    TCNT4  = 0;             //Reset Timer Count
    TIFR4  = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
    TIMSK4 = 0x01;          //Timer2 INT Reg: Timer2 Overflow Interrupt Enable
    TCCR4A = 0x00;          //Timer2 Control Reg A: Wave Gen Mode normal
    TCCR4B = (1 << CS12);   //Timer2 Control Reg B: Timer Prescaler set to 256. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg
   */
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
    if(schedule1Status == 2) { return; } //Check that we're not already part way through a schedule
    
    //We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
    //As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
    unsigned int absoluteTimeout = TCNT3 + (timeout / 16); //Each tick occurs every 16uS with the 256 prescaler, so divide the timeout by 16 to get ther required number of ticks. Add this to the current tick count to get the target time. This will automatically overflow as required
    OCR3A = absoluteTimeout;
    schedule1Duration = duration;
    schedule1StartCallback = startCallback; //Name the start callback function
    schedule1EndCallback = endCallback; //Name the start callback function
    schedule1Status = 1; //Turn this schedule on and set it
    TIMSK3 |= (1 << OCIE3A); //Turn on the compare unit (ie turn on the interrupt)
  }
  
//Timer3 (schedule 1) Overflow Interrupt Vector
//This needs to call the callback function if one has been provided and rest the timer
ISR(TIMER3_COMPA_vect)
  {
    if (schedule1Status == 1) //Check to see if this schedule is turn on
    {
      schedule1StartCallback(); //Replace with user provided callback
      schedule1Status = 2; //Turn off the callback 
      unsigned int absoluteTimeout = TCNT3 + (schedule1Duration / 16);
      OCR3A = absoluteTimeout;
    }
    else if (schedule1Status == 2)
    {
       schedule1EndCallback();
       schedule1Status = 0; //Turn off the callback
       TIMSK3 &= ~(1 << OCIE3A); //Turn off this output compare unit
    }
    
  TIFR3 = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
  }
