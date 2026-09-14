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
#include "scheduler.h"
#include "comms.h"
#include "comms_legacy.h"
#include "comms_secondary.h"
#include "maths.h"
#include "corrections.h"
#include "timers.h"
#include "decoders.h"
#include "idle.h"
#include "src/controllers/vvt/vvtController.h"
#include "sensors.h"
#include "storage.h"
#include "crankMaths.h"
#include "init.h"
#include "src/controllers/progammableIO/programmableIOControl.h"
#include "engineProtection.h"
#include "secondaryTables.h"
#include "comms_CAN.h"
#include "SD_logger.h"
#include "load_source.h"
#include "board_definition.h"
#include "unit_testing.h"
#include RTC_LIB_H //Defined in each boards .h file
#include "units.h"
#include "fuel_calcs.h"
#include "preprocessor.h"
#include "dwell.h"
#include "decoder_init.h"
#include "src/pins/pinMapping.h"
#include "resetControl.h"
#include "scheduler_ignition_controller.h"
#include "src/controllers/launch/launchController.h"
#include "src/controllers/fuelPump/fuelPumpController.h"
#include "scheduler_fuel_controller.h"
#include "src/controllers/tsCommand/tsCommandController.h"
#include "src/controllers/fan/fanController.h"
#include "src/controllers/boost/boostController.h"
#include "src/controllers/aircon/airconController.h"
#include "src/controllers/nitrous/nitrousController.h"
#include "src/controllers/auxChannels/auxChannelController.h"

#define CRANK_RUN_HYSTER    15

constexpr table2D_u8_u8_10 idleTargetTable(&configPage6.iacBins, &configPage6.iacCLValues);

#ifndef UNIT_TEST // Scope guard for unit testing

void setup(void)
{
  currentStatus.initialisationComplete = false; //Tracks whether the initialiseAll() function has run completely
  initialiseAll();
}

/** Lookup the current VE value from the primary 3D fuel map.
 * The Y axis value used for this lookup varies based on the fuel algorithm selected (speed density, alpha-n etc).
 * 
 * @return byte The current VE value
 */
static inline uint8_t getVE1(void)
{
  currentStatus.fuelLoad = getLoad(configPage2.fuelAlgorithm, currentStatus);
  return get3DTableValue(&fuelTable, currentStatus.fuelLoad, currentStatus.RPM); //Perform lookup into fuel map for RPM vs MAP value
}

/** Lookup the ignition advance from 3D ignition table.
 * The values used to look this up will be RPM and whatever load source the user has configured.
 * 
 * @return byte The current target advance value in degrees
 */
static inline int8_t getAdvance1(void)
{
  currentStatus.ignLoad = getLoad(configPage2.ignAlgorithm, currentStatus);
  return correctionsIgn(IGNITION_ADVANCE_LARGE.toUser(get3DTableValue(&ignitionTable, currentStatus.ignLoad, currentStatus.RPM))); //As above, but for ignition advance
}

static inline bool haveSwitchedToBatteryPower(uint8_t originalBatteryVoltage, const statuses &current)
{
  return (originalBatteryVoltage < 55U)
      && (current.battery10 > 70)
      && (current.RPM == 0U)
      ;
}

