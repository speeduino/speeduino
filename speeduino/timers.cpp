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
#include "scheduler.h"
#include "scheduledIO.h"
#include "scheduler.h"
#include "auxiliaries.h"
#include "comms.h"

#if defined(CORE_AVR)
  #include <avr/wdt.h>
#endif

volatile uint16_t lastRPM_100ms; //Need to record this for rpmDOT calculation

void initialiseTimers()
{
  lastRPM_100ms = 0;
}

char getTimerFlags()
{
  static uint8_t previousMillis1ms=0;
  static uint8_t previousMillis5ms=0;
  static uint8_t previousMillis20ms=0;
  static uint8_t previousMillis33ms=1;
  static uint8_t previousMillis66ms=2;
  static uint8_t previousMillis100ms=3;
  static uint16_t previousMillis250ms=4;
  static uint16_t previousMillis1000ms=6;
  uint16_t currentMillis;
  uint8_t interval;
  byte timerflags=0;

  currentMillis =(uint16_t)millis(); // capture the latest value of millis()
  //1000Hz loop
  interval=lowByte(currentMillis) - lowByte(previousMillis1ms);
  if(interval >= (uint8_t)1U )
  {
    previousMillis1ms=lowByte(currentMillis);
    BIT_SET(TIMER_mask, BIT_TIMER_1KHZ);
  }

    //200Hz loop
  interval=lowByte(currentMillis) - previousMillis5ms;
  if(interval >= (uint8_t)5U )
  {
    previousMillis5ms=lowByte(currentMillis); 
    BIT_SET(TIMER_mask, BIT_TIMER_200HZ);
  }

  //50Hz loop
  interval=lowByte(currentMillis) - previousMillis20ms;
  if(interval >= (uint8_t)20U )
  {
    previousMillis20ms=lowByte(currentMillis); 
    BIT_SET(TIMER_mask, BIT_TIMER_50HZ);
  }
  //30Hz loop
  interval=lowByte(currentMillis) - previousMillis33ms;
  if(interval >= (uint8_t)33U )
  {
    previousMillis33ms=lowByte(currentMillis);
    BIT_SET(TIMER_mask, BIT_TIMER_30HZ);
  }

  //15Hz loop
  interval=lowByte(currentMillis) - previousMillis66ms;
  if(interval >= (uint8_t)66U )
  {
    previousMillis66ms=lowByte(currentMillis);
    BIT_SET(TIMER_mask, BIT_TIMER_15HZ);
  }

  //10Hz loop
  interval=lowByte(currentMillis) - previousMillis100ms;
  if(interval >= (uint8_t)100U )
  {
    previousMillis100ms=lowByte(currentMillis);
    BIT_SET(TIMER_mask, BIT_TIMER_10HZ);

    currentStatus.rpmDOT = (currentStatus.RPM - lastRPM_100ms) * 10; //This is the RPM per second that the engine has accelerated/decelerated in the last loop
    lastRPM_100ms = currentStatus.RPM; //Record the current RPM for next calc

    if ( currentStatus.engineIsRunning ) { runSecsX10++; }
    else { runSecsX10 = 0; }

    if ( (currentStatus.injPrimed == false) && (seclx10 >= configPage2.primingDelay) && (currentStatus.RPM == 0) && (currentStatus.initialisationComplete == true) ) 
    { 
      beginInjectorPriming(); 
      currentStatus.injPrimed = true; 
    }
    seclx10++;
  }

  //4Hz loop
  if((uint16_t)(currentMillis - previousMillis250ms) >= (uint16_t)250U )
  {
    previousMillis250ms+=(uint16_t)250U;
    BIT_SET(TIMER_mask, BIT_TIMER_4HZ);
    #if defined(CORE_STM32) //debug purpose, only visual for running code
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    #endif
  }

  //1Hz loop
  if((uint16_t)(currentMillis - previousMillis1000ms) >= (uint16_t)1000U )
  {
    previousMillis1000ms+=(uint16_t)1000U;
    BIT_SET(TIMER_mask, BIT_TIMER_1HZ);

    dwellLimit_uS = (1000 * configPage4.dwellLimit); //Update uS value in case setting has changed
    currentStatus.crankRPM = ((unsigned int)configPage4.crankRPM * 10);

    //**************************************************************************************************************************************************
    //This updates the runSecs variable
    //If the engine is running or cranking, we need to update the run time counter.
    if (currentStatus.engineIsRunning)
    { //NOTE - There is a potential for a ~1sec gap between engine crank starting and the runSec number being incremented. This may delay ASE!
      if (currentStatus.runSecs <= (UINT8_MAX-1U)) //Ensure we cap out at 255 and don't overflow. (which would reset ASE and cause problems with the closed loop fuelling (Which has to wait for the O2 to warmup))
        { currentStatus.runSecs++; } //Increment our run counter by 1 second.
    }
    //**************************************************************************************************************************************************
    //This records the number of main loops the system has completed in the last second
    currentStatus.loopsPerSecond = mainLoopCount;
    mainLoopCount = 0;
    //**************************************************************************************************************************************************
    //increment secl (secl is simply a counter that increments every second and is used to track whether the system has unexpectedly reset
    currentStatus.secl++;
    //**************************************************************************************************************************************************
    //Check the fan output status
    if (configPage2.fanEnable >= 1)
    {
       fanControl();            // Function to turn the cooling fan on/off
    }

    //Check whether fuel pump priming is complete
    if(currentStatus.fpPrimed == false)
    {
      //fpPrimeTime is the time that the pump priming started. This is 0 on startup, but can be changed if the unit has been running on USB power and then had the ignition turned on (Which starts the priming again)
      if( (currentStatus.secl - fpPrimeTime) >= configPage2.fpPrime)
      {
        currentStatus.fpPrimed = true; //Mark the priming as being completed
        if(currentStatus.RPM == 0)
        {
          //If we reach here then the priming is complete, however only turn off the fuel pump if the engine isn't running
          FUEL_PUMP_OFF();
        }
      }
    }
    //**************************************************************************************************************************************************
    //Set the flex reading (if enabled). The flexCounter is updated with every pulse from the sensor. If cleared once per second, we get a frequency reading
    if(configPage2.flexEnabled == true)
    {
      byte tempEthPct = 0; 
      if(flexCounter < configPage2.flexFreqLow)
      {
        tempEthPct = 0U; //Standard GM Continental sensor reads from 50Hz (0 ethanol) to 150Hz (Pure ethanol). Subtracting 50 from the frequency therefore gives the ethanol percentage.
        flexCounter = 0U;
      }
      else if (flexCounter > (configPage2.flexFreqHigh + 1) ) //1 pulse buffer
      {

        if(flexCounter < (configPage2.flexFreqLow + 19)) //20Hz above the max freq is considered an error condition. Everything below that should be treated as max value
        {
          tempEthPct = 100U;
          flexCounter = 0U;
        }
        else
        {
          //This indicates an error condition. Spec of the sensor is that errors are above 170Hz)
          tempEthPct = 0U;
          flexCounter = 0U;
        }
      }
      else
      {
        tempEthPct = flexCounter - configPage2.flexFreqLow; //Standard GM Continental sensor reads from 50Hz (0 ethanol) to 150Hz (Pure ethanol). Subtracting 50 from the frequency therefore gives the ethanol percentage.
        flexCounter = 0;
      }

      //Off by 1 error check
      if (tempEthPct == 1U) { tempEthPct = 0U; }

      currentStatus.ethanolPct = (uint8_t)LOW_PASS_FILTER((uint16_t)tempEthPct, configPage4.FILTER_FLEX, (uint16_t)currentStatus.ethanolPct);

      //Continental flex sensor fuel temperature can be read with following formula: (Temperature = (41.25 * pulse width(ms)) - 81.25). 1000μs = -40C and 5000μs = 125C
      flexPulseWidth = constrain(flexPulseWidth, 1000UL, 5000UL);
      int32_t tempX100 = (int32_t)rshift<10>(4224UL * flexPulseWidth) - 8125L; //Split up for MISRA compliance
      currentStatus.fuelTemp = div100((int16_t)tempX100);     
    }
  }

  //Turn off any of the pulsed testing outputs if they are active and have been running for long enough
  if( currentStatus.isTestModeActive )
  {
    //Check for pulsed injector output test
    if( (HWTest_INJ_Pulsed > 0)  )
    {
      if(testInjectorPulseCount >= configPage13.hwTestInjDuration)
      {
        if(BIT_CHECK(HWTest_INJ_Pulsed, INJ1_CMD_BIT)) { closeInjector1(); }
        if(BIT_CHECK(HWTest_INJ_Pulsed, INJ2_CMD_BIT)) { closeInjector2(); }
        if(BIT_CHECK(HWTest_INJ_Pulsed, INJ3_CMD_BIT)) { closeInjector3(); }
        if(BIT_CHECK(HWTest_INJ_Pulsed, INJ4_CMD_BIT)) { closeInjector4(); }
        if(BIT_CHECK(HWTest_INJ_Pulsed, INJ5_CMD_BIT)) { closeInjector5(); }
        if(BIT_CHECK(HWTest_INJ_Pulsed, INJ6_CMD_BIT)) { closeInjector6(); }
        if(BIT_CHECK(HWTest_INJ_Pulsed, INJ7_CMD_BIT)) { closeInjector7(); }
        if(BIT_CHECK(HWTest_INJ_Pulsed, INJ8_CMD_BIT)) { closeInjector8(); }
        
        testInjectorPulseCount = 0;
      }
      else { testInjectorPulseCount++; }
    }
    

    //Check for pulsed ignition output test
    if( (HWTest_IGN_Pulsed > 0) )
    {
      if(testIgnitionPulseCount >= configPage13.hwTestIgnDuration)
      {
        if(BIT_CHECK(HWTest_IGN_Pulsed, IGN1_CMD_BIT)) { endCoil1Charge(); }
        if(BIT_CHECK(HWTest_IGN_Pulsed, IGN2_CMD_BIT)) { endCoil2Charge(); }
        if(BIT_CHECK(HWTest_IGN_Pulsed, IGN3_CMD_BIT)) { endCoil3Charge(); }
        if(BIT_CHECK(HWTest_IGN_Pulsed, IGN4_CMD_BIT)) { endCoil4Charge(); }
        if(BIT_CHECK(HWTest_IGN_Pulsed, IGN5_CMD_BIT)) { endCoil5Charge(); }
        if(BIT_CHECK(HWTest_IGN_Pulsed, IGN6_CMD_BIT)) { endCoil6Charge(); }
        if(BIT_CHECK(HWTest_IGN_Pulsed, IGN7_CMD_BIT)) { endCoil7Charge(); }
        if(BIT_CHECK(HWTest_IGN_Pulsed, IGN8_CMD_BIT)) { endCoil8Charge(); }

        testIgnitionPulseCount = 0;
      }
      else { testIgnitionPulseCount++; }
    }
    
  }
return timerflags;
}