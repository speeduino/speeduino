/** @file
 * Speeduino Initialisation (called at Arduino setup()).
 */
#include "globals.h"
#include "init.h"
#include "storage.h"
#include "updates.h"
#include "timers.h"
#include "comms.h"
#include "comms_secondary.h"
#include "comms_CAN.h"
#include "programmableIOControl.h"
#include "scheduler.h"
#include "schedule_calcs.h"
#include "auxiliaries.h"
#include "sensors.h"
#include "decoders.h"
#include "corrections.h"
#include "idle.h"
#include "table2d.h"
#include "acc_mc33810.h"
#include "board_definition.h"
#include "pages.h"
#ifdef SD_LOGGING
  #include "SD_logger.h"
  #include "rtc_common.h"
#endif
#include "fuel_calcs.h"
#include "decoder_init.h"
#include "scheduledIO_ign.h"
#include "scheduledIO_inj.h"
#include "scheduledIO_direct_ign.h"
#include "scheduledIO_direct_inj.h"
#include "src/pins/pinMapping.h"
#include "resetControl.h"
#include "scheduler_ignition_controller.h"
#include "maths.h"

#if defined(CORE_AVR)
#pragma GCC push_options
// This minimizes RAM usage at no performance cost
#pragma GCC optimize ("Os") 
#endif

static void __attribute__((optimize("Os"))) stopAllCoilsCharging(void)
{
  for (uint8_t index=1; index<=IGN_CHANNELS; ++index)
  {
    endCoilCharge(index);
  }
}

static void __attribute__((optimize("Os"))) closeAllInjectors(void)
{
  for (uint8_t index=1; index<=INJ_CHANNELS; ++index)
  {
    closeInjector(index);
  }
}

///
/// @brief Allow the user to reset the firmware storage (aka EPROM).
///
/// This gives the user the opportunity to clear the permanent storage
/// at start up. 
///
/// See https://github.com/noisymime/speeduino/pull/657
///
#if !defined(UNIT_TEST)
static void processResetStorageRequest(void) {
#if defined(EEPROM_RESET_PIN)

  constexpr uint32_t START_RESET_INTERVAL = MILLI_PER_SEC+50;
  constexpr uint32_t MIN_BUTTON_PRESSED_INTERVAL = MILLI_PER_SEC/2;
  constexpr uint32_t MAX_BUTTON_RELEASE_INTERVAL = MILLI_PER_SEC;
  
  uint32_t start_time = millis();
  bool exit_erase_loop = false; 
  pinMode(EEPROM_RESET_PIN, INPUT_PULLUP);  

  //only start routine when this pin is low because it is pulled low
  while (digitalRead(EEPROM_RESET_PIN) != HIGH && (millis() - start_time)<START_RESET_INTERVAL)
  {
    //make sure the key is pressed for at least 0.5 second 
    if ((millis() - start_time)>(MIN_BUTTON_PRESSED_INTERVAL)) {
      //if key is pressed afterboot for 0.5 second make led turn off
      digitalWrite(LED_BUILTIN, HIGH);

      //see if the user reacts to the led turned off with removing the keypress within 1 second
      while (((millis() - start_time)<MAX_BUTTON_RELEASE_INTERVAL) && (exit_erase_loop!=true)){

        //if user let go of key within 1 second erase eeprom
        if(digitalRead(EEPROM_RESET_PIN) != LOW){
          fillBlock(getStorageAPI(), 0, getStorageAPI().length(), UINT8_MAX);
          //if erase done exit while loop.
          exit_erase_loop = true;
        }
      }
    } 
  }
#endif
}
#endif

/** Initialise Speeduino for the main loop.
 * Top level init entry point for all initialisations:
 * - Initialise and set sizes of 3D tables
 * - Load config from EEPROM, update config structures to current version of SW if needed.
 * - Initialise board (The initBoard() is for board X implemented in board_X.ino file)
 * - Initialise timers (See timers.ino)
 * - Perform optional SD card and RTC battery inits
 * - Load calibration tables from EEPROM
 * - Perform pin mapping (calling @ref setPinMapping() based on @ref config2.pinMapping)
 * - Stop any coil charging and close injectors
 * - Initialise schedulers, Idle, Fan, auxPWM, Corrections, AD-conversions, Programmable I/O
 * - Initialise baro (ambient pressure) by reading MAP (before engine runs)
 * - Initialise triggers (by @ref initialiseTriggers() )
 * - Perform cyl. count based initialisations (@ref config2.nCylinders)
 * - Perform injection and spark mode based setup
 *   - Assign injector open/close and coil charge begin/end functions to their dedicated global vars
 * - Perform fuel pressure priming by turning fuel pump on
 * - Read CLT and TPS sensors to have cranking pulsewidths computed correctly
 * - Mark Initialisation completed (this flag-marking is used in code to prevent after-init changes)
 */
void initialiseAll(void)
{   
    currentStatus.fpPrimed = false;
    currentStatus.injPrimed = false;

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    #if defined(CORE_STM32)
    configPage9.intcan_available = 1;   // device has internal canbus
    //STM32 can not currently enabled
    #endif

    // Unit tests should be independent of any stored configuration on the board!
#if !defined(UNIT_TEST)
    setStorageAPI(getBoardStorageApi());
    processResetStorageRequest();
    loadAllPages();
    loadAllCalibrationTables(); 
    doUpdates(); //Check if any data items need updating (Occurs with firmware updates)
#endif

    //Always start with a clean slate on the bootloader capabilities level
    //This should be 0 until we hear otherwise from the 16u2
    configPage4.bootloaderCaps = 0;
    
    initBoard(115200); //This calls the current individual boards init function. See the board_xxx.ino files for these.
    initialiseTimers();
    
  #ifdef SD_LOGGING
    initRTC();
    if(configPage13.onboard_log_file_style) { initSD(); }
  #endif

    pPrimarySerial = &Serial; //Default to standard Serial interface
    currentStatus.allowLegacyComms = true; //Flag legacy comms as being allowed on startup

    //Set the pin mappings
    if((configPage2.pinMapping == 255) || (configPage2.pinMapping == 0)) //255 = EEPROM value in a blank AVR; 0 = EEPROM value in new FRAM
    {
      //First time running on this board
      setTuneToEmpty();
      configPage4.triggerTeeth = 4; //Avoiddiv by 0 when start decoders
      setPinMapping(3); //Force board to v0.4
    }
    else { setPinMapping(configPage2.pinMapping); }

    // Repeatedly initialising the CAN bus hangs the system when
    // running initialisation tests on Teensy 3.5
    #if defined(NATIVE_CAN_AVAILABLE) && !defined(UNIT_TEST)
      initCAN();
    #endif

    //Must come after setPinMapping() as secondary serial can be changed on a per board basis
    if (configPage9.enable_secondarySerial == 1) { secondarySerial.begin(115200); }

    //End all coil charges to ensure no stray sparks on startup
    stopAllCoilsCharging();

    //Similar for injectors, make sure they're turned off
    closeAllInjectors();
    
    //Set the tacho output default state
    digitalWrite(pinNumbers.pinTachOut, HIGH);
    //Perform all initialisations
    initialiseFuelSchedulers();
    initialiseIgnitionSchedules(configPage4.sparkMode, configPage2.nCylinders, configPage10.rotaryType);
    //initialiseDisplay();
    initialiseIdle(true);
    initialiseFan(pinNumbers.pinFan);
    initialiseAirCon();
    initialiseAuxPWM();
    initialiseCorrections();
    currentStatus.ioError = false; //Clear the I/O error bit. The bit will be set in initialiseADC() if there is problem in there.
    initialiseADC();
    initialiseMAPBaro();
    initialiseProgrammableIO(currentStatus, configPage13);
    initialiseFlexSensor(configPage2, currentStatus, pinNumbers.sensors.flex);

    //Same as above, but for the VSS input
    if (isExternalVssMode(configPage2)) // VSS modes 2 and 3 are interrupt drive (Mode 1 is CAN)
    {
      if(!pinIsReserved(pinNumbers.pinVSS)) { attachInterrupt(digitalPinToInterrupt(pinNumbers.pinVSS), vssPulse, RISING); }
    }
    //As above but for knock pulses
    if(configPage10.knock_mode == KNOCK_MODE_DIGITAL)
    {
      if(configPage10.knock_pullup) { pinMode(configPage10.knock_pin, INPUT_PULLUP); }
      else { pinMode(configPage10.knock_pin, INPUT); }

      if(!pinIsReserved(configPage10.knock_pin)) 
      { 
        if(configPage10.knock_trigger == KNOCK_TRIGGER_HIGH) { attachInterrupt(digitalPinToInterrupt(configPage10.knock_pin), knockPulse, RISING); }
        else { attachInterrupt(digitalPinToInterrupt(configPage10.knock_pin), knockPulse, FALLING); }
      }
    }

    //Once the configs have been loaded, a number of one time calculations can be completed

    if (configPage4.trigPatternSec == SEC_TRIGGER_POLL && configPage4.TrigPattern == DECODER_MISSING_TOOTH)
    { configPage4.TrigEdgeSec = configPage4.PollLevelPolarity; } // set the secondary trigger edge automatically to correct working value with poll level mode to enable cam angle detection in closed loop vvt.
    //Explanation: currently cam trigger for VVT is only captured when revolution one == 1. So we need to make sure that the edge trigger happens on the first revolution. So now when we set the poll level to be low
    //on revolution one and it's checked at tooth #1. This means that the cam signal needs to go high during the first revolution to be high on next revolution at tooth #1. So poll level low = cam trigger edge rising.

    //Begin the main crank trigger interrupt pin setup
    //The interrupt numbering is a bit odd - See here for reference: arduino.cc/en/Reference/AttachInterrupt
    //These assignments are based on the Arduino Mega AND VARY BETWEEN BOARDS. Please confirm the board you are using and update accordingly.
    currentStatus.setRpm(0U);
    currentStatus.runSecs = 0;
    currentStatus.secl = 0;
    //currentStatus.seclx10 = 0;
    currentStatus.startRevolutions = 0;
    currentStatus.syncLossCounter = 0;
    currentStatus.flatShiftingHard = false;
    currentStatus.launchingHard = false;
    currentStatus.crankRPM = ((unsigned int)configPage4.crankRPM * 10); //Crank RPM limit (Saves us calculating this over and over again. It's updated once per second in timers.ino)
    currentStatus.fuelPumpOn = false;
    currentStatus.engineProtect.reset();
    fpPrimeTime = 0;
    ms_counter = 0;
    fixedCrankingOverride = 0;
    toothHistoryIndex = 0;
    
    noInterrupts();
    currentStatus.decoder = buildDecoder(configPage4.TrigPattern);
    boardInitPins(pinNumbers);
    // The schedulers are all configured & pins are mapped - so start the schedulers
    startIgnitionSchedulers();
    startFuelSchedulers();
    
    //The secondary input can be used for VSS if nothing else requires it. Allows for the standard VR conditioner to be used for VSS. This MUST be run after the initialiseTriggers() function
    if( VSS_USES_RPM2() ) { attachInterrupt(digitalPinToInterrupt(pinNumbers.pinVSS), vssPulse, RISING); } //Secondary trigger input can safely be used for VSS
    if( FLEX_USES_RPM2() ) { attachInterrupt(digitalPinToInterrupt(pinNumbers.sensors.flex), flexPulse, CHANGE); } //Secondary trigger input can safely be used for Flex sensor

    //Initial values for loop times
    currentLoopTime = micros();
    mainLoopCount = 0;

    if(configPage2.divider == 0) { currentStatus.nSquirts = 2; } //Safety check.
    else { currentStatus.nSquirts = configPage2.nCylinders / configPage2.divider; } //The number of squirts being requested. This is manually overridden below for sequential setups (Due to TS req_fuel calc limitations)
    if(currentStatus.nSquirts == 0) { currentStatus.nSquirts = 1; } //Safety check. Should never happen as TS will give an error, but leave in case tune is manually altered etc. 

    //Calculate the number of degrees between cylinders
    //Set some default values. These will be updated below if required.
    CRANK_ANGLE_MAX_IGN = 360;
    CRANK_ANGLE_MAX_INJ = 360;

    currentStatus.maxInjOutputs = 1; // Disable all injectors expect channel 1

    if(configPage2.strokes == FOUR_STROKE) { CRANK_ANGLE_MAX_INJ = 720 / currentStatus.nSquirts; }
    else { CRANK_ANGLE_MAX_INJ = 360 / currentStatus.nSquirts; }

    switch (configPage2.nCylinders) {
    case 1:
        ignitionSchedule1.channelDegrees = 0;
        channel1InjDegrees = 0;
        currentStatus.maxIgnOutputs= 1;
        currentStatus.maxInjOutputs = 1;

        //Sequential ignition works identically on a 1 cylinder whether it's odd or even fire. 
        if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) ) { CRANK_ANGLE_MAX_IGN = 720; }

        if ( (configPage2.injLayout == INJ_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) )
        {
          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
        }

        //Check if injector staging is enabled
        if(configPage10.stagingEnabled == true)
        {
          currentStatus.maxInjOutputs = 2;
          channel2InjDegrees = channel1InjDegrees;
        }
        break;

    case 2:
        ignitionSchedule1.channelDegrees = 0;
        channel1InjDegrees = 0;
        currentStatus.maxIgnOutputs= 2;
        currentStatus.maxInjOutputs = 2;
        if (configPage2.engineType == EVEN_FIRE ) { ignitionSchedule2.channelDegrees = 180; }
        else { ignitionSchedule2.channelDegrees = configPage2.oddfire2; }

        //Sequential ignition works identically on a 2 cylinder whether it's odd or even fire (With the default being a 180 degree second cylinder).
        if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) ) { CRANK_ANGLE_MAX_IGN = 720; }

        if ( (configPage2.injLayout == INJ_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) )
        {
          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
        }
        //The below are true regardless of whether this is running sequential or not
        if (configPage2.engineType == EVEN_FIRE ) { channel2InjDegrees = 180; }
        else { channel2InjDegrees = configPage2.oddfire2; }
        if (!configPage2.injTiming) 
        { 
          //For simultaneous, all squirts happen at the same time
          channel1InjDegrees = 0;
          channel2InjDegrees = 0; 
        }

        //Check if injector staging is enabled
        if(configPage10.stagingEnabled == true)
        {
          currentStatus.maxInjOutputs = 4;

          channel3InjDegrees = channel1InjDegrees;
          channel4InjDegrees = channel2InjDegrees;
        }

        break;

    case 3:
        ignitionSchedule1.channelDegrees = 0;
        currentStatus.maxIgnOutputs= 3;
        currentStatus.maxInjOutputs = 3;
        if (configPage2.engineType == EVEN_FIRE )
        {
          //Sequential and Single channel modes both run over 720 crank degrees, but only on 4 stroke engines.
          if( ( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) || (configPage4.sparkMode == IGN_MODE_SINGLE) ) && (configPage2.strokes == FOUR_STROKE) )
          {
            ignitionSchedule2.channelDegrees = 240;
            ignitionSchedule3.channelDegrees = 480;

            CRANK_ANGLE_MAX_IGN = 720;
          }
          else
          {
            ignitionSchedule2.channelDegrees = 120;
            ignitionSchedule3.channelDegrees = 240;
          }
        }
        else
        {
          ignitionSchedule2.channelDegrees = configPage2.oddfire2;
          ignitionSchedule3.channelDegrees = configPage2.oddfire3;
        }

        //For alternating injection, the squirt occurs at different times for each channel
        if( (configPage2.injLayout == INJ_SEMISEQUENTIAL) || (configPage2.injLayout == INJ_PAIRED) )
        {
          channel1InjDegrees = 0;
          channel2InjDegrees = 120;
          channel3InjDegrees = 240;

          if(configPage2.injType == INJ_TYPE_PORT)
          { 
            //Force nSquirts to 2 for individual port injection. This prevents TunerStudio forcing the value to 3 even when this isn't wanted. 
            currentStatus.nSquirts = 2;
            if(configPage2.strokes == FOUR_STROKE) { CRANK_ANGLE_MAX_INJ = 360; }
            else { CRANK_ANGLE_MAX_INJ = 180; }
          }
          
          //Adjust the injection angles based on the number of squirts
          if (currentStatus.nSquirts > 2)
          {
            channel2InjDegrees = (channel2InjDegrees * 2) / currentStatus.nSquirts;
            channel3InjDegrees = (channel3InjDegrees * 2) / currentStatus.nSquirts;
          }

          if (!configPage2.injTiming) 
          { 
            //For simultaneous, all squirts happen at the same time
            channel1InjDegrees = 0;
            channel2InjDegrees = 0;
            channel3InjDegrees = 0; 
          } 
        }
        else if (configPage2.injLayout == INJ_SEQUENTIAL)
        {
          currentStatus.nSquirts = 1;

          if(configPage2.strokes == TWO_STROKE)
          {
            channel1InjDegrees = 0;
            channel2InjDegrees = 120;
            channel3InjDegrees = 240;
            CRANK_ANGLE_MAX_INJ = 360;
          }
          else
          {
            channel1InjDegrees = 0;
            channel2InjDegrees = 240;
            channel3InjDegrees = 480;
            CRANK_ANGLE_MAX_INJ = 720;
          }
        }
        else
        {
          //Should never happen, but default values
          channel1InjDegrees = 0;
          channel2InjDegrees = 120;
          channel3InjDegrees = 240;
        }

        //Check if injector staging is enabled
        if(configPage10.stagingEnabled == true)
        {
          #if INJ_CHANNELS >= 6
            currentStatus.maxInjOutputs = 6;

            channel4InjDegrees = channel1InjDegrees;
            channel5InjDegrees = channel2InjDegrees;
            channel6InjDegrees = channel3InjDegrees;
          #else
            //Staged output is on channel 4
            currentStatus.maxInjOutputs = 4;
            channel4InjDegrees = channel1InjDegrees;
          #endif
        }
        break;
    case 4:
        ignitionSchedule1.channelDegrees = 0;
        channel1InjDegrees = 0;
        currentStatus.maxIgnOutputs= 2; //Default value for 4 cylinder, may be changed below
        currentStatus.maxInjOutputs = 2;
        if (configPage2.engineType == EVEN_FIRE )
        {
          ignitionSchedule2.channelDegrees = 180;

          if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) )
          {
            ignitionSchedule3.channelDegrees = 360;
            ignitionSchedule4.channelDegrees = 540;

            CRANK_ANGLE_MAX_IGN = 720;
            currentStatus.maxIgnOutputs= 4;
          }
          if(configPage4.sparkMode == IGN_MODE_ROTARY)
          {
            //Rotary uses the ign 3 and 4 schedules for the trailing spark. They are offset from the ign 1 and 2 channels respectively and so use the same degrees as them
            ignitionSchedule3.channelDegrees = 0;
            ignitionSchedule4.channelDegrees = 180;
            currentStatus.maxIgnOutputs= 4;

            configPage4.IgInv = GOING_LOW; //Force Going Low ignition mode (Going high is never used for rotary)
          }
        }
        else
        {
          ignitionSchedule2.channelDegrees = configPage2.oddfire2;
          ignitionSchedule3.channelDegrees = configPage2.oddfire3;
          ignitionSchedule4.channelDegrees = configPage2.oddfire4;
          currentStatus.maxIgnOutputs= 4;
        }

        //For alternating injection, the squirt occurs at different times for each channel
        if( (configPage2.injLayout == INJ_SEMISEQUENTIAL) || (configPage2.injLayout == INJ_PAIRED) || (configPage2.strokes == TWO_STROKE) )
        {
          channel2InjDegrees = 180;

          if (!configPage2.injTiming) 
          { 
            //For simultaneous, all squirts happen at the same time
            channel1InjDegrees = 0;
            channel2InjDegrees = 0; 
          }
          else if (currentStatus.nSquirts > 2)
          {
            //Adjust the injection angles based on the number of squirts
            channel2InjDegrees = (channel2InjDegrees * 2) / currentStatus.nSquirts;
          }
          else { } //Do nothing, default values are correct
        }
        else if (configPage2.injLayout == INJ_SEQUENTIAL)
        {
          channel2InjDegrees = 180;
          channel3InjDegrees = 360;
          channel4InjDegrees = 540;

          currentStatus.maxInjOutputs = 4;

          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
        }
        else
        {
          //Should never happen, but default values
          currentStatus.maxInjOutputs = 2;
        }

        //Check if injector staging is enabled
        if(configPage10.stagingEnabled == true)
        {
          currentStatus.maxInjOutputs = 4;

          if( (configPage2.injLayout == INJ_SEQUENTIAL) || (configPage2.injLayout == INJ_SEMISEQUENTIAL) )
          {
            //Staging with 4 cylinders semi/sequential requires 8 total channels
            #if INJ_CHANNELS >= 8
              currentStatus.maxInjOutputs = 8;

              channel5InjDegrees = channel1InjDegrees;
              channel6InjDegrees = channel2InjDegrees;
              channel7InjDegrees = channel3InjDegrees;
              channel8InjDegrees = channel4InjDegrees;
            #else
              //This is an invalid config as there are not enough outputs to support sequential + staging
              //Put the staging output to the non-existent channel 5
              #if (INJ_CHANNELS >= 5)
              currentStatus.maxInjOutputs = 5;
              channel5InjDegrees = channel1InjDegrees;
              #endif
            #endif
          }
          else
          {
            channel3InjDegrees = channel1InjDegrees;
            channel4InjDegrees = channel2InjDegrees;
          }
        }

        break;
    case 5:
        ignitionSchedule1.channelDegrees = 0;
        ignitionSchedule2.channelDegrees = 72;
        ignitionSchedule3.channelDegrees = 144;
        ignitionSchedule4.channelDegrees = 216;
