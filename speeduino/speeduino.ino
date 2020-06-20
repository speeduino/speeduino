/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,la
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <stdint.h> //developer.mbed.org/handbook/C-Data-Types
//************************************************
#include "globals.h"
#include "speeduino.h"
#include "table.h"
#include "scheduler.h"
#include "comms.h"
#include "cancomms.h"
#include "maths.h"
#include "corrections.h"
#include "timers.h"
#include "decoders.h"
#include "idle.h"
#include "auxiliaries.h"
#include "sensors.h"
#include "storage.h"
#include "crankMaths.h"
#include "init.h"
#include "engineProtection.h"
#include BOARD_H //Note that this is not a real file, it is defined in globals.h. 

int ignition1StartAngle = 0;
int ignition2StartAngle = 0;
int ignition3StartAngle = 0;
int ignition4StartAngle = 0;
int ignition5StartAngle = 0;
int ignition6StartAngle = 0;
int ignition7StartAngle = 0;
int ignition8StartAngle = 0;

int channel1IgnDegrees = 0; /**< The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones) */
int channel2IgnDegrees = 0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
int channel3IgnDegrees = 0; /**< The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC */
int channel4IgnDegrees = 0; /**< The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC */
int channel5IgnDegrees = 0; /**< The number of crank degrees until cylinder 5 is at TDC */
int channel6IgnDegrees = 0; /**< The number of crank degrees until cylinder 6 is at TDC */
int channel7IgnDegrees = 0; /**< The number of crank degrees until cylinder 7 is at TDC */
int channel8IgnDegrees = 0; /**< The number of crank degrees until cylinder 8 is at TDC */
int channel1InjDegrees = 0; /**< The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones) */
int channel2InjDegrees = 0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
int channel3InjDegrees = 0; /**< The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC */
int channel4InjDegrees = 0; /**< The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC */
int channel5InjDegrees = 0; /**< The number of crank degrees until cylinder 5 is at TDC */
int channel6InjDegrees = 0; /**< The number of crank degrees until cylinder 6 is at TDC */
int channel7InjDegrees = 0; /**< The number of crank degrees until cylinder 7 is at TDC */
int channel8InjDegrees = 0; /**< The number of crank degrees until cylinder 8 is at TDC */

uint16_t req_fuel_uS = 0; /**< The required fuel variable (As calculated by TunerStudio) in uS */
uint16_t inj_opentime_uS = 0;

bool ignitionOn = false; /**< The current state of the ignition system (on or off) */
bool fuelOn = false; /**< The current state of the fuel system (on or off) */

byte maxIgnOutputs = 1; /**< Used for rolling rev limiter to indicate how many total ignition channels should currently be firing */
byte curRollingCut = 0; /**< Rolling rev limiter, current ignition channel being cut */
byte rollingCutCounter = 0; /**< how many times (revolutions) the ignition has been cut in a row */
uint32_t rollingCutLastRev = 0; /**< Tracks whether we're on the same or a different rev for the rolling cut */

uint16_t staged_req_fuel_mult_pri = 0;
uint16_t staged_req_fuel_mult_sec = 0;

#ifndef UNIT_TEST // Scope guard for unit testing
void setup()
{
  initialisationComplete = false; //Tracks whether the initialiseAll() function has run completely
  initialiseAll();
}