// The following is a check for if the voltage has jumped up from under 5.5v to over 7v.
// If this occurs, it's very likely that the system has gone from being powered by USB to being powered from the 12v power source.
// Should that happen, we re-trigger the fuel pump priming and idle homing (If using a stepper)
static void onPowerSourceSwitch(uint8_t originalBatteryVoltage, const statuses &current, const config2 &page2, const config6 &page6)
{
  if( haveSwitchedToBatteryPower(originalBatteryVoltage, current) )
  {
    //Re-prime the fuel pump
    startPumpPriming(current, page2);

    //Redo the stepper homing
    if(isStepperIac(page6) )
    {
      initialiseIdle(true);
    }
  }
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
 * single byte variable @ref statuses.LOOP_TIMER plays a big part here as:
 * - it contains expire-bits for interval based frequency driven events (e.g. 15Hz, 4Hz, 1Hz)
 * - Can be tested for certain frequency interval being expired by (eg) BIT_CHECK(currentStatus.LOOP_TIMER, BIT_TIMER_15HZ)
 * 
 * Sometimes loop() is inlined by LTO & sometimes not
 * When not inlined, there is a huge difference in stack usage: 60+ bytes
 * That eats into available RAM.
 * Adding __attribute__((always_inline)) forces the LTO process to inline.
 *
 * Since the function is declared in an Arduino header, we can't change
 * it to inline, so we need to suppress the resulting warning.
 */
BEGIN_LTO_ALWAYS_INLINE(void) loop(void)
{
  uint8_t originalBatteryVoltage = currentStatus.battery10;

      if(mainLoopCount < UINT16_MAX) { mainLoopCount++; }
      currentStatus.LOOP_TIMER = getAndClearTimerMask();

      //SERIAL Comms
      //Initially check that the last serial send values request is not still outstanding
      if (serialTransmitInProgress())
      {
        serialTransmit();
      }

      //Check for any new or in-progress requests from serial.
      if( (Serial.available() > 0) || serialRecieveInProgress() )
      {
        serialReceive();
      }
      
      //Check for any secondary comms requiring action. Note that AVR runs this at a fixed 30Hz. 
      if ((configPage9.enable_secondarySerial == 1)  //secondary serial interface enabled
      && (secondarySerial.available() > SERIAL_BUFFER_THRESHOLD))
      {
        secondserial_Command();
      }
      #if defined (NATIVE_CAN_AVAILABLE)
        if (configPage9.enable_intcan == 1) // use internal can module
        {            
          //check local can module
          while (CAN_read()) 
          {
            can_Command();
            readAuxCanBus();
            if (configPage2.canWBO > 0) { receiveCANwbo(); }
          }
        }   
      #endif
          
    if ( currentStatus.decoder.isEngineRunning(micros()) )
    {
      currentStatus.setRpm(currentStatus.decoder.getRPM());
      if (currentStatus.RPM > 0)
      {
        fuelPumpOn();
      }
    }
    else
    {
      //We reach here if the time between teeth is too great. This VERY likely means the engine has stopped
      currentStatus.setRpm(0);
      fuelSchedule1.pw = 0;
      currentStatus.VE = 0;
      currentStatus.VE2 = 0;
      currentStatus.decoder.reset();
      currentStatus.runSecs = 0; //Reset the counter for number of seconds running.
      currentStatus.startRevolutions = 0;
      resetMAPcycleAndEvent();
      currentStatus.rpmDOT = 0;
      initialiseCorrections();
      ignitionCount = 0;
      stopPumpPriming(currentStatus, configPage2); //Turn off the fuel pump, but only if the priming is complete
      if (configPage6.iacPWMrun == false) { disableIdle(); } //Turn off the idle PWM
      currentStatus.wueIsActive = false; //Same as above except for WUE
      currentStatus.rotationStatus = EngineRotationStatus::Stopped;
      currentStatus.aseIsActive = false; //Same as above except for ASE status
      currentStatus.isAcceleratingTPS = false; //Same as above but the accel enrich (If using MAP accel enrich a stall will cause this to trigger)
      currentStatus.isDeceleratingTPS = false; //Same as above but the decel enleanment
      //This is a safety check. If for some reason the interrupts have got screwed up (Leading to 0rpm), this resets them.
      //It can possibly be run much less frequently.
      //This should only be run if the high speed logger are off because it will change the trigger interrupts back to defaults rather than the logger versions
      if( (currentStatus.toothLogEnabled == false) && (currentStatus.compositeTriggerUsed == 0) ) { 
        currentStatus.decoder = buildDecoder(configPage4.TrigPattern);
      }
      if(configPage4.ignBypassEnabled > 0) { digitalWrite(pinNumbers.pinIgnBypass, LOW); } //Reset the ignition bypass ready for next crank attempt
    }
    //***Perform sensor reads***
    //-----------------------------------------------------------------------------------------------------
    readPolledSensors(currentStatus.LOOP_TIMER);
    boostControl(currentStatus, configPage2, configPage4, configPage6, configPage9, configPage10, configPage15);

    if(BIT_CHECK(currentStatus.LOOP_TIMER, BIT_TIMER_50HZ)) //50 hertz
    {
      #if defined(NATIVE_CAN_AVAILABLE)
      sendCANBroadcast(50);
      #endif
    }
    if(BIT_CHECK(currentStatus.LOOP_TIMER, BIT_TIMER_30HZ)) //30 hertz
    {
      //VVT may eventually need to be synced with the cam readings (ie run once per cam rev) but for now run at 30Hz
      vvtControl(currentStatus, configPage4, configPage6, configPage10);
      //Water methanol injection
      wmiControl(currentStatus, configPage10);
      
      #if defined(NATIVE_CAN_AVAILABLE)
      sendCANBroadcast(30);
      #endif

      #ifdef SD_LOGGING
        if(configPage13.onboard_log_file_rate == SD_LOGGER_RATE_30HZ) { writeSDLogEntry(); }
      #endif

      //AVR units process secondary serial requests at a fixed 30Hz
      #ifdef CORE_AVR
      if( (configPage9.enable_secondarySerial == 1) && (secondarySerial.available() > 0) ) //secondary serial interface enabled
      {
        secondserial_Command();
      }
      #endif

      //Check for any outstanding EEPROM writes.
      if( (isEepromWritePending() == true) && (serialStatusFlag == SERIAL_INACTIVE) && storageWriteTimeoutExpired()) { saveAllPages(); } 
    }
    if (BIT_CHECK(currentStatus.LOOP_TIMER, BIT_TIMER_15HZ)) //Every 32 loops
    {
      checkLaunchAndFlatShift(currentStatus, pinNumbers.pinLaunch, configPage2, configPage6, configPage10, configPage15); //Check for launch control and flat shift being active

      #if defined(NATIVE_CAN_AVAILABLE)
      sendCANBroadcast(15);
      #endif

      //And check whether the tooth log buffer is ready
      if(toothHistoryIndex > _countof(toothHistory)) { currentStatus.isToothLog1Full = true; }
    }
    if(BIT_CHECK(currentStatus.LOOP_TIMER, BIT_TIMER_10HZ)) //10 hertz
    {
      programmableIOControl(configPage13);
      
      // Air conditioning control
      airConControl(currentStatus, configPage15);

      #if defined(NATIVE_CAN_AVAILABLE)
      sendCANBroadcast(10);
      #endif

      #ifdef SD_LOGGING
        if(configPage13.onboard_log_file_rate == SD_LOGGER_RATE_10HZ) { writeSDLogEntry(); }
      #endif
    }
    if (BIT_CHECK(currentStatus.LOOP_TIMER, BIT_TIMER_4HZ))
    {
      nitrousControl();

      //Lookup the current target idle RPM. This is aligned with coolant and so needs to be calculated at the same rate CLT is read
      if( (configPage2.idleAdvEnabled != IDLEADVANCE_MODE_OFF) || (configPage6.iacAlgorithm != IAC_ALGORITHM_NONE) )
      {
        currentStatus.CLIdleTarget = table2D_getValue(&idleTargetTable, temperatureAddOffset(currentStatus.coolant)); //All temps are offset by 40 degrees
        if(currentStatus.acStatus.turningOn) { currentStatus.CLIdleTarget += configPage15.airConIdleUpRPMAdder;  } //Adds Idle Up RPM amount if active
      }

      #ifdef SD_LOGGING
        if(configPage13.onboard_log_file_rate == SD_LOGGER_RATE_4HZ) { writeSDLogEntry(); }
      #endif  
           
      if(BIT_CHECK(statusSensors, BIT_SENSORS_AUX_ENBL))
      {
        auxChannelControl(currentStatus, configPage9);
      } //aux channels are enabled
    } //4Hz timer
    if (BIT_CHECK(currentStatus.LOOP_TIMER, BIT_TIMER_1HZ)) //Once per second)
    {
      currentStatus.systemTemp = getSystemTemp();

      if ( (configPage10.wmiEnabled > 0) && (configPage10.wmiIndicatorEnabled > 0) )
      {
        // water tank empty
        if (currentStatus.wmiTankEmpty)
        {
          // flash with 1sec interval
          digitalWrite(pinNumbers.pinWMIIndicator, !digitalRead(pinNumbers.pinWMIIndicator));
        }
        else
        {
          digitalWrite(pinNumbers.pinWMIIndicator, configPage10.wmiIndicatorPolarity ? HIGH : LOW);
        } 
      }

      #ifdef SD_LOGGING
        if(configPage13.onboard_log_file_rate == SD_LOGGER_RATE_1HZ) { writeSDLogEntry(); }
        //SD log sync can take up to 8ms on slow SD cards. To prevent potential issues we only perform this if the RPM is under a safe speed so that there will always be sufficient time for a main loop to run. 
        //A sync will be forced if it hasn't taken place within a max period
        if( (currentStatus.RPM < SD_SYNC_RPM_THRESHOLD) || (msSinceLastSDSync > SD_SYNC_MAX_TIME_PERIOD) )
        { 
          if(syncSDLog()) { msSinceLastSDSync = 0; } //Run SD sync and reset  
        }
      #endif

      fanControl(currentStatus, configPage2, configPage6, configPage15);
    } //1Hz timer

    // Run idlecontrol every loop for stepper idle...
    if (isStepperIac(configPage6)
    // ...or to be run at 10Hz to align with the idle taper resolution of 0.1s
    || BIT_CHECK(currentStatus.LOOP_TIMER, BIT_TIMER_10HZ))
    {
      idleControl(); 
    }

    //VE and advance calculation were moved outside the sync/RPM check so that the fuel and ignition load value will be accurately shown when RPM=0
    currentStatus.VE1 = getVE1();
    currentStatus.VE = currentStatus.VE1; //Set the final VE value to be VE 1 as a default. This may be changed in the section below

    currentStatus.advance1 = getAdvance1();
    currentStatus.advance = currentStatus.advance1; //Set the final advance value to be advance 1 as a default. This may be changed in the section below

    calculateSecondaryFuel(configPage10, fuelTable2, currentStatus);
    calculateSecondarySpark(configPage2, configPage10, ignitionTable2, currentStatus);

    //Always check for sync
    //Main loop runs within this clause
    if ((currentStatus.decoder.getStatus().syncStatus!=SyncStatus::None) && (currentStatus.RPM > 0))
    {
        //Check whether running or cranking
        if(currentStatus.RPM > currentStatus.crankRPM) //Crank RPM in the config is stored as a x10. currentStatus.crankRPM is set in timers.ino and represents the true value
        {
          bool crankToRun = currentStatus.rotationStatus==EngineRotationStatus::Cranking;
          currentStatus.rotationStatus = EngineRotationStatus::Running;

         //Only need to do anything if we're transitioning from cranking to running
          if( crankToRun )
          {
            if(configPage4.ignBypassEnabled > 0) { digitalWrite(pinNumbers.pinIgnBypass, HIGH); }
          }
        }
        else
        {  
          if( (currentStatus.rotationStatus!=EngineRotationStatus::Running) || (currentStatus.RPM < (currentStatus.crankRPM - CRANK_RUN_HYSTER)) )
          {
            //Sets the engine cranking bit, clears the engine running bit
            currentStatus.rotationStatus = EngineRotationStatus::Cranking;
            currentStatus.runSecs = 0; //We're cranking (hopefully), so reset the engine run time to prompt ASE.
            if(configPage4.ignBypassEnabled > 0) { digitalWrite(pinNumbers.pinIgnBypass, LOW); }
          }
        }

      currentStatus.engineProtect = checkEngineProtection(currentStatus, configPage4, configPage6, configPage9, configPage10);
      //END SETTING ENGINE STATUSES
      //-----------------------------------------------------------------------------------------------------

      //Begin the fuel calculation
      
      //Check that the duty cycle of the chosen pulsewidth isn't too high.
      //Calculate an injector pulsewidth from the VE
      currentStatus.afrTarget = calculateAfrTarget(afrTable, currentStatus, configPage2, configPage6);
      currentStatus.corrections = correctionsFuel();

      pulseWidths pulse_widths = computePulseWidths(
                                    configPage2,
                                    configPage6,
                                    configPage10, 
                                    currentStatus);
      currentStatus.stagingActive = pulse_widths.secondary!=0U;

      applyPwToInjectorChannels(pulse_widths, configPage2, configPage4, configPage6, configPage10, currentStatus);
      
      //***********************************************************************************************
      //| BEGIN IGNITION CALCULATIONS

      //Set dwell
      currentStatus.dwell = correctionsDwell(computeDwell(currentStatus, configPage2, configPage4, dwellTable));

      // Convert the dwell time to dwell angle based on the current engine speed
      calculateIgnitionAngles(configPage2, configPage4, configPage13, currentStatus);

      //***********************************************************************************************
      //| BEGIN FUEL SCHEDULES
      //Finally calculate the time (uS) until we reach the firing angles and set the schedules
      //We only need to set the schedule if we're BEFORE the open angle
      //This may potentially be called a number of times as we get closer and closer to the opening time
      
      currentStatus.schedulerCutState = calculateFuelIgnitionChannelCut(currentStatus, configPage2, configPage4, configPage6, configPage9);
      if (currentStatus.schedulerCutState.status==SchedulerCutStatus::None)
      {
        currentStatus.engineProtect.reset();
      }
      
      currentStatus.injAngle = setFuelChannelSchedules(currentStatus);
    
      setIgnitionChannels(currentStatus, configPage4, currentStatus.decoder.getCrankAngle());

    } //Has sync and RPM
    matchResetControlToEngineState(currentStatus);
    pulsedCommandController(currentStatus, configPage13);
    onPowerSourceSwitch(originalBatteryVoltage, currentStatus, configPage2, configPage6);
} //loop()
END_LTO_INLINE()

#endif //Unit test guard