#if (IGN_CHANNELS >= 5)
        ignitionSchedule5.channelDegrees = 288;
#endif
        currentStatus.maxIgnOutputs= 5; //Only 4 actual outputs, so that's all that can be cut
        currentStatus.maxInjOutputs = 4; //Is updated below to 5 if there are enough channels

        if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
        {
          ignitionSchedule2.channelDegrees = 144;
          ignitionSchedule3.channelDegrees = 288;
          ignitionSchedule4.channelDegrees = 432;
#if (IGN_CHANNELS >= 5)
          ignitionSchedule5.channelDegrees = 576;
#endif

          CRANK_ANGLE_MAX_IGN = 720;
        }

        //For alternating injection, the squirt occurs at different times for each channel
        if( (configPage2.injLayout == INJ_SEMISEQUENTIAL) || (configPage2.injLayout == INJ_PAIRED) || (configPage2.strokes == TWO_STROKE) )
        {
          if (!configPage2.injTiming) 
          { 
            //For simultaneous, all squirts happen at the same time
            channel1InjDegrees = 0;
            channel2InjDegrees = 0;
            channel3InjDegrees = 0;
            channel4InjDegrees = 0;
#if (INJ_CHANNELS >= 5)
            channel5InjDegrees = 0; 
#endif
          }
          else
          {
            channel1InjDegrees = 0;
            channel2InjDegrees = 72;
            channel3InjDegrees = 144;
            channel4InjDegrees = 216;
#if (INJ_CHANNELS >= 5)
            channel5InjDegrees = 288;
#endif

            //Divide by currentStatus.nSquirts ?
          }
        }
    #if INJ_CHANNELS >= 5
        else if (configPage2.injLayout == INJ_SEQUENTIAL)
        {
          channel1InjDegrees = 0;
          channel2InjDegrees = 144;
          channel3InjDegrees = 288;
          channel4InjDegrees = 432;
          channel5InjDegrees = 576;

          currentStatus.maxInjOutputs = 5;

          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
        }
    #endif

    #if INJ_CHANNELS >= 6
          if(configPage10.stagingEnabled == true) { currentStatus.maxInjOutputs = 6; }
    #endif
        break;
    case 6:
        ignitionSchedule1.channelDegrees = 0;
        ignitionSchedule2.channelDegrees = 120;
        ignitionSchedule3.channelDegrees = 240;
        currentStatus.maxIgnOutputs= 3;
        currentStatus.maxInjOutputs = 3;

    #if IGN_CHANNELS >= 6
        if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL))
        {
        ignitionSchedule4.channelDegrees = 360;
        ignitionSchedule5.channelDegrees = 480;
        ignitionSchedule6.channelDegrees = 600;
        CRANK_ANGLE_MAX_IGN = 720;
        currentStatus.maxIgnOutputs= 6;
        }
    #endif

        //For alternating injection, the squirt occurs at different times for each channel
        if( (configPage2.injLayout == INJ_SEMISEQUENTIAL) || (configPage2.injLayout == INJ_PAIRED) )
        {
          channel1InjDegrees = 0;
          channel2InjDegrees = 120;
          channel3InjDegrees = 240;
          if (!configPage2.injTiming)
          {
            //For simultaneous, all squirts happen at the same time
            channel1InjDegrees = 0;
            channel2InjDegrees = 0;
            channel3InjDegrees = 0;
          }
          else if (currentStatus.nSquirts > 2)
          {
            //Adjust the injection angles based on the number of squirts
            channel2InjDegrees = (channel2InjDegrees * 2) / currentStatus.nSquirts;
            channel3InjDegrees = (channel3InjDegrees * 2) / currentStatus.nSquirts;
          }
        }

    #if INJ_CHANNELS >= 6
        if (configPage2.injLayout == INJ_SEQUENTIAL)
        {
          channel1InjDegrees = 0;
          channel2InjDegrees = 120;
          channel3InjDegrees = 240;
          channel4InjDegrees = 360;
          channel5InjDegrees = 480;
          channel6InjDegrees = 600;

          currentStatus.maxInjOutputs = 6;

          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
        }
        else if(configPage10.stagingEnabled == true) //Check if injector staging is enabled
        {
          currentStatus.maxInjOutputs = 6;

          if( (configPage2.injLayout == INJ_SEQUENTIAL) || (configPage2.injLayout == INJ_SEMISEQUENTIAL) )
          {
            //Staging with 6 cylinders semi/sequential requires 7 total channels
            #if INJ_CHANNELS >= 7
              currentStatus.maxInjOutputs = 7;

              channel5InjDegrees = channel1InjDegrees;
              channel6InjDegrees = channel2InjDegrees;
              channel7InjDegrees = channel3InjDegrees;
              channel8InjDegrees = channel4InjDegrees;
            #else
              //This is an invalid config as there are not enough outputs to support sequential + staging
              //No staging output will be active
              currentStatus.maxInjOutputs = 6;
            #endif
          }
        }
    #endif
        break;
    case 8:
        ignitionSchedule1.channelDegrees = 0;
        ignitionSchedule2.channelDegrees = 90;
        ignitionSchedule3.channelDegrees = 180;
        ignitionSchedule4.channelDegrees = 270;
        currentStatus.maxIgnOutputs= 4;
        currentStatus.maxInjOutputs = 4;


        if( (configPage4.sparkMode == IGN_MODE_SINGLE))
        {
          currentStatus.maxIgnOutputs= 4;
          CRANK_ANGLE_MAX_IGN = 360;
        }
    

    #if IGN_CHANNELS >= 8
        if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL))
        {
        ignitionSchedule5.channelDegrees = 360;
        ignitionSchedule6.channelDegrees = 450;
        ignitionSchedule7.channelDegrees = 540;
        ignitionSchedule8.channelDegrees = 630;
        currentStatus.maxIgnOutputs= 8;
        CRANK_ANGLE_MAX_IGN = 720;
        }
    #endif

        //For alternating injection, the squirt occurs at different times for each channel
        if( (configPage2.injLayout == INJ_SEMISEQUENTIAL) || (configPage2.injLayout == INJ_PAIRED) )
        {
          channel1InjDegrees = 0;
          channel2InjDegrees = 90;
          channel3InjDegrees = 180;
          channel4InjDegrees = 270;

          if (!configPage2.injTiming)
          {
            //For simultaneous, all squirts happen at the same time
            channel1InjDegrees = 0;
            channel2InjDegrees = 0;
            channel3InjDegrees = 0;
            channel4InjDegrees = 0;
          }
          else if (currentStatus.nSquirts > 2)
          {
            //Adjust the injection angles based on the number of squirts
            channel2InjDegrees = (channel2InjDegrees * 2) / currentStatus.nSquirts;
            channel3InjDegrees = (channel3InjDegrees * 2) / currentStatus.nSquirts;
            channel4InjDegrees = (channel4InjDegrees * 2) / currentStatus.nSquirts;
          }
        }

    #if INJ_CHANNELS >= 8
        else if (configPage2.injLayout == INJ_SEQUENTIAL)
        {
          channel1InjDegrees = 0;
          channel2InjDegrees = 90;
          channel3InjDegrees = 180;
          channel4InjDegrees = 270;
          channel5InjDegrees = 360;
          channel6InjDegrees = 450;
          channel7InjDegrees = 540;
          channel8InjDegrees = 630;

          currentStatus.maxInjOutputs = 8;

          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
        }
    #endif

        break;
    default: //Handle this better!!!
        channel1InjDegrees = 0;
        channel2InjDegrees = 180;
        break;
    }

    currentStatus.nSquirtsStatus = currentStatus.nSquirts; //Top 3 bits of the status3 variable are the number of squirts. This must be done after the above section due to nSquirts being forced to 1 for sequential
    
    //Special case:
    //3 or 5 squirts per cycle MUST be tracked over 720 degrees. This is because the angles for them (Eg 720/3=240) are not evenly divisible into 360
    //This is ONLY the case on 4 stroke systems
    if( (currentStatus.nSquirts == 3) || (currentStatus.nSquirts == 5) )
    {
      if(configPage2.strokes == FOUR_STROKE) { CRANK_ANGLE_MAX_INJ = (720U / currentStatus.nSquirts); }
    }
    
    switch(configPage2.injLayout)
    {
    case INJ_PAIRED:
        //Paired injection
        setCallbacks(fuelSchedule1, openInjector1, closeInjector1);
        setCallbacks(fuelSchedule2, openInjector2, closeInjector2);
        setCallbacks(fuelSchedule3, openInjector3, closeInjector3);
        setCallbacks(fuelSchedule4, openInjector4, closeInjector4);
#if INJ_CHANNELS >= 5
        setCallbacks(fuelSchedule5, openInjector5, closeInjector5);
#endif
        break;

    case INJ_SEMISEQUENTIAL:
        //Semi-Sequential injection. Currently possible with 4, 6 and 8 cylinders. 5 cylinder is a special case
        if( configPage2.nCylinders == 4 )
        {
          if(configPage4.inj4cylPairing == INJ_PAIR_13_24)
          {
            setCallbacks(fuelSchedule1, openInjector1and3, closeInjector1and3);
            setCallbacks(fuelSchedule2, openInjector2and4, closeInjector2and4);
          }
          else
          {
            setCallbacks(fuelSchedule1, openInjector1and4, closeInjector1and4);
            setCallbacks(fuelSchedule2, openInjector2and3, closeInjector2and3);
          }
        }
        else if( configPage2.nCylinders == 5 ) //This is similar to the paired injection but uses five injector outputs instead of four
        {
          setCallbacks(fuelSchedule1, openInjector1, closeInjector1);
          setCallbacks(fuelSchedule2, openInjector2, closeInjector2);
          setCallbacks(fuelSchedule3, openInjector3and5, closeInjector3and5);
          setCallbacks(fuelSchedule4, openInjector4, closeInjector4);
        }
        else if( configPage2.nCylinders == 6 )
        {
          setCallbacks(fuelSchedule1, openInjector1and4, closeInjector1and4);
          setCallbacks(fuelSchedule2, openInjector2and5, closeInjector2and5);
          setCallbacks(fuelSchedule3, openInjector3and6, closeInjector3and6);
        }
        else if( configPage2.nCylinders == 8 )
        {
          setCallbacks(fuelSchedule1, openInjector1and5, closeInjector1and5);
          setCallbacks(fuelSchedule2, openInjector2and6, closeInjector2and6);
          setCallbacks(fuelSchedule3, openInjector3and7, closeInjector3and7);
          setCallbacks(fuelSchedule4, openInjector4and8, closeInjector4and8);
        }
        else
        {
          //Fall back to paired injection
          setCallbacks(fuelSchedule1, openInjector1, closeInjector1);
          setCallbacks(fuelSchedule2, openInjector2, closeInjector2);
          setCallbacks(fuelSchedule3, openInjector3, closeInjector3);
          setCallbacks(fuelSchedule4, openInjector4, closeInjector4);
#if INJ_CHANNELS >= 5
          setCallbacks(fuelSchedule5, openInjector5, closeInjector5);
#endif
        }
        break;

    case INJ_SEQUENTIAL:
        //Sequential injection
        setCallbacks(fuelSchedule1, openInjector1, closeInjector1);
        setCallbacks(fuelSchedule2, openInjector2, closeInjector2);
        setCallbacks(fuelSchedule3, openInjector3, closeInjector3);
        setCallbacks(fuelSchedule4, openInjector4, closeInjector4);
#if INJ_CHANNELS >= 5
        setCallbacks(fuelSchedule5, openInjector5, closeInjector5);
#endif
#if INJ_CHANNELS >= 6
        setCallbacks(fuelSchedule6, openInjector6, closeInjector6);
#endif
#if INJ_CHANNELS >= 7
        setCallbacks(fuelSchedule7, openInjector7, closeInjector7);
#endif
#if INJ_CHANNELS >= 8
        setCallbacks(fuelSchedule8, openInjector8, closeInjector8);
#endif
        break;

    default:
        //Paired injection
        setCallbacks(fuelSchedule1, openInjector1, closeInjector1);
        setCallbacks(fuelSchedule2, openInjector2, closeInjector2);
        setCallbacks(fuelSchedule3, openInjector3, closeInjector3);
        setCallbacks(fuelSchedule4, openInjector4, closeInjector4);
#if INJ_CHANNELS >= 5
        setCallbacks(fuelSchedule5, openInjector5, closeInjector5);
#endif
        break;
    }
    
    currentStatus.fpPrimed = initialiseFuelPump(configPage2, pinNumbers.pinFuelPump);

    interrupts();
    initialiseCLT();
    initialiseTPS();

    /* tacho sweep function. */
    currentStatus.tachoSweepEnabled = (configPage2.useTachoSweep > 0);
    /* SweepMax is stored as a byte, RPM/100. divide by 60 to convert min to sec (net 5/3).  Multiply by ignition pulses per rev.
       tachoSweepIncr is also the number of tach pulses per second */
    tachoSweepIncr = configPage2.tachoSweepMaxRPM * currentStatus.maxIgnOutputs * 5 / 3;
   
    currentStatus.initialisationComplete = true;
    digitalWrite(LED_BUILTIN, HIGH);

}