void loop()
{
      mainLoopCount++;
      LOOP_TIMER = TIMER_mask;
      //Check for any requets from serial. Serial operations are checked under 2 scenarios:
      // 1) Every 64 loops (64 Is more than fast enough for TunerStudio). This function is equivalent to ((loopCount % 64) == 1) but is considerably faster due to not using the mod or division operations
      // 2) If the amount of data in the serial buffer is greater than a set threhold (See globals.h). This is to avoid serial buffer overflow when large amounts of data is being sent
      //if ( (BIT_CHECK(TIMER_mask, BIT_TIMER_15HZ)) || (Serial.available() > SERIAL_BUFFER_THRESHOLD) )
      if ( ((mainLoopCount & 31) == 1) or (Serial.available() > SERIAL_BUFFER_THRESHOLD) )
      {
        if (Serial.available() > 0) { command(); }
        else if(cmdPending == true)
        {
          //This is a special case just for the tooth and composite loggers
          if (currentCommand == 'T') { command(); }
        }
        
      }
      #if defined(CANSerial_AVAILABLE)
        //if can or secondary serial interface is enabled then check for requests.
        if (configPage9.enable_secondarySerial == 1)  //secondary serial interface enabled
        {
          if ( ((mainLoopCount & 31) == 1) or (CANSerial.available() > SERIAL_BUFFER_THRESHOLD) )
          {
            if (CANSerial.available() > 0)  { secondserial_Command(); }
          }
        }
      #endif
      #if defined(CORE_TEENSY)
          //currentStatus.canin[12] = configPage9.enable_intcan;
          if (configPage9.enable_intcan == 1) // use internal can module
          {
            //check local can module
            // if ( BIT_CHECK(LOOP_TIMER, BIT_TIMER_15HZ) or (CANbus0.available())
            while (Can0.read(inMsg) ) 
            {
              can_Command();
              //Can0.read(inMsg);
              //currentStatus.canin[12] = inMsg.buf[5];
              //currentStatus.canin[13] = inMsg.id;
            } 
          }
      #endif
      
      #if  defined(CORE_STM32)
          else if (configPage9.enable_intcan == 1) // can module enabled
          {
            //check local can module
          }
      #endif
          
    //Displays currently disabled
    // if (configPage2.displayType && (mainLoopCount & 255) == 1) { updateDisplay();}

    previousLoopTime = currentLoopTime;
    currentLoopTime = micros_safe();
    unsigned long timeToLastTooth = (currentLoopTime - toothLastToothTime);
    if ( (timeToLastTooth < MAX_STALL_TIME) || (toothLastToothTime > currentLoopTime) ) //Check how long ago the last tooth was seen compared to now. If it was more than half a second ago then the engine is probably stopped. toothLastToothTime can be greater than currentLoopTime if a pulse occurs between getting the lastest time and doing the comparison
    {
      currentStatus.longRPM = getRPM(); //Long RPM is included here
      currentStatus.RPM = currentStatus.longRPM;
      currentStatus.RPMdiv100 = currentStatus.RPM / 100;
      FUEL_PUMP_ON();
      currentStatus.fuelPumpOn = true; //Not sure if this is needed.
    }
    else
    {
      //We reach here if the time between teeth is too great. This VERY likely means the engine has stopped
      currentStatus.RPM = 0;
      currentStatus.PW1 = 0;
      currentStatus.VE = 0;
      currentStatus.VE2 = 0;
      toothLastToothTime = 0;
      toothLastSecToothTime = 0;
      //toothLastMinusOneToothTime = 0;
      currentStatus.hasSync = false;
      currentStatus.runSecs = 0; //Reset the counter for number of seconds running.
      currentStatus.startRevolutions = 0;
      toothSystemCount = 0;
      secondaryToothCount = 0;
      MAPcurRev = 0;
      MAPcount = 0;
      currentStatus.rpmDOT = 0;
      AFRnextCycle = 0;
      ignitionCount = 0;
      ignitionOn = false;
      fuelOn = false;
      if (fpPrimed == true) { FUEL_PUMP_OFF(); currentStatus.fuelPumpOn = false; } //Turn off the fuel pump, but only if the priming is complete
      disableIdle(); //Turn off the idle PWM
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK); //Clear cranking bit (Can otherwise get stuck 'on' even with 0 rpm)
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_WARMUP); //Same as above except for WUE
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_RUN); //Same as above except for RUNNING status
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ASE); //Same as above except for ASE status
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ACC); //Same as above but the accel enrich (If using MAP accel enrich a stall will cause this to trigger)
      //This is a safety check. If for some reason the interrupts have got screwed up (Leading to 0rpm), this resets them.
      //It can possibly be run much less frequently.
      //This should only be run if the high speed logger are off because it will change the trigger interrupts back to defaults rather than the logger versions
      if( (currentStatus.toothLogEnabled == false) && (currentStatus.compositeLogEnabled == false) ) { initialiseTriggers(); }

      VVT_PIN_LOW();
      DISABLE_VVT_TIMER();
      boostDisable();
      if(configPage4.ignBypassEnabled > 0) { digitalWrite(pinIgnBypass, LOW); } //Reset the ignition bypass ready for next crank attempt
    }

    //***Perform sensor reads***
    //-----------------------------------------------------------------------------------------------------
    readMAP();
    
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_15HZ)) //Every 32 loops
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_15HZ);
      readTPS(); //TPS reading to be performed every 32 loops (any faster and it can upset the TPSdot sampling time)
      #if  defined(CORE_TEENSY)       
          if (configPage9.enable_intcan == 1) // use internal can module
          {
           // this is just to test the interface is sending
           sendCancommand(3,(configPage9.realtime_base_address+ 0x100),currentStatus.TPS,0,0x200);
          }
      #endif     

      //Check for launching/flat shift (clutch) can be done around here too
      previousClutchTrigger = clutchTrigger;
      if(configPage6.launchHiLo > 0) { clutchTrigger = digitalRead(pinLaunch); }
      else { clutchTrigger = !digitalRead(pinLaunch); }

      if(previousClutchTrigger != clutchTrigger) { currentStatus.clutchEngagedRPM = currentStatus.RPM; }

      if (configPage6.launchEnabled && clutchTrigger && (currentStatus.clutchEngagedRPM < ((unsigned int)(configPage6.flatSArm) * 100)) && (currentStatus.RPM > ((unsigned int)(configPage6.lnchHardLim) * 100)) && (currentStatus.TPS >= configPage10.lnchCtrlTPS) ) 
      { 
        //HardCut rev limit for 2-step launch control.
        currentStatus.launchingHard = true; 
        BIT_SET(currentStatus.spark, BIT_SPARK_HLAUNCH); 
      } 
      else 
      { 
        //FLag launch as being off
        currentStatus.launchingHard = false; 
        BIT_CLEAR(currentStatus.spark, BIT_SPARK_HLAUNCH); 

        //If launch is not active, check whether flat shift should be active
        if(configPage6.flatSEnable && clutchTrigger && (currentStatus.RPM > ((unsigned int)(configPage6.flatSArm) * 100)) && (currentStatus.RPM > currentStatus.clutchEngagedRPM) ) { currentStatus.flatShiftingHard = true; }
        else { currentStatus.flatShiftingHard = false; }
      }

      //And check whether the tooth log buffer is ready
      if(toothHistoryIndex > TOOTH_LOG_SIZE) { BIT_SET(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY); }

    }
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_30HZ)) //30 hertz
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_30HZ);
      //Most boost tends to run at about 30Hz, so placing it here ensures a new target time is fetched frequently enough
      boostControl();
      //VVT may eventually need to be synced with the cam readings (ie run once per cam rev) but for now run at 30Hz
      vvtControl();
    }
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_4HZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_4HZ);
      //The IAT and CLT readings can be done less frequently (4 times per second)
      readCLT();
      readIAT();
      readO2();
      readO2_2();
      readBat();
      nitrousControl();
      idleControl(); //Perform any idle related actions. Even at higher frequencies, running 4x per second is sufficient.
      currentStatus.vss = getSpeed();
      currentStatus.gear = getGear();
      currentStatus.fuelPressure = getFuelPressure();
      currentStatus.oilPressure = getOilPressure();

      if(eepromWritesPending == true) { writeAllConfig(); } //Check for any outstanding EEPROM writes.

      if(auxIsEnabled == true)
      {
        //TODO dazq to clean this right up :)
        //check through the Aux input channels if enabed for Can or local use
        for (byte AuxinChan = 0; AuxinChan <16 ; AuxinChan++)
        {
          currentStatus.current_caninchannel = AuxinChan;          
          
          if (((configPage9.caninput_sel[currentStatus.current_caninchannel]&12) == 4) 
              && (((configPage9.enable_secondarySerial == 1) && ((configPage9.enable_intcan == 0)&&(configPage9.intcan_available == 1)))
              || ((configPage9.enable_secondarySerial == 1) && ((configPage9.enable_intcan == 1)&&(configPage9.intcan_available == 1))&& 
              ((configPage9.caninput_sel[currentStatus.current_caninchannel]&64) == 0))
              || ((configPage9.enable_secondarySerial == 1) && ((configPage9.enable_intcan == 1)&&(configPage9.intcan_available == 0)))))              
          { //if current input channel is enabled as external & secondary serial enabled & internal can disabled(but internal can is available)
            // or current input channel is enabled as external & secondary serial enabled & internal can enabled(and internal can is available)
            //currentStatus.canin[13] = 11;  Dev test use only!
            if (configPage9.enable_secondarySerial == 1)  // megas only support can via secondary serial
            {
              sendCancommand(2,0,currentStatus.current_caninchannel,0,((configPage9.caninput_source_can_address[currentStatus.current_caninchannel]&2047)+0x100));
              //send an R command for data from caninput_source_address[currentStatus.current_caninchannel] from CANSERIAL
            }
          }  
          else if (((configPage9.caninput_sel[currentStatus.current_caninchannel]&12) == 4) 
              && (((configPage9.enable_secondarySerial == 1) && ((configPage9.enable_intcan == 1)&&(configPage9.intcan_available == 1))&& 
              ((configPage9.caninput_sel[currentStatus.current_caninchannel]&64) == 64))
              || ((configPage9.enable_secondarySerial == 0) && ((configPage9.enable_intcan == 1)&&(configPage9.intcan_available == 1))&& 
              ((configPage9.caninput_sel[currentStatus.current_caninchannel]&128) == 128))))                             
          { //if current input channel is enabled as external for canbus & secondary serial enabled & internal can enabled(and internal can is available)
            // or current input channel is enabled as external for canbus & secondary serial disabled & internal can enabled(and internal can is available)
            //currentStatus.canin[13] = 12;  Dev test use only!  
          #if defined(CORE_STM32) || defined(CORE_TEENSY)
           if (configPage9.enable_intcan == 1) //  if internal can is enabled 
           {
              sendCancommand(3,configPage9.speeduino_tsCanId,currentStatus.current_caninchannel,0,((configPage9.caninput_source_can_address[currentStatus.current_caninchannel]&2047)+0x100));  
              //send an R command for data from caninput_source_address[currentStatus.current_caninchannel] from internal canbus
           }
          #endif
          }   
          else if ((((configPage9.enable_secondarySerial == 1) || ((configPage9.enable_intcan == 1) && (configPage9.intcan_available == 1))) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&12) == 8)
                  || (((configPage9.enable_secondarySerial == 0) && ( (configPage9.enable_intcan == 1) && (configPage9.intcan_available == 0) )) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&3) == 2)  
                  || (((configPage9.enable_secondarySerial == 0) && (configPage9.enable_intcan == 0)) && ((configPage9.caninput_sel[currentStatus.current_caninchannel]&3) == 2)))  
          { //if current input channel is enabled as analog local pin
            //read analog channel specified
            //currentStatus.canin[13] = (configPage9.Auxinpina[currentStatus.current_caninchannel]&63);  Dev test use only!127
            currentStatus.canin[currentStatus.current_caninchannel] = readAuxanalog(configPage9.Auxinpina[currentStatus.current_caninchannel]&63);
          }
          else if ((((configPage9.enable_secondarySerial == 1) || ((configPage9.enable_intcan == 1) && (configPage9.intcan_available == 1))) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&12) == 12)
                  || (((configPage9.enable_secondarySerial == 0) && ( (configPage9.enable_intcan == 1) && (configPage9.intcan_available == 0) )) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&3) == 3)
                  || (((configPage9.enable_secondarySerial == 0) && (configPage9.enable_intcan == 0)) && ((configPage9.caninput_sel[currentStatus.current_caninchannel]&3) == 3)))
          { //if current input channel is enabled as digital local pin
            //read digital channel specified
            //currentStatus.canin[14] = ((configPage9.Auxinpinb[currentStatus.current_caninchannel]&63)+1);  Dev test use only!127+1
            currentStatus.canin[currentStatus.current_caninchannel] = readAuxdigital((configPage9.Auxinpinb[currentStatus.current_caninchannel]&63)+1);
          } //Channel type
        } //For loop going through each channel
      } //aux channels are enabled
    } //4Hz timer
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_1HZ)) //Once per second)
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_1HZ);
      readBaro(); //Infrequent baro readings are not an issue.
    } //1Hz timer

    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_CL) )  { idleControl(); } //Run idlecontrol every loop for stepper idle.

    
    //VE calculation was moved outside the sync/RPM check so that the fuel load value will be accurately shown when RPM=0
    currentStatus.VE1 = getVE1();
    currentStatus.VE = currentStatus.VE1; //Set the final VE value to be VE 1 as a default. This may be changed in the section belo

    //If the secondary fuel table is in use, also get the VE value from there
    BIT_CLEAR(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE); //Clear the bit indicating that the 2nd fuel table is in use. 
    if(configPage10.fuel2Mode > 0)
    { 
      if(configPage10.fuel2Mode == FUEL2_MODE_MULTIPLY)
      {
        currentStatus.VE2 = getVE2();
        //Fuel 2 table is treated as a % value. Table 1 and 2 are multiplied together and divded by 100
        uint16_t combinedVE = ((uint16_t)currentStatus.VE1 * (uint16_t)currentStatus.VE2) / 100;
        if(combinedVE <= 255) { currentStatus.VE = combinedVE; }
        else { currentStatus.VE = 255; }
      }
      else if(configPage10.fuel2Mode == FUEL2_MODE_ADD)
      {
        currentStatus.VE2 = getVE2();
        //Fuel tables are added together, but a check is made to make sure this won't overflow the 8-bit VE value
        uint16_t combinedVE = (uint16_t)currentStatus.VE1 + (uint16_t)currentStatus.VE2;
        if(combinedVE <= 255) { currentStatus.VE = combinedVE; }
        else { currentStatus.VE = 255; }
      }
      else if(configPage10.fuel2Mode == FUEL2_MODE_CONDITIONAL_SWITCH )
      {
        if(configPage10.fuel2SwitchVariable == FUEL2_CONDITION_RPM)
        {
          if(currentStatus.RPM > configPage10.fuel2SwitchValue)
          {
            BIT_SET(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE); //Set the bit indicating that the 2nd fuel table is in use. 
            currentStatus.VE2 = getVE2();
            currentStatus.VE = currentStatus.VE2;
          }
        }
        else if(configPage10.fuel2SwitchVariable == FUEL2_CONDITION_MAP)
        {
          if(currentStatus.MAP > configPage10.fuel2SwitchValue)
          {
            BIT_SET(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE); //Set the bit indicating that the 2nd fuel table is in use. 
            currentStatus.VE2 = getVE2();
            currentStatus.VE = currentStatus.VE2;
          }
        }
        else if(configPage10.fuel2SwitchVariable == FUEL2_CONDITION_TPS)
        {
          if(currentStatus.TPS > configPage10.fuel2SwitchValue)
          {
            BIT_SET(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE); //Set the bit indicating that the 2nd fuel table is in use. 
            currentStatus.VE2 = getVE2();
            currentStatus.VE = currentStatus.VE2;
          }
        }
        else if(configPage10.fuel2SwitchVariable == FUEL2_CONDITION_ETH)
        {
          if(currentStatus.ethanolPct > configPage10.fuel2SwitchValue)
          {
            BIT_SET(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE); //Set the bit indicating that the 2nd fuel table is in use. 
            currentStatus.VE2 = getVE2();
            currentStatus.VE = currentStatus.VE2;
          }
        }
      }
      else if(configPage10.fuel2Mode == FUEL2_MODE_INPUT_SWITCH)
      {
        if(digitalRead(pinFuel2Input) == configPage10.fuel2InputPolarity)
        {
          BIT_SET(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE); //Set the bit indicating that the 2nd fuel table is in use. 
          currentStatus.VE2 = getVE2();
          currentStatus.VE = currentStatus.VE2;
        }
      }
    }

    //Always check for sync
    //Main loop runs within this clause
    if (currentStatus.hasSync && (currentStatus.RPM > 0))
    {
        if(currentStatus.startRevolutions >= configPage4.StgCycles)  { ignitionOn = true; fuelOn = true; } //Enable the fuel and ignition, assuming staging revolutions are complete
        //Check whether running or cranking
        if(currentStatus.RPM > currentStatus.crankRPM) //Crank RPM in the config is stored as a x10. currentStatus.crankRPM is set in timers.ino and represents the true value
        {
          BIT_SET(currentStatus.engine, BIT_ENGINE_RUN); //Sets the engine running bit
          //Only need to do anything if we're transitioning from cranking to running
          if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
          {
            BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
            if(configPage4.ignBypassEnabled > 0) { digitalWrite(pinIgnBypass, HIGH); }
          }
        }
        else
        {  
          //Sets the engine cranking bit, clears the engine running bit
          BIT_SET(currentStatus.engine, BIT_ENGINE_CRANK);
          BIT_CLEAR(currentStatus.engine, BIT_ENGINE_RUN);
          currentStatus.runSecs = 0; //We're cranking (hopefully), so reset the engine run time to prompt ASE.
          if(configPage4.ignBypassEnabled > 0) { digitalWrite(pinIgnBypass, LOW); }

          //Check whether the user has selected to disable to the fan during cranking
          if(configPage2.fanWhenCranking == 0) { FAN_OFF(); }
        }
      //END SETTING STATUSES
      //-----------------------------------------------------------------------------------------------------

      //Begin the fuel calculation
      //Calculate an injector pulsewidth from the VE
      currentStatus.corrections = correctionsFuel();

      currentStatus.advance = getAdvance();

      currentStatus.PW1 = PW(req_fuel_uS, currentStatus.VE, currentStatus.MAP, currentStatus.corrections, inj_opentime_uS);

      //Manual adder for nitrous. These are not in correctionsFuel() because they are direct adders to the ms value, not % based
      if( (currentStatus.nitrous_status == NITROUS_STAGE1) || (currentStatus.nitrous_status == NITROUS_BOTH) )
      { 
        int16_t adderRange = (configPage10.n2o_stage1_maxRPM - configPage10.n2o_stage1_minRPM) * 100;
        int16_t adderPercent = ((currentStatus.RPM - (configPage10.n2o_stage1_minRPM * 100)) * 100) / adderRange; //The percentage of the way through the RPM range
        adderPercent = 100 - adderPercent; //Flip the percentage as we go from a higher adder to a lower adder as the RPMs rise
        currentStatus.PW1 = currentStatus.PW1 + (configPage10.n2o_stage1_adderMax + percentage(adderPercent, (configPage10.n2o_stage1_adderMin - configPage10.n2o_stage1_adderMax))) * 100; //Calculate the above percentage of the calculated ms value.
      }
      if( (currentStatus.nitrous_status == NITROUS_STAGE2) || (currentStatus.nitrous_status == NITROUS_BOTH) )
      {
        int16_t adderRange = (configPage10.n2o_stage2_maxRPM - configPage10.n2o_stage2_minRPM) * 100;
        int16_t adderPercent = ((currentStatus.RPM - (configPage10.n2o_stage2_minRPM * 100)) * 100) / adderRange; //The percentage of the way through the RPM range
        adderPercent = 100 - adderPercent; //Flip the percentage as we go from a higher adder to a lower adder as the RPMs rise
        currentStatus.PW1 = currentStatus.PW1 + (configPage10.n2o_stage2_adderMax + percentage(adderPercent, (configPage10.n2o_stage2_adderMin - configPage10.n2o_stage2_adderMax))) * 100; //Calculate the above percentage of the calculated ms value.
      }

      int injector1StartAngle = 0;
      uint16_t injector2StartAngle = 0;
      uint16_t injector3StartAngle = 0;
      uint16_t injector4StartAngle = 0;

      #if INJ_CHANNELS >= 5
      uint16_t injector5StartAngle = 0;
      #endif
      #if INJ_CHANNELS >= 6
      uint16_t injector6StartAngle = 0;
      #endif
      #if INJ_CHANNELS >= 7
      uint16_t injector7StartAngle = 0;
      #endif
      #if INJ_CHANNELS >= 8
      uint16_t injector8StartAngle = 0;
      #endif
      //These are used for comparisons on channels above 1 where the starting angle (for injectors or ignition) can be less than a single loop time
      //(Don't ask why this is needed, it's just there)
      int tempCrankAngle;
      int tempStartAngle;

      doCrankSpeedCalcs(); //In crankMaths.ino

      //Check that the duty cycle of the chosen pulsewidth isn't too high.
      unsigned long pwLimit = percentage(configPage2.dutyLim, revolutionTime); //The pulsewidth limit is determined to be the duty cycle limit (Eg 85%) by the total time it takes to perform 1 revolution
      //Handle multiple squirts per rev
      if (configPage2.strokes == FOUR_STROKE) { pwLimit = pwLimit * 2 / currentStatus.nSquirts; } 
      else { pwLimit = pwLimit / currentStatus.nSquirts; }
      //Apply the pwLimit if staging is dsiabled and engine is not cranking
      if( (!BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK)) && (configPage10.stagingEnabled == false) ) { if (currentStatus.PW1 > pwLimit) { currentStatus.PW1 = pwLimit; } }

      //Calculate staging pulsewidths if used
      //To run staged injection, the number of cylinders must be less than or equal to the injector channels (ie Assuming you're running paired injection, you need at least as many injector channels as you have cylinders, half for the primaries and half for the secondaries)
      if( (configPage10.stagingEnabled == true) && (configPage2.nCylinders <= INJ_CHANNELS) && (currentStatus.PW1 > inj_opentime_uS) ) //Final check is to ensure that DFCO isn't active, which would cause an overflow below (See #267)
      {
        //Scale the 'full' pulsewidth by each of the injector capacities
        currentStatus.PW1 -= inj_opentime_uS; //Subtract the opening time from PW1 as it needs to be multiplied out again by the pri/sec req_fuel values below. It is added on again after that calculation. 
        uint32_t tempPW1 = (((unsigned long)currentStatus.PW1 * staged_req_fuel_mult_pri) / 100);

        if(configPage10.stagingMode == STAGING_MODE_TABLE)
        {
          uint32_t tempPW3 = (((unsigned long)currentStatus.PW1 * staged_req_fuel_mult_sec) / 100); //This is ONLY needed in in table mode. Auto mode only calculates the difference.

          byte stagingSplit = get3DTableValue(&stagingTable, currentStatus.MAP, currentStatus.RPM);
          currentStatus.PW1 = ((100 - stagingSplit) * tempPW1) / 100;
          currentStatus.PW1 += inj_opentime_uS; 

          if(stagingSplit > 0) 
          { 
            currentStatus.PW3 = (stagingSplit * tempPW3) / 100; 
            currentStatus.PW3 += inj_opentime_uS;
          }
          else { currentStatus.PW3 = 0; }
        }
        else if(configPage10.stagingMode == STAGING_MODE_AUTO)
        {
          currentStatus.PW1 = tempPW1;
          //If automatic mode, the primary injectors are used all the way up to their limit (Configured by the pulsewidth limit setting)
          //If they exceed their limit, the extra duty is passed to the secondaries
          if(tempPW1 > pwLimit)
          {
            uint32_t extraPW = tempPW1 - pwLimit + inj_opentime_uS; //The open time must be added here AND below because tempPW1 does not include an open time. The addition of it here takes into account the fact that pwLlimit does not contain an allowance for an open time. 
            currentStatus.PW1 = pwLimit;
            currentStatus.PW3 = ((extraPW * staged_req_fuel_mult_sec) / staged_req_fuel_mult_pri); //Convert the 'left over' fuel amount from primary injector scaling to secondary
            currentStatus.PW3 += inj_opentime_uS;
          }
          else { currentStatus.PW3 = 0; } //If tempPW1 < pwLImit it means that the entire fuel load can be handled by the primaries. Simply set the secondaries to 0
        }

        //Set the 2nd channel of each stage with the same pulseWidth
        currentStatus.PW2 = currentStatus.PW1;
        currentStatus.PW4 = currentStatus.PW3;
      }
      else 
      { 
        //If staging is off, all the pulse widths are set the same (Sequential and other adjustments may be made below)
        currentStatus.PW2 = currentStatus.PW1;
        currentStatus.PW3 = currentStatus.PW1;
        currentStatus.PW4 = currentStatus.PW1;
        currentStatus.PW5 = currentStatus.PW1;
        currentStatus.PW6 = currentStatus.PW1;
        currentStatus.PW7 = currentStatus.PW1;
        currentStatus.PW8 = currentStatus.PW1;
      }

      //***********************************************************************************************
      //BEGIN INJECTION TIMING
      currentStatus.injAngle = table2D_getValue(&injectorAngleTable, currentStatus.RPM / 100);
      unsigned int PWdivTimerPerDegree = div(currentStatus.PW1, timePerDegree).quot; //How many crank degrees the calculated PW will take at the current speed

      //This is a little primitive, but is based on the idea that all fuel needs to be delivered before the inlet valve opens. See www.extraefi.co.uk/sequential_fuel.html for more detail
      //if(configPage2.inj1Ang > PWdivTimerPerDegree) { injector1StartAngle = configPage2.inj1Ang - ( PWdivTimerPerDegree ); }
      //else { injector1StartAngle = configPage2.inj1Ang + CRANK_ANGLE_MAX_INJ - PWdivTimerPerDegree; } //Just incase 
      //while(injector1StartAngle > CRANK_ANGLE_MAX_INJ) { injector1StartAngle -= CRANK_ANGLE_MAX_INJ; }

      injector1StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees);

      //Repeat the above for each cylinder
      switch (configPage2.nCylinders)
      {
        //Single cylinder
        case 1:
          //The only thing that needs to be done for single cylinder is to check for staging. 
          if( (configPage10.stagingEnabled == true) && (currentStatus.PW3 > 0) )
          {
            PWdivTimerPerDegree = div(currentStatus.PW3, timePerDegree).quot; //Need to redo this for PW3 as it will be dramatically different to PW1 when staging
            //injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);
            injector3StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees);
          }
          break;
        //2 cylinders
        case 2:
          //injector2StartAngle = calculateInjector2StartAngle(PWdivTimerPerDegree);
          injector2StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees);
          if( (configPage10.stagingEnabled == true) && (currentStatus.PW3 > 0) )
          {
            PWdivTimerPerDegree = div(currentStatus.PW3, timePerDegree).quot; //Need to redo this for PW3 as it will be dramatically different to PW1 when staging
            //injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);
            injector3StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees);

            injector4StartAngle = injector3StartAngle + (CRANK_ANGLE_MAX_INJ / 2); //Phase this either 180 or 360 degrees out from inj3 (In reality this will always be 180 as you can't have sequential and staged currently)
            if(injector4StartAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { injector4StartAngle -= CRANK_ANGLE_MAX_INJ; }
          }
          break;
        //3 cylinders
        case 3:
          //injector2StartAngle = calculateInjector2StartAngle(PWdivTimerPerDegree);
          //injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);
          injector2StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees);
          injector3StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees);
          break;
        //4 cylinders
        case 4:
          //injector2StartAngle = calculateInjector2StartAngle(PWdivTimerPerDegree);
          injector2StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees);

          if(configPage2.injLayout == INJ_SEQUENTIAL)
          {
            //injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);
            //injector4StartAngle = calculateInjector4StartAngle(PWdivTimerPerDegree);
            injector3StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees);
            injector4StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees);

            if(configPage6.fuelTrimEnabled > 0)
            {
              unsigned long pw1percent = 100 + (byte)get3DTableValue(&trim1Table, currentStatus.MAP, currentStatus.RPM) - OFFSET_FUELTRIM;
              unsigned long pw2percent = 100 + (byte)get3DTableValue(&trim2Table, currentStatus.MAP, currentStatus.RPM) - OFFSET_FUELTRIM;
              unsigned long pw3percent = 100 + (byte)get3DTableValue(&trim3Table, currentStatus.MAP, currentStatus.RPM) - OFFSET_FUELTRIM;
              unsigned long pw4percent = 100 + (byte)get3DTableValue(&trim4Table, currentStatus.MAP, currentStatus.RPM) - OFFSET_FUELTRIM;

              if (pw1percent != 100) { currentStatus.PW1 = (pw1percent * currentStatus.PW1) / 100; }
              if (pw2percent != 100) { currentStatus.PW2 = (pw2percent * currentStatus.PW2) / 100; }
              if (pw3percent != 100) { currentStatus.PW3 = (pw3percent * currentStatus.PW3) / 100; }
              if (pw4percent != 100) { currentStatus.PW4 = (pw4percent * currentStatus.PW4) / 100; }
            }
          }
          else if( (configPage10.stagingEnabled == true) && (currentStatus.PW3 > 0) )
          {
            PWdivTimerPerDegree = div(currentStatus.PW3, timePerDegree).quot; //Need to redo this for PW3 as it will be dramatically different to PW1 when staging
            //injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);
            injector3StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees);

            injector4StartAngle = injector3StartAngle + (CRANK_ANGLE_MAX_INJ / 2); //Phase this either 180 or 360 degrees out from inj3 (In reality this will always be 180 as you can't have sequential and staged currently)
            if(injector4StartAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { injector4StartAngle -= CRANK_ANGLE_MAX_INJ; }
          }
          break;
        //5 cylinders
        case 5:
          injector2StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees);
          injector3StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees);
          injector4StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees);
          #if INJ_CHANNELS >= 5
            injector5StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel5InjDegrees);
          #endif
          break;
        //6 cylinders
        case 6:
          //injector2StartAngle = calculateInjector2StartAngle(PWdivTimerPerDegree);
          //injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);

          injector2StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees);
          injector3StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees);
          
          #if INJ_CHANNELS >= 6
            if(configPage2.injLayout == INJ_SEQUENTIAL)
            {
              injector4StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees);
              injector5StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel5InjDegrees);
              injector6StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel6InjDegrees);
            }
          #endif
          break;
        //8 cylinders
        case 8:
        /*
          injector2StartAngle = calculateInjector2StartAngle(PWdivTimerPerDegree);
          injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);
          injector4StartAngle = calculateInjector4StartAngle(PWdivTimerPerDegree);
          */

          injector2StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees);
          injector3StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees);
          injector4StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees);

          #if INJ_CHANNELS >= 8
            if(configPage2.injLayout == INJ_SEQUENTIAL)
            {
              injector5StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel5InjDegrees);
              injector6StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel6InjDegrees);
              injector7StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel7InjDegrees);
              injector8StartAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channel8InjDegrees);
            }
          #endif
          break;

        //Will hit the default case on 1 cylinder or >8 cylinders. Do nothing in these cases
        default:
          break;
      }

      //***********************************************************************************************
      //| BEGIN IGNITION CALCULATIONS

      //Set dwell
      //Dwell is stored as ms * 10. ie Dwell of 4.3ms would be 43 in configPage4. This number therefore needs to be multiplied by 100 to get dwell in uS
      if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) ) { currentStatus.dwell =  (configPage4.dwellCrank * 100); }
      else { currentStatus.dwell =  (configPage4.dwellRun * 100); }
      currentStatus.dwell = correctionsDwell(currentStatus.dwell);

      int dwellAngle = timeToAngle(currentStatus.dwell, CRANKMATH_METHOD_INTERVAL_REV); //Convert the dwell time to dwell angle based on the current engine speed

      calculateIgnitionAngles(dwellAngle);

      //If ignition timing is being tracked per tooth, perform the calcs to get the end teeth
      //This only needs to be run if the advance figure has changed, otherwise the end teeth will still be the same
      //if( (configPage2.perToothIgn == true) && (lastToothCalcAdvance != currentStatus.advance) ) { triggerSetEndTeeth(); }
      if( (configPage2.perToothIgn == true) ) { triggerSetEndTeeth(); }

      //***********************************************************************************************
      //| BEGIN FUEL SCHEDULES
      //Finally calculate the time (uS) until we reach the firing angles and set the schedules
      //We only need to set the shcedule if we're BEFORE the open angle
      //This may potentially be called a number of times as we get closer and closer to the opening time

      //Determine the current crank angle
      int crankAngle = getCrankAngle();
      while(crankAngle > CRANK_ANGLE_MAX_INJ ) { crankAngle = crankAngle - CRANK_ANGLE_MAX_INJ; } //Continue reducing the crank angle by the max injection amount until it's below the required limit. This will usually only run (at most) once, but in cases where there is sequential ignition and more than 2 squirts per cycle, it may run up to 4 times. 

      // if(Serial && false)
      // {
      //   if(ignition1StartAngle > crankAngle)
      //   {
      //     noInterrupts();
      //     Serial.print("Time2LastTooth:"); Serial.println(micros()-toothLastToothTime);
      //     Serial.print("elapsedTime:"); Serial.println(elapsedTime);
      //     Serial.print("CurAngle:"); Serial.println(crankAngle);
      //     Serial.print("RPM:"); Serial.println(currentStatus.RPM);
      //     Serial.print("Tooth:"); Serial.println(toothCurrentCount);
      //     Serial.print("timePerDegree:"); Serial.println(timePerDegree);
      //     Serial.print("IGN1Angle:"); Serial.println(ignition1StartAngle);
      //     Serial.print("TimeToIGN1:"); Serial.println(angleToTime((ignition1StartAngle - crankAngle), CRANKMATH_METHOD_INTERVAL_REV));
      //     interrupts();
      //   }
      // }
      
      //Check for any of the engine protections or rev limiters being turned on
      if(checkEngineProtect() || currentStatus.launchingHard || currentStatus.flatShiftingHard)
      {
        if(currentStatus.RPMdiv100 > configPage4.engineProtectMaxRPM)
        {
          if(configPage2.hardCutType == HARD_CUT_FULL) 
          { 
            switch(configPage6.engineProtectType)
            {
              case PROTECT_CUT_OFF:
                ignitionOn = true;
                fuelOn = true;
                break;
              case PROTECT_CUT_IGN:
                ignitionOn = false;
                break;
              case PROTECT_CUT_FUEL:
                fuelOn = false;
                break;
              case PROTECT_CUT_BOTH:
                ignitionOn = false;
                fuelOn = false;
                break;
              default:
                ignitionOn = false;
                fuelOn = false;
                break;
            }
          }
          else 
          { 
            if(rollingCutCounter >= 2) //Vary this number to change the intensity of the roll. The higher the number, the closer is it to full cut
            { 
              //Rolls through each of the active ignition channels based on how many revolutions have taken place
              //curRollingCut = ( (currentStatus.startRevolutions / 2) % maxIgnOutputs) + 1;
              rollingCutCounter = 0;
              ignitionOn = true;
              curRollingCut = 0;
            }
            else
            {
              if(rollingCutLastRev == 0) { rollingCutLastRev = currentStatus.startRevolutions; } //
              if (rollingCutLastRev != currentStatus.startRevolutions)
              {
                rollingCutLastRev = currentStatus.startRevolutions;
                rollingCutCounter++;
              }
              ignitionOn = false; //Finally the ignition is fully cut completely
            }
          } //Hard/Rolling cut check
        } //RPM Check
        else { currentStatus.engineProtectStatus = 0; } //Force all engine protection flags to be off as we're below the minimum RPM
      } //Protection active check
      else { curRollingCut = 0; } //Disables the rolling hard cut

