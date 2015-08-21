#include <avr/interrupt.h> 
#include <avr/io.h>

//#define clockspeed 16000000

/*
unsigned long schedule1Duration; //How long (uS) after calling the start callback to we call the end callback
unsigned long schedule2Duration;
void (*schedule1StartCallback)(); //Start Callback function for schedule1
void (*schedule2StartCallback)();
void (*schedule1EndCallback)(); //End Callback function for schedule1
void (*schedule2EndCallback)();
*/

void initialiseSchedulers();
void setFuelSchedule1(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
void setSchedule2(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());

enum ScheduleStatus {OFF, PENDING, RUNNING};

struct Schedule {
  unsigned long duration;
  ScheduleStatus Status;
  void (*StartCallback)(); //Start Callback function for schedule1
  void (*EndCallback)(); //Start Callback function for schedule1
};

Schedule fuelSchedule1;
Schedule fuelSchedule2;
Schedule ignitionSchedule1;
Schedule ignitionSchedule2;

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
  
  setFuelSchedule1(callback, uSInFuture, uSInFuture, callback);
  
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
    //Fuel Schedules, which uses timer 3
    TCCR3B = 0x00;          //Disbale Timer3 while we set it up
    TCNT3  = 0;             //Reset Timer Count
    TIFR3  = 0x00;          //Timer3 INT Flag Reg: Clear Timer Overflow Flag
    TCCR3A = 0x00;          //Timer3 Control Reg A: Wave Gen Mode normal
    TCCR3B = (1 << CS12);   //Timer3 Control Reg B: Timer Prescaler set to 256. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg
    fuelSchedule1.Status = OFF;
    fuelSchedule2.Status = OFF;

    //Ignition Schedules, which uses timer 3
    TCCR4B = 0x00;          //Disbale Timer3 while we set it up
    TCNT4  = 0;             //Reset Timer Count
    TIFR4  = 0x00;          //Timer3 INT Flag Reg: Clear Timer Overflow Flag
    TCCR4A = 0x00;          //Timer3 Control Reg A: Wave Gen Mode normal
    TCCR4B = (1 << CS12);   //Timer3 Control Reg B: Timer Prescaler set to 256. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg
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
    unsigned int absoluteTimeout = TCNT3 + (timeout / 16); //Each tick occurs every 16uS with the 256 prescaler, so divide the timeout by 16 to get ther required number of ticks. Add this to the current tick count to get the target time. This will automatically overflow as required
    OCR3A = absoluteTimeout;
    fuelSchedule1.duration = duration;
    fuelSchedule1.StartCallback = startCallback; //Name the start callback function
    fuelSchedule1.EndCallback = endCallback; //Name the start callback function
    fuelSchedule1.Status = PENDING; //Turn this schedule on
    TIMSK3 |= (1 << OCIE3A); //Turn on the compare unit (ie turn on the interrupt)
  }
  
//Timer3A (schedule 1) Overflow Compare Vector
//This needs to call the callback function if one has been provided and rest the timer
ISR(TIMER3_COMPA_vect)
  {
    if (fuelSchedule1.Status == PENDING) //Check to see if this schedule is turn on
    {
      fuelSchedule1.StartCallback();
      fuelSchedule1.Status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
      unsigned int absoluteTimeout = TCNT3 + (fuelSchedule1.duration / 16);
      OCR3A = absoluteTimeout;
    }
    else if (fuelSchedule1.Status == RUNNING)
    {
       fuelSchedule1.EndCallback();
       fuelSchedule1.Status = OFF; //Turn off the schedule
       TIMSK3 &= ~(1 << OCIE3A); //Turn off this output compare unit (This simply writes 0 to the OCIE3A bit of TIMSK3)
    }
  TIFR3 = 0x00;          //Timer3 INT Flag Reg: Clear Timer Overflow Flag. I'm not 100% sure this is necessary, but better to be safe
  }