static pinNumbers_t getV02PinMapping(void)
{
  pinNumbers_t pins;
#ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
  //Pin mappings as per the v0.2 shield
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 8, 9, 10, 11, 12, };
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 28, 24, 40, 36, 34, };
  pins.triggerPins.primary = 20; //The CAS pin
  pins.triggerPins.secondary = 21; //The Cam Sensor pin
  pins.triggerPins.tertiary = 3; //The Cam sensor 2 pin
  pins.sensors.TPS = A2; //TPS input pin
  pins.sensors.MAP = A3; //MAP sensor pin
  pins.sensors.IAT = A0; //IAT sensor pin
  pins.sensors.CLT = A1; //CLS sensor pin
  pins.sensors.O2 = A8; //O2 Sensor pin
  pins.sensors.Bat = A4; //Battery reference voltage pin
  pins.pinTachOut = 49; //Tacho output pin
  pins.idle.idle1 = 30; //Single wire idle control
  pins.idle.idle2 = 31; //2 wire idle control
  pins.idle.stepperDir = 16; //Direction pin  for DRV8825 driver
  pins.idle.stepperStep = 17; //Step pin for DRV8825 driver
  pins.pinFan = 47; //Pin for the fan output
  pins.pinFuelPump = 4; //Fuel pump output
  pins.sensors.flex = 2; // Flex sensor (Must be external interrupt enabled)
  pins.pinResetControl = 43; //Reset control output

  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));
  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
#endif

  return pins;
}

static pinNumbers_t getV03PinMapping(void)
{
  pinNumbers_t pins;

#ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
  //Pin mappings as per the v0.3 shield
  pins.triggerPins.primary = 19; //The CAS pin
  pins.triggerPins.secondary = 18; //The Cam Sensor pin
  pins.triggerPins.tertiary = 3; //The Cam sensor 2 pin
  pins.sensors.TPS = A2;//TPS input pin
  pins.sensors.MAP = A3; //MAP sensor pin
  pins.sensors.IAT = A0; //IAT sensor pin
  pins.sensors.CLT = A1; //CLS sensor pin
  pins.sensors.O2 = A8; //O2 Sensor pin
  pins.sensors.Bat = A4; //Battery reference voltage pin
  pins.pinTachOut = 49; //Tacho output pin
  pins.idle.idle1 = 5; //Single wire idle control
  pins.idle.idle2 = 53; //2 wire idle control
  pins.pinBoost = 7; //Boost control
  pins.pinVVT_1 = 6; //Default VVT output
  pins.pinVVT_2 = 48; //Default VVT2 output
  pins.pinFuelPump = 4; //Fuel pump output
  pins.idle.stepperDir = 16; //Direction pin  for DRV8825 driver
  pins.idle.stepperStep = 17; //Step pin for DRV8825 driver
  pins.idle.stepperEnable = 26; //Enable pin for DRV8825
  pins.pinFan = A13; //Pin for the fan output
  pins.pinLaunch = 51; //Can be overwritten below
  pins.sensors.flex = 2; // Flex sensor (Must be external interrupt enabled)
  pins.pinResetControl = 50; //Reset control output
  pins.sensors.baro = A5;
  pins.pinVSS = 20;

#if defined(CORE_TEENSY35)
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 31, 24, 30, 21, 34 /* Pin for coil 5 PLACEHOLDER value for now */, };
  pins.triggerPins.primary = 23;
  pins.idle.stepperDir = 33;
  pins.idle.stepperStep = 34;
  pins.pinTachOut = 28;
  pins.pinFan = 27;
  pins.sensors.O2 = A22;
#else
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 28, 24, 40, 36, 34 /* Pin for coil 5 PLACEHOLDER value for now */, };
#endif
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 8, 9, 10, 11, 12 };
  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
#endif

  return pins;
}

static pinNumbers_t getV04PinMapping(void)
{
  pinNumbers_t pins;

  //Pin mappings as per the v0.4 shield
  pins.triggerPins.primary = 19; //The CAS pin
  pins.triggerPins.secondary = 18; //The Cam Sensor pin
  pins.triggerPins.tertiary = 3; //The Cam sensor 2 pin
  pins.sensors.TPS = A2;//TPS input pin
  pins.sensors.MAP = A3; //MAP sensor pin
  pins.sensors.IAT = A0; //IAT sensor pin
  pins.sensors.CLT = A1; //CLS sensor pin
  pins.sensors.O2 = A8; //O2 Sensor pin
  pins.sensors.Bat = A4; //Battery reference voltage pin
  pins.pinTachOut = 49; //Tacho output pin  (Goes to ULN2803)
  pins.idle.idle1 = 5; //Single wire idle control
  pins.idle.idle2 = 6; //2 wire idle control
  pins.pinBoost = 7; //Boost control
  pins.pinVVT_1 = 4; //Default VVT output
  pins.pinVVT_2 = 48; //Default VVT2 output
  pins.pinFuelPump = 45; //Fuel pump output  (Goes to ULN2803)
  pins.idle.stepperDir = 16; //Direction pin  for DRV8825 driver
  pins.idle.stepperStep = 17; //Step pin for DRV8825 driver
  pins.idle.stepperEnable = 24; //Enable pin for DRV8825
  pins.pinFan = 47; //Pin for the fan output (Goes to ULN2803)
  pins.pinLaunch = 51; //Can be overwritten below
  pins.sensors.flex = 2; // Flex sensor (Must be external interrupt enabled)
  pins.pinResetControl = 43; //Reset control output
  pins.sensors.baro = A5;
  pins.pinVSS = 20;
  pins.wmi.empty = 46;
  pins.wmi.indicator = 44;
  pins.wmi.enabled = 42;

#if defined(CORE_TEENSY35)
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 8, 9, 10, 11, 12, 50 /* CAUTION: Uses the same as Coil 4 below. */, 51};
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 31, 32, 30, 29 };
  pins.triggerPins.primary = 23;
  pins.triggerPins.secondary = 36;
  pins.idle.stepperDir = 34;
  pins.idle.stepperStep = 35;
  pins.pinTachOut = 28;
  pins.pinFan = 27;
  pins.sensors.O2 = A22;

  //Make sure the CAN pins aren't overwritten
  pins.triggerPins.tertiary = 54;
  pins.pinVVT_1 = 55;

#elif defined(CORE_TEENSY41)
  //These are only to prevent lockups or weird behaviour on T4.1 when this board is used as the default
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 8, 9, 10, 11, 12, 50 /* CAUTION: Uses the same as Coil 4 below. */};
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 31, 32, 30, 29 };
  pins.sensors.baro = A4; 
  pins.sensors.MAP = A5;
  pins.sensors.TPS = A3; //TPS input pin
  pins.sensors.IAT = A0; //IAT sensor pin
  pins.sensors.CLT = A1; //CLS sensor pin
  pins.sensors.O2 = A2; //O2 Sensor pin
  pins.sensors.Bat = A15; //Battery reference voltage pin. Needs Alpha4+
  pins.pinLaunch = 34; //Can be overwritten below
  pins.pinVSS = 35;

  pins.triggerPins.primary = 20; //The CAS pin
  pins.triggerPins.secondary = 21; //The Cam Sensor pin
  pins.triggerPins.tertiary = 24;

  pins.idle.stepperDir = 34;
  pins.idle.stepperStep = 35;
  
  pins.pinTachOut = 28;
  pins.pinFan = 27;
  pins.pinFuelPump = 33;
  pins.wmi.empty = 34;
  pins.wmi.indicator = 35;
  pins.wmi.enabled = 36;
