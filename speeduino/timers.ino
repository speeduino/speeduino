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

#if defined(CORE_AVR)
  #include <avr/wdt.h>
#endif

void initialiseTimers()
{
#if defined(CORE_AVR) //AVR chips use the ISR for this
   //Configure Timer2 for our low-freq interrupt code.
   TCCR2B = 0x00;          //Disbale Timer2 while we set it up
   TCNT2  = 131;           //Preload timer2 with 131 cycles, leaving 125 till overflow. As the timer runs at 125Khz, this causes overflow to occur at 1Khz = 1ms
   TIFR2  = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
   TIMSK2 = 0x01;          //Timer2 Set Overflow Interrupt enabled.
   TCCR2A = 0x00;          //Timer2 Control Reg A: Wave Gen Mode normal
   /* Now configure the prescaler to CPU clock divided by 128 = 125Khz */
   TCCR2B |= (1<<CS22)  | (1<<CS20); // Set bits
   TCCR2B &= ~(1<<CS21);             // Clear bit

   //Enable the watchdog timer for 2 second resets (Good reference: https://tushev.org/articles/arduino/5/arduino-and-watchdog-timer)
   //wdt_enable(WDTO_2S); //Boooooooooo WDT is currently broken on Mega 2560 bootloaders :(

#elif defined (CORE_TEENSY)
   //Uses the PIT timer on Teensy.
   lowResTimer.begin(oneMSInterval, 1000);

#elif defined(CORE_STM32)
  Timer4.setChannel1Mode(TIMER_OUTPUTCOMPARE);
  Timer4.setPeriod(1000);
  Timer4.attachCompare1Interrupt(oneMSInterval);
#endif

  dwellLimit_uS = (1000 * configPage2.dwellLimit);
  lastRPM_100ms = 0;
}


//Timer2 Overflow Interrupt Vector, called when the timer overflows.
//Executes every ~1ms.
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //AVR chips use the ISR for this
ISR(TIMER2_OVF_vect, ISR_NOBLOCK)
#elif defined (CORE_TEENSY) || defined(CORE_STM32)
void oneMSInterval() //Most ARM chips can simply call a function
#endif
{

  //Increment Loop Counters
  loop100ms++;
  loop250ms++;
  loopSec++;

  unsigned long targetOverdwellTime;

  //Overdwell check
  targetOverdwellTime = micros() - dwellLimit_uS; //Set a target time in the past that all coil charging must have begun after. If the coil charge began before this time, it's been running too long
  //Check first whether each spark output is currently on. Only check it's dwell time if it is

  if(ignitionSchedule1.Status == RUNNING) { if(ignitionSchedule1.startTime < targetOverdwellTime && configPage2.useDwellLim) { endCoil1Charge(); } }
  if(ignitionSchedule2.Status == RUNNING) { if(ignitionSchedule2.startTime < targetOverdwellTime && configPage2.useDwellLim) { endCoil2Charge(); } }
  if(ignitionSchedule3.Status == RUNNING) { if(ignitionSchedule3.startTime < targetOverdwellTime && configPage2.useDwellLim) { endCoil3Charge(); } }
  if(ignitionSchedule4.Status == RUNNING) { if(ignitionSchedule4.startTime < targetOverdwellTime && configPage2.useDwellLim) { endCoil4Charge(); } }
  if(ignitionSchedule5.Status == RUNNING) { if(ignitionSchedule5.startTime < targetOverdwellTime && configPage2.useDwellLim) { endCoil5Charge(); } }

  //Loop executed every 100ms loop
  //Anything inside this if statement will run every 100ms.
  if (loop100ms == 100)
  {
    loop100ms = 0; //Reset counter

    currentStatus.rpmDOT = (currentStatus.RPM - lastRPM_100ms) * 10; //This is the RPM per second that the engine has accelerated/decelleratedin the last loop
    lastRPM_100ms = currentStatus.RPM; //Record the current RPM for next calc
  }

  //Loop executed every 250ms loop (1ms x 250 = 250ms)
  //Anything inside this if statement will run every 250ms.
  if (loop250ms == 250)
  {
    loop250ms = 0; //Reset Counter.
    #if defined(CORE_AVR)
      //wdt_reset(); //Reset watchdog timer
    #endif
  }

  //Loop executed every 1 second (1ms x 1000 = 1000ms)
  if (loopSec == 1000)
  {
    loopSec = 0; //Reset counter.

    dwellLimit_uS = (1000 * configPage2.dwellLimit); //Update uS value incase setting has changed
    if ( configPage2.ignCranklock && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK)) { dwellLimit_uS = dwellLimit_uS * 3; } //Make sure the overdwell doesn't clobber the fixed ignition cranking if enabled.

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
        currentStatus.ethanolPct = 0; //Standard GM Continental sensor reads from 50Hz (0 ethanol) to 150Hz (Pure ethanol). Subtracting 50 from the frequency therefore gives the ethanol percentage.
        flexCounter = 0;
      }
      else if (flexCounter > 151) //1 pulse buffer
      {

        if(flexCounter < 169)
        {
          currentStatus.ethanolPct = 100;
          flexCounter = 0;
        }
        else
        {
          //This indicates an error condition. Spec of the sensor is that errors are above 170Hz)
          currentStatus.ethanolPct = 0;
          flexCounter = 0;
        }
      }
      else
      {
        currentStatus.ethanolPct = flexCounter - 50; //Standard GM Continental sensor reads from 50Hz (0 ethanol) to 150Hz (Pure ethanol). Subtracting 50 from the frequency therefore gives the ethanol percentage.
        flexCounter = 0;
      }

      //Off by 1 error check
      if (currentStatus.ethanolPct == 1) { currentStatus.ethanolPct = 0; }

    }

  }
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //AVR chips use the ISR for this
    //Reset Timer2 to trigger in another ~1ms
    TCNT2 = 131;            //Preload timer2 with 100 cycles, leaving 156 till overflow.
    TIFR2  = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
#endif
}