#if INJ_CHANNELS >= 1
      if (fuelOn && !BIT_CHECK(currentStatus.status1, BIT_STATUS1_BOOSTCUT))
      {
        if(currentStatus.PW1 >= inj_opentime_uS)
        {
          if ( (injector1StartAngle <= crankAngle) && (fuelSchedule1.Status == RUNNING) ) { injector1StartAngle += CRANK_ANGLE_MAX_INJ; }
          if (injector1StartAngle > crankAngle)
          {
            setFuelSchedule1(
                      ((injector1StartAngle - crankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW1
                      );
          }
        }
#endif

        /*-----------------------------------------------------------------------------------------
        | A Note on tempCrankAngle and tempStartAngle:
        |   The use of tempCrankAngle/tempStartAngle is described below. It is then used in the same way for channels 2, 3 and 4 on both injectors and ignition
        |   Essentially, these 2 variables are used to realign the current crank angle and the desired start angle around 0 degrees for the given cylinder/output
        |   Eg: If cylinder 2 TDC is 180 degrees after cylinder 1 (Eg a standard 4 cylidner engine), then tempCrankAngle is 180* less than the current crank angle and
        |       tempStartAngle is the desired open time less 180*. Thus the cylinder is being treated relative to its own TDC, regardless of its offset
        |
        |   This is done to avoid problems with very short of very long times until tempStartAngle.
        |   This will very likely need to be rewritten when sequential is enabled
        |------------------------------------------------------------------------------------------
        */
#if INJ_CHANNELS >= 2
        if( (channel2InjEnabled) && (currentStatus.PW2 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel2InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector2StartAngle - channel2InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule2.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            setFuelSchedule2(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW2
                      );
          }
        }
#endif

#if INJ_CHANNELS >= 3
        if( (channel3InjEnabled) && (currentStatus.PW3 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel3InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector3StartAngle - channel3InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule3.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            setFuelSchedule3(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW3
                      );
          }
        }
#endif

#if INJ_CHANNELS >= 4
        if( (channel4InjEnabled) && (currentStatus.PW4 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel4InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector4StartAngle - channel4InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule4.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            setFuelSchedule4(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW4
                      );
          }
        }
#endif

#if INJ_CHANNELS >= 5
        if( (channel5InjEnabled) && (currentStatus.PW5 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel5InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector5StartAngle - channel5InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule5.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            //Note the hacky use of fuel schedule 3 below
            /*
            setFuelSchedule3(openInjector3and5,
                      ((unsigned long)(tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW1,
                      closeInjector3and5
                    );*/
            
            setFuelSchedule5(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW5
                      );
          }
        }
#endif

#if INJ_CHANNELS >= 6
        if( (channel6InjEnabled) && (currentStatus.PW6 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel6InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector6StartAngle - channel6InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule6.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            setFuelSchedule6(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW6
                      );
          }
        }
#endif

#if INJ_CHANNELS >= 7
        if( (channel7InjEnabled) && (currentStatus.PW7 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel7InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector7StartAngle - channel7InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule7.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            setFuelSchedule7(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW7
                      );
          }
        }
#endif

#if INJ_CHANNELS >= 8
        if( (channel8InjEnabled) && (currentStatus.PW8 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel8InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector8StartAngle - channel8InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule8.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            setFuelSchedule8(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW8
                      );
          }
        }
#endif
      }
      //***********************************************************************************************
      //| BEGIN IGNITION SCHEDULES
      //Same as above, except for ignition

      //fixedCrankingOverride is used to extend the dwell during cranking so that the decoder can trigger the spark upon seeing a certain tooth. Currently only available on the basic distributor and 4g63 decoders.
      if ( configPage4.ignCranklock && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && (decoderHasFixedCrankingTiming == true) )
      {
        fixedCrankingOverride = currentStatus.dwell * 3;
        //This is a safety step to prevent the ignition start time occuring AFTER the target tooth pulse has already occcured. It simply moves the start time forward a little, which is compensated for by the increase in the dwell time
        if(currentStatus.RPM < 250)
        {
          ignition1StartAngle -= 5;
          ignition2StartAngle -= 5;
          ignition3StartAngle -= 5;
          ignition4StartAngle -= 5;
          ignition5StartAngle -= 5;
          ignition6StartAngle -= 5;
          ignition7StartAngle -= 5;
          ignition8StartAngle -= 5;
        }
      }
      else { fixedCrankingOverride = 0; }

      if(ignitionOn)
      {
        //Refresh the current crank angle info
        //ignition1StartAngle = 335;
        crankAngle = getCrankAngle(); //Refresh with the latest crank angle
        while (crankAngle > CRANK_ANGLE_MAX_IGN ) { crankAngle -= CRANK_ANGLE_MAX_IGN; }

#if IGN_CHANNELS >= 1
        if ( (ignition1StartAngle <= crankAngle) && (ignitionSchedule1.Status == RUNNING) ) { ignition1StartAngle += CRANK_ANGLE_MAX_IGN; }
        if ( (ignition1StartAngle > crankAngle) && (curRollingCut != 1) )
        {
          setIgnitionSchedule1(ign1StartFunction,
                    //((unsigned long)(ignition1StartAngle - crankAngle) * (unsigned long)timePerDegree),
                    angleToTime((ignition1StartAngle - crankAngle), CRANKMATH_METHOD_INTERVAL_REV),
                    currentStatus.dwell + fixedCrankingOverride, //((unsigned long)((unsigned long)currentStatus.dwell* currentStatus.RPM) / newRPM) + fixedCrankingOverride,
                    ign1EndFunction
                    );
        }
#endif

#if defined(USE_IGN_REFRESH)
        if( (ignitionSchedule1.Status == RUNNING) && (ignition1EndAngle > crankAngle) && (configPage4.StgCycles == 0) && (configPage2.perToothIgn != true) )
        {
          unsigned long uSToEnd = 0;

          crankAngle = getCrankAngle(); //Refresh with the latest crank angle
          if (crankAngle > CRANK_ANGLE_MAX_IGN ) { crankAngle -= 360; }
          
          //ONLY ONE OF THE BELOW SHOULD BE USED (PROBABLY THE FIRST):
          //*********
          if(ignition1EndAngle > crankAngle) { uSToEnd = fastDegreesToUS( (ignition1EndAngle - crankAngle) ); }
          else { uSToEnd = fastDegreesToUS( (360 + ignition1EndAngle - crankAngle) ); }
          //*********
          //uSToEnd = ((ignition1EndAngle - crankAngle) * (toothLastToothTime - toothLastMinusOneToothTime)) / triggerToothAngle;
          //*********

          refreshIgnitionSchedule1( uSToEnd + fixedCrankingOverride );
        }
  #endif
        


#if IGN_CHANNELS >= 2
        tempCrankAngle = crankAngle - channel2IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_IGN; }
        tempStartAngle = ignition2StartAngle - channel2IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
        //if (tempStartAngle > tempCrankAngle)
        {
            unsigned long ignition2StartTime = 0;
            if ( (tempStartAngle <= tempCrankAngle) && (ignitionSchedule2.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
            if(tempStartAngle > tempCrankAngle) { ignition2StartTime = angleToTime((tempStartAngle - tempCrankAngle), CRANKMATH_METHOD_INTERVAL_REV); }
            //else if (tempStartAngle < tempCrankAngle) { ignition2StartTime = ((long)(360 - tempCrankAngle + tempStartAngle) * (long)timePerDegree); }
            else { ignition2StartTime = 0; }

            if( (ignition2StartTime > 0) && (curRollingCut != 2) )
            {
              setIgnitionSchedule2(ign2StartFunction,
                        ignition2StartTime,
                        currentStatus.dwell + fixedCrankingOverride,
                        ign2EndFunction
                        );
            }
        }
#endif

#if IGN_CHANNELS >= 3
        tempCrankAngle = crankAngle - channel3IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_IGN; }
        tempStartAngle = ignition3StartAngle - channel3IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
        //if (tempStartAngle > tempCrankAngle)
        {
            unsigned long ignition3StartTime = 0;
            if ( (tempStartAngle <= tempCrankAngle) && (ignitionSchedule3.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
            if(tempStartAngle > tempCrankAngle) { ignition3StartTime = angleToTime((tempStartAngle - tempCrankAngle), CRANKMATH_METHOD_INTERVAL_REV); }
            //else if (tempStartAngle < tempCrankAngle) { ignition3StartTime = ((long)(360 - tempCrankAngle + tempStartAngle) * (long)timePerDegree); }
            else { ignition3StartTime = 0; }

            if( (ignition3StartTime > 0) && (curRollingCut != 3) )
            {
              setIgnitionSchedule3(ign3StartFunction,
                        ignition3StartTime,
                        currentStatus.dwell + fixedCrankingOverride,
                        ign3EndFunction
                        );
            }
        }
#endif

#if IGN_CHANNELS >= 4
        tempCrankAngle = crankAngle - channel4IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_IGN; }
        tempStartAngle = ignition4StartAngle - channel4IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
        //if (tempStartAngle > tempCrankAngle)
        {

            unsigned long ignition4StartTime = 0;
            if ( (tempStartAngle <= tempCrankAngle) && (ignitionSchedule4.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
            if(tempStartAngle > tempCrankAngle) { ignition4StartTime = angleToTime((tempStartAngle - tempCrankAngle), CRANKMATH_METHOD_INTERVAL_REV); }
            //else if (tempStartAngle < tempCrankAngle) { ignition4StartTime = ((long)(360 - tempCrankAngle + tempStartAngle) * (long)timePerDegree); }
            else { ignition4StartTime = 0; }

            if( (ignition4StartTime > 0) && (curRollingCut != 4) )
            {
              setIgnitionSchedule4(ign4StartFunction,
                        ignition4StartTime,
                        currentStatus.dwell + fixedCrankingOverride,
                        ign4EndFunction
                        );
            }
        }
#endif

#if IGN_CHANNELS >= 5
        tempCrankAngle = crankAngle - channel5IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_IGN; }
        tempStartAngle = ignition5StartAngle - channel5IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
        //if (tempStartAngle > tempCrankAngle)
        {

            unsigned long ignition5StartTime = 0;
            if ( (tempStartAngle <= tempCrankAngle) && (ignitionSchedule5.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
            if(tempStartAngle > tempCrankAngle) { ignition5StartTime = angleToTime((tempStartAngle - tempCrankAngle), CRANKMATH_METHOD_INTERVAL_REV); }
            //else if (tempStartAngle < tempCrankAngle) { ignition5StartTime = ((long)(360 - tempCrankAngle + tempStartAngle) * (long)timePerDegree); }
            else { ignition5StartTime = 0; }

            if( (ignition5StartTime > 0) && (curRollingCut != 5) )
            {
              setIgnitionSchedule5(ign5StartFunction,
                        ignition5StartTime,
                        currentStatus.dwell + fixedCrankingOverride,
                        ign5EndFunction
                        );
            }
        }
#endif

#if IGN_CHANNELS >= 6
        tempCrankAngle = crankAngle - channel6IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_IGN; }
        tempStartAngle = ignition6StartAngle - channel6IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
        //if (tempStartAngle > tempCrankAngle)
        {
            unsigned long ignition6StartTime = 0;
            if ( (tempStartAngle <= tempCrankAngle) && (ignitionSchedule6.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
            if(tempStartAngle > tempCrankAngle) { ignition6StartTime = angleToTime((tempStartAngle - tempCrankAngle), CRANKMATH_METHOD_INTERVAL_REV); }
            //else if (tempStartAngle < tempCrankAngle) { ignition6StartTime = ((long)(360 - tempCrankAngle + tempStartAngle) * (long)timePerDegree); }
            else { ignition6StartTime = 0; }

            if( (ignition6StartTime > 0) && (curRollingCut != 6) )
            {
              setIgnitionSchedule6(ign6StartFunction,
                        ignition6StartTime,
                        currentStatus.dwell + fixedCrankingOverride,
                        ign6EndFunction
                        );
            }
        }
#endif

#if IGN_CHANNELS >= 7
        tempCrankAngle = crankAngle - channel7IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_IGN; }
        tempStartAngle = ignition7StartAngle - channel7IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
        //if (tempStartAngle > tempCrankAngle)
        {
            unsigned long ignition7StartTime = 0;
            if ( (tempStartAngle <= tempCrankAngle) && (ignitionSchedule7.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
            if(tempStartAngle > tempCrankAngle) { ignition7StartTime = angleToTime((tempStartAngle - tempCrankAngle), CRANKMATH_METHOD_INTERVAL_REV); }
            //else if (tempStartAngle < tempCrankAngle) { ignition7StartTime = ((long)(360 - tempCrankAngle + tempStartAngle) * (long)timePerDegree); }
            else { ignition7StartTime = 0; }

            if( (ignition7StartTime > 0) && (curRollingCut != 7) )
            {
              setIgnitionSchedule7(ign7StartFunction,
                        ignition7StartTime,
                        currentStatus.dwell + fixedCrankingOverride,
                        ign7EndFunction
                        );
            }
        }
#endif

#if IGN_CHANNELS >= 8
        tempCrankAngle = crankAngle - channel8IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_IGN; }
        tempStartAngle = ignition8StartAngle - channel8IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
        //if (tempStartAngle > tempCrankAngle)
        {
            unsigned long ignition8StartTime = 0;
            if ( (tempStartAngle <= tempCrankAngle) && (ignitionSchedule8.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
            if(tempStartAngle > tempCrankAngle) { ignition8StartTime = angleToTime((tempStartAngle - tempCrankAngle), CRANKMATH_METHOD_INTERVAL_REV); }
            //else if (tempStartAngle < tempCrankAngle) { ignition8StartTime = ((long)(360 - tempCrankAngle + tempStartAngle) * (long)timePerDegree); }
            else { ignition8StartTime = 0; }

            if( (ignition8StartTime > 0) && (curRollingCut != 8) )
            {
              setIgnitionSchedule8(ign8StartFunction,
                        ignition8StartTime,
                        currentStatus.dwell + fixedCrankingOverride,
                        ign8EndFunction
                        );
            }
        }
#endif

      } //Ignition schedules on

      if ( (!BIT_CHECK(currentStatus.status3, BIT_STATUS3_RESET_PREVENT)) && (resetControl == RESET_CONTROL_PREVENT_WHEN_RUNNING) ) 
      {
        //Reset prevention is supposed to be on while the engine is running but isn't. Fix that.
        digitalWrite(pinResetControl, HIGH);
        BIT_SET(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);
      }
    } //Has sync and RPM
    else if ( (BIT_CHECK(currentStatus.status3, BIT_STATUS3_RESET_PREVENT) > 0) && (resetControl == RESET_CONTROL_PREVENT_WHEN_RUNNING) )
    {
      digitalWrite(pinResetControl, LOW);
      BIT_CLEAR(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);
    }
} //loop()
#endif //Unit test guard

/**
 * @brief This function calculates the required pulsewidth time (in us) given the current system state
 * 
 * @param REQ_FUEL The required fuel value in uS, as calculated by TunerStudio
 * @param VE Lookup from the main fuel table. This can either have been MAP or TPS based, depending on the algorithm used
 * @param MAP In KPa, read from the sensor (This is used when performing a multiply of the map only. It is applicable in both Speed density and Alpha-N)
 * @param corrections Sum of Enrichment factors (Cold start, acceleration). This is a multiplication factor (Eg to add 10%, this should be 110)
 * @param injOpen Injector opening time. The time the injector take to open minus the time it takes to close (Both in uS)
 * @return uint16_t The injector pulse width in uS
 */
uint16_t PW(int REQ_FUEL, byte VE, long MAP, uint16_t corrections, int injOpen)
{
  //Standard float version of the calculation
  //return (REQ_FUEL * (float)(VE/100.0) * (float)(MAP/100.0) * (float)(TPS/100.0) * (float)(corrections/100.0) + injOpen);
  //Note: The MAP and TPS portions are currently disabled, we use VE and corrections only
  uint16_t iVE, iCorrections;
  uint16_t iMAP = 100;
  uint16_t iAFR = 147;

  //100% float free version, does sacrifice a little bit of accuracy, but not much.

  //If corrections are huge, use less bitshift to avoid overflow. Sacrifices a bit more accuracy (basically only during very cold temp cranking)
  byte bitShift = 7;
  if (corrections > 511 ) { bitShift = 6; }
  if (corrections > 1023) { bitShift = 5; }
  
  iVE = ((unsigned int)VE << 7) / 100;
  if ( configPage2.multiplyMAP == true ) {
    iMAP = ((unsigned int)MAP << 7) / currentStatus.baro;  //Include multiply MAP (vs baro) if enabled
  }
  if ( (configPage2.includeAFR == true) && (configPage6.egoType == 2) && (currentStatus.runSecs > configPage6.ego_sdelay) ) {
    iAFR = ((unsigned int)currentStatus.O2 << 7) / currentStatus.afrTarget;  //Include AFR (vs target) if enabled
  }
  iCorrections = (corrections << bitShift) / 100;


  unsigned long intermediate = ((uint32_t)REQ_FUEL * (uint32_t)iVE) >> 7; //Need to use an intermediate value to avoid overflowing the long
  if ( configPage2.multiplyMAP == true ) {
    intermediate = (intermediate * (unsigned long)iMAP) >> 7;
  }
  if ( (configPage2.includeAFR == true) && (configPage6.egoType == 2) && (currentStatus.runSecs > configPage6.ego_sdelay) ) {
    //EGO type must be set to wideband and the AFR warmup time must've elapsed for this to be used
    intermediate = (intermediate * (unsigned long)iAFR) >> 7;  
  }
  intermediate = (intermediate * (unsigned long)iCorrections) >> bitShift;
  if (intermediate != 0)
  {
    //If intermeditate is not 0, we need to add the opening time (0 typically indicates that one of the full fuel cuts is active)
    intermediate += injOpen; //Add the injector opening time
    if ( intermediate > 65535)
    {
      intermediate = 65535;  //Make sure this won't overflow when we convert to uInt. This means the maximum pulsewidth possible is 65.535mS
    }
  }
  return (unsigned int)(intermediate);
}

/**
 * @brief Lookup the current VE value from the primary 3D fuel map. The Y axis value used for this lookup varies based on the fuel algorithm selected (speed density, alpha-n etc)
 * 
 * @return byte The current VE value
 */
byte getVE1()
{
  byte tempVE = 100;
  if (configPage2.fuelAlgorithm == LOAD_SOURCE_MAP) //Check which fuelling algorithm is being used
  {
    //Speed Density
    currentStatus.fuelLoad = currentStatus.MAP;
  }
  else if (configPage2.fuelAlgorithm == LOAD_SOURCE_TPS)
  {
    //Alpha-N
    currentStatus.fuelLoad = currentStatus.TPS;
  }
  else if (configPage2.fuelAlgorithm == LOAD_SOURCE_IMAPEMAP)
  {
    //IMAP / EMAP
    currentStatus.fuelLoad = (currentStatus.MAP * 100) / currentStatus.EMAP;
  }
  else { currentStatus.fuelLoad = currentStatus.MAP; } //Fallback position
  tempVE = get3DTableValue(&fuelTable, currentStatus.fuelLoad, currentStatus.RPM); //Perform lookup into fuel map for RPM vs MAP value

  return tempVE;
}

/**
 * @brief Looks up and returns the VE value from the secondary fuel table
 * 
 * This performs largely the same operations as getVE() however the lookup is of the secondary fuel table and uses the secondary load source
 * @return byte 
 */
byte getVE2()
{
  byte tempVE = 100;
  if( configPage10.fuel2Algorithm == LOAD_SOURCE_MAP)
  {
    //Speed Density
    currentStatus.fuelLoad2 = currentStatus.MAP;
  }
  else if (configPage10.fuel2Algorithm == LOAD_SOURCE_TPS)
  {
    //Alpha-N
    currentStatus.fuelLoad2 = currentStatus.TPS;
  }
  else if (configPage10.fuel2Algorithm == LOAD_SOURCE_IMAPEMAP)
  {
    //IMAP / EMAP
    currentStatus.fuelLoad2 = (currentStatus.MAP * 100) / currentStatus.EMAP;
  }
  else { currentStatus.fuelLoad2 = currentStatus.MAP; } //Fallback position
  tempVE = get3DTableValue(&fuelTable2, currentStatus.fuelLoad2, currentStatus.RPM); //Perform lookup into fuel map for RPM vs MAP value

  return tempVE;
}

/**
 * @brief Performs a lookup of the ignition advance table. The values used to look this up will be RPM and whatever load source the user has configured
 * 
 * @return byte The current target advance value in degrees
 */
byte getAdvance()
{
  byte tempAdvance = 0;
  if (configPage2.ignAlgorithm == LOAD_SOURCE_MAP) //Check which fuelling algorithm is being used
  {
    //Speed Density
    currentStatus.ignLoad = currentStatus.MAP;
  }
  else if(configPage2.ignAlgorithm == LOAD_SOURCE_TPS)
  {
    //Alpha-N
    currentStatus.ignLoad = currentStatus.TPS;

  }
  else if (configPage2.fuelAlgorithm == LOAD_SOURCE_IMAPEMAP)
  {
    //IMAP / EMAP
    currentStatus.ignLoad = (currentStatus.MAP * 100) / currentStatus.EMAP;
  }
  tempAdvance = get3DTableValue(&ignitionTable, currentStatus.ignLoad, currentStatus.RPM) - OFFSET_IGNITION; //As above, but for ignition advance
  tempAdvance = correctionsIgn(tempAdvance);

  return tempAdvance;
}

/*
uint16_t calculateInjector2StartAngle(unsigned int PWdivTimerPerDegree)
{
  uint16_t tempInjector2StartAngle = (currentStatus.injAngle + channel2InjDegrees); //This makes the start angle equal to the end angle
  if(tempInjector2StartAngle < PWdivTimerPerDegree) { tempInjector2StartAngle += CRANK_ANGLE_MAX_INJ; }
  tempInjector2StartAngle -= PWdivTimerPerDegree; //Subtract the number of degrees the PW will take to get the start angle
  if(tempInjector2StartAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { tempInjector2StartAngle -= CRANK_ANGLE_MAX_INJ; }

  return tempInjector2StartAngle;
}
*/

uint16_t calculateInjectorStartAngle(uint16_t PWdivTimerPerDegree, int16_t injChannelDegrees)
{
  uint16_t tempInjectorStartAngle = (currentStatus.injAngle + injChannelDegrees);
  if(tempInjectorStartAngle < PWdivTimerPerDegree) { tempInjectorStartAngle += CRANK_ANGLE_MAX_INJ; }
  tempInjectorStartAngle -= PWdivTimerPerDegree;
  while(tempInjectorStartAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { tempInjectorStartAngle -= CRANK_ANGLE_MAX_INJ; }

  return tempInjectorStartAngle;
}

void calculateIgnitionAngles(int dwellAngle)
{
  //Calculate start and eng angle for each channel

  //1 cylinder (Everyone gets this)
  ignition1EndAngle = CRANK_ANGLE_MAX_IGN - currentStatus.advance;
  if(ignition1EndAngle > CRANK_ANGLE_MAX_IGN) {ignition1EndAngle -= CRANK_ANGLE_MAX_IGN;}
  ignition1StartAngle = ignition1EndAngle - dwellAngle; // 360 - desired advance angle - number of degrees the dwell will take
  if(ignition1StartAngle < 0) {ignition1StartAngle += CRANK_ANGLE_MAX_IGN;}

  //This test for more cylinders and do the same thing
  switch (configPage2.nCylinders)
  {
    //2 cylinders
    case 2:
      ignition2EndAngle = channel2IgnDegrees - currentStatus.advance;
      if(ignition2EndAngle > CRANK_ANGLE_MAX_IGN) {ignition2EndAngle -= CRANK_ANGLE_MAX_IGN;}
      ignition2StartAngle = ignition2EndAngle - dwellAngle;
      if(ignition2StartAngle < 0) {ignition2StartAngle += CRANK_ANGLE_MAX_IGN;}
      break;
    //3 cylinders
    case 3:
      ignition2EndAngle = channel2IgnDegrees - currentStatus.advance;
      if(ignition2EndAngle > CRANK_ANGLE_MAX_IGN) {ignition2EndAngle -= CRANK_ANGLE_MAX_IGN;}
      ignition2StartAngle = ignition2EndAngle - dwellAngle;
      if(ignition2StartAngle < 0) {ignition2StartAngle += CRANK_ANGLE_MAX_IGN;}

      ignition3EndAngle = channel3IgnDegrees - currentStatus.advance;
      if(ignition3EndAngle > CRANK_ANGLE_MAX_IGN) {ignition3EndAngle -= CRANK_ANGLE_MAX_IGN;}
      ignition3StartAngle = ignition3EndAngle - dwellAngle;
      if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}
      break;
    //4 cylinders
    case 4:
      ignition2EndAngle = channel2IgnDegrees - currentStatus.advance;
      if(ignition2EndAngle > CRANK_ANGLE_MAX_IGN) {ignition2EndAngle -= CRANK_ANGLE_MAX_IGN;}
      ignition2StartAngle = ignition2EndAngle - dwellAngle;
      if(ignition2StartAngle < 0) {ignition2StartAngle += CRANK_ANGLE_MAX_IGN;}

      if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
      {
        ignition3EndAngle = channel3IgnDegrees - currentStatus.advance;
        if(ignition3EndAngle > CRANK_ANGLE_MAX_IGN) {ignition3EndAngle -= CRANK_ANGLE_MAX_IGN;}
        ignition3StartAngle = ignition3EndAngle - dwellAngle;
        if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}

        ignition4EndAngle = channel4IgnDegrees - currentStatus.advance;
        if(ignition4EndAngle > CRANK_ANGLE_MAX_IGN) {ignition4EndAngle -= CRANK_ANGLE_MAX_IGN;}
        ignition4StartAngle = ignition4EndAngle - dwellAngle;
        if(ignition4StartAngle < 0) {ignition4StartAngle += CRANK_ANGLE_MAX_IGN;}
      }
      else if(configPage4.sparkMode == IGN_MODE_ROTARY)
      {
        byte splitDegrees = 0;
        if (configPage2.fuelAlgorithm == LOAD_SOURCE_MAP) { splitDegrees = table2D_getValue(&rotarySplitTable, currentStatus.MAP/2); }
        else { splitDegrees = table2D_getValue(&rotarySplitTable, currentStatus.TPS/2); }

        //The trailing angles are set relative to the leading ones
        ignition3EndAngle = ignition1EndAngle + splitDegrees;
        ignition3StartAngle = ignition3EndAngle - dwellAngle;
        if(ignition3StartAngle > CRANK_ANGLE_MAX_IGN) {ignition3StartAngle -= CRANK_ANGLE_MAX_IGN;}
        if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}

        ignition4EndAngle = ignition2EndAngle + splitDegrees;
        ignition4StartAngle = ignition4EndAngle - dwellAngle;
        if(ignition4StartAngle > CRANK_ANGLE_MAX_IGN) {ignition4StartAngle -= CRANK_ANGLE_MAX_IGN;}
        if(ignition4StartAngle < 0) {ignition4StartAngle += CRANK_ANGLE_MAX_IGN;}
      }
      break;
    //5 cylinders
    case 5:
      ignition2EndAngle = channel2IgnDegrees - currentStatus.advance;
      if(ignition2EndAngle > CRANK_ANGLE_MAX_IGN) {ignition2EndAngle -= CRANK_ANGLE_MAX_IGN;}
      ignition2StartAngle = ignition2EndAngle - dwellAngle;
      if(ignition2StartAngle < 0) {ignition2StartAngle += CRANK_ANGLE_MAX_IGN;}

      ignition3EndAngle = channel3IgnDegrees - currentStatus.advance;
      if(ignition3EndAngle > CRANK_ANGLE_MAX_IGN) {ignition3EndAngle -= CRANK_ANGLE_MAX_IGN;}
      ignition3StartAngle = ignition3EndAngle - dwellAngle;
      if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}

      ignition4EndAngle = channel4IgnDegrees - currentStatus.advance;
      if(ignition4EndAngle > CRANK_ANGLE_MAX_IGN) {ignition4EndAngle -= CRANK_ANGLE_MAX_IGN;}
      ignition4StartAngle = ignition4EndAngle - dwellAngle;
      if(ignition4StartAngle < 0) {ignition4StartAngle += CRANK_ANGLE_MAX_IGN;}

      ignition5EndAngle = channel5IgnDegrees - currentStatus.advance;
      if(ignition5EndAngle > CRANK_ANGLE_MAX_IGN) {ignition5EndAngle -= CRANK_ANGLE_MAX_IGN;}
      ignition5StartAngle = ignition5EndAngle - dwellAngle;
      if(ignition5StartAngle < 0) {ignition5StartAngle += CRANK_ANGLE_MAX_IGN;}

      break;
    //6 cylinders
    case 6:
      ignition2EndAngle = channel2IgnDegrees - currentStatus.advance;
      if(ignition2EndAngle > CRANK_ANGLE_MAX_IGN) {ignition2EndAngle -= CRANK_ANGLE_MAX_IGN;}
      ignition2StartAngle = ignition2EndAngle - dwellAngle;
      if(ignition2StartAngle < 0) {ignition2StartAngle += CRANK_ANGLE_MAX_IGN;}

      ignition3EndAngle = channel3IgnDegrees - currentStatus.advance;
      if(ignition3EndAngle > CRANK_ANGLE_MAX_IGN) {ignition3EndAngle -= CRANK_ANGLE_MAX_IGN;}
      ignition3StartAngle = ignition3EndAngle - dwellAngle;
      if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}
      #if IGN_CHANNELS >= 6
      if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
      {
        ignition4EndAngle = channel4IgnDegrees - currentStatus.advance;
        if(ignition4EndAngle > CRANK_ANGLE_MAX_IGN) {ignition4EndAngle -= CRANK_ANGLE_MAX_IGN;}
        ignition4StartAngle = ignition4EndAngle - dwellAngle;
        if(ignition4StartAngle < 0) {ignition4StartAngle += CRANK_ANGLE_MAX_IGN;}

        ignition5EndAngle = channel5IgnDegrees - currentStatus.advance;
        if(ignition5EndAngle > CRANK_ANGLE_MAX_IGN) {ignition5EndAngle -= CRANK_ANGLE_MAX_IGN;}
        ignition5StartAngle = ignition5EndAngle - dwellAngle;
        if(ignition5StartAngle < 0) {ignition5StartAngle += CRANK_ANGLE_MAX_IGN;}

        ignition6EndAngle = channel6IgnDegrees - currentStatus.advance;
        if(ignition6EndAngle > CRANK_ANGLE_MAX_IGN) {ignition6EndAngle -= CRANK_ANGLE_MAX_IGN;}
        ignition6StartAngle = ignition6EndAngle - dwellAngle;
        if(ignition6StartAngle < 0) {ignition6StartAngle += CRANK_ANGLE_MAX_IGN;}
      }
      #endif
      break;
    //8 cylinders
    case 8:
      ignition2EndAngle = channel2IgnDegrees - currentStatus.advance;
      if(ignition2EndAngle > CRANK_ANGLE_MAX_IGN) {ignition2EndAngle -= CRANK_ANGLE_MAX_IGN;}
      ignition2StartAngle = ignition2EndAngle - dwellAngle;
      if(ignition2StartAngle < 0) {ignition2StartAngle += CRANK_ANGLE_MAX_IGN;}

      ignition3EndAngle = channel3IgnDegrees - currentStatus.advance;
      if(ignition3EndAngle > CRANK_ANGLE_MAX_IGN) {ignition3EndAngle -= CRANK_ANGLE_MAX_IGN;}
      ignition3StartAngle = ignition3EndAngle - dwellAngle;
      if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}

      ignition4EndAngle = channel4IgnDegrees - currentStatus.advance;
      if(ignition4EndAngle > CRANK_ANGLE_MAX_IGN) {ignition4EndAngle -= CRANK_ANGLE_MAX_IGN;}
      ignition4StartAngle = ignition4EndAngle - dwellAngle;
      if(ignition4StartAngle < 0) {ignition4StartAngle += CRANK_ANGLE_MAX_IGN;}
      #if IGN_CHANNELS >= 8
      if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
      {
        ignition5EndAngle = channel5IgnDegrees - currentStatus.advance;
        if(ignition5EndAngle > CRANK_ANGLE_MAX_IGN) {ignition5EndAngle -= CRANK_ANGLE_MAX_IGN;}
        ignition5StartAngle = ignition5EndAngle - dwellAngle;
        if(ignition5StartAngle < 0) {ignition5StartAngle += CRANK_ANGLE_MAX_IGN;}

        ignition6EndAngle = channel6IgnDegrees - currentStatus.advance;
        if(ignition6EndAngle > CRANK_ANGLE_MAX_IGN) {ignition6EndAngle -= CRANK_ANGLE_MAX_IGN;}
        ignition6StartAngle = ignition6EndAngle - dwellAngle;
        if(ignition6StartAngle < 0) {ignition6StartAngle += CRANK_ANGLE_MAX_IGN;}

        ignition7EndAngle = channel7IgnDegrees - currentStatus.advance;
        if(ignition7EndAngle > CRANK_ANGLE_MAX_IGN) {ignition7EndAngle -= CRANK_ANGLE_MAX_IGN;}
        ignition7StartAngle = ignition7EndAngle - dwellAngle;
        if(ignition7StartAngle < 0) {ignition7StartAngle += CRANK_ANGLE_MAX_IGN;}

        ignition8EndAngle = channel8IgnDegrees - currentStatus.advance;
        if(ignition8EndAngle > CRANK_ANGLE_MAX_IGN) {ignition8EndAngle -= CRANK_ANGLE_MAX_IGN;}
        ignition8StartAngle = ignition8EndAngle - dwellAngle;
        if(ignition8StartAngle < 0) {ignition8StartAngle += CRANK_ANGLE_MAX_IGN;}
      }
      #endif
      break;

    //Will hit the default case on 1 cylinder or >8 cylinders. Do nothing in these cases
    default:
      break;
  }
}