#elif defined(STM32F407xx)
//Pin definitions for experimental board Tjeerd 
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { PD12, PD13, PD14, PD15, PE11, PE12, };
  static constexpr uint8_t boardCoilPins[] PROGMEM = { PD7, PB9, PA8, PB10, PD9 };
  //Black F407VE wiki.stm32duino.com/index.php?title=STM32F407

  //******************************************
  //******** PORTA CONNECTIONS *************** 
  //******************************************
  /* = PA0 */ //Wakeup ADC123
  // = PA1;
  // = PA2;
  // = PA3;
  // = PA4;
  /* = PA5; */ //ADC12
  /* = PA6; */ //ADC12 LED_BUILTIN_1
  pins.pinFuelPump = PA7; //ADC12 LED_BUILTIN_2
  /* = PA9 */ //TXD1
  /* = PA10 */ //RXD1
  /* = PA11 */ //(DO NOT USE FOR SPEEDUINO) USB
  /* = PA12 */ //(DO NOT USE FOR SPEEDUINO) USB 
  /* = PA13 */ //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK
  /* = PA14 */ //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK
  /* = PA15 */ //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK

  //******************************************
  //******** PORTB CONNECTIONS *************** 
  //******************************************
  /* = PB0; */ //(DO NOT USE FOR SPEEDUINO) ADC123 - SPI FLASH CHIP CS pin
  pins.sensors.baro = PB1; //ADC12
  /* = PB2; */ //(DO NOT USE FOR SPEEDUINO) BOOT1 
  /* = PB3; */ //(DO NOT USE FOR SPEEDUINO) SPI1_SCK FLASH CHIP
  /* = PB4; */ //(DO NOT USE FOR SPEEDUINO) SPI1_MISO FLASH CHIP
  /* = PB5; */ //(DO NOT USE FOR SPEEDUINO) SPI1_MOSI FLASH CHIP
  /* = PB6; */ //NRF_CE
  /* = PB7; */ //NRF_CS
  /* = PB8; */ //NRF_IRQ
  /* = PB9; */ //
  pins.idle.idle1 = PB11; //RXD3
  pins.idle.idle2 = PB12; //
  pins.pinBoost = PB12; //
  /* = PB13; */ //SPI2_SCK
  /* = PB14; */ //SPI2_MISO
  /* = PB15; */ //SPI2_MOSI

  //******************************************
  //******** PORTC CONNECTIONS *************** 
  //******************************************
  pins.sensors.MAP = PC0; //ADC123 
  pins.sensors.TPS = PC1; //ADC123
  pins.sensors.IAT = PC2; //ADC123
  pins.sensors.CLT = PC3; //ADC123
  pins.sensors.O2 = PC4;  //ADC12
  pins.sensors.Bat = PC5; //ADC12
  pins.pinVVT_1 = PC6; //
  /* = PC8; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D0
  /* = PC9; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D1
  /* = PC10; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D2
  /* = PC11; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D3
  /* = PC12; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_SCK
  pins.pinTachOut = PC13; //
  /* = PC14; */ //(DO NOT USE FOR SPEEDUINO) - OSC32_IN
  /* = PC15; */ //(DO NOT USE FOR SPEEDUINO) - OSC32_OUT

  //******************************************
  //******** PORTD CONNECTIONS *************** 
  //******************************************
  /* = PD0; */ //CANRX
  /* = PD1; */ //CANTX
  /* = PD2; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_CMD
  pins.pinVVT_2 = PD3; //
  pins.sensors.flex = PD4;
  /* = PD5;*/ //TXD2
  /* = PD6; */ //RXD2
  /* = PD8; */ //
  /* = PD10; */ //
  /* = PD11; */ //

  //******************************************
  //******** PORTE CONNECTIONS *************** 
  //******************************************
  pins.triggerPins.primary = PE0; //
  pins.triggerPins.secondary = PE1; //
  pins.idle.stepperEnable = PE2; //
  /* = PE3; */ //ONBOARD KEY1
  /* = PE4; */ //ONBOARD KEY2
  pins.idle.stepperStep = PE5; //
  pins.pinFan = PE6; //
  pins.idle.stepperDir = PE7; //
  /* = PE8; */ //
  /* = PE9; */ //
  /* = PE10; */ //
  /* = PE13; */ //
  /* = PE14; */ //
  /* = PE15; */ //

#elif defined(CORE_STM32)
  //https://github.com/stm32duino/Arduino_Core_STM32/blob/master/variants/Generic_F411Cx/variant.h#L28
  //pins PA12, PA11 are used for USB or CAN couldn't be used for GPIO
  //pins PB12, PB13, PB14 and PB15 are used to SPI FLASH
  //PB2 can't be used as input because it's the BOOT pin
  static constexpr uint8_t boardCoilPins[] PROGMEM = { PB9, PB8, PB3, PA15 };
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { PB7, PB6, PB5, PB4, };
  pins.sensors.TPS = A2;//TPS input pin
  pins.sensors.MAP = A3; //MAP sensor pin
  pins.sensors.IAT = A0; //IAT sensor pin
  pins.sensors.CLT = A1; //CLS sensor pin
  pins.sensors.O2 = A8; //O2 Sensor pin
  pins.sensors.Bat = A4; //Battery reference voltage pin
  pins.sensors.baro = pins.sensors.MAP;
  pins.pinTachOut = PB1; //Tacho output pin  (Goes to ULN2803)
  pins.idle.idle1 = PB2; //Single wire idle control
  pins.idle.idle2 = PB10; //2 wire idle control
  pins.pinBoost = PA6; //Boost control
  pins.idle.stepperDir = PB10; //Direction pin  for DRV8825 driver
  pins.idle.stepperStep = PB2; //Step pin for DRV8825 driver
  pins.pinFuelPump = PA8; //Fuel pump output
  pins.pinFan = PA5; //Pin for the fan output (Goes to ULN2803)
  //external interrupt enabled pins
  pins.sensors.flex = PC14; // Flex sensor (Must be external interrupt enabled)
  pins.triggerPins.primary = PC13; //The CAS pin also led pin so bad idea
  pins.triggerPins.secondary = PC15; //The Cam Sensor pin
#else
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 40, 38, 52, 50, 34 /* Pin for coil 5 PLACEHOLDER value for now */, };
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 8, 9, 10, 11, 12, 50 /* CAUTION: Uses the same as Coil 4 below. */};
#endif
  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));

  return pins;
}

static pinNumbers_t getMiataNB2Mapping(void)
{
  pinNumbers_t pins;
#ifndef SMALL_FLASH_MODE
  //Pin mappings as per the 2001-05 MX5 PNP shield
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 44, 46, 47, 45, 14, };
  pins.triggerPins.primary = 19; //The CAS pin
  pins.triggerPins.secondary = 18; //The Cam Sensor pin
  pins.triggerPins.tertiary = 2; //The Cam sensor 2 pin
  pins.sensors.TPS = A2;//TPS input pin
  pins.sensors.MAP = A5; //MAP sensor pin
  pins.sensors.IAT = A0; //IAT sensor pin
  pins.sensors.CLT = A1; //CLS sensor pin
  pins.sensors.O2 = A3; //O2 Sensor pin
  pins.sensors.Bat = A4; //Battery reference voltage pin
  pins.pinTachOut = 23; //Tacho output pin  (Goes to ULN2803)
  pins.idle.idle1 = 5; //Single wire idle control
  pins.pinBoost = 4;
  pins.pinVVT_1 = 11; //Default VVT output
  pins.pinVVT_2 = 48; //Default VVT2 output
  pins.idle.idle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
  pins.pinFuelPump = 40; //Fuel pump output
  pins.idle.stepperDir = 16; //Direction pin  for DRV8825 driver
  pins.idle.stepperStep = 17; //Step pin for DRV8825 driver
  pins.idle.stepperEnable = 24;
  pins.pinFan = 41; //Pin for the fan output
  pins.pinLaunch = 12; //Can be overwritten below
  pins.sensors.flex = 3; // Flex sensor (Must be external interrupt enabled)
  pins.pinResetControl = 39; //Reset control output
  pins.pinVSS = 2;

  //This is NOT correct. It has not yet been tested with this board
#if defined(CORE_TEENSY35)
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 33, 24, 51, 52 };        
  pins.triggerPins.primary = 23;
  pins.triggerPins.secondary = 36;
  pins.idle.stepperDir = 34;
  pins.idle.stepperStep = 35;
  pins.pinFuelPump = 26; //Requires PVT4 adapter or above
  pins.pinFan = 50; //Won't work (No mapping for pin 35)
  pins.pinTachOut = 28; //Done
#else
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 42, 43, 32, 33, 34, };
#endif

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));
#endif

  return pins;
}

static pinNumbers_t getMiataNA18PinMapping(void)
{
  pinNumbers_t pins;
#ifndef SMALL_FLASH_MODE
  //Pin mappings as per the 1996-97 MX5 PNP shield
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 11, 10, 9,       8, 14, };
  pins.triggerPins.primary = 19; //The CAS pin
  pins.triggerPins.secondary = 18; //The Cam Sensor pin
  pins.sensors.TPS = A2;//TPS input pin
  pins.sensors.MAP = A5; //MAP sensor pin
  pins.sensors.IAT = A0; //IAT sensor pin
  pins.sensors.CLT = A1; //CLS sensor pin
  pins.sensors.O2 = A3; //O2 Sensor pin
  pins.sensors.Bat = A4; //Battery reference voltage pin
  pins.pinTachOut = A9; //Tacho output pin  (Goes to ULN2803)
  pins.idle.idle1 = 2; //Single wire idle control
  pins.pinBoost = 4;
  pins.idle.idle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
  pins.pinFuelPump = 49; //Fuel pump output
  pins.idle.stepperDir = 16; //Direction pin  for DRV8825 driver
  pins.idle.stepperStep = 17; //Step pin for DRV8825 driver
  pins.idle.stepperEnable = 24;
  pins.pinFan = 35; //Pin for the fan output
  pins.pinLaunch = 37; //Can be overwritten below
  pins.sensors.flex = 3; // Flex sensor (Must be external interrupt enabled)
  pins.pinResetControl = 44; //Reset control output

  //This is NOT correct. It has not yet been tested with this board
#if defined(CORE_TEENSY35)
  pins.triggerPins.primary = 23;
  pins.triggerPins.secondary = 36;
  pins.idle.stepperDir = 34;
  pins.idle.stepperStep = 35;
  pins.pinFuelPump = 26; //Requires PVT4 adapter or above
  pins.pinFan = 50; //Won't work (No mapping for pin 35)
  pins.pinTachOut = 28; //Done
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 33, 24, 51, 52 };        
#else
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 39, 41, 32, 33, 34, };
#endif

  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));
  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
#endif

  return pins;
}

static pinNumbers_t getMiataNA16PinMapping(void)
{
  pinNumbers_t pins;
#ifndef SMALL_FLASH_MODE
  //Pin mappings as per the 89-95 MX5 PNP shield
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 11, 10, 9,       8, 14, };
  pins.triggerPins.primary = 19; //The CAS pin
  pins.triggerPins.secondary = 18; //The Cam Sensor pin
  pins.sensors.TPS = A2;//TPS input pin
  pins.sensors.MAP = A5; //MAP sensor pin
  pins.sensors.IAT = A0; //IAT sensor pin
  pins.sensors.CLT = A1; //CLS sensor pin
  pins.sensors.O2 = A3; //O2 Sensor pin
  pins.sensors.Bat = A4; //Battery reference voltage pin
  pins.pinTachOut = 49; //Tacho output pin  (Goes to ULN2803)
  pins.idle.idle1 = 2; //Single wire idle control
  pins.pinBoost = 4;
  pins.idle.idle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
  pins.pinFuelPump = 37; //Fuel pump output
  //Note that there is no stepper driver output on the PNP boards. These pins are unconnected and remain here just to prevent issues with random pin numbers occurring
  pins.idle.stepperEnable = 15; //Enable pin for the DRV8825
  pins.idle.stepperDir = 16; //Direction pin  for DRV8825 driver
  pins.idle.stepperStep = 17; //Step pin for DRV8825 driver
  pins.pinFan = 35; //Pin for the fan output
  pins.pinLaunch = 12; //Can be overwritten below
  pins.sensors.flex = 3; // Flex sensor (Must be external interrupt enabled)
  pins.pinResetControl = 44; //Reset control output
  pins.pinVSS = 20;
  pins.idle.idleUp = 48;
  pins.sensors.CTPS = 47;

#if defined(CORE_TEENSY35)
  pins.triggerPins.primary = 23;
  pins.triggerPins.secondary = 36;
  pins.idle.stepperDir = 34;
  pins.idle.stepperStep = 35;
  pins.pinFuelPump = 26; //Requires PVT4 adapter or above
  pins.pinFan = 50; //Won't work (No mapping for pin 35)
  pins.pinTachOut = 28; //Done
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 33, 24, 51, 52 };        
#else
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 39, 41, 32, 33, 34, };
#endif

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));
#endif

  return pins;
}

static pinNumbers_t getTurtanasPinMapping(void)
{
  pinNumbers_t pins;

#ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
  //Pin mappings for user turtanas PCB
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 4, 5, 6, 7, 8, 9, 10, 11, };
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 24, 28, 36, 40, 34, };
  pins.triggerPins.primary = 18; //The CAS pin
  pins.triggerPins.secondary = 19; //The Cam Sensor pin
  pins.sensors.TPS = A2;//TPS input pin
  pins.sensors.MAP = A3; //MAP sensor pin
  pins.sensors.IAT = A0; //IAT sensor pin
  pins.sensors.CLT = A1; //CLS sensor pin
  pins.sensors.O2 = A4; //O2 Sensor pin
  pins.sensors.Bat = A7; //Battery reference voltage pin
  pins.pinTachOut = 41; //Tacho output pin transistor is missing 2n2222 for this and 1k for 12v
  pins.pinFuelPump = 42; //Fuel pump output 2n2222
  pins.pinFan = 47; //Pin for the fan output
  pins.pinTachOut = 49; //Tacho output pin
  pins.sensors.flex = 2; // Flex sensor (Must be external interrupt enabled)
  pins.pinResetControl = 26; //Reset control output

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));
#endif

  return pins;
}

