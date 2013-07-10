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

int schedule1Active; //Value=0 means do nothing, value=1 means call the startCallback, value=2 means call the endCallback
int schedule2Active;
unsigned long schedule1Duration; //How long (uS) after calling the start callback to we call the end callback
unsigned long schedule2Duration;
void (*schedule1StartCallback)(); //Start Callback function for schedule1
void (*schedule2StartCallback)();
void (*schedule1EndCallback)(); //End Callback function for schedule1
void (*schedule2EndCallback)();


void initialiseScheduler();
void setSchedule1(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
void setSchedule2(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
