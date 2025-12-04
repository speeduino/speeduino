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
#include "auxiliaries.h"
#include "sensors.h"
#include "storage.h"
#include "crankMaths.h"
#include "init.h"
#include "utilities.h"
#include "engineProtection.h"
#include "scheduledIO.h"
#include "secondaryTables.h"
#include "comms_CAN.h"
#include "SD_logger.h"
#include "schedule_calcs.h"
#include "auxiliaries.h"
#include "load_source.h"
#include "board_definition.h"
#include "unit_testing.h"
#include RTC_LIB_H //Defined in each boards .h file
#include "units.h"
#include "fuel_calcs.h"
#include "preprocessor.h"
#include "dwell.h"
#include "decoder_init.h"

#define CRANK_RUN_HYSTER    15

// Forward declarations
void calculateIgnitionAngles(uint16_t dwellAngle);
void checkLaunchAndFlatShift();

uint8_t ignitionChannelsOn; /**< The current state of the ignition system (on or off) */
uint8_t ignitionChannelsPending = 0; /**< Any ignition channels that are pending injections before they are resumed */
uint8_t fuelChannelsOn; /**< The current state of the fuel system (on or off) */
uint32_t rollingCutLastRev = 0; /**< Tracks whether we're on the same or a different rev for the rolling cut */
uint32_t revLimitAllowedEndTime = 0;

constexpr table2D_u8_u16_4 injectorAngleTable(&configPage2.injAngRPM, &configPage2.injAng);
constexpr table2D_u8_u8_8 rotarySplitTable(&configPage10.rotarySplitBins, &configPage10.rotarySplitValues);
constexpr table2D_i8_u8_4 rollingCutTable(&configPage15.rollingProtRPMDelta, &configPage15.rollingProtCutPercent);
constexpr table2D_u8_u8_10 idleTargetTable(&configPage6.iacBins, &configPage6.iacCLValues);

#ifndef UNIT_TEST // Scope guard for unit testing

void setup(void)
{
  currentStatus.initialisationComplete = false; //Tracks whether the initialiseAll() function has run completely
  initialiseAll();
}

static inline uint16_t applyFuelTrimToPW(trimTable3d *pTrimTable, uint16_t fuelLoad, int16_t RPM, uint16_t currentPW)
{
    uint8_t pw1percent = 100U + get3DTableValue(pTrimTable, fuelLoad, RPM) - OFFSET_FUELTRIM;
    return percentageApprox(pw1percent, currentPW);
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
  return correctionsIgn((int16_t)get3DTableValue(&ignitionTable, currentStatus.ignLoad, currentStatus.RPM) - INT16_C(OFFSET_IGNITION)); //As above, but for ignition advance
}

static inline void setFuelSchedule(FuelSchedule &schedule, uint8_t channel, uint16_t pw, uint16_t startAngle, uint16_t crankAngle)
{
  if( (pw != 0U) && (BIT_CHECK(fuelChannelsOn, INJ1_CMD_BIT+channel-1U)) )
  {
    uint32_t timeOut = calculateInjectorTimeout(schedule, startAngle, crankAngle);
    if (timeOut>0U)
    {
      setFuelSchedule(schedule, timeOut, pw);
    }
  }
}

