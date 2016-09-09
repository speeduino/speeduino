/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

/*
Timers are used for having actions performed repeatedly at a fixed interval (Eg every 100ms)
They should not be confused with Schedulers, which are for performing an action once at a given point of time in the future

Timers are typically low resolution (Compared to Schedulers), with maximum frequency currently being approximately every 10ms
*/
#include "timers.h"
#include "globals.h"
#include "sensors.h"

void initialiseTimers() 
{  
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //AVR chips use the ISR for this
   //Configure Timer2 for our low-freq interrupt code. 
   TCCR2B = 0x00;          //Disbale Timer2 while we set it up
   TCNT2  = 131;           //Preload timer2 with 131 cycles, leaving 125 till overflow. As the timer runs at 125Khz, this causes overflow to occur at 1Khz = 1ms
   TIFR2  = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
   TIMSK2 = 0x01;          //Timer2 Set Overflow Interrupt enabled.
   TCCR2A = 0x00;          //Timer2 Control Reg A: Wave Gen Mode normal
   /* Now configure the prescaler to CPU clock divided by 128 = 125Khz */
   TCCR2B |= (1<<CS22)  | (1<<CS20); // Set bits
   TCCR2B &= ~(1<<CS21);             // Clear bit
#endif
}


//Timer2 Overflow Interrupt Vector, called when the timer overflows.
//Executes every ~1ms.
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //AVR chips use the ISR for this
ISR(TIMER2_OVF_vect, ISR_NOBLOCK) 
#elif defined (CORE_TEENSY) && defined (__MK20DX256__)
void timer2Overflowinterrupt() //Most ARM chips can simply call a function
#endif
{
  
  //Increment Loop Counters
  loop250ms++;
  loopSec++;
  
  //Overdwell check
  targetOverdwellTime = micros() - (1000 * configPage2.dwellLimit); //Set a target time in the past that all coil charging must have begun after. If the coil charge began before this time, it's been running too long
  targetTachoPulseTime = micros() - (1500);
  //Check first whether each spark output is currently on. Only check it's dwell time if it is
  if(ignitionSchedule1.Status == RUNNING) { if(ignitionSchedule1.startTime < targetOverdwellTime && configPage2.useDwellLim) { endCoil1Charge(); } if(ignitionSchedule1.startTime < targetTachoPulseTime) { digitalWrite(pinTachOut, HIGH); } }
  if(ignitionSchedule2.Status == RUNNING) { if(ignitionSchedule2.startTime < targetOverdwellTime && configPage2.useDwellLim) { endCoil2Charge(); } if(ignitionSchedule2.startTime < targetTachoPulseTime) { digitalWrite(pinTachOut, HIGH); } }
  if(ignitionSchedule3.Status == RUNNING) { if(ignitionSchedule3.startTime < targetOverdwellTime && configPage2.useDwellLim) { endCoil3Charge(); } if(ignitionSchedule3.startTime < targetTachoPulseTime) { digitalWrite(pinTachOut, HIGH); } }
  if(ignitionSchedule4.Status == RUNNING) { if(ignitionSchedule4.startTime < targetOverdwellTime && configPage2.useDwellLim) { endCoil4Charge(); } if(ignitionSchedule4.startTime < targetTachoPulseTime) { digitalWrite(pinTachOut, HIGH); } }  
  
  //Loop executed every 250ms loop (1ms x 250 = 250ms)
  //Anything inside this if statement will run every 250ms.
  if (loop250ms == 250) 
  {
    loop250ms = 0; //Reset Counter.
  }
  
  //Loop executed every 1 second (1ms x 1000 = 1000ms)
  if (loopSec == 1000) 
  {
    loopSec = 0; //Reset counter.

    //**************************************************************************************************************************************************
    //This updates the runSecs variable
    //If the engine is running or cranking, we need ot update the run time counter.
    if (BIT_CHECK(currentStatus.engine, BIT_ENGINE_RUN))
    { //NOTE - There is a potential for a ~1sec gap between engine crank starting and ths runSec number being incremented. This may delay ASE!         
      if (currentStatus.runSecs <= 254) //Ensure we cap out at 255 and don't overflow. (which would reset ASE)
        { currentStatus.runSecs++; } //Increment our run counter by 1 second.
    }
    //**************************************************************************************************************************************************
    //This records the number of main loops the system has completed in the last second
    currentStatus.loopsPerSecond = mainLoopCount;
    mainLoopCount = 0;
    //**************************************************************************************************************************************************
    //increament secl (secl is simply a counter that increments every second and is used to track whether the system has unexpectedly reset
    currentStatus.secl++;
    //**************************************************************************************************************************************************
    //Check the fan output status
    if (configPage4.fanEnable == 1)
    { 
       fanControl();            // Fucntion to turn the cooling fan on/off 
    }
    
    //Check whether fuel pump priming is complete
    if(!fpPrimed)
    {
      if(currentStatus.secl >= configPage1.fpPrime) 
      {
        fpPrimed = true; //Mark the priming as being completed
        if(currentStatus.RPM == 0) { digitalWrite(pinFuelPump, LOW); fuelPumpOn = false; } //If we reach here then the priming is complete, however only turn off the fuel pump if the engine isn't running
      }
    }
    //**************************************************************************************************************************************************
    //Set the flex reading (if enabled). The flexCounter is updated with every pulse from the sensor. If cleared once per second, we get a frequency reading
    if(configPage1.flexEnabled)
    {
      if(flexCounter < 50)
      {
        currentStatus.flex = 0; //Standard GM Continental sensor reads from 50Hz (0 ethanol) to 150Hz (Pure ethanol). Subtracting 50 from the frequency therefore gives the ethanol percentage.
        flexCounter = 0;
      }
      else if (flexCounter > 151) //1 pulse buffer
      {
        
        if(flexCounter < 169)
        {
          currentStatus.flex = 100; //Standard GM Continental sensor reads from 50Hz (0 ethanol) to 150Hz (Pure ethanol). Subtracting 50 from the frequency therefore gives the ethanol percentage.
          flexCounter = 0;
        }
        else
        {
          //This indicates an error condition. Spec of the sensor is that errors are above 170Hz)
          currentStatus.flex = 0;
          flexCounter = 0;
        }
      }
      else
      {
        currentStatus.flex = flexCounter - 50; //Standard GM Continental sensor reads from 50Hz (0 ethanol) to 150Hz (Pure ethanol). Subtracting 50 from the frequency therefore gives the ethanol percentage.
        flexCounter = 0;
      }
      
    }

  }
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //AVR chips use the ISR for this
    //Reset Timer2 to trigger in another ~1ms 
    TCNT2 = 131;            //Preload timer2 with 100 cycles, leaving 156 till overflow.
    TIFR2  = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
#endif  
}