static pinNumbers_t getLevinPinMapping(void)
{
  pinNumbers_t pins;

// Pin mappings for the Levin board
#if defined(STM32F407xx)
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { PB15, PA8, PB13, PB14, PE13, PB12, PE7, PE10, };
  static constexpr uint8_t boardCoilPins[] PROGMEM = { PC13, PE6, PE5, PE4, PE3, PE2, PB9, PD12, };
  pins.triggerPins.primary = PD3;        // The CAS pin
  pins.triggerPins.secondary = PD4;       // The Cam Sensor pin
  pins.sensors.TPS = PA2;            // TPS input pin
  pins.sensors.MAP = PA3;            // MAP sensor pin
  pins.sensors.EMAP = PC5;           // EMAP sensor pin (placeholder)
  pins.sensors.IAT = PA0;            // IAT sensor pin
  pins.sensors.CLT = PA1;            // CLS sensor pin
  pins.sensors.O2 = PB0;             // O2 Sensor pin
  pins.sensors.Bat = PA4;            // Battery reference voltage pin
  pins.sensors.baro = PA5;           // Baro sensor pin
  pins.pinTachOut = PE8;        // Tacho output pin  (Goes to UNL2803)
  pins.idle.idle1 = PD10;         // ICV pin1  (Goes to UNL2803)
  pins.idle.idle2 = PD9;          // ICV pin3  (Goes to UNL2803)
  pins.pinBoost = PD8;          // Boost control
  pins.pinVVT_1 = PD11;         // VVT1 output (intake vanos)
  pins.pinVVT_2 = PC6;          // VVT2 output (exhaust vanos)
  pins.pinFuelPump = PE11;      // Fuel pump output  (Goes to UNL2803)
  pins.idle.stepperDir = PB10;    // Stepper valve isn't used with these
  pins.idle.stepperStep = PB11;   // Stepper valve isn't used with these
  pins.idle.stepperEnable = PA15; // Stepper valve isn't used with these
  pins.pinFan = PE9;            // Pin for the fan output (Goes to UNL2803)
  pins.pinLaunch = PB8;         // Launch control pin
  pins.sensors.flex = PD7;           // Flex sensor
  pins.pinResetControl = PB7;   // Reset control output
  pins.pinVSS = PB6;            // VSS input pin
  pins.wmi.empty = PA6;       //(placeholder)
  pins.wmi.indicator = PC3;   //(placeholder)
  pins.wmi.enabled = PE15;    //(placeholder)
  pins.idle.idleUp = PC7;         //(placeholder)

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));
#endif

  return pins;
}

static pinNumbers_t getPlazomatv10PinMapping(void)
{
  pinNumbers_t pins;

#if defined(CORE_AVR) && !defined(SMALL_FLASH_MODE) //No support for bluepill here anyway
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 8, 9, 10, 11, 12, };
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 28, 24, 40, 36, 34, };
  pins.triggerPins.primary = 20; //The CAS pin
  pins.triggerPins.secondary = 21; //The Cam Sensor pin
  pins.sensors.O2 = A8; //O2 Sensor pin
  pins.sensors.Bat = A4; //Battery reference voltage pin
  pins.sensors.MAP = A3; //MAP sensor pin
  pins.sensors.TPS = A2;//TPS input pin
  pins.sensors.CLT = A1; //CLS sensor pin
  pins.sensors.IAT = A0; //IAT sensor pin
  pins.pinFan = 47; //Pin for the fan output
  pins.pinFuelPump = 4; //Fuel pump output
  pins.pinTachOut = 49; //Tacho output pin
  pins.pinResetControl = 26; //Reset control output

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));
#endif

  return pins;
}

static pinNumbers_t getDazV6PinMapping(void)
{
  pinNumbers_t pins;

#ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
  //Pin mappings as per the dazv6 shield
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 8, 9, 10, 11, 12, };
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 40, 38, 50, 52, 34, };
  pins.triggerPins.primary = 19; //The CAS pin
  pins.triggerPins.secondary = 18; //The Cam Sensor pin
  pins.triggerPins.tertiary = 17; // cam sensor 2 pin, pin17 isn't external trigger enabled in arduino mega??
  pins.sensors.TPS = A2;//TPS input pin
  pins.sensors.MAP = A3; //MAP sensor pin
  pins.sensors.IAT = A0; //IAT sensor pin
  pins.sensors.CLT = A1; //CLS sensor pin
  pins.sensors.O2 = A8; //O2 Sensor pin
  pins.sensors.O2_2 = A9; //O2 sensor pin (second sensor)
  pins.sensors.Bat = A4; //Battery reference voltage pin
  pins.pinTachOut = 49; //Tacho output pin
  pins.idle.idle1 = 5; //Single wire idle control
  pins.pinFuelPump = 45; //Fuel pump output
  pins.idle.stepperDir = 20; //Direction pin  for DRV8825 driver
  pins.idle.stepperStep = 21; //Step pin for DRV8825 driver
  pins.pinBoost = 7;
  pins.pinFan = 47; //Pin for the fan output

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));
#endif

  return pins;
}

static pinNumbers_t getBMWPnPPinMapping(void)
{
  pinNumbers_t pins;

  //Pin mappings for the BMW PnP PCBs by pazi88.
#if defined(CORE_AVR)
  //This is the regular MEGA2560 pin mapping
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 8, 9, 10, 11, 12, 50, 39, 42, };
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 40, 38, 52, 48, 36, 34, 46, 53, };
  pins.triggerPins.primary = 19; //The CAS pin
  pins.triggerPins.secondary = 18; //The Cam Sensor pin
  pins.triggerPins.tertiary = 20; //The Cam sensor 2 pin
  pins.sensors.TPS = A2;//TPS input pin
  pins.sensors.MAP = A3; //MAP sensor pin
  pins.sensors.EMAP = A15; //EMAP sensor pin
  pins.sensors.IAT = A0; //IAT sensor pin
  pins.sensors.CLT = A1; //CLT sensor pin
  pins.sensors.O2 = A8; //O2 Sensor pin
  pins.sensors.O2_2 = A12; //O2 Sensor pin
  pins.sensors.Bat = A4; //Battery reference voltage pin
  pins.sensors.baro = A5; //Baro sensor pin
  pins.pinTachOut = 49; //Tacho output pin  (Goes to ULN2003)
  pins.idle.idle1 = 5; //ICV pin1
  pins.idle.idle2 = 6; //ICV pin3
  pins.pinBoost = 7; //Boost control
  pins.pinVVT_1 = 4; //VVT1 output (intake vanos)
  pins.pinVVT_2 = 26; //VVT2 output (exhaust vanos)
  pins.pinFuelPump = 45; //Fuel pump output  (Goes to ULN2003)
  pins.idle.stepperDir = 16; //Stepper valve isn't used with these
  pins.idle.stepperStep = 17; //Stepper valve isn't used with these
  pins.idle.stepperEnable = 24; //Stepper valve isn't used with these
  pins.pinFan = 47; //Pin for the fan output (Goes to ULN2003)
  pins.pinLaunch = 51; //Launch control pin
  pins.sensors.flex = 2; // Flex sensor
  pins.pinResetControl = 43; //Reset control output
  pins.pinVSS = 3; //VSS input pin
  pins.wmi.empty = 31; //(placeholder)
  pins.wmi.indicator = 33; //(placeholder)
  pins.wmi.enabled = 35; //(placeholder)
  pins.idle.idleUp = 37; //(placeholder)
  pins.idle.idleUpOutput = 41; //(placeholder)
  pins.sensors.CTPS = A6; //(placeholder)

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));
#elif defined(STM32F407xx)
  static constexpr uint8_t boardInjectorPins[] PROGMEM = {  PB15, PB14, PB12, PB13, PA8, PE7, PE13, PE10, };
  static constexpr uint8_t boardCoilPins[] PROGMEM = { PE2, PE3, PC13, PE6, PE4, PE5, PE0, PB9, };
  pins.triggerPins.primary = PD3; //The CAS pin
  pins.triggerPins.secondary = PD4; //The Cam Sensor pin
  pins.sensors.TPS = PA2;//TPS input pin
  pins.sensors.MAP = PA3; //MAP sensor pin
  pins.sensors.EMAP = PC5; //EMAP sensor pin
  pins.sensors.IAT = PA0; //IAT sensor pin
  pins.sensors.CLT = PA1; //CLS sensor pin
  pins.sensors.O2 = PB0; //O2 Sensor pin
  pins.sensors.O2_2 = PC2; //O2 Sensor pin
  pins.sensors.Bat = PA4; //Battery reference voltage pin
  pins.sensors.baro = PA5; //Baro sensor pin
  pins.pinTachOut = PE8; //Tacho output pin  (Goes to ULN2003)
  pins.idle.idle1 = PD10; //ICV pin1
  pins.idle.idle2 = PD9; //ICV pin3
  pins.pinBoost = PD8; //Boost control
  pins.pinVVT_1 = PD11; //VVT1 output (intake vanos)
  pins.pinVVT_2 = PC7; //VVT2 output (exhaust vanos)
  pins.pinFuelPump = PE11; //Fuel pump output  (Goes to ULN2003)
  pins.idle.stepperDir = PB10; //Stepper valve isn't used with these
  pins.idle.stepperStep = PB11; //Stepper valve isn't used with these
  pins.idle.stepperEnable = PA15; //Stepper valve isn't used with these
  pins.pinFan = PE9; //Pin for the fan output (Goes to ULN2003)
  pins.pinLaunch = PB8; //Launch control pin
  pins.sensors.flex = PD7; // Flex sensor
  pins.pinResetControl = PB7; //Reset control output
  pins.pinVSS = PB6; //VSS input pin
  pins.wmi.empty = PD15; //(placeholder)
  pins.wmi.indicator = PD13; //(placeholder)
  pins.wmi.enabled = PE15; //(placeholder)
  pins.idle.idleUp = PE14; //(placeholder)
  pins.idle.idleUpOutput = PE12; //(placeholder)
  pins.sensors.CTPS = PA6; //(placeholder)
  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));
#endif

  return pins;
}

static pinNumbers_t getNO2CPinMapping(void)
{
  pinNumbers_t pins;

#ifndef SMALL_FLASH_MODE
  //Pin mappings as per the NO2C shield
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 8, 9, 11, 12, 13, };
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 23, 22, 2, 3, 46, };
  pins.triggerPins.primary = 19; //The CAS pin
  pins.triggerPins.secondary = 18; //The Cam Sensor pin
  pins.triggerPins.tertiary = 21; //The Cam sensor 2 pin
  pins.sensors.TPS = A3; //TPS input pin
  pins.sensors.MAP = A0; //MAP sensor pin
  pins.sensors.IAT = A5; //IAT sensor pin
  pins.sensors.CLT = A4; //CLT sensor pin
  pins.sensors.O2 = A2; //O2 sensor pin
  pins.sensors.Bat = A1; //Battery reference voltage pin
  pins.sensors.baro = A6; //Baro sensor pin - ONLY WITH DB
  pins.pinTachOut = 38; //Tacho output pin
  pins.idle.idle1 = 5; //Single wire idle control
  pins.idle.idle2 = 47; //2 wire idle control - NOT USED
  pins.pinBoost = 7; //Boost control
  pins.pinVVT_1 = 6; //Default VVT output
  pins.pinVVT_2 = 48; //Default VVT2 output
  pins.pinFuelPump = 4; //Fuel pump output
  pins.idle.stepperDir = 25; //Direction pin for DRV8825 driver
  pins.idle.stepperStep = 24; //Step pin for DRV8825 driver
  pins.idle.stepperEnable = 27; //Enable pin for DRV8825 driver
  pins.pinLaunch = 10; //Can be overwritten below
  pins.sensors.flex = 20; // Flex sensor (Must be external interrupt enabled) - ONLY WITH DB
  pins.pinFan = 30; //Pin for the fan output - ONLY WITH DB
  pins.pinResetControl = 26; //Reset control output

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));

#endif

  return pins;
}

static pinNumbers_t getUA4CPinMapping(void)
{
  pinNumbers_t pins;

#ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
  //Pin mappings as per the UA4C shield
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 8, 7, 6, 5, 45, };
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 35, 36, 33, 34, 44, };
  pins.triggerPins.primary = 19; //The CAS pin
  pins.triggerPins.secondary = 18; //The Cam Sensor pin
  pins.triggerPins.tertiary = 3; //The Cam sensor 2 pin
  pins.sensors.flex = 20; // Flex sensor
  pins.sensors.TPS = A3; //TPS input pin
  pins.sensors.MAP = A0; //MAP sensor pin
  pins.sensors.baro = A7; //Baro sensor pin
  pins.sensors.IAT = A5; //IAT sensor pin
  pins.sensors.CLT = A4; //CLS sensor pin
  pins.sensors.O2 = A1; //O2 Sensor pin
  pins.sensors.O2_2 = A9; //O2 sensor pin (second sensor)
  pins.sensors.Bat = A2; //Battery reference voltage pin
  pins.pinLaunch = 37; //Can be overwritten below
  pins.pinTachOut = 22; //Tacho output pin
  pins.idle.idle1 = 9; //Single wire idle control
  pins.idle.idle2 = 10; //2 wire idle control
  pins.pinFuelPump = 23; //Fuel pump output
  pins.pinVVT_1 = 11; //Default VVT output
  pins.pinVVT_2 = 48; //Default VVT2 output
  pins.idle.stepperDir = 32; //Direction pin  for DRV8825 driver
  pins.idle.stepperStep = 31; //Step pin for DRV8825 driver
  pins.idle.stepperEnable = 30; //Enable pin for DRV8825 driver
  pins.pinBoost = 12; //Boost control
  pins.pinFan = 24; //Pin for the fan output
  pins.pinResetControl = 46; //Reset control output PLACEHOLDER value for now
  pins.pinVSS = 2;

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));
#endif

  return pins;
}

static pinNumbers_t getBlitzboxBL49spPinMapping(void)
{
  pinNumbers_t pins;

  //Pin mappings for all BlitzboxBL49sp variants
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 6, 7, 8, 9, };
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 24, 25, 23, 22, };
  pins.triggerPins.primary = 19; //The CRANK Sensor pin
  pins.triggerPins.secondary = 18; //The Cam Sensor pin
  pins.sensors.flex = 20; // Flex sensor PLACEHOLDER value for now
  pins.sensors.TPS = A0; //TPS input pin
  pins.sensors.O2 = A2; //O2 Sensor pin
  pins.sensors.IAT = A3; //IAT sensor pin
  pins.sensors.CLT = A4; //CLT sensor pin
  pins.sensors.MAP = A7; //internal MAP sensor
  pins.sensors.Bat = A6; //Battery reference voltage pin
  pins.sensors.baro = A5; //external MAP/Baro sensor pin
  pins.sensors.O2_2 = A9; //O2 sensor pin (second sensor) PLACEHOLDER value for now
  pins.pinLaunch = 2; //Can be overwritten below
  pins.pinTachOut = 10; //Tacho output pin
  pins.idle.idle1 = 11; //Single wire idle control
  pins.idle.idle2 = 14; //2 wire idle control PLACEHOLDER value for now
  pins.pinFuelPump = 3; //Fuel pump output
  pins.pinVVT_1 = 15; //Default VVT output PLACEHOLDER value for now
  pins.pinBoost = 5; //Boost control
  pins.pinFan = 12; //Pin for the fan output
  pins.pinResetControl = 46; //Reset control output PLACEHOLDER value for now

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));

  return pins;
}

