/*
This scheduler is designed to maintain 2 schedules for use by the fuel and ignition systems. 
It functions by waiting for the overflow vectors from each of the timers in use to overflow, which triggers an interrupt

//Technical
Currently I am prescaling the 16-bit timers to 256. This means that the counter increments every 16us and will overflow every 1048576uS
Max Period = (Prescale)*(1/Frequency)*(2^17)
(See http://playground.arduino.cc/code/timer1)
This means that the precision of the scheduler is 16uS (+/- 8uS of target)

/Features
This differs from most other schedulers in that its calls are non-recurring (IE You schedule an event at a certain time and once it has occurred, it will not reoccur unless you explicitely ask for it)
Each timer can have only 1 callback associated with it at any given time. If you call the setCallback function a 2nd time, the original schedule will be overwritten and not occur

Timer identification
The Arduino timer3 is used for schedule 1
The Arduino timer4 is used for schedule 2
Both of these are 16-bit timers (ie count to 65536)
See page 136 of the processors datasheet: http://www.atmel.com/Images/doc2549.pdf

256 prescale gives tick every 16uS
256 prescale gives overflow every 1048576uS (This means maximum wait time is 1.0485 seconds)

*/

#include <avr/interrupt.h> 
#include <avr/io.h>

//#define clockspeed 16000000

int schedule1Active; 
int schedule2Active;
void (*schedule1Callback)(); //Callback function for schedule1
void (*schedule2Callback)();

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
   
    schedule1Active = 0; 
    schedule2Active = 0;
  }
  
/*
This turns schedule 1 on, gives it a callback functino and resets the relevant timer based on the time in the future that this should be triggered
Args:
callback: The function to be called once the timeout is reach
timeout: The number of uS in the future that the callback should be triggered
*/
void setSchedule1(void (*callback)(), unsigned long timeout)
  {
    //We need to calculate the value to reset the timer to (preload) in order to achieve the desired overflow time
    //As the timer is ticking every 16uS (Time per Tick = (Prescale)*(1/Frequency)) 
    //TODO: Need to add check for timeout > 1048576 ????
    TCNT3 = 65536 - (timeout / 16); //Each tick occurs every 16uS with a 256 prescaler so divide the timeout by 16 to get ther required number of ticks. Subtract this from the total number of tick (65536 for 16-bit timer)
    //TCNT3 = 0;
    schedule1Callback = callback; //Name the callback function
    schedule1Active = 1; //Turn this schedule on
  }
  
//As above, but for schedule2
void setSchedule2(void (*callback)(), unsigned long timeout)
  {
    //TODO: Need to add check for timeout > 1048576 ????
    TCNT4 = 65536 - (timeout / 16); //Each tick occurs every 16uS with a 256 prescaler so divide the timeout by 16 to get ther required number of ticks. Subtract this from the total number of tick (65536 for 16-bit timer)
    schedule2Callback = callback; //Name the callback function
    schedule2Active = 1; //Turn this schedule on
  }
  
//Timer3 (schedule 1) Overflow Interrupt Vector
//This needs to call the callback function if one has been provided and rest the timer
ISR(TIMER3_OVF_vect)
  {
    if (schedule1Active > 0) //Check to see if this schedule is turn on
    {
      schedule1Callback(); //Replace with user provided callback
      schedule1Active = 0; //Turn off the callback 
    }
    
  TCNT3 = 0;           //Reset Timer to 0 out of 255
  TIFR3 = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
  }

//AS above for schedule2
ISR(TIMER4_OVF_vect)
  {
    if (schedule2Active > 0) //Check to see if this schedule is turn on
    {
      schedule2Callback(); //Replace with user provided callback
      schedule2Active = 0; //Turn off the callback 
    }
    
  TCNT3 = 0;           //Reset Timer to 0 out of 255
  TIFR3 = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
  }
