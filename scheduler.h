/*
This scheduler is designed to maintain 2 schedules for use by the fuel and ignition systems. 
It functions by waiting for the overflow vectors from each of the timers in use to overflow, which triggers an interrupt

//Technical
Currently I am prescaling the 8-bit timers to 256. This means that the counter increments every 16us and will overflow every 2017mS
Max Period = (Prescale)*(1/Frequency)*(2^17)
(See http://playground.arduino.cc/code/timer1)
Because the maximum overflow occurs roughly every 2 seconds, you cannot schedule anything to be more than 2 seconds in the future.
This also means that the precision of the scheduler is 16uS

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

#define clockspeed 16000000

int schedule1Active;
int schedule2Active;
void (*schedule1Callback)();
void (*schedule2Callback)();

void initialiseScheduler()
  {
    
    // Much help in this from http://arduinomega.blogspot.com.au/2011/05/timer2-and-overflow-interrupt-lets-get.html
    //Timer 2, which is actually timer 1
    TCCR3B = 0x00;          //Disbale Timer2 while we set it up
    TCNT3  = 130;           //Reset Timer Count to 130 out of 255
    TIFR3  = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
    TIMSK3 = 0x01;          //Timer2 INT Reg: Timer2 Overflow Interrupt Enable
    TCCR3A = 0x00;          //Timer2 Control Reg A: Wave Gen Mode normal
    TCCR3B = (1 << CS12);   //Timer2 Control Reg B: Timer Prescaler set to 256. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg
    //TCCR3B = (1 << CS11) | (1 << CS10);   //Timer2 Control Reg B: Timer Prescaler set to 64
   
    
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
  
//Timer2 (schedule 1) Overflow Interrupt Vector
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