static inline void setFuelSchedules(const statuses &current, const uint16_t (&injectionStartAngles)[INJ_CHANNELS], uint16_t crankAngle)
{
#define SET_FUEL_CHANNEL(channel) \
  setFuelSchedule(fuelSchedule ##channel, UINT8_C(channel), current.PW ##channel, injectionStartAngles[(channel)-1U], crankAngle);

#if INJ_CHANNELS >= 1
  SET_FUEL_CHANNEL(1)
#endif

#if INJ_CHANNELS >= 2
  SET_FUEL_CHANNEL(2)
#endif

#if INJ_CHANNELS >= 3
  SET_FUEL_CHANNEL(3)
#endif

#if INJ_CHANNELS >= 4
  SET_FUEL_CHANNEL(4)
#endif

#if INJ_CHANNELS >= 5
  SET_FUEL_CHANNEL(5)
#endif

#if INJ_CHANNELS >= 6
  SET_FUEL_CHANNEL(6)
#endif

#if INJ_CHANNELS >= 7
  SET_FUEL_CHANNEL(7)
#endif

#if INJ_CHANNELS >= 8
  SET_FUEL_CHANNEL(8)
#endif

#undef SET_FUEL_CHANNEL
}

static inline __attribute__((always_inline))  void setIgnitionChannel(IgnitionSchedule &schedule, uint8_t channel, uint16_t channelDegrees, uint16_t startAngle, uint16_t crankAngle, uint16_t dwell) {
  if ((currentStatus.maxIgnOutputs >= channel) && BIT_CHECK(ignitionChannelsOn, channel-1U)) {
    uint32_t timeOut = calculateIgnitionTimeout(schedule, startAngle, channelDegrees, crankAngle);
    if (timeOut > 0U)
    {
      setIgnitionSchedule(schedule, timeOut, dwell);
    }
  }
}

static inline __attribute__((always_inline))  void setIgnitionChannels(uint16_t crankAngle, uint16_t dwell) {
#define SET_IGNITION_CHANNEL(channelIdx) \
  setIgnitionChannel(ignitionSchedule ##channelIdx, UINT8_C((channelIdx)), channel ##channelIdx ##IgnDegrees, ignition ##channelIdx ##StartAngle, crankAngle, dwell);

#if IGN_CHANNELS >= 1
  SET_IGNITION_CHANNEL(1)
#endif

#if defined(USE_IGN_REFRESH)
  if( (isRunning(ignitionSchedule1)) && (ignition1EndAngle > (int)crankAngle) && (configPage4.StgCycles == 0) && (configPage2.perToothIgn != true) )
  {
    unsigned long uSToEnd = 0;

    crankAngle = ignitionLimits(getDecoder().getCrankAngle()); //Refresh the crank angle info
    
    //ONLY ONE OF THE BELOW SHOULD BE USED (PROBABLY THE FIRST):
    //*********
    if(ignition1EndAngle > (int)crankAngle) { uSToEnd = angleToTimeMicroSecPerDegree( (ignition1EndAngle - crankAngle) ); }
    else { uSToEnd = angleToTimeMicroSecPerDegree( (360 + ignition1EndAngle - crankAngle) ); }
    //*********
    //uSToEnd = ((ignition1EndAngle - crankAngle) * (toothLastToothTime - toothLastMinusOneToothTime)) / triggerToothAngle;
    //*********

    refreshIgnitionSchedule1( uSToEnd + fixedCrankingOverride );
  }
#endif
  
#if IGN_CHANNELS >= 2
  SET_IGNITION_CHANNEL(2)
#endif

#if IGN_CHANNELS >= 3
  SET_IGNITION_CHANNEL(3)
#endif

#if IGN_CHANNELS >= 4
  SET_IGNITION_CHANNEL(4)
#endif

#if IGN_CHANNELS >= 5
  SET_IGNITION_CHANNEL(5)
#endif

#if IGN_CHANNELS >= 6
  SET_IGNITION_CHANNEL(6)
#endif

#if IGN_CHANNELS >= 7
  SET_IGNITION_CHANNEL(7)
#endif

#if IGN_CHANNELS >= 8
  SET_IGNITION_CHANNEL(8)
#endif

#undef SET_IGNITION_CHANNEL
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
      if(mainLoopCount < UINT16_MAX) { mainLoopCount++; }
      LOOP_TIMER = TIMER_mask;

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
      if (configPage9.enable_secondarySerial == 1)  //secondary serial interface enabled
      {
        #ifndef CORE_AVR
          if (secondarySerial.available() > 0)  { secondserial_Command(); }
        #else
          if (secondarySerial.available() > SERIAL_BUFFER_THRESHOLD) { secondserial_Command(); } //Special case for AVR units. This prevents potential overflow of the receive buffer
        #endif
      }
      #if defined (NATIVE_CAN_AVAILABLE)
        if (configPage9.enable_intcan == 1) // use internal can module
        {            
          //check local can module
          // if ( BIT_CHECK(LOOP_TIMER, BIT_TIMER_15HZ) or (CANbus0.available())
          while (CAN_read()) 
          {
            can_Command();
            readAuxCanBus();
            if (configPage2.canWBO > 0) { receiveCANwbo(); }
          }
        }   
      #endif
          
    if(currentLoopTime > micros())
    {
      //Occurs when micros() has overflowed
      deferEEPROMWritesUntil = 0; //Required to ensure that EEPROM writes are not deferred indefinitely
    }

    currentLoopTime = micros();
    if ( getDecoder().isEngineRunning(currentLoopTime) )
    {
      setRpm(currentStatus, getDecoder().getRPM());
      if( (currentStatus.RPM > 0) && (currentStatus.fuelPumpOn == false) )
      {
        FUEL_PUMP_ON();
      }
    }
    else
    {
      //We reach here if the time between teeth is too great. This VERY likely means the engine has stopped
      setRpm(currentStatus, 0);
      currentStatus.PW1 = 0;
      currentStatus.VE = 0;
      currentStatus.VE2 = 0;
      getDecoder().reset();
      currentStatus.runSecs = 0; //Reset the counter for number of seconds running.
      currentStatus.startRevolutions = 0;
      resetMAPcycleAndEvent();
      currentStatus.rpmDOT = 0;
      initialiseCorrections();
      ignitionCount = 0;
      ignitionChannelsOn = 0;
      fuelChannelsOn = 0;
      if (currentStatus.fpPrimed == true) { FUEL_PUMP_OFF(); } //Turn off the fuel pump, but only if the priming is complete
      if (configPage6.iacPWMrun == false) { disableIdle(); } //Turn off the idle PWM
      currentStatus.engineIsCranking = false; //Clear cranking bit (Can otherwise get stuck 'on' even with 0 rpm)
      currentStatus.wueIsActive = false; //Same as above except for WUE
      currentStatus.engineIsRunning = false; //Same as above except for RUNNING status
      currentStatus.aseIsActive = false; //Same as above except for ASE status
      currentStatus.isAcceleratingTPS = false; //Same as above but the accel enrich (If using MAP accel enrich a stall will cause this to trigger)
      currentStatus.isDeceleratingTPS = false; //Same as above but the decel enleanment
      //This is a safety check. If for some reason the interrupts have got screwed up (Leading to 0rpm), this resets them.
      //It can possibly be run much less frequently.
      //This should only be run if the high speed logger are off because it will change the trigger interrupts back to defaults rather than the logger versions
      if( (currentStatus.toothLogEnabled == false) && (currentStatus.compositeTriggerUsed == 0) ) { 
        setDecoder(configPage4.TrigPattern);
      }

      VVT1_PIN_LOW();
      VVT2_PIN_LOW();
      DISABLE_VVT_TIMER();
      boostDisable();
      if(configPage4.ignBypassEnabled > 0) { digitalWrite(pinIgnBypass, LOW); } //Reset the ignition bypass ready for next crank attempt
    }
    //***Perform sensor reads***
    //-----------------------------------------------------------------------------------------------------
    readPolledSensors(LOOP_TIMER);

    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_1KHZ)) //Every 1ms. NOTE: This is NOT guaranteed to run at 1kHz on AVR systems. It will run at 1kHz if possible or as fast as loops/s allows if not. 
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_1KHZ);
    }
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_200HZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_200HZ);
    }
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_50HZ)) //50 hertz
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_50HZ);

      #if defined(NATIVE_CAN_AVAILABLE)
      sendCANBroadcast(50);
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
      sendCANBroadcast(30);
      #endif

      #ifdef SD_LOGGING
        if(configPage13.onboard_log_file_rate == LOGGER_RATE_30HZ) { writeSDLogEntry(); }
      #endif

      //AVR units process secondary serial requests at a fixed 30Hz
      #ifdef CORE_AVR
      if( (configPage9.enable_secondarySerial == 1) && (secondarySerial.available() > 0) ) //secondary serial interface enabled
      {
        secondserial_Command();
      }
      #endif

      //Check for any outstanding EEPROM writes.
      if( (isEepromWritePending() == true) && (serialStatusFlag == SERIAL_INACTIVE) && (micros() > deferEEPROMWritesUntil)) { writeAllConfig(); } 
    }
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_15HZ)) //Every 32 loops
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_15HZ);
      #if  defined(CORE_TEENSY35)       
          if (configPage9.enable_intcan == 1) // use internal can module
          {
           // this is just to test the interface is sending
           //sendCancommand(3,((configPage9.realtime_base_address & 0x3FF)+ 0x100),currentStatus.TPS,0,0x200);
          }
      #endif     

      checkLaunchAndFlatShift(); //Check for launch control and flat shift being active

      #if defined(NATIVE_CAN_AVAILABLE)
      sendCANBroadcast(15);
      #endif

      //And check whether the tooth log buffer is ready
      if(toothHistoryIndex > TOOTH_LOG_SIZE) { currentStatus.isToothLog1Full = true; }
    }
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ)) //10 hertz
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_10HZ);
      //updateFullStatus();
      checkProgrammableIO();
      idleControl(); //Perform any idle related actions. This needs to be run at 10Hz to align with the idle taper resolution of 0.1s
      
      // Air conditioning control
      airConControl();

      #if defined(NATIVE_CAN_AVAILABLE)
      sendCANBroadcast(10);
      #endif

      #ifdef SD_LOGGING
        if(configPage13.onboard_log_file_rate == LOGGER_RATE_10HZ) { writeSDLogEntry(); }
      #endif
    }
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_4HZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_4HZ);
      nitrousControl();

      //Lookup the current target idle RPM. This is aligned with coolant and so needs to be calculated at the same rate CLT is read
      if( (configPage2.idleAdvEnabled != IDLEADVANCE_MODE_OFF) || (configPage6.iacAlgorithm != IAC_ALGORITHM_NONE) )
      {
        currentStatus.CLIdleTarget = table2D_getValue(&idleTargetTable, temperatureAddOffset(currentStatus.coolant)); //All temps are offset by 40 degrees
        if(currentStatus.airconTurningOn) { currentStatus.CLIdleTarget += configPage15.airConIdleUpRPMAdder;  } //Adds Idle Up RPM amount if active
      }

      #ifdef SD_LOGGING
        if(configPage13.onboard_log_file_rate == LOGGER_RATE_4HZ) { writeSDLogEntry(); }
      #endif  
           
      if(BIT_CHECK(statusSensors, BIT_SENSORS_AUX_ENBL))
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
              //send an R command for data from caninput_source_address[currentStatus.current_caninchannel] from secondarySerial
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
      currentStatus.systemTemp = getSystemTemp();

      if ( (configPage10.wmiEnabled > 0) && (configPage10.wmiIndicatorEnabled > 0) )
      {
        // water tank empty
        if (currentStatus.wmiTankEmpty)
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
        //SD log sync can take up to 8ms on slow SD cards. To prevent potential issues we only perform this if the RPM is under a safe speed so that there will always be sufficient time for a main loop to run. 
        //A sync will be forced if it hasn't taken place within a max period
        if( (currentStatus.RPM < SD_SYNC_RPM_THRESHOLD) || (msSinceLastSDSync > SD_SYNC_MAX_TIME_PERIOD) )
        { 
          if(syncSDLog()) { msSinceLastSDSync = 0; } //Run SD sync and reset  
        }
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

    calculateSecondaryFuel(configPage10, fuelTable2, currentStatus);
    calculateSecondarySpark(configPage2, configPage10, ignitionTable2, currentStatus);

    //Always check for sync
    //Main loop runs within this clause
    if ((getDecoderStatus().syncStatus!=SyncStatus::None) && (currentStatus.RPM > 0))
    {
        //Check whether running or cranking
        if(currentStatus.RPM > currentStatus.crankRPM) //Crank RPM in the config is stored as a x10. currentStatus.crankRPM is set in timers.ino and represents the true value
        {
          currentStatus.engineIsRunning = true; //Sets the engine running bit
          //Only need to do anything if we're transitioning from cranking to running
          if( currentStatus.engineIsCranking )
          {
            currentStatus.engineIsCranking = false;
            if(configPage4.ignBypassEnabled > 0) { digitalWrite(pinIgnBypass, HIGH); }
          }
        }
        else
        {  
          if( !currentStatus.engineIsRunning || (currentStatus.RPM < (currentStatus.crankRPM - CRANK_RUN_HYSTER)) )
          {
            //Sets the engine cranking bit, clears the engine running bit
            currentStatus.engineIsCranking = true;
            currentStatus.engineIsRunning = false;
            currentStatus.runSecs = 0; //We're cranking (hopefully), so reset the engine run time to prompt ASE.
            if(configPage4.ignBypassEnabled > 0) { digitalWrite(pinIgnBypass, LOW); }

            //Check whether the user has selected to disable to the fan during cranking
            if(configPage2.fanWhenCranking == 0) { FAN_OFF(); }
          }
        }
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
                                    getDecoderStatus(),
                                    currentStatus);
      currentStatus.stagingActive = pulse_widths.secondary!=0U;
      applyPwToInjectorChannels(pulse_widths, configPage2, currentStatus);

      //***********************************************************************************************
      //BEGIN INJECTION TIMING
      currentStatus.injAngle = table2D_getValue(&injectorAngleTable, currentStatus.RPMdiv100);
      if(currentStatus.injAngle > uint16_t(CRANK_ANGLE_MAX_INJ)) { currentStatus.injAngle = uint16_t(CRANK_ANGLE_MAX_INJ); }

      unsigned int PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW1); //How many crank degrees the calculated PW will take at the current speed

      uint16_t injectionStartAngles[INJ_CHANNELS];
      injectionStartAngles[0] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);

      //Repeat the above for each cylinder
      switch (configPage2.nCylinders)
      {
        //Single cylinder
        case 1:
          //The only thing that needs to be done for single cylinder is to check for staging. 
          if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
          {
            PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW2); //Need to redo this for PW2 as it will be dramatically different to PW1 when staging
            injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
          }
          break;
        //2 cylinders
        case 2:
          injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
          
          if ( (configPage2.injLayout == INJ_SEQUENTIAL) && (configPage6.fuelTrimEnabled > 0) )
          {
            currentStatus.PW1 = applyFuelTrimToPW(&trim1Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW1);
            currentStatus.PW2 = applyFuelTrimToPW(&trim2Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW2);
          }
          else if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
          {
            PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW3); //Need to redo this for PW3 as it will be dramatically different to PW1 when staging
            injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
            // injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);

            injectionStartAngles[3] = injectionStartAngles[2] + (CRANK_ANGLE_MAX_INJ / 2); //Phase this either 180 or 360 degrees out from inj3 (In reality this will always be 180 as you can't have sequential and staged currently)
            if(injectionStartAngles[3] > (uint16_t)CRANK_ANGLE_MAX_INJ) { injectionStartAngles[3] -= CRANK_ANGLE_MAX_INJ; }
          }
          break;
        //3 cylinders
        case 3:
          injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
          injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
          
          if ( (configPage2.injLayout == INJ_SEQUENTIAL) && (configPage6.fuelTrimEnabled > 0) )
          {
            currentStatus.PW1 = applyFuelTrimToPW(&trim1Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW1);
            currentStatus.PW2 = applyFuelTrimToPW(&trim2Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW2);
            currentStatus.PW3 = applyFuelTrimToPW(&trim3Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW3);

            #if INJ_CHANNELS >= 6
              if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
              {
                PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW4); //Need to redo this for PW4 as it will be dramatically different to PW1 when staging
                injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
                injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
                injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
              }
            #endif
          }
          else if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
          {
            PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW4); //Need to redo this for PW3 as it will be dramatically different to PW1 when staging
            injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
            #if INJ_CHANNELS >= 6
              injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
              injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
            #endif
          }
          break;
        //4 cylinders
        case 4:
          injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);

          if((configPage2.injLayout == INJ_SEQUENTIAL) && getDecoderStatus().syncStatus==SyncStatus::Full)
          {
            if( CRANK_ANGLE_MAX_INJ != 720 ) { changeHalfToFullSync(configPage2, configPage4, currentStatus); }

            injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
            injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees, currentStatus.injAngle);
            #if INJ_CHANNELS >= 8
              if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
              {
                PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW5); //Need to redo this for PW5 as it will be dramatically different to PW1 when staging
                injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
                injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
                injectionStartAngles[6] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
                injectionStartAngles[7] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees, currentStatus.injAngle);
              }
            #endif

            if(configPage6.fuelTrimEnabled > 0)
            {
              currentStatus.PW1 = applyFuelTrimToPW(&trim1Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW1);
              currentStatus.PW2 = applyFuelTrimToPW(&trim2Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW2);
              currentStatus.PW3 = applyFuelTrimToPW(&trim3Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW3);
              currentStatus.PW4 = applyFuelTrimToPW(&trim4Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW4);
            }
          }
          else if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
          {
            PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW3); //Need to redo this for PW3 as it will be dramatically different to PW1 when staging
            injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
            injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
          }
          else
          {
            if( getDecoderStatus().syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_INJ != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }
          }
          break;
        //5 cylinders
        case 5:
          injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
          injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
          injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees, currentStatus.injAngle);
          #if INJ_CHANNELS >= 5
            injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel5InjDegrees, currentStatus.injAngle);
          #endif

          //Staging is possible by using the 6th channel if available
          #if INJ_CHANNELS >= 6
            if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
            {
              PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW6);
              injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel6InjDegrees, currentStatus.injAngle);
            }
          #endif

          break;
        //6 cylinders
        case 6:
          injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
          injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
          
          #if INJ_CHANNELS >= 6
            if((configPage2.injLayout == INJ_SEQUENTIAL) && getDecoderStatus().syncStatus==SyncStatus::Full)
            {
              if( CRANK_ANGLE_MAX_INJ != 720 ) { changeHalfToFullSync(configPage2, configPage4, currentStatus); }

              injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees, currentStatus.injAngle);
              injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel5InjDegrees, currentStatus.injAngle);
              injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel6InjDegrees, currentStatus.injAngle);

              if(configPage6.fuelTrimEnabled > 0)
              {
                currentStatus.PW1 = applyFuelTrimToPW(&trim1Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW1);
                currentStatus.PW2 = applyFuelTrimToPW(&trim2Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW2);
                currentStatus.PW3 = applyFuelTrimToPW(&trim3Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW3);
                currentStatus.PW4 = applyFuelTrimToPW(&trim4Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW4);
                currentStatus.PW5 = applyFuelTrimToPW(&trim5Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW5);
                currentStatus.PW6 = applyFuelTrimToPW(&trim6Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW6);
              }

              //Staging is possible with sequential on 8 channel boards by using outputs 7 + 8 for the staged injectors
              #if INJ_CHANNELS >= 8
                if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
                {
                  PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW4); //Need to redo this for staging PW as it will be dramatically different to PW1 when staging
                  injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
                  injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
                  injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
                }
              #endif
            }
            else
            {
              if( getDecoderStatus().syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_INJ != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }

              if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
              {
                PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW4); //Need to redo this for staging PW as it will be dramatically different to PW1 when staging
                injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
                injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
                injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle); 
              }
            }
          #endif
          break;
        //8 cylinders
        case 8:
          injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
          injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
          injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees, currentStatus.injAngle);

          #if INJ_CHANNELS >= 8
            if((configPage2.injLayout == INJ_SEQUENTIAL) && getDecoderStatus().syncStatus==SyncStatus::Full)
            {
              if( CRANK_ANGLE_MAX_INJ != 720 ) { changeHalfToFullSync(configPage2, configPage4, currentStatus); }

              injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel5InjDegrees, currentStatus.injAngle);
              injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel6InjDegrees, currentStatus.injAngle);
              injectionStartAngles[6] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel7InjDegrees, currentStatus.injAngle);
              injectionStartAngles[7] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel8InjDegrees, currentStatus.injAngle);

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
              if( getDecoderStatus().syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_INJ != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }

              if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
              {
                PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW5); //Need to redo this for PW3 as it will be dramatically different to PW1 when staging
                injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
                injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
                injectionStartAngles[6] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
                injectionStartAngles[7] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees, currentStatus.injAngle);
              }
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
      currentStatus.dwell = correctionsDwell(computeDwell(currentStatus, configPage2, configPage4, dwellTable));

      // Convert the dwell time to dwell angle based on the current engine speed
      calculateIgnitionAngles(timeToAngleDegPerMicroSec(currentStatus.dwell));

      //If ignition timing is being tracked per tooth, perform the calcs to get the end teeth
      //This only needs to be run if the advance figure has changed, otherwise the end teeth will still be the same
      //if( (configPage2.perToothIgn == true) && (lastToothCalcAdvance != currentStatus.advance) ) { triggerSetEndTeeth(); }
      if( (configPage2.perToothIgn == true) && (getDecoderFeatures().supportsPerToothIgnition) ) { getDecoder().setEndTeeth(); }

      //***********************************************************************************************
      //| BEGIN FUEL SCHEDULES
      //Finally calculate the time (uS) until we reach the firing angles and set the schedules
      //We only need to set the schedule if we're BEFORE the open angle
      //This may potentially be called a number of times as we get closer and closer to the opening time

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
      uint16_t maxAllowedRPM = checkRevLimit(currentStatus, configPage4, configPage6, configPage9); //The maximum RPM allowed by all the potential limiters (Engine protection, 2-step, flat shift etc). Divided by 100. `checkRevLimit()` returns the current maximum RPM allow (divided by 100) based on either the fixed hard limit or the current coolant temp
      //Check each of the functions that has an RPM limit. Update the max allowed RPM if the function is active and has a lower RPM than already set
      if( checkEngineProtect(currentStatus, configPage4, configPage6, configPage9, configPage10) && (configPage4.engineProtectMaxRPM < maxAllowedRPM)) { maxAllowedRPM = configPage4.engineProtectMaxRPM; }
      if ( (currentStatus.launchingHard == true) && (configPage6.lnchHardLim < maxAllowedRPM) ) { maxAllowedRPM = configPage6.lnchHardLim; }
      maxAllowedRPM = maxAllowedRPM * 100; //All of the above limits are divided by 100, convert back to RPM
      if ( (currentStatus.flatShiftingHard == true) && (currentStatus.clutchEngagedRPM < maxAllowedRPM) ) { maxAllowedRPM = currentStatus.clutchEngagedRPM; } //Flat shifting is a special case as the RPM limit is based on when the clutch was engaged. It is not divided by 100 as it is set with the actual RPM
    
      if(currentStatus.RPM >= maxAllowedRPM)
      {
        currentStatus.hardLimitActive = true;
      }
      else if(currentStatus.hardLimitActive)
      {
        //if(micros() > revLimitAllowedEndTime) //Hysteresis check disabled for now. 
        {
          revLimitAllowedEndTime = 0;
          currentStatus.hardLimitActive = false;
        }
      }

      if( (configPage2.hardCutType == HARD_CUT_FULL) && currentStatus.hardLimitActive)
      {
        //Full hard cut turns outputs off completely. 
        switch(configPage6.engineProtectType)
        {
          case PROTECT_CUT_OFF:
            //Make sure all channels are turned on
            ignitionChannelsOn = 0xFF;
            fuelChannelsOn = 0xFF;
            resetEngineProtect(currentStatus);
            break;
          case PROTECT_CUT_IGN:
            ignitionChannelsOn = 0;
            disableAllIgnSchedules();
            break;
          case PROTECT_CUT_FUEL:
            fuelChannelsOn = 0;
            disableAllFuelSchedules();
            break;
          case PROTECT_CUT_BOTH:
            ignitionChannelsOn = 0;
            fuelChannelsOn = 0;
            disableAllIgnSchedules();
            disableAllFuelSchedules();
            break;
          default:
            ignitionChannelsOn = 0;
            fuelChannelsOn = 0;
            break;
        }
      } //Hard cut check
      else if( (configPage2.hardCutType == HARD_CUT_ROLLING) && (currentStatus.RPM > (maxAllowedRPM + (configPage15.rollingProtRPMDelta[0] * 10))) ) //Limit for rolling is the max allowed RPM minus the lowest value in the delta table (Delta values are negative!)
      { 
        uint8_t revolutionsToCut = 1;
        if(configPage2.strokes == FOUR_STROKE) { revolutionsToCut *= 2; } //4 stroke needs to cut for at least 2 revolutions
        if( (configPage4.sparkMode != IGN_MODE_SEQUENTIAL) || (configPage2.injLayout != INJ_SEQUENTIAL) ) { revolutionsToCut *= 2; } //4 stroke and non-sequential will cut for 4 revolutions minimum. This is to ensure no half fuel ignition cycles take place

        if(rollingCutLastRev == 0) { rollingCutLastRev = currentStatus.startRevolutions; } //First time check
        if ( (currentStatus.startRevolutions >= (rollingCutLastRev + revolutionsToCut)) || (currentStatus.RPM > maxAllowedRPM) ) //If current RPM is over the max allowed RPM always cut, otherwise check if the required number of revolutions have passed since the last cut
        { 
          uint8_t cutPercent = 0;
          int16_t rpmDelta = currentStatus.RPM - maxAllowedRPM;
          if(rpmDelta >= 0) { cutPercent = 100; } //If the current RPM is over the max allowed RPM then cut is full (100%)
          else { cutPercent = table2D_getValue(&rollingCutTable, (int8_t)(rpmDelta / 10) ); } //
          

          for(uint8_t x=0; x<max(currentStatus.maxIgnOutputs, currentStatus.maxInjOutputs); x++)
          {  
            if( (cutPercent == 100) || (random1to100() < cutPercent) )
            {
              switch(configPage6.engineProtectType)
              {
                case PROTECT_CUT_OFF:
                  //Make sure all channels are turned on
                  ignitionChannelsOn = 0xFF;
                  fuelChannelsOn = 0xFF;
                  break;
                case PROTECT_CUT_IGN:
                  BIT_CLEAR(ignitionChannelsOn, x); //Turn off this ignition channel
                  disableIgnSchedule(x);
                  break;
                case PROTECT_CUT_FUEL:
                  BIT_CLEAR(fuelChannelsOn, x); //Turn off this fuel channel
                  disableFuelSchedule(x);
                  break;
                case PROTECT_CUT_BOTH:
                  BIT_CLEAR(ignitionChannelsOn, x); //Turn off this ignition channel
                  BIT_CLEAR(fuelChannelsOn, x); //Turn off this fuel channel
                  disableFuelSchedule(x);
                  disableIgnSchedule(x);
                  break;
                default:
                  BIT_CLEAR(ignitionChannelsOn, x); //Turn off this ignition channel
                  BIT_CLEAR(fuelChannelsOn, x); //Turn off this fuel channel
                  break;
              }
            }
            else
            {
              //Turn fuel and ignition channels on

              //Special case for non-sequential, 4-stroke where both fuel and ignition are cut. The ignition pulses should wait 1 cycle after the fuel channels are turned back on before firing again
              if( (revolutionsToCut == 4) &&                          //4 stroke and non-sequential
                  (BIT_CHECK(fuelChannelsOn, x) == false) &&          //Fuel on this channel is currently off, meaning it is the first revolution after a cut
                  (configPage6.engineProtectType == PROTECT_CUT_BOTH) //Both fuel and ignition are cut
                )
              { BIT_SET(ignitionChannelsPending, x); } //Set this ignition channel as pending
              else { BIT_SET(ignitionChannelsOn, x); } //Turn on this ignition channel
                
              
              BIT_SET(fuelChannelsOn, x); //Turn on this fuel channel
            }
          }
          rollingCutLastRev = currentStatus.startRevolutions;
        }

        //Check whether there are any ignition channels that are waiting for injection pulses to occur before being turned back on. This can only occur when at least 2 revolutions have taken place since the fuel was turned back on
        //Note that ignitionChannelsPending can only be >0 on 4 stroke, non-sequential fuel when protect type is Both
        if( (ignitionChannelsPending > 0) && (currentStatus.startRevolutions >= (rollingCutLastRev + 2)) )
        {
          ignitionChannelsOn = fuelChannelsOn;
          ignitionChannelsPending = 0;
        }
      } //Rolling cut check
      else
      {
        resetEngineProtect(currentStatus);
        //No engine protection active, so turn all the channels on
        if(currentStatus.startRevolutions >= configPage4.StgCycles)
        { 
          //Enable the fuel and ignition, assuming staging revolutions are complete 
          ignitionChannelsOn = 0xff; 
          fuelChannelsOn = 0xff; 
        } 
      }
      
      setFuelSchedules(currentStatus, injectionStartAngles, injectorLimits(getDecoder().getCrankAngle()));
    
      //***********************************************************************************************
      //| BEGIN IGNITION SCHEDULES
      //Same as above, except for ignition

      //fixedCrankingOverride is used to extend the dwell during cranking so that the decoder can trigger the spark upon seeing a certain tooth. Currently only available on the basic distributor and 4g63 decoders.
      if ( configPage4.ignCranklock && currentStatus.engineIsCranking && (getDecoderFeatures().hasFixedCrankingTiming) )
      {
        fixedCrankingOverride = currentStatus.dwell * 3;
        //This is a safety step to prevent the ignition start time occurring AFTER the target tooth pulse has already occurred. It simply moves the start time forward a little, which is compensated for by the increase in the dwell time
        if(currentStatus.RPM < 250)
        {
          ignition1StartAngle -= 5;
          ignition2StartAngle -= 5;
          ignition3StartAngle -= 5;
          ignition4StartAngle -= 5;
#if IGN_CHANNELS >= 5
          ignition5StartAngle -= 5;
#endif
#if IGN_CHANNELS >= 6          
          ignition6StartAngle -= 5;
#endif
#if IGN_CHANNELS >= 7
          ignition7StartAngle -= 5;
#endif
#if IGN_CHANNELS >= 8
          ignition8StartAngle -= 5;
#endif
        }
      }
      else { fixedCrankingOverride = 0; }

      if(ignitionChannelsOn > 0)
      {
        setIgnitionChannels(ignitionLimits(getDecoder().getCrankAngle()), currentStatus.dwell + fixedCrankingOverride);
      } //Ignition schedules on

      if ( (!currentStatus.resetPreventActive) && (resetControl == RESET_CONTROL_PREVENT_WHEN_RUNNING) ) 
      {
        //Reset prevention is supposed to be on while the engine is running but isn't. Fix that.
        digitalWrite(pinResetControl, HIGH);
        currentStatus.resetPreventActive = true;
      }
    } //Has sync and RPM
    else if ( (currentStatus.resetPreventActive) && (resetControl == RESET_CONTROL_PREVENT_WHEN_RUNNING) )
    {
      digitalWrite(pinResetControl, LOW);
      currentStatus.resetPreventActive = false;
    }
} //loop()
END_LTO_INLINE()

