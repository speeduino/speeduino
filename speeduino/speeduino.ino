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
/** @file
 * Speeduino initialisation and main loop.
 */
#include <stdint.h> //developer.mbed.org/handbook/C-Data-Types
//************************************************
#include "globals.h"
#include "speeduino.h"
#include "scheduler.h"
#include "comms.h"
#include "comms_legacy.h"
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
#include "utilities.h"
#include "engineProtection.h"
#include "scheduledIO.h"
#include "secondaryTables.h"
#include "canBroadcast.h"
#include "SD_logger.h"
#include RTC_LIB_H //Defined in each boards .h file
#include BOARD_H //Note that this is not a real file, it is defined in globals.h. 


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

byte curRollingCut = 0; /**< Rolling rev limiter, current ignition channel being cut */
byte rollingCutCounter = 0; /**< how many times (revolutions) the ignition has been cut in a row */
uint32_t rollingCutLastRev = 0; /**< Tracks whether we're on the same or a different rev for the rolling cut */

uint16_t staged_req_fuel_mult_pri = 0;
uint16_t staged_req_fuel_mult_sec = 0;   
#ifndef UNIT_TEST // Scope guard for unit testing
void setup(void)
{
  initialisationComplete = false; //Tracks whether the initialiseAll() function has run completely
  initialiseAll();
}

inline uint16_t applyFuelTrimToPW(trimTable3d *pTrimTable, int16_t fuelLoad, int16_t RPM, uint16_t currentPW)
{
    unsigned long pw1percent = 100 + get3DTableValue(pTrimTable, fuelLoad, RPM) - OFFSET_FUELTRIM;
    if (pw1percent != 100) { return div100(pw1percent * currentPW); }
    return currentPW;
}

/** Speeduino main loop.
 * 
 * Main loop chores (roughly in the order that they are performed):
 * - Check if serial comms or tooth logging are in progress (send or receive, prioritise communication)
 * - Record loop timing vars
 * - Check tooth time, update @ref statuses (currentStatus) variables
 * - Read sensors
 * - get VE for fuel calcs and spark advance for ignition
 * - Check crank/cam/tooth/timing sync (skip remaining ops if out-of-sync)
 * - execute doCrankSpeedCalcs()
 * 
 * single byte variable @ref LOOP_TIMER plays a big part here as:
 * - it contains expire-bits for interval based frequency driven events (e.g. 15Hz, 4Hz, 1Hz)
 * - Can be tested for certain frequency interval being expired by (eg) BIT_CHECK(LOOP_TIMER, BIT_TIMER_15HZ)
 * 
 */