static pinNumbers_t getDIYEFICORE4v10PinMapping(void)
{
  pinNumbers_t pins;

#ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
  //Pin mappings for the DIY-EFI CORE4 Module. This is an AVR only module
#if defined(CORE_AVR)
  static constexpr uint8_t boardInjectorPins[] PROGMEM = {  10, 11, 12, 9, 33, 34};
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 39, 29, 28, 27, 26 /* Pin for coil 5 PLACEHOLDER value for now */, };
  pins.triggerPins.primary = 19; //The CAS pin
  pins.triggerPins.secondary = 18; //The Cam Sensor pin
  pins.triggerPins.tertiary = 21;// The Cam sensor 2 pin
  pins.sensors.flex = 20; // Flex sensor
  pins.sensors.TPS = A3; //TPS input pin
  pins.sensors.MAP = A2; //MAP sensor pin
  pins.sensors.baro = A15; //Baro sensor pin
  pins.sensors.IAT = A11; //IAT sensor pin
  pins.sensors.CLT = A4; //CLS sensor pin
  pins.sensors.O2 = A12; //O2 Sensor pin
  pins.sensors.O2_2 = A5; //O2 sensor pin (second sensor)
  pins.sensors.Bat = A1; //Battery reference voltage pin
  pins.pinLaunch = 24; //Can be overwritten below
  pins.pinTachOut = 38; //Tacho output pin
  pins.idle.idle1 = 42; //Single wire idle control
  pins.idle.idle2 = 43; //2 wire idle control
  pins.pinFuelPump = 41; //Fuel pump output
  pins.pinVVT_1 = 44; //Default VVT output
  pins.pinVVT_2 = 48; //Default VVT2 output
  pins.idle.stepperDir = 32; //Direction pin  for DRV8825 driver
  pins.idle.stepperStep = 31; //Step pin for DRV8825 driver
  pins.idle.stepperEnable = 30; //Enable pin for DRV8825 driver
  pins.pinBoost = 45; //Boost control
  pins.pinFan = 40; //Pin for the fan output
  pins.pinResetControl = 46; //Reset control output PLACEHOLDER value for now

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));
#endif
#endif

  return pins;
}

#if defined(CORE_TEENSY35)
static pinNumbers_t getDvjTeensyRevAPinMapping(void)
{
  pinNumbers_t pins;
  //Pin mappings as per the teensy rev A shield
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 2, 10, 6, 9, };
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 29, 30, 31, 32, };
  pins.triggerPins.primary = 23; //The CAS pin
  pins.triggerPins.secondary = 36; //The Cam Sensor pin
  pins.sensors.TPS = 16; //TPS input pin
  pins.sensors.MAP = 17; //MAP sensor pin
  pins.sensors.IAT = 14; //IAT sensor pin
  pins.sensors.CLT = 15; //CLT sensor pin
  pins.sensors.O2 = A22; //O2 sensor pin
  pins.sensors.O2_2 = A21; //O2 sensor pin (second sensor)
  pins.sensors.Bat = 18; //Battery reference voltage pin
  pins.pinTachOut = 20; //Tacho output pin
  pins.idle.idle1 = 5; //Single wire idle control
  pins.pinBoost = 11; //Boost control
  pins.pinFuelPump = 38; //Fuel pump output
  pins.idle.stepperDir = 34; //Direction pin for DRV8825 driver
  pins.idle.stepperStep = 35; //Step pin for DRV8825 driver
  pins.idle.stepperEnable = 33; //Enable pin for DRV8825 driver
  pins.pinLaunch = 26; //Can be overwritten below
  pins.pinFan = 37; //Pin for the fan output - ONLY WITH DB

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));

  return pins;
}
#endif

#if defined(CORE_TEENSY35)
static pinNumbers_t getDvjTeensyRevBPinMapping(void)
{
  pinNumbers_t pins;

  //Pin mappings as per the teensy revB board shield
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 2, 10, 6, 9, };
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 29, 30, 31, 32, };
  pins.triggerPins.primary = 23; //The CAS pin
  pins.triggerPins.secondary = 36; //The Cam Sensor pin
  pins.sensors.TPS = 16; //TPS input pin
  pins.sensors.MAP = 17; //MAP sensor pin
  pins.sensors.IAT = 14; //IAT sensor pin
  pins.sensors.CLT = 15; //CLT sensor pin
  pins.sensors.O2 = A22; //O2 sensor pin
  pins.sensors.O2_2 = A21; //O2 sensor pin (second sensor)
  pins.sensors.Bat = 18; //Battery reference voltage pin
  pins.pinTachOut = 20; //Tacho output pin
  pins.idle.idle1 = 5; //Single wire idle control
  pins.pinBoost = 11; //Boost control
  pins.pinFuelPump = 38; //Fuel pump output
  pins.idle.stepperDir = 34; //Direction pin for DRV8825 driver
  pins.idle.stepperStep = 35; //Step pin for DRV8825 driver
  pins.idle.stepperEnable = 33; //Enable pin for DRV8825 driver
  pins.pinLaunch = 26; //Can be overwritten below
  pins.pinFan = 37; //Pin for the fan output - ONLY WITH DB

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));

  return pins;
}
#endif

#if defined(CORE_TEENSY35)
static pinNumbers_t getJuiceBoxPinMapping(void)
{
  pinNumbers_t pins;

  //Pin mappings for the Juice Box (ignition only board)
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 2, 56, 6, 50, };
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 29, 30, 31, 32, };
  pins.triggerPins.primary = 37; //The CAS pin
  pins.triggerPins.secondary = 38; //The Cam Sensor pin - NOT USED
  pins.sensors.TPS = A2; //TPS input pin
  pins.sensors.MAP = A7; //MAP sensor pin
  pins.sensors.IAT = A1; //IAT sensor pin
  pins.sensors.CLT = A5; //CLT sensor pin
  pins.sensors.O2 = A0; //O2 sensor pin
  pins.sensors.O2_2 = A21; //O2 sensor pin (second sensor) - NOT USED
  pins.sensors.Bat = A6; //Battery reference voltage pin
  pins.pinTachOut = 28; //Tacho output pin
  pins.idle.idle1 = 5; //Single wire idle control - NOT USED
  pins.pinBoost = 11; //Boost control - NOT USED
  pins.pinFuelPump = 24; //Fuel pump output
  pins.idle.stepperDir = 3; //Direction pin for DRV8825 driver - NOT USED
  pins.idle.stepperStep = 4; //Step pin for DRV8825 driver - NOT USED
  pins.idle.stepperEnable = 6; //Enable pin for DRV8825 driver - NOT USED
  pins.pinLaunch = 26; //Can be overwritten below
  pins.pinFan = 25; //Pin for the fan output

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));

  return pins;
}
#endif

#if defined(CORE_TEENSY)
static pinNumbers_t getDropBearPinMapping(void)
{
  pinNumbers_t pins;

  //Pin mappings for the DropBear
  // injectorOutputControl = OUTPUT_CONTROL_MC33810;
  // ignitionOutputControl = OUTPUT_CONTROL_MC33810;

  //The injector pins below are not used directly as the control is via SPI through the MC33810s, however the pin numbers are set to be the SPI pins (SCLK, MOSI, MISO and CS) so that nothing else will set them as inputs
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 13, 11, 12, 10, 9, 9, };
  //Dummy pins, without these pin 0 (Serial1 RX) gets overwritten
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 40, 41, };
  pins.triggerPins.primary = 19; //The CAS pin
  pins.triggerPins.secondary = 18; //The Cam Sensor pin
  pins.triggerPins.tertiary = 22; //Uses one of the protected spare digital inputs. This must be set or Serial1 (Pin 0) gets broken
  pins.sensors.flex = A16; // Flex sensor
  pins.sensors.MAP = A1; //MAP sensor pin
  pins.sensors.baro = A0; //Baro sensor pin
  pins.sensors.Bat = A14; //Battery reference voltage pin
  pins.pinLaunch = A15; //Can be overwritten below
  pins.pinTachOut = 5; //Tacho output pin
  pins.idle.idle1 = 27; //Single wire idle control
  pins.idle.idle2 = 29; //2 wire idle control. Shared with Spare 1 output
  pins.pinFuelPump = 8; //Fuel pump output
  pins.pinVVT_1 = 28; //Default VVT output
  pins.idle.stepperDir = 32; //Direction pin  for DRV8825 driver
  pins.idle.stepperStep = 31; //Step pin for DRV8825 driver
  pins.idle.stepperEnable = 30; //Enable pin for DRV8825 driver
  pins.pinBoost = 24; //Boost control
  pins.pinFan = 25; //Pin for the fan output
  pins.pinResetControl = 46; //Reset control output PLACEHOLDER value for now
  pins.pinVSS = 22;

  pins.wmi.empty = 23; //Spare digital input
  pins.wmi.indicator = 26; //Spare output
  pins.wmi.enabled = 29; //Spare output

  #if defined(CORE_TEENSY35)
    pins.sensors.TPS = A22; //TPS input pin
    pins.sensors.IAT = A19; //IAT sensor pin
    pins.sensors.CLT = A20; //CLS sensor pin
    pins.sensors.O2 = A21; //O2 Sensor pin
    pins.sensors.O2_2 = A18; //Spare 2

    pSecondarySerial = &Serial1; //Header that is broken out on Dropbear boards is attached to Serial1
  #elif defined(CORE_TEENSY41)
    //New pins for the actual T4.1 version of the Dropbear
    pins.sensors.baro = A4; 
    pins.sensors.MAP = A5;
    pins.sensors.TPS = A3; //TPS input pin
    pins.sensors.IAT = A0; //IAT sensor pin
    pins.sensors.CLT = A1; //CLS sensor pin
    pins.sensors.O2 = A2; //O2 Sensor pin
    pins.sensors.Bat = A15; //Battery reference voltage pin. Needs Alpha4+
    pins.pinLaunch = 36;
    pins.sensors.flex = 37; // Flex sensor

    pins.triggerPins.primary = 20; //The CAS pin
    pins.triggerPins.secondary = 21; //The Cam Sensor pin
    pins.triggerPins.tertiary = 34; //Uses one of the protected spare digital inputs.

    pins.pinFuelPump = 5; //Fuel pump output
    pins.pinTachOut = 0; //Tacho output pin

    pins.pinResetControl = 49; //PLaceholder only. Cannot use 42-47 as these are the SD card
    pins.wmi.empty = 35; //Spare digital input
    pins.pinVSS = 34;
  #endif

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));

  //Pin alignment to the MC33810 outputs
  pins.mc33810.CS_1 = 10;
  pins.mc33810.CS_2 = 9;

  pins.mc33810.injBits[0] = 3;
  pins.mc33810.injBits[1] = 1;
  pins.mc33810.injBits[2] = 0;
  pins.mc33810.injBits[3] = 2;
  pins.mc33810.ignBits[0] = 4;
  pins.mc33810.ignBits[1] = 5;
  pins.mc33810.ignBits[2] = 6;
  pins.mc33810.ignBits[3] = 7;

  pins.mc33810.injBits[4] = 3;
  pins.mc33810.injBits[5] = 1;
  pins.mc33810.injBits[6] = 0;
  pins.mc33810.injBits[7] = 2;
  pins.mc33810.ignBits[4] = 4;
  pins.mc33810.ignBits[5] = 5;
  pins.mc33810.ignBits[6] = 6;
  pins.mc33810.ignBits[7] = 7;
  return pins;
}
#endif

#if defined(CORE_TEENSY41)
static pinNumbers_t getBearCubPinMapping(void)
{
  pinNumbers_t pins;

  //Pin mappings for the Bear Cub (Teensy 4.1)
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { 6, 7, 9, 8, 0, };
  static constexpr uint8_t boardCoilPins[] PROGMEM = { 2, 3, 4, 5, };
  pins.triggerPins.primary = 20; //The CAS pin
  pins.triggerPins.secondary = 21; //The Cam Sensor pin
  pins.sensors.flex = 37; // Flex sensor
  pins.sensors.MAP = A5; //MAP sensor pin
  pins.sensors.baro = A4; //Baro sensor pin
  pins.sensors.Bat = A15; //Battery reference voltage pin
  pins.sensors.TPS = A3; //TPS input pin
  pins.sensors.IAT = A0; //IAT sensor pin
  pins.sensors.CLT = A1; //CLS sensor pin
  pins.sensors.O2 = A2; //O2 Sensor pin
  pins.pinLaunch = 36;

  pins.pinTachOut = 38; //Tacho output pin
  pins.idle.idle1 = 27; //Single wire idle control
  pins.idle.idle2 = 26; //2 wire idle control. Shared with Spare 1 output
  pins.pinFuelPump = 10; //Fuel pump output
  pins.pinVVT_1 = 28; //Default VVT output
  pins.idle.stepperDir = 32; //Direction pin  for DRV8825 driver
  pins.idle.stepperStep = 31; //Step pin for DRV8825 driver
  pins.idle.stepperEnable = 30; //Enable pin for DRV8825 driver
  pins.pinBoost = 24; //Boost control
  pins.pinFan = 25; //Pin for the fan output
  pins.pinResetControl = 46; //Reset control output PLACEHOLDER value for now

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));

  return pins;
}
#endif