#endif //Unit test guard

/** Calculate the Ignition angles for all cylinders (based on @ref config2.nCylinders).
 * both start and end angles are calculated for each channel.
 * Also the mode of ignition firing - wasted spark vs. dedicated spark per cyl. - is considered here.
 */
void calculateIgnitionAngles(uint16_t dwellAngle)
{
  //This test for more cylinders and do the same thing
  switch (configPage2.nCylinders)
  {
    //1 cylinder
    case 1:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      break;
    //2 cylinders
    case 2:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);
      break;
    //3 cylinders
    case 3:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);
      calculateIgnitionAngle(dwellAngle, channel3IgnDegrees, currentStatus.advance, &ignition3EndAngle, &ignition3StartAngle);
      break;
    //4 cylinders
    case 4:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);

      #if IGN_CHANNELS >= 4
      if((configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && getDecoderStatus().syncStatus==SyncStatus::Full)
      {
        if( CRANK_ANGLE_MAX_IGN != 720 ) { changeHalfToFullSync(configPage2, configPage4, currentStatus); }

        calculateIgnitionAngle(dwellAngle, channel3IgnDegrees, currentStatus.advance, &ignition3EndAngle, &ignition3StartAngle);
        calculateIgnitionAngle(dwellAngle, channel4IgnDegrees, currentStatus.advance, &ignition4EndAngle, &ignition4StartAngle);
      }
      else if(configPage4.sparkMode == IGN_MODE_ROTARY)
      {
        byte splitDegrees = 0;
        splitDegrees = table2D_getValue(&rotarySplitTable, (uint8_t)currentStatus.ignLoad);

        //The trailing angles are set relative to the leading ones
        calculateIgnitionTrailingRotary(dwellAngle, splitDegrees, ignition1EndAngle, &ignition3EndAngle, &ignition3StartAngle);
        calculateIgnitionTrailingRotary(dwellAngle, splitDegrees, ignition2EndAngle, &ignition4EndAngle, &ignition4StartAngle);
      }
      else
      {
        if( getDecoderStatus().syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_IGN != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }
      }
      #endif
      break;
    //5 cylinders
    case 5:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);
      calculateIgnitionAngle(dwellAngle, channel3IgnDegrees, currentStatus.advance, &ignition3EndAngle, &ignition3StartAngle);
      calculateIgnitionAngle(dwellAngle, channel4IgnDegrees, currentStatus.advance, &ignition4EndAngle, &ignition4StartAngle);
      #if (IGN_CHANNELS >= 5)
      calculateIgnitionAngle(dwellAngle, channel5IgnDegrees, currentStatus.advance, &ignition5EndAngle, &ignition5StartAngle);
      #endif
      break;
    //6 cylinders
    case 6:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);
      calculateIgnitionAngle(dwellAngle, channel3IgnDegrees, currentStatus.advance, &ignition3EndAngle, &ignition3StartAngle);

      #if IGN_CHANNELS >= 6
      if((configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && getDecoderStatus().syncStatus==SyncStatus::Full)
      {
        if( CRANK_ANGLE_MAX_IGN != 720 ) { changeHalfToFullSync(configPage2, configPage4, currentStatus); }

        calculateIgnitionAngle(dwellAngle, channel4IgnDegrees, currentStatus.advance, &ignition4EndAngle, &ignition4StartAngle);
        calculateIgnitionAngle(dwellAngle, channel5IgnDegrees, currentStatus.advance, &ignition5EndAngle, &ignition5StartAngle);
        calculateIgnitionAngle(dwellAngle, channel6IgnDegrees, currentStatus.advance, &ignition6EndAngle, &ignition6StartAngle);
      }
      else
      {
        if( getDecoderStatus().syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_IGN != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }
      }
      #endif
      break;
    //8 cylinders
    case 8:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);
      calculateIgnitionAngle(dwellAngle, channel3IgnDegrees, currentStatus.advance, &ignition3EndAngle, &ignition3StartAngle);
      calculateIgnitionAngle(dwellAngle, channel4IgnDegrees, currentStatus.advance, &ignition4EndAngle, &ignition4StartAngle);

      #if IGN_CHANNELS >= 8
      if((configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && getDecoderStatus().syncStatus==SyncStatus::Full)
      {
        if( CRANK_ANGLE_MAX_IGN != 720 ) { changeHalfToFullSync(configPage2, configPage4, currentStatus); }

        calculateIgnitionAngle(dwellAngle, channel5IgnDegrees, currentStatus.advance, &ignition5EndAngle, &ignition5StartAngle);
        calculateIgnitionAngle(dwellAngle, channel6IgnDegrees, currentStatus.advance, &ignition6EndAngle, &ignition6StartAngle);
        calculateIgnitionAngle(dwellAngle, channel7IgnDegrees, currentStatus.advance, &ignition7EndAngle, &ignition7StartAngle);
        calculateIgnitionAngle(dwellAngle, channel8IgnDegrees, currentStatus.advance, &ignition8EndAngle, &ignition8StartAngle);
      }
      else
      {
        if( getDecoderStatus().syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_IGN != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }
      }
      #endif
      break;

    //Will hit the default case on >8 cylinders. Do nothing in these cases
    default:
      break;
  }
}