void loop(void)
{
      unsigned int dwellLimit_uS;      
      
      int injector1EndAngle = 0;
      uint16_t injector2EndAngle = 0;
      uint16_t injector3EndAngle = 0;
      uint16_t injector4EndAngle = 0;

      #if INJ_CHANNELS >= 5
      uint16_t injector5EndAngle = 0;
      #endif
      #if INJ_CHANNELS >= 6
      uint16_t injector6EndAngle = 0;
      #endif
      #if INJ_CHANNELS >= 7
      uint16_t injector7EndAngle = 0;
      #endif
      #if INJ_CHANNELS >= 8
      uint16_t injector8EndAngle = 0;
      #endif
      //These are used for comparisons on channels above 1 where the starting angle (for injectors or ignition) can be less than a single loop time
      //(Don't ask why this is needed, it's just there)
      //int tempCrankAngle;
      //int tempStartAngle;

      mainLoopCount++;
      LOOP_TIMER = TIMER_mask;

      //SERIAL Comms
      //Initially check that the last serial send values request is not still outstanding
      if (serialInProgress == true) 
      { 
        if(Serial.availableForWrite() > 16) { sendValues(inProgressOffset, inProgressLength, 0x30, 0); }
      }
      //Perform the same check for the tooth and composite logs
      if( toothLogSendInProgress == true)
      {
        if(Serial.availableForWrite() > 16) 
        { 
          if(legacySerial == true) { sendToothLog_legacy(inProgressOffset); }
          else { sendToothLog(inProgressOffset); }
        }
      }
      if( compositeLogSendInProgress == true)
      {
        if(Serial.availableForWrite() > 16) { sendCompositeLog(inProgressOffset); }
      }
      if(serialWriteInProgress == true)
      {
        if(Serial.availableForWrite() > 16) { continueSerialTransmission(); }
      }

      //Check for any new requests from serial.
      //if ( (Serial.available()) > 0) { command(); }
      if ( (Serial.available()) > 0) { parseSerial(); }
      
      else if(cmdPending == true)
      {
        //This is a special case just for the tooth and composite loggers
        if (currentCommand == 'T') { legacySerialCommand(); }
      }

      //Check for any CAN comms requiring action 
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
      #if defined (NATIVE_CAN_AVAILABLE)
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
          
    if(currentLoopTime > micros_safe())
    {
      //Occurs when micros() has overflowed
      deferEEPROMWritesUntil = 0; //Required to ensure that EEPROM writes are not deferred indefinitely
    }

    currentLoopTime = micros_safe();
    unsigned long timeToLastTooth = (currentLoopTime - toothLastToothTime);
    if ( (timeToLastTooth < MAX_STALL_TIME) || (toothLastToothTime > currentLoopTime) ) //Check how long ago the last tooth was seen compared to now. If it was more than half a second ago then the engine is probably stopped. toothLastToothTime can be greater than currentLoopTime if a pulse occurs between getting the latest time and doing the comparison
    {
      currentStatus.longRPM = getRPM(); //Long RPM is included here
      currentStatus.RPM = currentStatus.longRPM;
      currentStatus.RPMdiv100 = div100(currentStatus.RPM);
      FUEL_PUMP_ON();
      currentStatus.fuelPumpOn = true; //Not sure if this is needed.
    }
    else
    {
      //We reach here if the time between teeth is too great. This VERY likely means the engine has stopped
      currentStatus.RPM = 0;
      currentStatus.RPMdiv100 = 0;
      currentStatus.PW1 = 0;
      currentStatus.VE = 0;
      currentStatus.VE2 = 0;
      toothLastToothTime = 0;
      toothLastSecToothTime = 0;
      //toothLastMinusOneToothTime = 0;
      currentStatus.hasSync = false;
      BIT_CLEAR(currentStatus.status3, BIT_STATUS3_HALFSYNC);
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
      if (configPage6.iacPWMrun == false) { disableIdle(); } //Turn off the idle PWM
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK); //Clear cranking bit (Can otherwise get stuck 'on' even with 0 rpm)
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_WARMUP); //Same as above except for WUE
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_RUN); //Same as above except for RUNNING status
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ASE); //Same as above except for ASE status
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ACC); //Same as above but the accel enrich (If using MAP accel enrich a stall will cause this to trigger)
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_DCC); //Same as above but the decel enleanment
      //This is a safety check. If for some reason the interrupts have got screwed up (Leading to 0rpm), this resets them.
      //It can possibly be run much less frequently.
      //This should only be run if the high speed logger are off because it will change the trigger interrupts back to defaults rather than the logger versions
      if( (currentStatus.toothLogEnabled == false) && (currentStatus.compositeLogEnabled == false) ) { initialiseTriggers(); }

      VVT1_PIN_LOW();
      VVT2_PIN_LOW();
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
      #if TPS_READ_FREQUENCY == 15
        readTPS(); //TPS reading to be performed every 32 loops (any faster and it can upset the TPSdot sampling time)
      #endif
      #if  defined(CORE_TEENSY35)       
          if (configPage9.enable_intcan == 1) // use internal can module
          {
           // this is just to test the interface is sending
           //sendCancommand(3,((configPage9.realtime_base_address & 0x3FF)+ 0x100),currentStatus.TPS,0,0x200);
          }
      #endif     

      //Check for launching/flat shift (clutch) can be done around here too
      previousClutchTrigger = clutchTrigger;
      //Only check for pinLaunch if any function using it is enabled. Else pins might break starting a board
      if(configPage6.flatSEnable || configPage6.launchEnabled){
        if(configPage6.launchHiLo > 0) { clutchTrigger = digitalRead(pinLaunch); }
        else { clutchTrigger = !digitalRead(pinLaunch); }
      }

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
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ)) //10 hertz
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_10HZ);
      //updateFullStatus();
      checkProgrammableIO();
      idleControl(); //Perform any idle related actions. This needs to be run at 10Hz to align with the idle taper resolution of 0.1s
      
      // Air conditioning control
      airConControl();

      //if( (isEepromWritePending() == true) && (serialReceivePending == false) && (micros() > deferEEPROMWritesUntil)) { writeAllConfig(); } //Used for slower EEPROM writes (Currently this runs in the 30Hz block)
      
      currentStatus.vss = getSpeed();
      currentStatus.gear = getGear();

      #ifdef SD_LOGGING
        if(configPage13.onboard_log_file_rate == LOGGER_RATE_10HZ) { writeSDLogEntry(); }
      #endif
    }
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_30HZ)) //30 hertz
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_30HZ);
      //Most boost tends to run at about 30Hz, so placing it here ensures a new target time is fetched frequently enough
      boostControl();
      //VVT may eventually need to be synced with the cam readings (ie run once per cam rev) but for now run at 30Hz
      vvtControl();
      //Water methanol injection
      wmiControl();
      #if defined(NATIVE_CAN_AVAILABLE)
      if (configPage2.canBMWCluster == true) { sendBMWCluster(); }
      if (configPage2.canVAGCluster == true) { sendVAGCluster(); }
      #endif
      #if TPS_READ_FREQUENCY == 30
        readTPS();
      #endif
      readO2();
      readO2_2();

      if(isEepromWritePending() == true) { writeAllConfig(); } //Check for any outstanding EEPROM writes.

      #ifdef SD_LOGGING
        if(configPage13.onboard_log_file_rate == LOGGER_RATE_30HZ) { writeSDLogEntry(); }
      #endif

      //Check for any outstanding EEPROM writes.
      if( (isEepromWritePending() == true) && (serialReceivePending == false) && (micros() > deferEEPROMWritesUntil)) { writeAllConfig(); } 
    }
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_4HZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_4HZ);
      //The IAT and CLT readings can be done less frequently (4 times per second)
      readCLT();
      readIAT();
      readBat();
      nitrousControl();

      //Lookup the current target idle RPM. This is aligned with coolant and so needs to be calculated at the same rate CLT is read
      if( (configPage2.idleAdvEnabled >= 1) || (configPage6.iacAlgorithm != IAC_ALGORITHM_NONE) )
      {
        currentStatus.CLIdleTarget = (byte)table2D_getValue(&idleTargetTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //All temps are offset by 40 degrees
        if(BIT_CHECK(currentStatus.airConStatus, BIT_AIRCON_TURNING_ON)) { currentStatus.CLIdleTarget += configPage15.airConIdleUpRPMAdder;  } //Adds Idle Up RPM amount if active
      }

      #ifdef SD_LOGGING
        if(configPage13.onboard_log_file_rate == LOGGER_RATE_4HZ) { writeSDLogEntry(); }
      #endif  
      
      currentStatus.fuelPressure = getFuelPressure();
      currentStatus.oilPressure = getOilPressure();
      
      if(auxIsEnabled == true)
      {
        //TODO dazq to clean this right up :)
        //check through the Aux input channels if enabled for Can or local use
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
            currentStatus.canin[currentStatus.current_caninchannel] = readAuxanalog(pinTranslateAnalog(configPage9.Auxinpina[currentStatus.current_caninchannel]&63));
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

      currentStatus.crankRPM = ((unsigned int)configPage4.crankRPM * 10); //Infrequent crankRPM treshold updates are not an issue.
      readBaro(); //Infrequent baro readings are not an issue.

      if ( (configPage10.wmiEnabled > 0) && (configPage10.wmiIndicatorEnabled > 0) )
      {
        // water tank empty
        if (BIT_CHECK(currentStatus.status4, BIT_STATUS4_WMI_EMPTY) > 0)
        {
          // flash with 1sec interval
          digitalWrite(pinWMIIndicator, !digitalRead(pinWMIIndicator));
        }
        else
        {
          digitalWrite(pinWMIIndicator, configPage10.wmiIndicatorPolarity ? HIGH : LOW);
        } 
      }

      #ifdef SD_LOGGING
        if(configPage13.onboard_log_file_rate == LOGGER_RATE_1HZ) { writeSDLogEntry(); }
      #endif

    } //1Hz timer

    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OL)
    || (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_CL)
    || (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OLCL) )
    {
      idleControl(); //Run idlecontrol every loop for stepper idle.
    }

    
    //VE and advance calculation were moved outside the sync/RPM check so that the fuel and ignition load value will be accurately shown when RPM=0
    currentStatus.VE1 = getVE1();
    currentStatus.VE = currentStatus.VE1; //Set the final VE value to be VE 1 as a default. This may be changed in the section below

    currentStatus.advance1 = getAdvance1();
    currentStatus.advance = currentStatus.advance1; //Set the final advance value to be advance 1 as a default. This may be changed in the section below

    calculateSecondaryFuel();
    calculateSecondarySpark();

    //Always check for sync
    //Main loop runs within this clause
    if ((currentStatus.hasSync || BIT_CHECK(currentStatus.status3, BIT_STATUS3_HALFSYNC)) && (currentStatus.RPM > 0))
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
      //END SETTING ENGINE STATUSES
      //-----------------------------------------------------------------------------------------------------

      //Begin the fuel calculation
      //Calculate an injector pulsewidth from the VE
      currentStatus.corrections = correctionsFuel();

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


      doCrankSpeedCalcs(); //In crankMaths.ino

      //Check that the duty cycle of the chosen pulsewidth isn't too high.
      unsigned long pwLimit;
      if(configPage2.dutyLim!=100){pwLimit = percentage(configPage2.dutyLim, revolutionTime);} //The pulsewidth limit is determined to be the duty cycle limit (Eg 85%) by the total time it takes to perform 1 revolution
      else{pwLimit=revolutionTime;}
      //Handle multiple squirts per rev
      if (configPage2.strokes == FOUR_STROKE) { pwLimit = pwLimit * 2 / currentStatus.nSquirts; } 
      else { pwLimit = pwLimit / currentStatus.nSquirts; }
      //Apply the pwLimit if staging is disabled and engine is not cranking
      if( (!BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK)) && (configPage10.stagingEnabled == false) ) { if (currentStatus.PW1 > pwLimit) { currentStatus.PW1 = pwLimit; } }

      //Calculate staging pulsewidths if used
      //To run staged injection, the number of cylinders must be less than or equal to the injector channels (ie Assuming you're running paired injection, you need at least as many injector channels as you have cylinders, half for the primaries and half for the secondaries)
      if( (configPage10.stagingEnabled == true) && (configPage2.nCylinders <= INJ_CHANNELS || configPage2.injType == INJ_TYPE_TBODY) && (currentStatus.PW1 > inj_opentime_uS) ) //Final check is to ensure that DFCO isn't active, which would cause an overflow below (See #267)
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

      injector1EndAngle= calculateInjectorEndAngle(channel1InjDegrees);

      //Repeat the above for each cylinder
      switch (configPage2.nCylinders)
      {
        //Single cylinder
        case 1:
          //The only thing that needs to be done for single cylinder is to check for staging. 
          if( (configPage10.stagingEnabled == true) && (currentStatus.PW3 > 0) )
          {
            injector3EndAngle= calculateInjectorEndAngle(channel3InjDegrees);
          }
          break;
        //2 cylinders
        case 2:
          injector2EndAngle= calculateInjectorEndAngle(channel2InjDegrees);
          
          if ( (configPage2.injLayout == INJ_SEQUENTIAL) && (configPage6.fuelTrimEnabled > 0) )
            {
              currentStatus.PW1 = applyFuelTrimToPW(&trim1Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW1);
              currentStatus.PW2 = applyFuelTrimToPW(&trim2Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW2);
            }
          else if( (configPage10.stagingEnabled == true) && (currentStatus.PW3 > 0) )
          {
            injector3EndAngle= calculateInjectorEndAngle(channel3InjDegrees);

            injector4EndAngle = injector3EndAngle + (CRANK_ANGLE_MAX_INJ / 2); //Phase this either 180 or 360 degrees out from inj3 (In reality this will always be 180 as you can't have sequential and staged currently)
            if(injector4EndAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { injector4EndAngle -= CRANK_ANGLE_MAX_INJ; }
          }
          break;
        //3 cylinders
        case 3:
          injector2EndAngle= calculateInjectorEndAngle(channel2InjDegrees);
          injector3EndAngle= calculateInjectorEndAngle(channel3InjDegrees);
          
          if ( (configPage2.injLayout == INJ_SEQUENTIAL) && (configPage6.fuelTrimEnabled > 0) )
            {
              currentStatus.PW1 = applyFuelTrimToPW(&trim1Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW1);
              currentStatus.PW2 = applyFuelTrimToPW(&trim2Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW2);
              currentStatus.PW3 = applyFuelTrimToPW(&trim3Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW3);
            }
          break;
        //4 cylinders
        case 4:
          injector2EndAngle= calculateInjectorEndAngle(channel2InjDegrees);

          if((configPage2.injLayout == INJ_SEQUENTIAL) && currentStatus.hasSync)
          {
            if( CRANK_ANGLE_MAX_INJ != 720 ) { changeHalfToFullSync(); }

            injector3EndAngle= calculateInjectorEndAngle(channel3InjDegrees);
            injector4EndAngle= calculateInjectorEndAngle(channel4InjDegrees);

            if(configPage6.fuelTrimEnabled > 0)
            {
              currentStatus.PW1 = applyFuelTrimToPW(&trim1Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW1);
              currentStatus.PW2 = applyFuelTrimToPW(&trim2Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW2);
              currentStatus.PW3 = applyFuelTrimToPW(&trim3Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW3);
              currentStatus.PW4 = applyFuelTrimToPW(&trim4Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW4);
            }
          }
          else if( (configPage10.stagingEnabled == true) && (currentStatus.PW3 > 0) )
          {
            injector3EndAngle= calculateInjectorEndAngle(channel3InjDegrees);

            injector4EndAngle = injector3EndAngle + (CRANK_ANGLE_MAX_INJ / 2); //Phase this either 180 or 360 degrees out from inj3 (In reality this will always be 180 as you can't have sequential and staged currently)
            if(injector4EndAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { injector4EndAngle -= CRANK_ANGLE_MAX_INJ; }
          }
          else
          {
            if( BIT_CHECK(currentStatus.status3, BIT_STATUS3_HALFSYNC) && (CRANK_ANGLE_MAX_INJ != 360) ) { changeFullToHalfSync(); }
          }
          break;
        //5 cylinders
        case 5:
          injector2EndAngle= calculateInjectorEndAngle(channel2InjDegrees);
          injector3EndAngle= calculateInjectorEndAngle(channel3InjDegrees);
          injector4EndAngle= calculateInjectorEndAngle(channel4InjDegrees);
          #if INJ_CHANNELS >= 5
            injector5EndAngle= calculateInjectorEndAngle(channel5InjDegrees);
          #endif
          break;
        //6 cylinders
        case 6:
          injector2EndAngle= calculateInjectorEndAngle(channel2InjDegrees);
          injector3EndAngle= calculateInjectorEndAngle(channel3InjDegrees);
          
          #if INJ_CHANNELS >= 6
            if((configPage2.injLayout == INJ_SEQUENTIAL) && currentStatus.hasSync)
            {
            if( CRANK_ANGLE_MAX_INJ != 720 ) { changeHalfToFullSync(); }

              injector4EndAngle= calculateInjectorEndAngle(channel4InjDegrees);
              injector5EndAngle= calculateInjectorEndAngle(channel5InjDegrees);
              injector6EndAngle= calculateInjectorEndAngle(channel6InjDegrees);

              if(configPage6.fuelTrimEnabled > 0)
              {
                currentStatus.PW1 = applyFuelTrimToPW(&trim1Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW1);
                currentStatus.PW2 = applyFuelTrimToPW(&trim2Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW2);
                currentStatus.PW3 = applyFuelTrimToPW(&trim3Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW3);
                currentStatus.PW4 = applyFuelTrimToPW(&trim4Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW4);
                currentStatus.PW5 = applyFuelTrimToPW(&trim5Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW5);
                currentStatus.PW6 = applyFuelTrimToPW(&trim6Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW6);
              }
            }
            else
            {
              if( BIT_CHECK(currentStatus.status3, BIT_STATUS3_HALFSYNC) && (CRANK_ANGLE_MAX_INJ != 360) ) { changeFullToHalfSync(); }
            }
          #endif
          break;
        //8 cylinders
        case 8:
          injector2EndAngle= calculateInjectorEndAngle(channel2InjDegrees);
          injector3EndAngle= calculateInjectorEndAngle(channel3InjDegrees);
          injector4EndAngle= calculateInjectorEndAngle(channel4InjDegrees);

          #if INJ_CHANNELS >= 8
            if((configPage2.injLayout == INJ_SEQUENTIAL) && currentStatus.hasSync)
            {
              if( CRANK_ANGLE_MAX_INJ != 720 ) { changeHalfToFullSync(); }
              injector5EndAngle= calculateInjectorEndAngle(channel5InjDegrees);
              injector6EndAngle= calculateInjectorEndAngle(channel6InjDegrees);
              injector7EndAngle= calculateInjectorEndAngle(channel7InjDegrees);
              injector8EndAngle= calculateInjectorEndAngle(channel8InjDegrees);

              if(configPage6.fuelTrimEnabled > 0)
              {
                currentStatus.PW1 = applyFuelTrimToPW(&trim1Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW1);
                currentStatus.PW2 = applyFuelTrimToPW(&trim2Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW2);
                currentStatus.PW3 = applyFuelTrimToPW(&trim3Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW3);
                currentStatus.PW4 = applyFuelTrimToPW(&trim4Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW4);
                currentStatus.PW5 = applyFuelTrimToPW(&trim5Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW5);
                currentStatus.PW6 = applyFuelTrimToPW(&trim6Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW6);
                currentStatus.PW7 = applyFuelTrimToPW(&trim7Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW7);
                currentStatus.PW8 = applyFuelTrimToPW(&trim8Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW8);
              }
            }
            else
            {
              if( BIT_CHECK(currentStatus.status3, BIT_STATUS3_HALFSYNC) && (CRANK_ANGLE_MAX_INJ != 360) ) { changeFullToHalfSync(); }
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
      if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) ) {
        currentStatus.dwell =  ((uint16_t)configPage4.dwellCrank * 100); //use cranking dwell
      }
      else 
      {
        if ( configPage2.useDwellMap == true )
        {
          currentStatus.dwell = (get3DTableValue(&dwellTable, currentStatus.MAP, currentStatus.RPM) * 100); //use running dwell from map
        }
        else
        {
          currentStatus.dwell =  (configPage4.dwellRun * 100); //use fixed running dwell
        }
      }
      currentStatus.dwell = correctionsDwell(currentStatus.dwell);

      calculateIgnitionAngles();

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
      int16_t crankAngle = getCrankAngle();
      while(crankAngle > CRANK_ANGLE_MAX_INJ ) { crankAngle = crankAngle - CRANK_ANGLE_MAX_INJ; } //Continue reducing the crank angle by the max injection amount until it's below the required limit. This will usually only run (at most) once, but in cases where there is sequential ignition and more than 2 squirts per cycle, it may run up to 4 times. 

      
      //Check for any of the engine protections or rev limiters being turned on
      if(checkEngineProtect() || currentStatus.launchingHard || currentStatus.flatShiftingHard)
      {
        if( (currentStatus.RPMdiv100 > configPage4.engineProtectMaxRPM) || currentStatus.launchingHard || currentStatus.flatShiftingHard) //Ugly, but the launch/flat shift check needs to be here as well to prevent these limiters not happening when under the the Engine Protection min rpm
        {
          if( (configPage2.hardCutType == HARD_CUT_FULL) || (configPage6.engineProtectType == PROTECT_CUT_FUEL) ) 
          { 
            //Full hard cut turns outputs off completely. Note that hard cut is ALWAYS used on fuel cut only. 
            switch(configPage6.engineProtectType)
            {
              case PROTECT_CUT_OFF:
                ignitionOn = true;
                fuelOn = true;
                currentStatus.engineProtectStatus = 0;
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
            //if(rollingCutCounter >= 2) //Vary this number to change the intensity of the roll. The higher the number, the closer is it to full cut
            if(rollingCutLastRev == 0) { rollingCutLastRev = currentStatus.startRevolutions; } //
            if (currentStatus.startRevolutions > (rollingCutLastRev+1) )
            { 
              //Rolls through each of the active ignition channels based on how many revolutions have taken place
              //curRollingCut = ( (currentStatus.startRevolutions / 2) % maxIgnOutputs) + 1;
              curRollingCut = 0;
              rollingCutCounter += 1;
              if(rollingCutCounter > (maxIgnOutputs-1)) { rollingCutCounter = 0; }
              BIT_SET(curRollingCut, rollingCutCounter);

              ignitionOn = true;
              rollingCutLastRev = currentStatus.startRevolutions;
              //curRollingCut = 0;
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
          setFuelSchedule(&fuelSchedule1, crankAngle, injector1EndAngle, (unsigned long)currentStatus.PW1);          
        }
#endif

#if INJ_CHANNELS >= 2
        if( (channel2InjEnabled) && (currentStatus.PW2 >= inj_opentime_uS) )
        {
          setFuelSchedule(&fuelSchedule2, crankAngle, injector2EndAngle, (unsigned long)currentStatus.PW2);    
        }
#endif

#if INJ_CHANNELS >= 3
        if( (channel3InjEnabled) && (currentStatus.PW3 >= inj_opentime_uS) )
        {
          setFuelSchedule(&fuelSchedule3, crankAngle, injector3EndAngle, (unsigned long)currentStatus.PW3);  
        }
#endif

#if INJ_CHANNELS >= 4
        if( (channel4InjEnabled) && (currentStatus.PW4 >= inj_opentime_uS) )
        {
          setFuelSchedule(&fuelSchedule4, crankAngle, injector4EndAngle, (unsigned long)currentStatus.PW4);  
        }
#endif

#if INJ_CHANNELS >= 5
        if( (channel5InjEnabled) && (currentStatus.PW5 >= inj_opentime_uS) )
        {
          setFuelSchedule(&fuelSchedule5, crankAngle, injector5EndAngle, (unsigned long)currentStatus.PW5);  
        }
#endif

#if INJ_CHANNELS >= 6
        if( (channel6InjEnabled) && (currentStatus.PW6 >= inj_opentime_uS) )
        {
          setFuelSchedule(&fuelSchedule6, crankAngle, injector6EndAngle, (unsigned long)currentStatus.PW6);  
        }
#endif

#if INJ_CHANNELS >= 7
        if( (channel7InjEnabled) && (currentStatus.PW7 >= inj_opentime_uS) )
        {
          setFuelSchedule(&fuelSchedule7, crankAngle, injector7EndAngle, (unsigned long)currentStatus.PW7);  
        }
#endif

#if INJ_CHANNELS >= 8
        if( (channel8InjEnabled) && (currentStatus.PW8 >= inj_opentime_uS) )
        {
          setFuelSchedule(&fuelSchedule8, crankAngle, injector8EndAngle, (unsigned long)currentStatus.PW8);
        }
#endif
      }
      //***********************************************************************************************
      //| BEGIN IGNITION SCHEDULES
      //Same as above, except for ignition


      //fixed Cranking Override is currently only available on the basic distributor and 4g63 and Miata9905 and MazdaAU and Subaru 67 and Daihatsu decoders.
      if(ignitionOn && !(configPage4.ignCranklock && (decoderHasFixedCrankingTiming == true) && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK))) //fixed cranking override do not need schedulers here, it starts directly from decoders
      {
        //Refresh the current crank angle info
        crankAngle = getCrankAngle(); //Refresh with the latest crank angle
        while (crankAngle > CRANK_ANGLE_MAX_IGN ) { crankAngle -= CRANK_ANGLE_MAX_IGN; }
        //dwell limiter(overdwell protection), makes it impossible to set too long pulses using schedulers.
        if(configPage4.useDwellLim){
          dwellLimit_uS = (1000 * configPage4.dwellLimit); //Update uS value incase setting has changed        
          if ((currentStatus.dwell) > dwellLimit_uS ){
            currentStatus.dwell = dwellLimit_uS;  //cap dwell
          }
        }          
       

#if IGN_CHANNELS >= 1
        if (!BIT_CHECK(curRollingCut, IGN1_CMD_BIT) )
        {
          setIgnitionSchedule(&ignitionSchedule1, crankAngle, ignition1EndAngle, currentStatus.dwell);
        }
#endif

        
#if IGN_CHANNELS >= 2
        if (maxIgnOutputs >= 2 && !BIT_CHECK(curRollingCut, IGN2_CMD_BIT))
        {
          setIgnitionSchedule(&ignitionSchedule2, crankAngle, ignition2EndAngle, currentStatus.dwell);
        }
#endif

#if IGN_CHANNELS >= 3
        if (maxIgnOutputs >= 3 && !BIT_CHECK(curRollingCut, IGN3_CMD_BIT))
        {
          setIgnitionSchedule(&ignitionSchedule3, crankAngle, ignition3EndAngle, currentStatus.dwell);
        }
#endif

#if IGN_CHANNELS >= 4
        if (maxIgnOutputs >= 4 && !BIT_CHECK(curRollingCut, IGN4_CMD_BIT))
        {
          setIgnitionSchedule(&ignitionSchedule4, crankAngle, ignition4EndAngle, currentStatus.dwell);
        }
#endif

#if IGN_CHANNELS >= 5
        if (maxIgnOutputs >= 5  && !BIT_CHECK(curRollingCut, IGN5_CMD_BIT))
        {
          setIgnitionSchedule(&ignitionSchedule5, crankAngle, ignition5EndAngle, currentStatus.dwell);
        }
#endif

#if IGN_CHANNELS >= 6
        if (maxIgnOutputs >= 6 && !BIT_CHECK(curRollingCut, IGN6_CMD_BIT))
        {
          setIgnitionSchedule(&ignitionSchedule6, crankAngle, ignition6EndAngle, currentStatus.dwell);
        }
#endif

#if IGN_CHANNELS >= 7
        if (maxIgnOutputs >= 7 && !BIT_CHECK(curRollingCut, IGN7_CMD_BIT))
        {
          setIgnitionSchedule(&ignitionSchedule7, crankAngle, ignition7EndAngle, currentStatus.dwell);
        }
#endif

#if IGN_CHANNELS >= 8
        if (maxIgnOutputs >= 8  && !BIT_CHECK(curRollingCut, IGN8_CMD_BIT))
        {
          setIgnitionSchedule(&ignitionSchedule8, crankAngle, ignition8EndAngle, currentStatus.dwell);
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

  //Check whether either of the multiply MAP modes is turned on
  if ( configPage2.multiplyMAP == MULTIPLY_MAP_MODE_100) { iMAP = ((unsigned int)MAP << 7) / 100; }
  else if( configPage2.multiplyMAP == MULTIPLY_MAP_MODE_BARO) { iMAP = ((unsigned int)MAP << 7) / currentStatus.baro; }
  
  if ( (configPage2.includeAFR == true) && (configPage6.egoType == EGO_TYPE_WIDE) && (currentStatus.runSecs > configPage6.ego_sdelay) ) {
    iAFR = ((unsigned int)currentStatus.O2 << 7) / currentStatus.afrTarget;  //Include AFR (vs target) if enabled
  }
  if ( (configPage2.incorporateAFR == true) && (configPage2.includeAFR == false) ) {
    iAFR = ((unsigned int)configPage2.stoich << 7) / currentStatus.afrTarget;  //Incorporate stoich vs target AFR, if enabled.
  }
  iCorrections = (corrections << bitShift) / 100;


  unsigned long intermediate = ((uint32_t)REQ_FUEL * (uint32_t)iVE) >> 7; //Need to use an intermediate value to avoid overflowing the long
  if ( configPage2.multiplyMAP > 0 ) { intermediate = (intermediate * (unsigned long)iMAP) >> 7; }
  
  if ( (configPage2.includeAFR == true) && (configPage6.egoType == EGO_TYPE_WIDE) && (currentStatus.runSecs > configPage6.ego_sdelay) ) {
    //EGO type must be set to wideband and the AFR warmup time must've elapsed for this to be used
    intermediate = (intermediate * (unsigned long)iAFR) >> 7;  
  }
  if ( (configPage2.incorporateAFR == true) && (configPage2.includeAFR == false) ) {
    intermediate = (intermediate * (unsigned long)iAFR) >> 7;
  }
  
  intermediate = (intermediate * (unsigned long)iCorrections) >> bitShift;
  if (intermediate != 0)
  {
    //If intermediate is not 0, we need to add the opening time (0 typically indicates that one of the full fuel cuts is active)
    intermediate += injOpen; //Add the injector opening time
    //AE calculation only when ACC is active.
    if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_ACC) )
    {
      //AE Adds % of req_fuel
      if ( configPage2.aeApplyMode == AE_MODE_ADDER )
        {
          intermediate += ( ((unsigned long)REQ_FUEL) * (currentStatus.AEamount - 100) ) / 100;
        }
    }

    if ( intermediate > 65535)
    {
      intermediate = 65535;  //Make sure this won't overflow when we convert to uInt. This means the maximum pulsewidth possible is 65.535mS
    }
  }
  return (unsigned int)(intermediate);
}

/** Lookup the current VE value from the primary 3D fuel map.
 * The Y axis value used for this lookup varies based on the fuel algorithm selected (speed density, alpha-n etc).
 * 
 * @return byte The current VE value
 */
byte getVE1(void)
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

/** Lookup the ignition advance from 3D ignition table.
 * The values used to look this up will be RPM and whatever load source the user has configured.
 * 
 * @return byte The current target advance value in degrees
 */
byte getAdvance1(void)
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

uint16_t calculateInjectorEndAngle(int16_t injChannelDegrees)
{
  uint16_t tempInjectorEndAngle;  
  tempInjectorEndAngle=currentStatus.injAngle + injChannelDegrees;
  while(tempInjectorEndAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { tempInjectorEndAngle -= CRANK_ANGLE_MAX_INJ; }

  return tempInjectorEndAngle;
}

int calculateIgnitionAngle(int channelDegrees)
{
  int ignitionEndAngleTemp;
  ignitionEndAngleTemp = channelDegrees - currentStatus.advance;
  if(ignitionEndAngleTemp > CRANK_ANGLE_MAX_IGN) {ignitionEndAngleTemp -= CRANK_ANGLE_MAX_IGN;}
  return ignitionEndAngleTemp;
}

// ignition 3 for rotary
void calculateIgnitionAngle3(int rotarySplitDegrees)
{
  ignition3EndAngle = ignition1EndAngle + rotarySplitDegrees;
}

// ignition 4 for rotary
void calculateIgnitionAngle4(int rotarySplitDegrees)
{
  ignition4EndAngle = ignition2EndAngle + rotarySplitDegrees;
}

/** Calculate the Ignition angles for all cylinders (based on @ref config2.nCylinders).
 * both start and end angles are calculated for each channel.
 * Also the mode of ignition firing - wasted spark vs. dedicated spark per cyl. - is considered here.
 */
void calculateIgnitionAngles()
{
  

  //This test for more cylinders and do the same thing
  switch (configPage2.nCylinders)
  {
    //1 cylinder
    case 1:
      ignition1EndAngle=calculateIgnitionAngle(ignitionSchedule1.channelDegrees);
      break;
    //2 cylinders
    case 2:
      ignition1EndAngle=calculateIgnitionAngle(ignitionSchedule1.channelDegrees);
      ignition2EndAngle=calculateIgnitionAngle(ignitionSchedule2.channelDegrees);
      break;
    //3 cylinders
    case 3:
      ignition1EndAngle=calculateIgnitionAngle(ignitionSchedule1.channelDegrees);
      ignition2EndAngle=calculateIgnitionAngle(ignitionSchedule2.channelDegrees);
      ignition3EndAngle=calculateIgnitionAngle(ignitionSchedule3.channelDegrees);
      break;
    //4 cylinders
    case 4:
      ignition1EndAngle=calculateIgnitionAngle(ignitionSchedule1.channelDegrees);
      ignition2EndAngle=calculateIgnitionAngle(ignitionSchedule2.channelDegrees);

      #if IGN_CHANNELS >= 4
      if((configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && currentStatus.hasSync)
      {
        if( CRANK_ANGLE_MAX_IGN != 720 ) { changeHalfToFullSync(); }

        ignition3EndAngle=calculateIgnitionAngle(ignitionSchedule3.channelDegrees);
        ignition4EndAngle=calculateIgnitionAngle(ignitionSchedule4.channelDegrees);
      }
      else if(configPage4.sparkMode == IGN_MODE_ROTARY)
      {
        byte splitDegrees = 0;
        if (configPage2.fuelAlgorithm == LOAD_SOURCE_MAP) { splitDegrees = table2D_getValue(&rotarySplitTable, currentStatus.MAP/2); }
        else { splitDegrees = table2D_getValue(&rotarySplitTable, currentStatus.TPS/2); }

        //The trailing angles are set relative to the leading ones
        calculateIgnitionAngle3(splitDegrees);
        calculateIgnitionAngle4(splitDegrees);
      }
      else
      {
        if( BIT_CHECK(currentStatus.status3, BIT_STATUS3_HALFSYNC) && (CRANK_ANGLE_MAX_IGN != 360) ) { changeFullToHalfSync(); }
      }
      #endif
      break;
    //5 cylinders
    case 5:
      ignition1EndAngle=calculateIgnitionAngle(ignitionSchedule1.channelDegrees);
      ignition2EndAngle=calculateIgnitionAngle(ignitionSchedule2.channelDegrees);
      ignition3EndAngle=calculateIgnitionAngle(ignitionSchedule3.channelDegrees);
      ignition4EndAngle=calculateIgnitionAngle(ignitionSchedule4.channelDegrees);
      ignition5EndAngle=calculateIgnitionAngle(ignitionSchedule5.channelDegrees);
      break;
    //6 cylinders
    case 6:
      ignition1EndAngle=calculateIgnitionAngle(ignitionSchedule1.channelDegrees);
      ignition2EndAngle=calculateIgnitionAngle(ignitionSchedule2.channelDegrees);
      ignition3EndAngle=calculateIgnitionAngle(ignitionSchedule3.channelDegrees);

      #if IGN_CHANNELS >= 6
      if((configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && currentStatus.hasSync)
      {
        if( CRANK_ANGLE_MAX_IGN != 720 ) { changeHalfToFullSync(); }

      ignition4EndAngle=calculateIgnitionAngle(ignitionSchedule4.channelDegrees);
      ignition5EndAngle=calculateIgnitionAngle(ignitionSchedule5.channelDegrees);
      ignition6EndAngle=calculateIgnitionAngle(ignitionSchedule6.channelDegrees);
      }
      else
      {
        if( BIT_CHECK(currentStatus.status3, BIT_STATUS3_HALFSYNC) && (CRANK_ANGLE_MAX_IGN != 360) ) { changeFullToHalfSync(); }
      }
      #endif
      break;
    //8 cylinders
    case 8:
      ignition1EndAngle=calculateIgnitionAngle(ignitionSchedule1.channelDegrees);
      ignition2EndAngle=calculateIgnitionAngle(ignitionSchedule2.channelDegrees);
      ignition3EndAngle=calculateIgnitionAngle(ignitionSchedule3.channelDegrees);
      ignition4EndAngle=calculateIgnitionAngle(ignitionSchedule4.channelDegrees);

      #if IGN_CHANNELS >= 8
      if((configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && currentStatus.hasSync)
      {
        if( CRANK_ANGLE_MAX_IGN != 720 ) { changeHalfToFullSync(); }

      ignition5EndAngle=calculateIgnitionAngle(ignitionSchedule5.channelDegrees);
      ignition6EndAngle=calculateIgnitionAngle(ignitionSchedule6.channelDegrees);
      ignition7EndAngle=calculateIgnitionAngle(ignitionSchedule7.channelDegrees);
      ignition8EndAngle=calculateIgnitionAngle(ignitionSchedule8.channelDegrees);
      }
      else
      {
        if( BIT_CHECK(currentStatus.status3, BIT_STATUS3_HALFSYNC) && (CRANK_ANGLE_MAX_IGN != 360) ) { changeFullToHalfSync(); }
      }
      #endif
      break;

    //Will hit the default case on >8 cylinders. Do nothing in these cases
    default:
      break;
  }
}