static pinNumbers_t getSpectreV05PinMapping(void)
{
  pinNumbers_t pins;

#if defined(STM32F407xx)
  //Pin definitions for experimental board Tjeerd 
  //Black F407VE wiki.stm32duino.com/index.php?title=STM32F407
  //https://github.com/Tjeerdie/SPECTRE/tree/master/SPECTRE_V0.5
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { PD12, PD13, PD14, PD15, PE9, PE11, PE14, PE13, };
  static constexpr uint8_t boardCoilPins[] PROGMEM = { PD7, PB9, PA8, PD10, PD9, PB7, };
  //******************************************
  //******** PORTA CONNECTIONS *************** 
  //******************************************
  // = PA0; //Wakeup ADC123
  // = PA1; //ADC123
  // = PA2; //ADC123
  // = PA3; //ADC123
  // = PA4; //ADC12
  // = PA5; //ADC12
  // = PA6; //ADC12 LED_BUILTIN_1
  // = PA7; //ADC12 LED_BUILTIN_2
  // = PA9;  //TXD1=Bluetooth module
  // = PA10; //RXD1=Bluetooth module
  // = PA11; //(DO NOT USE FOR SPEEDUINO) USB
  // = PA12; //(DO NOT USE FOR SPEEDUINO) USB 
  // = PA13;  //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK
  // = PA14;  //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK
  // = PA15;  //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK

  //******************************************
  //******** PORTB CONNECTIONS *************** 
  //******************************************
  // = PB0;  //(DO NOT USE FOR SPEEDUINO) ADC123 - SPI FLASH CHIP CS pin
  pins.sensors.baro = PB1; //ADC12
  // = PB2;  //(DO NOT USE FOR SPEEDUINO) BOOT1 
  // = PB3;  //(DO NOT USE FOR SPEEDUINO) SPI1_SCK FLASH CHIP
  // = PB4;  //(DO NOT USE FOR SPEEDUINO) SPI1_MISO FLASH CHIP
  // = PB5;  //(DO NOT USE FOR SPEEDUINO) SPI1_MOSI FLASH CHIP
  // = PB6;  //NRF_CE
  // = PB8;  //NRF_IRQ
  // = PB9;  //
  // = PB10; //TXD3
  // = PB11; //RXD3
  // = PB12; //
  // = PB13;  //SPI2_SCK
  // = PB14;  //SPI2_MISO
  // = PB15;  //SPI2_MOSI

  //******************************************
  //******** PORTC CONNECTIONS *************** 
  //******************************************
  pins.sensors.IAT = PC0; //ADC123 
  pins.sensors.TPS = PC1; //ADC123
  pins.sensors.MAP = PC2; //ADC123 
  pins.sensors.CLT = PC3; //ADC123
  pins.sensors.O2 = PC4; //ADC12
  pins.sensors.Bat = PC5;  //ADC12
  pins.pinBoost = PC6; //
  pins.idle.idle1 = PC7; //
  // = PC8;  //(DO NOT USE FOR SPEEDUINO) - SDIO_D0
  // = PC9;  //(DO NOT USE FOR SPEEDUINO) - SDIO_D1
  // = PC10;  //(DO NOT USE FOR SPEEDUINO) - SDIO_D2
  // = PC11;  //(DO NOT USE FOR SPEEDUINO) - SDIO_D3
  // = PC12;  //(DO NOT USE FOR SPEEDUINO) - SDIO_SCK
  pins.pinTachOut = PC13; //
  // = PC14;  //(DO NOT USE FOR SPEEDUINO) - OSC32_IN
  // = PC15;  //(DO NOT USE FOR SPEEDUINO) - OSC32_OUT

  //******************************************
  //******** PORTD CONNECTIONS *************** 
  //******************************************
  // = PD0;  //CANRX
  // = PD1;  //CANTX
  // = PD2;  //(DO NOT USE FOR SPEEDUINO) - SDIO_CMD
  pins.idle.idle2 = PD3; //
  // = PD4;  //
  pins.sensors.flex = PD4;
  // = PD5; //TXD2
  // = PD6;  //RXD2
  // = PD7;  //
  // = PD8;  //
  // = PD11;  //

  //******************************************
  //******** PORTE CONNECTIONS *************** 
  //******************************************
  pins.triggerPins.primary = PE0; //
  pins.triggerPins.secondary = PE1; //
  pins.idle.stepperEnable = PE2; //
  pins.pinFuelPump = PE3; //ONBOARD KEY1
  // = PE4;  //ONBOARD KEY2
  pins.idle.stepperStep = PE5; //
  pins.pinFan = PE6; //
  pins.idle.stepperDir = PE7; //
  // = PE8;  //
  // = PE15;  //

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));
#elif (defined(STM32F411xE) || defined(STM32F401xC))
  //pins PA12, PA11 are used for USB or CAN couldn't be used for GPIO
  //PB2 can't be used as input because is BOOT pin
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { PB7, PB6, PB5, PB4 };
  static constexpr uint8_t boardCoilPins[] PROGMEM = { PB9, PB8, PB3, PA15, };
  pins.sensors.TPS = A2;//TPS input pin
  pins.sensors.MAP = A3; //MAP sensor pin
  pins.sensors.IAT = A0; //IAT sensor pin
  pins.sensors.CLT = A1; //CLS sensor pin
  pins.sensors.O2 = A8; //O2 Sensor pin
  pins.sensors.Bat = A4; //Battery reference voltage pin
  pins.sensors.baro = pins.sensors.MAP;
  pins.pinTachOut = PB1; //Tacho output pin  (Goes to ULN2803)
  pins.idle.idle1 = PB2; //Single wire idle control
  pins.idle.idle2 = PB10; //2 wire idle control
  pins.pinBoost = PA6; //Boost control
  pins.idle.stepperDir = PB10; //Direction pin  for DRV8825 driver
  pins.idle.stepperStep = PB2; //Step pin for DRV8825 driver
  pins.pinFuelPump = PA8; //Fuel pump output
  pins.pinFan = PA5; //Pin for the fan output (Goes to ULN2803)

  //external interrupt enabled pins
  pins.sensors.flex = PC14; // Flex sensor (Must be external interrupt enabled)
  pins.triggerPins.primary = PC13; //The CAS pin also led pin so bad idea
  pins.triggerPins.secondary = PC15; //The Cam Sensor pin

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));
#elif defined(CORE_STM32)
  //blue pill wiki.stm32duino.com/index.php?title=Blue_Pill
  //Maple mini wiki.stm32duino.com/index.php?title=Maple_Mini
  //pins PA12, PA11 are used for USB or CAN couldn't be used for GPIO
  //PB2 can't be used as input because is BOOT pin
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { PB7, PB6, PB5, PB4 };
  static constexpr uint8_t boardCoilPins[] PROGMEM = { PB3, PA15, PA14, PA9, PA8, };
  pins.sensors.TPS = A0; //TPS input pin
  pins.sensors.MAP = A1; //MAP sensor pin
  pins.sensors.IAT = A2; //IAT sensor pin
  pins.sensors.CLT = A3; //CLS sensor pin
  pins.sensors.O2 = A4; //O2 Sensor pin
  pins.sensors.Bat = A5; //Battery reference voltage pin
  pins.sensors.baro = pins.sensors.MAP;
  pins.idle.idle1 = PB2; //Single wire idle control
  pins.idle.idle2 = PA2; //2 wire idle control
  pins.pinBoost = PA1; //Boost control
  pins.pinVVT_1 = PA0; //Default VVT output
  pins.pinVVT_2 = PA2; //Default VVT2 output
  pins.idle.stepperDir = PC15; //Direction pin  for DRV8825 driver
  pins.idle.stepperStep = PC14; //Step pin for DRV8825 driver
  pins.idle.stepperEnable = PC13; //Enable pin for DRV8825
  pins.pinFan = PB1; //Pin for the fan output
  pins.pinFuelPump = PB11; //Fuel pump output
  pins.pinTachOut = PB10; //Tacho output pin
  //external interrupt enabled pins
  pins.sensors.pinFlex = PB8; // Flex sensor (Must be external interrupt enabled)
  pins.triggerPins.primary = PA10; //The CAS pin
  pins.triggerPins.secondary = PA13; //The Cam Sensor pin

  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));
#endif

  return pins;
}

#if defined(STM32F407xx)
static pinNumbers_t getDefaultSTM32PinMapping(void)
{
  pinNumbers_t pins;
  //Pin definitions for experimental board Tjeerd 
  //Black F407VE wiki.stm32duino.com/index.php?title=STM32F407
  static constexpr uint8_t boardInjectorPins[] PROGMEM = { PD12, PD13, PD14, PD15, PE11, PE12 };
  pins.injectorPins.copy_P(boardInjectorPins, _countof(boardInjectorPins), configPage2);
  static constexpr uint8_t boardCoilPins[] PROGMEM = { PD7, PB9, PA8, PB10, PD9, };
  pins.coilPins.copy_P(boardCoilPins, _countof(boardCoilPins));
  //******************************************
  //******** PORTA CONNECTIONS *************** 
  //******************************************
  /* = PA0 */ //Wakeup ADC123
  // = PA1;
  // = PA2;
  // = PA3;
  // = PA4;
  /* = PA5; */ //ADC12
  pins.pinFuelPump = PA6; //ADC12 LED_BUILTIN_1
  /* = PA7; */ //ADC12 LED_BUILTIN_2
  /* = PA9 */ //TXD1
  /* = PA10 */ //RXD1
  /* = PA11 */ //(DO NOT USE FOR SPEEDUINO) USB
  /* = PA12 */ //(DO NOT USE FOR SPEEDUINO) USB 
  /* = PA13 */ //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK
  /* = PA14 */ //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK
  /* = PA15 */ //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK

  //******************************************
  //******** PORTB CONNECTIONS *************** 
  //******************************************
  /* = PB0; */ //(DO NOT USE FOR SPEEDUINO) ADC123 - SPI FLASH CHIP CS pin
  pins.sensors.baro = PB1; //ADC12
  /* = PB2; */ //(DO NOT USE FOR SPEEDUINO) BOOT1 
  /* = PB3; */ //(DO NOT USE FOR SPEEDUINO) SPI1_SCK FLASH CHIP
  /* = PB4; */ //(DO NOT USE FOR SPEEDUINO) SPI1_MISO FLASH CHIP
  /* = PB5; */ //(DO NOT USE FOR SPEEDUINO) SPI1_MOSI FLASH CHIP
  /* = PB6; */ //NRF_CE
  /* = PB7; */ //NRF_CS
  /* = PB8; */ //NRF_IRQ
  /* = PB9; */ //
  pins.idle.idle1 = PB11; //RXD3
  pins.idle.idle2 = PB12; //
  /* pins.pinBoost = PB12; */ //
  /* = PB13; */ //SPI2_SCK
  /* = PB14; */ //SPI2_MISO
  /* = PB15; */ //SPI2_MOSI

  //******************************************
  //******** PORTC CONNECTIONS *************** 
  //******************************************
  pins.sensors.MAP = PC0; //ADC123 
  pins.sensors.TPS = PC1; //ADC123
  pins.sensors.IAT = PC2; //ADC123
  pins.sensors.CLT = PC3; //ADC123
  pins.sensors.O2 = PC4; //ADC12
  pins.sensors.Bat = PC5; //ADC12
  /*pins.pinVVT_1 = PC6; */ //
  /* = PC8; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D0
  /* = PC9; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D1
  /* = PC10; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D2
  /* = PC11; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D3
  /* = PC12; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_SCK
  pins.pinTachOut = PC13; //
  /* = PC14; */ //(DO NOT USE FOR SPEEDUINO) - OSC32_IN
  /* = PC15; */ //(DO NOT USE FOR SPEEDUINO) - OSC32_OUT

  //******************************************
  //******** PORTD CONNECTIONS *************** 
  //******************************************
  /* = PD0; */ //CANRX
  /* = PD1; */ //CANTX
  /* = PD2; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_CMD
  /* = PD3; */ //
  /* = PD4; */ //
  pins.sensors.flex = PD4;
  /* = PD5;*/ //TXD2
  /* = PD6; */ //RXD2
  /* = PD7; */ //
  /* = PD8; */ //
  /* = PD10; */ //
  /* = PD11; */ //

  //******************************************
  //******** PORTE CONNECTIONS *************** 
  //******************************************
  pins.triggerPins.primary = PE0; //
  pins.triggerPins.secondary = PE1; //
  pins.idle.stepperEnable = PE2; //
  /* = PE3; */ //ONBOARD KEY1
  /* = PE4; */ //ONBOARD KEY2
  pins.idle.stepperStep = PE5; //
  pins.pinFan = PE6; //
  pins.idle.stepperDir = PE7; //
  /* = PE8; */ //
  /* = PE9; */ //
  /* = PE10; */ //
  /* = PE13; */ //
  /* = PE14; */ //
  /* = PE15; */ //

  return pins;
}
#endif

static pinNumbers_t getDefaultPinMapping(void)
{
  return
#if defined(STM32F407xx)
    getDefaultSTM32PinMapping();
#else
    getV02PinMapping();
#endif  
}

static pinNumbers_t getPinMapping(uint8_t boardID)
{
  switch (boardID)
  {
    //Note: Case 0 (Speeduino v0.1) was removed in Nov 2020 to handle default case for blank FRAM modules

    case 1: return getV02PinMapping(); break;
    case 2: return getV03PinMapping(); break;
    case 3: return getV04PinMapping(); break;
    case 6: return getMiataNB2Mapping(); break;
    case 8: return getMiataNA18PinMapping(); break;
    case 9: return getMiataNA16PinMapping(); break;
    case 10: return getTurtanasPinMapping(); break;
    case 14: return getLevinPinMapping(); break;
    case 20: return getPlazomatv10PinMapping(); break;
    case 30: return getDazV6PinMapping(); break;
    case 31: return getBMWPnPPinMapping(); break;
    case 40: return getNO2CPinMapping(); break;
    case 41: return getUA4CPinMapping(); break;
    case 42: return getBlitzboxBL49spPinMapping(); break;
    case 45: return getDIYEFICORE4v10PinMapping(); break;

    #if defined(CORE_TEENSY35)
    case 50: return getDvjTeensyRevAPinMapping(); break;
    case 51: return getDvjTeensyRevBPinMapping(); break;
    case 53: return getJuiceBoxPinMapping(); break;
    #endif
    #if defined(CORE_TEENSY)
    case 55: return getDropBearPinMapping(); break;
    #endif   
    #if defined(CORE_TEENSY41)
    case 56: return getBearCubPinMapping(); break;
    #endif   
    case 60: return getSpectreV05PinMapping(); break;
    default: break;
  }
  return getDefaultPinMapping();  
}

/** Set board / microcontroller specific pin mappings / assignments.
 * The boardID is switch-case compared against raw boardID integers (not enum or defined label, and probably no need for that either)
 * which are originated from tuning SW (e.g. TS) set values and are available in reference/speeduino.ini (See pinLayout, note also that
 * numbering is not contiguous here).
 */