void checkLaunchAndFlatShift()
{
  //Check for launching/flat shift (clutch) based on the current and previous clutch states
  currentStatus.previousClutchTrigger = currentStatus.clutchTrigger;
  //Only check for pinLaunch if any function using it is enabled. Else pins might break starting a board
  if(configPage6.flatSEnable || configPage6.launchEnabled)
  {
    if(configPage6.launchHiLo > 0) { currentStatus.clutchTrigger = digitalRead(pinLaunch); }
    else { currentStatus.clutchTrigger = !digitalRead(pinLaunch); }

    currentStatus.clutchTriggerActive = currentStatus.clutchTrigger; //Stores the value to send to TunerStudio
  }
  if(currentStatus.clutchTrigger && (currentStatus.previousClutchTrigger != currentStatus.clutchTrigger) ) { currentStatus.clutchEngagedRPM = currentStatus.RPM; } //Check whether the clutch has been engaged or disengaged and store the current RPM if so

  //Default flags to off
  currentStatus.launchingHard = false; 
  currentStatus.hardLaunchActive = false;
  currentStatus.flatShiftingHard = false;

  if (configPage6.launchEnabled && currentStatus.clutchTrigger && (currentStatus.clutchEngagedRPM < ((unsigned int)(configPage6.flatSArm) * 100)) && (currentStatus.TPS >= configPage10.lnchCtrlTPS) ) 
  { 
    //Only enable if VSS is not used or if it is, make sure we're not above the speed limit
    if( (configPage2.vssMode == 0) || ((configPage2.vssMode > 0) && (currentStatus.vss < configPage10.lnchCtrlVss)) )
    {
      //Check whether RPM is above the launch limit
      uint16_t launchRPMLimit = (configPage6.lnchHardLim * 100);
      if( (configPage2.hardCutType == HARD_CUT_ROLLING) ) { launchRPMLimit += (configPage15.rollingProtRPMDelta[0] * 10); } //Add the rolling cut delta if enabled (Delta is a negative value)

      if(currentStatus.RPM > launchRPMLimit)
      {
        //HardCut rev limit for 2-step launch control.
        currentStatus.launchingHard = true; 
        currentStatus.hardLaunchActive = true;
      }
    }
  } 
  else 
  { 
    //If launch is not active, check whether flat shift should be active
    if(configPage6.flatSEnable && currentStatus.clutchTrigger && (currentStatus.clutchEngagedRPM >= ((unsigned int)(configPage6.flatSArm * 100)) ) ) 
    { 
      uint16_t flatRPMLimit = currentStatus.clutchEngagedRPM;
      if( (configPage2.hardCutType == HARD_CUT_ROLLING) ) { flatRPMLimit += (configPage15.rollingProtRPMDelta[0] * 10); } //Add the rolling cut delta if enabled (Delta is a negative value)

      if(currentStatus.RPM > flatRPMLimit)
      {
        //Flat shift rev limit
        currentStatus.flatShiftingHard = true;
      }
    }
  }
}