void setPinMapping(byte boardID)
{
  //Force set defaults. Will be overwritten below if needed.
  InjIoControlMode injControlMode = InjIoControlMode::Direct;
  IgnIoControlMode ignControlMode = IgnIoControlMode::Direct;

  if( configPage4.triggerTeeth == 0 ) { configPage4.triggerTeeth = 4; } //Avoid potential divide by 0 when starting decoders

  pinNumbers = getPinMapping(boardID);

  //Setup any devices that are using selectable pins

  if ( (configPage6.launchPin != 0) && (configPage6.launchPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinLaunch = pinTranslate(configPage6.launchPin); }
  if ( (configPage4.ignBypassPin != 0) && (configPage4.ignBypassPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinIgnBypass = pinTranslate(configPage4.ignBypassPin); }
  if ( (configPage2.tachoPin != 0) && (configPage2.tachoPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinTachOut = pinTranslate(configPage2.tachoPin); }
  if ( (configPage4.fuelPumpPin != 0) && (configPage4.fuelPumpPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinFuelPump = pinTranslate(configPage4.fuelPumpPin); }
  if ( (configPage6.fanPin != 0) && (configPage6.fanPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinFan = pinTranslate(configPage6.fanPin); }
  if ( (configPage6.boostPin != 0) && (configPage6.boostPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinBoost = pinTranslate(configPage6.boostPin); }
  if ( (configPage6.vvt1Pin != 0) && (configPage6.vvt1Pin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinVVT_1 = pinTranslate(configPage6.vvt1Pin); }
  if ( (configPage6.useExtBaro != 0) && (configPage6.baroPin < BOARD_MAX_IO_PINS) ) { pinNumbers.sensors.baro = pinTranslateAnalog(configPage6.baroPin); }
  if ( (configPage6.useEMAP != 0) && (configPage10.EMAPPin < BOARD_MAX_IO_PINS) ) { pinNumbers.sensors.EMAP = pinTranslateAnalog(configPage10.EMAPPin); }
  if ( (configPage10.fuel2InputPin != 0) && (configPage10.fuel2InputPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinFuel2Input = pinTranslate(configPage10.fuel2InputPin); }
  if ( (configPage10.spark2InputPin != 0) && (configPage10.spark2InputPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinSpark2Input = pinTranslate(configPage10.spark2InputPin); }
  if ( (configPage2.vssPin != 0) && (configPage2.vssPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinVSS = pinTranslate(configPage2.vssPin); }
  if ( (configPage10.fuelPressureEnable) && (configPage10.fuelPressurePin < BOARD_MAX_IO_PINS) ) { pinNumbers.sensors.fuelPressure = pinTranslateAnalog(configPage10.fuelPressurePin); }
  if ( (configPage10.oilPressureEnable) && (configPage10.oilPressurePin < BOARD_MAX_IO_PINS) ) { pinNumbers.sensors.oilPressure = pinTranslateAnalog(configPage10.oilPressurePin); }
  
  if ( (configPage10.wmiEmptyPin != 0) && (configPage10.wmiEmptyPin < BOARD_MAX_IO_PINS) ) { pinNumbers.wmi.empty = pinTranslate(configPage10.wmiEmptyPin); }
  if ( (configPage10.wmiIndicatorPin != 0) && (configPage10.wmiIndicatorPin < BOARD_MAX_IO_PINS) ) { pinNumbers.wmi.indicator = pinTranslate(configPage10.wmiIndicatorPin); }
  if ( (configPage10.wmiEnabledPin != 0) && (configPage10.wmiEnabledPin < BOARD_MAX_IO_PINS) ) { pinNumbers.wmi.enabled = pinTranslate(configPage10.wmiEnabledPin); }
  if ( (configPage10.vvt2Pin != 0) && (configPage10.vvt2Pin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinVVT_2 = pinTranslate(configPage10.vvt2Pin); }
#ifdef SD_LOGGING
  if ( (configPage13.onboard_log_trigger_Epin != 0 ) && (configPage13.onboard_log_trigger_Epin != 0) && (configPage13.onboard_log_tr5_Epin_pin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinSDEnable = pinTranslate(configPage13.onboard_log_tr5_Epin_pin); }
#endif  

  //Currently there's no default pin for Idle Up
  
  pinNumbers.idle.idleUp = pinTranslate(configPage2.idleUpPin);

  //Currently there's no default pin for Idle Up Output
  pinNumbers.idle.idleUpOutput = pinTranslate(configPage2.idleUpOutputPin);

  //Currently there's no default pin for closed throttle position sensor
  pinNumbers.sensors.CTPS = pinTranslate(configPage2.CTPSPin);
  
  // Air conditioning control initialisation
  if ((configPage15.airConCompPin != 0) && (configPage15.airConCompPin < BOARD_MAX_IO_PINS) ) { pinNumbers.airCon.compressor = pinTranslate(configPage15.airConCompPin); }
  if ((configPage15.airConFanPin != 0) && (configPage15.airConFanPin < BOARD_MAX_IO_PINS) ) { pinNumbers.airCon.fan = pinTranslate(configPage15.airConFanPin); }
  if ((configPage15.airConReqPin != 0) && (configPage15.airConReqPin < BOARD_MAX_IO_PINS) ) { pinNumbers.airCon.request = pinTranslate(configPage15.airConReqPin); }
    
  /* Reset control is a special case. If reset control is enabled, it needs its initial state set BEFORE its pinMode.
     If that doesn't happen and reset control is in "Serial Command" mode, the Arduino will end up in a reset loop
     because the control pin will go low as soon as the pinMode is set to OUTPUT. */
  if ( (configPage4.resetControlConfig != 0) && (configPage4.resetControlPin < BOARD_MAX_IO_PINS) )
  {
    if (configPage4.resetControlPin!=0U) {
      pinNumbers.pinResetControl = pinTranslate(configPage4.resetControlPin);
    }
    initialiseResetControl((ResetControlMode)configPage4.resetControlConfig, pinNumbers.pinResetControl);
  }
  

  //Finally, set the relevant pin modes for outputs
  pinMode(pinNumbers.idle.idle1, OUTPUT);
  pinMode(pinNumbers.idle.idle2, OUTPUT);
  pinMode(pinNumbers.idle.idleUpOutput, OUTPUT);
  pinMode(pinNumbers.idle.stepperDir, OUTPUT);
  pinMode(pinNumbers.idle.stepperStep, OUTPUT);
  pinMode(pinNumbers.idle.stepperEnable, OUTPUT);
  if(configPage4.ignBypassEnabled > 0) { pinMode(pinNumbers.pinIgnBypass, OUTPUT); }

  //This is a legacy mode option to revert the MAP reading behaviour to match what was in place prior to the 201905 firmware
  if(configPage2.legacyMAP > 0) { digitalWrite(pinNumbers.sensors.MAP, HIGH); }

  if(ignControlMode == IgnIoControlMode::Direct)
  {
    initIgnDirectIO(configPage4, pinNumbers.coilPins);
  } 

  if(injControlMode == InjIoControlMode::Direct)
  {
    initInjDirectIO(pinNumbers.injectorPins);
  }
  
#if defined(MC33810_SUPPORT)
  if( (ignControlMode == IgnIoControlMode::MC33810) || (injControlMode == InjIoControlMode::MC33810) )
  {
    initMC33810(configPage4, pinNumbers);
    if( (LED_BUILTIN != SCK) && (LED_BUILTIN != MOSI) && (LED_BUILTIN != MISO) ) pinMode(LED_BUILTIN, OUTPUT); //This is required on as the LED pin can otherwise be reset to an input
  }
#endif
  initInjIoControl(injControlMode);
  initIgnIoControl(ignControlMode);

//CS pin number is now set in a compile flag. 
// #ifdef USE_SPI_EEPROM
//   //We need to send the flash CS (SS) pin if we're using SPI flash. It cannot read from globals.
//   EEPROM.begin(USE_SPI_EEPROM);
// #endif

  initTacho(pinNumbers.pinTachOut);

  //And for inputs
  #if defined(CORE_STM32)
    #ifdef INPUT_ANALOG
      pinMode(pinNumbers.sensors.MAP, INPUT_ANALOG);
      pinMode(pinNumbers.sensors.O2, INPUT_ANALOG);
      pinMode(pinNumbers.sensors.O2_2, INPUT_ANALOG);
      pinMode(pinNumbers.sensors.TPS, INPUT_ANALOG);
      pinMode(pinNumbers.sensors.IAT, INPUT_ANALOG);
      pinMode(pinNumbers.sensors.CLT, INPUT_ANALOG);
      pinMode(pinNumbers.sensors.Bat, INPUT_ANALOG);
      pinMode(pinNumbers.sensors.baro, INPUT_ANALOG);
    #else
      pinMode(pinNumbers.sensors.MAP, INPUT);
      pinMode(pinNumbers.sensors.O2, INPUT);
      pinMode(pinNumbers.sensors.O2_2, INPUT);
      pinMode(pinNumbers.sensors.TPS, INPUT);
      pinMode(pinNumbers.sensors.IAT, INPUT);
      pinMode(pinNumbers.sensors.CLT, INPUT);
      pinMode(pinNumbers.sensors.Bat, INPUT);
      pinMode(pinNumbers.sensors.baro, INPUT);
    #endif
  #elif defined(CORE_TEENSY41)
    //Teensy 4.1 has a weak pull down resistor that needs to be disabled for all analog pinNumbers. 
    pinMode(pinNumbers.sensors.MAP, INPUT_DISABLE);
    pinMode(pinNumbers.sensors.O2, INPUT_DISABLE);
    pinMode(pinNumbers.sensors.O2_2, INPUT_DISABLE);
    pinMode(pinNumbers.sensors.TPS, INPUT_DISABLE);
    pinMode(pinNumbers.sensors.IAT, INPUT_DISABLE);
    pinMode(pinNumbers.sensors.CLT, INPUT_DISABLE);
    pinMode(pinNumbers.sensors.Bat, INPUT_DISABLE);
    pinMode(pinNumbers.sensors.baro, INPUT_DISABLE);
  #endif

  //Each of the below are only set when their relevant function is enabled. This can help prevent pin conflicts that users aren't aware of with unused functions
  if( isExternalVssMode(configPage2) && (!pinIsOutput(pinNumbers.pinVSS)) ) //Pin mode 1 for VSS is CAN
  {
    pinMode(pinNumbers.pinVSS, INPUT);
  }
  if( (configPage6.launchEnabled > 0) && (!pinIsOutput(pinNumbers.pinLaunch)) )
  {
    if (configPage6.lnchPullRes == true) { pinMode(pinNumbers.pinLaunch, INPUT_PULLUP); }
    else { pinMode(pinNumbers.pinLaunch, INPUT); } //If Launch Pull Resistor is not set make input float.
  }
  if( (configPage2.idleUpEnabled > 0) && (!pinIsOutput(pinNumbers.idle.idleUp)) )
  {
    if (configPage2.idleUpPolarity == 0) { pinMode(pinNumbers.idle.idleUp, INPUT_PULLUP); } //Normal setting
    else { pinMode(pinNumbers.idle.idleUp, INPUT); } //inverted setting
  }
  if( (configPage2.CTPSEnabled > 0) && (!pinIsOutput(pinNumbers.sensors.CTPS)) )
  {
    if (configPage2.CTPSPolarity == 0) { pinMode(pinNumbers.sensors.CTPS, INPUT_PULLUP); } //Normal setting
    else { pinMode(pinNumbers.sensors.CTPS, INPUT); } //inverted setting
  }
  if( (configPage10.fuel2Mode == FUEL2_MODE_INPUT_SWITCH) && (!pinIsOutput(pinNumbers.pinFuel2Input)) )
  {
    if (configPage10.fuel2InputPullup == true) { pinMode(pinNumbers.pinFuel2Input, INPUT_PULLUP); } //With pullup
    else { pinMode(pinNumbers.pinFuel2Input, INPUT); } //Normal input
  }
  if( (configPage10.spark2Mode == SPARK2_MODE_INPUT_SWITCH) && (!pinIsOutput(pinNumbers.pinSpark2Input)) )
  {
    if (configPage10.spark2InputPullup == true) { pinMode(pinNumbers.pinSpark2Input, INPUT_PULLUP); } //With pullup
    else { pinMode(pinNumbers.pinSpark2Input, INPUT); } //Normal input
  }
  if( (configPage10.fuelPressureEnable > 0)  && (!pinIsOutput(pinNumbers.sensors.fuelPressure)) )
  {
    pinMode(pinNumbers.sensors.fuelPressure, INPUT);
  }
  if( (configPage10.oilPressureEnable > 0) && (!pinIsOutput(pinNumbers.sensors.oilPressure)) )
  {
    pinMode(pinNumbers.sensors.oilPressure, INPUT);
  }
#ifdef SD_LOGGING
  if( (configPage13.onboard_log_trigger_Epin > 0) && (!pinIsOutput(pinNumbers.pinSDEnable)) )
  {
    pinMode(pinNumbers.pinSDEnable, INPUT);
  }
#endif  
  if(configPage10.wmiEnabled > 0)
  {
    pinMode(pinNumbers.wmi.enabled, OUTPUT);
    if(configPage10.wmiIndicatorEnabled > 0)
    {
      pinMode(pinNumbers.wmi.indicator, OUTPUT);
      if (configPage10.wmiIndicatorPolarity > 0) { digitalWrite(pinNumbers.wmi.indicator, HIGH); }
    }
    if( (configPage10.wmiEmptyEnabled > 0) && (!pinIsOutput(pinNumbers.wmi.empty)) )
    {
      if (configPage10.wmiEmptyPolarity == 0) { pinMode(pinNumbers.wmi.empty, INPUT_PULLUP); } //Normal setting
      else { pinMode(pinNumbers.wmi.empty, INPUT); } //inverted setting
    }
  } 
}

/** Initialise the chosen trigger decoder.
 * - Set Interrupt numbers @ref triggerInterrupt, @ref triggerInterrupt2 and @ref triggerInterrupt3  by pin their numbers (based on board CORE_* define)
 * - Call decoder specific setup function triggerSetup_*() (by @ref config4.TrigPattern, set to one of the DECODER_* defines) and do any additional initialisations needed.
 * 
 * @todo Explain why triggerSetup_*() alone cannot do all the setup, but there's ~10+ lines worth of extra init for each of decoders.
 */

#if defined(CORE_AVR)
#pragma GCC pop_options
#endif