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
#include "scheduler_fuel_controller.h"
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
#include "src/controllers/fuelPump/fuelPumpController.h"

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

/** @brief Limit all injection schedule channel angles to 0-CRANK_ANGLE_MAX_INJ */
static void clampInjectionChannelAngles(void)
{
  fuelSchedule1.channelDegrees = injectorLimits(fuelSchedule1.channelDegrees);
#if INJ_CHANNELS>=2
  fuelSchedule2.channelDegrees = injectorLimits(fuelSchedule2.channelDegrees);
#endif
#if INJ_CHANNELS>=3
  fuelSchedule3.channelDegrees = injectorLimits(fuelSchedule3.channelDegrees);
#endif
#if INJ_CHANNELS>=4
  fuelSchedule4.channelDegrees = injectorLimits(fuelSchedule4.channelDegrees);
#endif
#if INJ_CHANNELS>=5
  fuelSchedule5.channelDegrees = injectorLimits(fuelSchedule5.channelDegrees);
#endif
#if INJ_CHANNELS>=6
  fuelSchedule6.channelDegrees = injectorLimits(fuelSchedule6.channelDegrees);
#endif
#if INJ_CHANNELS>=7
  fuelSchedule7.channelDegrees = injectorLimits(fuelSchedule7.channelDegrees);
#endif
#if INJ_CHANNELS>=8
  fuelSchedule8.channelDegrees = injectorLimits(fuelSchedule8.channelDegrees);
#endif
}

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
      configPage2.pinMapping = 3; //Force board to v0.4
    }
    setPinMapping(configPage2.pinMapping);

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
    initialiseIgnitionSchedules(configPage4.sparkMode, configPage2.nCylinders, configPage10.rotaryType);
    initialiseFuelSchedules(currentStatus, configPage2, configPage4);
    initialiseIdle(true);
    initialiseFan(pinNumbers.pinFan);
    initialiseAirCon();
    initialiseAuxPWM();
    initialiseCorrections();
    currentStatus.ioError = false; //Clear the I/O error bit. The bit will be set in initialiseADC() if there is problem in there.
    initialiseADC();
    initialiseMAPBaro();
    initialiseProgrammableIO(currentStatus, configPage13);
    initialiseFlexSensor(configPage2, currentStatus, pinNumbers.pinFlex);

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
    currentStatus.engineProtect.reset();
    ms_counter = 0;
    fixedCrankingOverride = 0;
    toothHistoryIndex = 0;
    
    noInterrupts();
    currentStatus.decoder = buildDecoder(configPage4.TrigPattern);
    boardInitPins(configPage2.pinMapping, pinNumbers);
    // The schedulers are all configured & pins are mapped - so start the schedulers
    startIgnitionSchedulers();
    startFuelSchedulers();
    
    //The secondary input can be used for VSS if nothing else requires it. Allows for the standard VR conditioner to be used for VSS. This MUST be run after the initialiseTriggers() function
    if( VSS_USES_RPM2() ) { attachInterrupt(digitalPinToInterrupt(pinNumbers.pinVSS), vssPulse, RISING); } //Secondary trigger input can safely be used for VSS
    if( FLEX_USES_RPM2() ) { attachInterrupt(digitalPinToInterrupt(pinNumbers.pinFlex), flexPulse, CHANGE); } //Secondary trigger input can safely be used for Flex sensor

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

    currentStatus.numPrimaryInjOutputs = 1; // Disable all injectors expect channel 1
    currentStatus.numSecondaryInjOutputs = 0;

    if(configPage2.strokes == FOUR_STROKE) { CRANK_ANGLE_MAX_INJ = 720 / currentStatus.nSquirts; }
    else { CRANK_ANGLE_MAX_INJ = 360 / currentStatus.nSquirts; }

    switch (configPage2.nCylinders) {
    case 1:
        ignitionSchedule1.channelDegrees = 0;
        fuelSchedule1.channelDegrees = 0;
        currentStatus.maxIgnOutputs = 1;
        currentStatus.numPrimaryInjOutputs = 1;

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
          currentStatus.numSecondaryInjOutputs = 1;
          fuelSchedule2.channelDegrees = fuelSchedule1.channelDegrees;
        }
        break;

    case 2:
        ignitionSchedule1.channelDegrees = 0;
        fuelSchedule1.channelDegrees = 0;
        currentStatus.maxIgnOutputs = 2;
        currentStatus.numPrimaryInjOutputs = 2;
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
        if (configPage2.engineType == EVEN_FIRE ) { fuelSchedule2.channelDegrees = 180; }
        else { fuelSchedule2.channelDegrees = configPage2.oddfire2; }
        if (!configPage2.injTiming) 
        { 
          //For simultaneous, all squirts happen at the same time
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 0; 
        }

        //Check if injector staging is enabled
        if(configPage10.stagingEnabled == true)
        {
          currentStatus.numSecondaryInjOutputs = 2;

          fuelSchedule3.channelDegrees = fuelSchedule1.channelDegrees;
          fuelSchedule4.channelDegrees = fuelSchedule2.channelDegrees;
        }

        break;

    case 3:
        ignitionSchedule1.channelDegrees = 0;
        currentStatus.maxIgnOutputs= 3;
        currentStatus.numPrimaryInjOutputs = 3;
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
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 120;
          fuelSchedule3.channelDegrees = 240;

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
            fuelSchedule2.channelDegrees = (fuelSchedule2.channelDegrees * 2) / currentStatus.nSquirts;
            fuelSchedule3.channelDegrees = (fuelSchedule3.channelDegrees * 2) / currentStatus.nSquirts;
          }

          if (!configPage2.injTiming) 
          { 
            //For simultaneous, all squirts happen at the same time
            fuelSchedule1.channelDegrees = 0;
            fuelSchedule2.channelDegrees = 0;
            fuelSchedule3.channelDegrees = 0; 
          } 
        }
        else if (configPage2.injLayout == INJ_SEQUENTIAL)
        {
          currentStatus.nSquirts = 1;

          if(configPage2.strokes == TWO_STROKE)
          {
            fuelSchedule1.channelDegrees = 0;
            fuelSchedule2.channelDegrees = 120;
            fuelSchedule3.channelDegrees = 240;
            CRANK_ANGLE_MAX_INJ = 360;
          }
          else
          {
            fuelSchedule1.channelDegrees = 0;
            fuelSchedule2.channelDegrees = 240;
            fuelSchedule3.channelDegrees = 480;
            CRANK_ANGLE_MAX_INJ = 720;
          }
        }
        else
        {
          //Should never happen, but default values
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 120;
          fuelSchedule3.channelDegrees = 240;
        }

        //Check if injector staging is enabled
        if(configPage10.stagingEnabled == true)
        {
          #if INJ_CHANNELS >= 6
            currentStatus.numSecondaryInjOutputs = 3;

            fuelSchedule4.channelDegrees = fuelSchedule1.channelDegrees;
            fuelSchedule5.channelDegrees = fuelSchedule2.channelDegrees;
            fuelSchedule6.channelDegrees = fuelSchedule3.channelDegrees;
          #else
            //Staged output is on channel 4
            currentStatus.numSecondaryInjOutputs = 1;
            fuelSchedule4.channelDegrees = fuelSchedule1.channelDegrees;
          #endif
        }
        break;
    case 4:
        ignitionSchedule1.channelDegrees = 0;
        fuelSchedule1.channelDegrees = 0;
        currentStatus.maxIgnOutputs = 2; //Default value for 4 cylinder, may be changed below
        currentStatus.numPrimaryInjOutputs = 2;
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
          fuelSchedule2.channelDegrees = 180;

          if (!configPage2.injTiming) 
          { 
            //For simultaneous, all squirts happen at the same time
            fuelSchedule1.channelDegrees = 0;
            fuelSchedule2.channelDegrees = 0; 
          }
          else if (currentStatus.nSquirts > 2)
          {
            //Adjust the injection angles based on the number of squirts
            fuelSchedule2.channelDegrees = (fuelSchedule2.channelDegrees * 2) / currentStatus.nSquirts;
          }
          else { } //Do nothing, default values are correct
        }
        else if (configPage2.injLayout == INJ_SEQUENTIAL)
        {
          fuelSchedule2.channelDegrees = 180;
          fuelSchedule3.channelDegrees = 360;
          fuelSchedule4.channelDegrees = 540;

          currentStatus.numPrimaryInjOutputs = 4;

          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
        }
        else
        {
          //Should never happen, but default values
          currentStatus.numPrimaryInjOutputs = 2;
        }

        //Check if injector staging is enabled
        if(configPage10.stagingEnabled == true)
        {
          currentStatus.numPrimaryInjOutputs = 4;

          if( (configPage2.injLayout == INJ_SEQUENTIAL) || (configPage2.injLayout == INJ_SEMISEQUENTIAL) )
          {
            //Staging with 4 cylinders semi/sequential requires 8 total channels
            #if INJ_CHANNELS >= 8
              currentStatus.numSecondaryInjOutputs = 4;

              fuelSchedule5.channelDegrees = fuelSchedule1.channelDegrees;
              fuelSchedule6.channelDegrees = fuelSchedule2.channelDegrees;
              fuelSchedule7.channelDegrees = fuelSchedule3.channelDegrees;
              fuelSchedule8.channelDegrees = fuelSchedule4.channelDegrees;
            #else
              //This is an invalid config as there are not enough outputs to support sequential + staging
              //Put the staging output to the non-existent channel 5
              #if (INJ_CHANNELS >= 5)
              currentStatus.numSecondaryInjOutputs = 1;
              fuelSchedule5.channelDegrees = fuelSchedule1.channelDegrees;
              #endif
            #endif
          }
          else
          {
            fuelSchedule3.channelDegrees = fuelSchedule1.channelDegrees;
            fuelSchedule4.channelDegrees = fuelSchedule2.channelDegrees;
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
        currentStatus.numPrimaryInjOutputs = 4; //Is updated below to 5 if there are enough channels

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
            fuelSchedule1.channelDegrees = 0;
            fuelSchedule2.channelDegrees = 0;
            fuelSchedule3.channelDegrees = 0;
            fuelSchedule4.channelDegrees = 0;
#if (INJ_CHANNELS >= 5)
            fuelSchedule5.channelDegrees = 0; 
#endif
          }
          else
          {
            fuelSchedule1.channelDegrees = 0;
            fuelSchedule2.channelDegrees = 72;
            fuelSchedule3.channelDegrees = 144;
            fuelSchedule4.channelDegrees = 216;
#if (INJ_CHANNELS >= 5)
            fuelSchedule5.channelDegrees = 288;
#endif

            //Divide by currentStatus.nSquirts ?
          }
        }
    #if INJ_CHANNELS >= 5
        else if (configPage2.injLayout == INJ_SEQUENTIAL)
        {
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 144;
          fuelSchedule3.channelDegrees = 288;
          fuelSchedule4.channelDegrees = 432;
          fuelSchedule5.channelDegrees = 576;

          currentStatus.numPrimaryInjOutputs = 5;

          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
        }
    #endif

    #if INJ_CHANNELS >= 6
          if(configPage10.stagingEnabled == true) { currentStatus.numSecondaryInjOutputs = 1; }
    #endif
        break;
    case 6:
        ignitionSchedule1.channelDegrees = 0;
        ignitionSchedule2.channelDegrees = 120;
        ignitionSchedule3.channelDegrees = 240;
        currentStatus.maxIgnOutputs= 3;
        currentStatus.numPrimaryInjOutputs = 3;

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
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 120;
          fuelSchedule3.channelDegrees = 240;
          if (!configPage2.injTiming)
          {
            //For simultaneous, all squirts happen at the same time
            fuelSchedule1.channelDegrees = 0;
            fuelSchedule2.channelDegrees = 0;
            fuelSchedule3.channelDegrees = 0;
          }
          else if (currentStatus.nSquirts > 2)
          {
            //Adjust the injection angles based on the number of squirts
            fuelSchedule2.channelDegrees = (fuelSchedule2.channelDegrees * 2) / currentStatus.nSquirts;
            fuelSchedule3.channelDegrees = (fuelSchedule3.channelDegrees * 2) / currentStatus.nSquirts;
          }
        }

    #if INJ_CHANNELS >= 6
        if (configPage2.injLayout == INJ_SEQUENTIAL)
        {
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 120;
          fuelSchedule3.channelDegrees = 240;
          fuelSchedule4.channelDegrees = 360;
          fuelSchedule5.channelDegrees = 480;
          fuelSchedule6.channelDegrees = 600;

          currentStatus.numPrimaryInjOutputs = 6;

          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
        }
        else if(configPage10.stagingEnabled == true) //Check if injector staging is enabled
        {
          currentStatus.numSecondaryInjOutputs = 3;

          if( (configPage2.injLayout == INJ_SEQUENTIAL) || (configPage2.injLayout == INJ_SEMISEQUENTIAL) )
          {
            //Staging with 6 cylinders semi/sequential requires 7 total channels
            #if INJ_CHANNELS >= 7
              currentStatus.numSecondaryInjOutputs = 4;

              fuelSchedule5.channelDegrees = fuelSchedule1.channelDegrees;
              fuelSchedule6.channelDegrees = fuelSchedule2.channelDegrees;
              fuelSchedule7.channelDegrees = fuelSchedule3.channelDegrees;
              // TODO: this makes no sense!!
              fuelSchedule8.channelDegrees = fuelSchedule4.channelDegrees;
            #else
              //This is an invalid config as there are not enough outputs to support sequential + staging
              //No staging output will be active
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
        currentStatus.numPrimaryInjOutputs = 4;


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
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 90;
          fuelSchedule3.channelDegrees = 180;
          fuelSchedule4.channelDegrees = 270;

          if (!configPage2.injTiming)
          {
            //For simultaneous, all squirts happen at the same time
            fuelSchedule1.channelDegrees = 0;
            fuelSchedule2.channelDegrees = 0;
            fuelSchedule3.channelDegrees = 0;
            fuelSchedule4.channelDegrees = 0;
          }
          else if (currentStatus.nSquirts > 2)
          {
            //Adjust the injection angles based on the number of squirts
            fuelSchedule2.channelDegrees = (fuelSchedule2.channelDegrees * 2) / currentStatus.nSquirts;
            fuelSchedule3.channelDegrees = (fuelSchedule3.channelDegrees * 2) / currentStatus.nSquirts;
            fuelSchedule4.channelDegrees = (fuelSchedule4.channelDegrees * 2) / currentStatus.nSquirts;
          }
        }

    #if INJ_CHANNELS >= 8
        else if (configPage2.injLayout == INJ_SEQUENTIAL)
        {
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 90;
          fuelSchedule3.channelDegrees = 180;
          fuelSchedule4.channelDegrees = 270;
          fuelSchedule5.channelDegrees = 360;
          fuelSchedule6.channelDegrees = 450;
          fuelSchedule7.channelDegrees = 540;
          fuelSchedule8.channelDegrees = 630;

          currentStatus.numPrimaryInjOutputs = 8;

          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
        }
    #endif

        break;
    default: //Handle this better!!!
        fuelSchedule1.channelDegrees = 0;
        fuelSchedule2.channelDegrees = 180;
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
    if ((configPage2.nCylinders==2U) && (configPage10.stagingEnabled == true))
    {
      //Phase this either 180 or 360 degrees out from inj3 (In reality this will always be 180 as you can't have sequential and staged currently)
      fuelSchedule4.channelDegrees = fuelSchedule3.channelDegrees + (uint16_t)(CRANK_ANGLE_MAX_INJ / 2U); 
      if (fuelSchedule4.channelDegrees>=(uint16_t)CRANK_ANGLE_MAX_INJ) { fuelSchedule4.channelDegrees -= (uint16_t)CRANK_ANGLE_MAX_INJ; }
    }
    clampInjectionChannelAngles();
    
    initialiseFuelPump(currentStatus, configPage2, pinNumbers.pinFuelPump);

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

  switch (boardID)
  {
    //Note: Case 0 (Speeduino v0.1) was removed in Nov 2020 to handle default case for blank FRAM modules

    case 1:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the v0.2 shield
      pinNumbers.pinInjector1 = 8; //Output pin injector 1 is on
      pinNumbers.pinInjector2 = 9; //Output pin injector 2 is on
      pinNumbers.pinInjector3 = 10; //Output pin injector 3 is on
      pinNumbers.pinInjector4 = 11; //Output pin injector 4 is on
      pinNumbers.pinInjector5 = 12; //Output pin injector 5 is on
      pinNumbers.pinCoil1 = 28; //Pin for coil 1
      pinNumbers.pinCoil2 = 24; //Pin for coil 2
      pinNumbers.pinCoil3 = 40; //Pin for coil 3
      pinNumbers.pinCoil4 = 36; //Pin for coil 4
      pinNumbers.pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinNumbers.pinTrigger = 20; //The CAS pin
      pinNumbers.pinTrigger2 = 21; //The Cam Sensor pin
      pinNumbers.pinTrigger3 = 3; //The Cam sensor 2 pin
      pinNumbers.pinTPS = A2; //TPS input pin
      pinNumbers.pinMAP = A3; //MAP sensor pin
      pinNumbers.pinIAT = A0; //IAT sensor pin
      pinNumbers.pinCLT = A1; //CLS sensor pin
      pinNumbers.pinO2 = A8; //O2 Sensor pin
      pinNumbers.pinBat = A4; //Battery reference voltage pin
      pinNumbers.pinDisplayReset = 48; // OLED reset pin
      pinNumbers.pinTachOut = 49; //Tacho output pin
      pinNumbers.pinIdle1 = 30; //Single wire idle control
      pinNumbers.pinIdle2 = 31; //2 wire idle control
      pinNumbers.pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinNumbers.pinStepperStep = 17; //Step pin for DRV8825 driver
      pinNumbers.pinFan = 47; //Pin for the fan output
      pinNumbers.pinFuelPump = 4; //Fuel pump output
      pinNumbers.pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
      pinNumbers.pinResetControl = 43; //Reset control output
      break;
    #endif
    case 2:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the v0.3 shield
      pinNumbers.pinInjector1 = 8; //Output pin injector 1 is on
      pinNumbers.pinInjector2 = 9; //Output pin injector 2 is on
      pinNumbers.pinInjector3 = 10; //Output pin injector 3 is on
      pinNumbers.pinInjector4 = 11; //Output pin injector 4 is on
      pinNumbers.pinInjector5 = 12; //Output pin injector 5 is on
      pinNumbers.pinCoil1 = 28; //Pin for coil 1
      pinNumbers.pinCoil2 = 24; //Pin for coil 2
      pinNumbers.pinCoil3 = 40; //Pin for coil 3
      pinNumbers.pinCoil4 = 36; //Pin for coil 4
      pinNumbers.pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinNumbers.pinTrigger = 19; //The CAS pin
      pinNumbers.pinTrigger2 = 18; //The Cam Sensor pin
      pinNumbers.pinTrigger3 = 3; //The Cam sensor 2 pin
      pinNumbers.pinTPS = A2;//TPS input pin
      pinNumbers.pinMAP = A3; //MAP sensor pin
      pinNumbers.pinIAT = A0; //IAT sensor pin
      pinNumbers.pinCLT = A1; //CLS sensor pin
      pinNumbers.pinO2 = A8; //O2 Sensor pin
      pinNumbers.pinBat = A4; //Battery reference voltage pin
      pinNumbers.pinDisplayReset = 48; // OLED reset pin
      pinNumbers.pinTachOut = 49; //Tacho output pin
      pinNumbers.pinIdle1 = 5; //Single wire idle control
      pinNumbers.pinIdle2 = 53; //2 wire idle control
      pinNumbers.pinBoost = 7; //Boost control
      pinNumbers.pinVVT_1 = 6; //Default VVT output
      pinNumbers.pinVVT_2 = 48; //Default VVT2 output
      pinNumbers.pinFuelPump = 4; //Fuel pump output
      pinNumbers.pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinNumbers.pinStepperStep = 17; //Step pin for DRV8825 driver
      pinNumbers.pinStepperEnable = 26; //Enable pin for DRV8825
      pinNumbers.pinFan = A13; //Pin for the fan output
      pinNumbers.pinLaunch = 51; //Can be overwritten below
      pinNumbers.pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
      pinNumbers.pinResetControl = 50; //Reset control output
      pinNumbers.pinBaro = A5;
      pinNumbers.pinVSS = 20;

      #if defined(CORE_TEENSY35)
        pinNumbers.pinTrigger = 23;
        pinNumbers.pinStepperDir = 33;
        pinNumbers.pinStepperStep = 34;
        pinNumbers.pinCoil1 = 31;
        pinNumbers.pinTachOut = 28;
        pinNumbers.pinFan = 27;
        pinNumbers.pinCoil4 = 21;
        pinNumbers.pinCoil3 = 30;
        pinNumbers.pinO2 = A22;
      #endif
    #endif
      break;

    case 3:
      //Pin mappings as per the v0.4 shield
      pinNumbers.pinInjector1 = 8; //Output pin injector 1 is on
      pinNumbers.pinInjector2 = 9; //Output pin injector 2 is on
      pinNumbers.pinInjector3 = 10; //Output pin injector 3 is on
      pinNumbers.pinInjector4 = 11; //Output pin injector 4 is on
      pinNumbers.pinInjector5 = 12; //Output pin injector 5 is on
      pinNumbers.pinInjector6 = 50; //CAUTION: Uses the same as Coil 4 below. 
      pinNumbers.pinCoil1 = 40; //Pin for coil 1
      pinNumbers.pinCoil2 = 38; //Pin for coil 2
      pinNumbers.pinCoil3 = 52; //Pin for coil 3
      pinNumbers.pinCoil4 = 50; //Pin for coil 4
      pinNumbers.pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinNumbers.pinTrigger = 19; //The CAS pin
      pinNumbers.pinTrigger2 = 18; //The Cam Sensor pin
      pinNumbers.pinTrigger3 = 3; //The Cam sensor 2 pin
      pinNumbers.pinTPS = A2;//TPS input pin
      pinNumbers.pinMAP = A3; //MAP sensor pin
      pinNumbers.pinIAT = A0; //IAT sensor pin
      pinNumbers.pinCLT = A1; //CLS sensor pin
      pinNumbers.pinO2 = A8; //O2 Sensor pin
      pinNumbers.pinBat = A4; //Battery reference voltage pin
      pinNumbers.pinDisplayReset = 48; // OLED reset pin
      pinNumbers.pinTachOut = 49; //Tacho output pin  (Goes to ULN2803)
      pinNumbers.pinIdle1 = 5; //Single wire idle control
      pinNumbers.pinIdle2 = 6; //2 wire idle control
      pinNumbers.pinBoost = 7; //Boost control
      pinNumbers.pinVVT_1 = 4; //Default VVT output
      pinNumbers.pinVVT_2 = 48; //Default VVT2 output
      pinNumbers.pinFuelPump = 45; //Fuel pump output  (Goes to ULN2803)
      pinNumbers.pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinNumbers.pinStepperStep = 17; //Step pin for DRV8825 driver
      pinNumbers.pinStepperEnable = 24; //Enable pin for DRV8825
      pinNumbers.pinFan = 47; //Pin for the fan output (Goes to ULN2803)
      pinNumbers.pinLaunch = 51; //Can be overwritten below
      pinNumbers.pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
      pinNumbers.pinResetControl = 43; //Reset control output
      pinNumbers.pinBaro = A5;
      pinNumbers.pinVSS = 20;
      pinNumbers.pinWMIEmpty = 46;
      pinNumbers.pinWMIIndicator = 44;
      pinNumbers.pinWMIEnabled = 42;

      #if defined(CORE_TEENSY35)
        pinNumbers.pinInjector6 = 51;

        pinNumbers.pinTrigger = 23;
        pinNumbers.pinTrigger2 = 36;
        pinNumbers.pinStepperDir = 34;
        pinNumbers.pinStepperStep = 35;
        pinNumbers.pinCoil1 = 31;
        pinNumbers.pinCoil2 = 32;
        pinNumbers.pinTachOut = 28;
        pinNumbers.pinFan = 27;
        pinNumbers.pinCoil4 = 29;
        pinNumbers.pinCoil3 = 30;
        pinNumbers.pinO2 = A22;

        //Make sure the CAN pins aren't overwritten
        pinNumbers.pinTrigger3 = 54;
        pinNumbers.pinVVT_1 = 55;

      #elif defined(CORE_TEENSY41)
        //These are only to prevent lockups or weird behaviour on T4.1 when this board is used as the default
        pinNumbers.pinBaro = A4; 
        pinNumbers.pinMAP = A5;
        pinNumbers.pinTPS = A3; //TPS input pin
        pinNumbers.pinIAT = A0; //IAT sensor pin
        pinNumbers.pinCLT = A1; //CLS sensor pin
        pinNumbers.pinO2 = A2; //O2 Sensor pin
        pinNumbers.pinBat = A15; //Battery reference voltage pin. Needs Alpha4+
        pinNumbers.pinLaunch = 34; //Can be overwritten below
        pinNumbers.pinVSS = 35;
        pinNumbers.pinSpareTemp2 = A16; //WRONG! Needs updating!!
        pinNumbers.pinSpareTemp2 = A17; //WRONG! Needs updating!!

        pinNumbers.pinTrigger = 20; //The CAS pin
        pinNumbers.pinTrigger2 = 21; //The Cam Sensor pin
        pinNumbers.pinTrigger3 = 24;

        pinNumbers.pinStepperDir = 34;
        pinNumbers.pinStepperStep = 35;
        
        pinNumbers.pinCoil1 = 31;
        pinNumbers.pinCoil2 = 32;
        pinNumbers.pinCoil4 = 29;
        pinNumbers.pinCoil3 = 30;

        pinNumbers.pinTachOut = 28;
        pinNumbers.pinFan = 27;
        pinNumbers.pinFuelPump = 33;
        pinNumbers.pinWMIEmpty = 34;
        pinNumbers.pinWMIIndicator = 35;
        pinNumbers.pinWMIEnabled = 36;
      #elif defined(STM32F407xx)
     //Pin definitions for experimental board Tjeerd 
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
        pinNumbers.pinFuelPump = PA7; //ADC12 LED_BUILTIN_2
        pinNumbers.pinCoil3 = PA8;
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
        pinNumbers.pinBaro = PB1; //ADC12
        /* = PB2; */ //(DO NOT USE FOR SPEEDUINO) BOOT1 
        /* = PB3; */ //(DO NOT USE FOR SPEEDUINO) SPI1_SCK FLASH CHIP
        /* = PB4; */ //(DO NOT USE FOR SPEEDUINO) SPI1_MISO FLASH CHIP
        /* = PB5; */ //(DO NOT USE FOR SPEEDUINO) SPI1_MOSI FLASH CHIP
        /* = PB6; */ //NRF_CE
        /* = PB7; */ //NRF_CS
        /* = PB8; */ //NRF_IRQ
        pinNumbers.pinCoil2 = PB9; //
        /* = PB9; */ //
        pinNumbers.pinCoil4 = PB10; //TXD3
        pinNumbers.pinIdle1 = PB11; //RXD3
        pinNumbers.pinIdle2 = PB12; //
        pinNumbers.pinBoost = PB12; //
        /* = PB13; */ //SPI2_SCK
        /* = PB14; */ //SPI2_MISO
        /* = PB15; */ //SPI2_MOSI

        //******************************************
        //******** PORTC CONNECTIONS *************** 
        //******************************************
        pinNumbers.pinMAP = PC0; //ADC123 
        pinNumbers.pinTPS = PC1; //ADC123
        pinNumbers.pinIAT = PC2; //ADC123
        pinNumbers.pinCLT = PC3; //ADC123
        pinNumbers.pinO2 = PC4;  //ADC12
        pinNumbers.pinBat = PC5; //ADC12
        pinNumbers.pinVVT_1 = PC6; //
        pinNumbers.pinDisplayReset = PC7; //
        /* = PC8; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D0
        /* = PC9; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D1
        /* = PC10; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D2
        /* = PC11; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D3
        /* = PC12; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_SCK
        pinNumbers.pinTachOut = PC13; //
        /* = PC14; */ //(DO NOT USE FOR SPEEDUINO) - OSC32_IN
        /* = PC15; */ //(DO NOT USE FOR SPEEDUINO) - OSC32_OUT

        //******************************************
        //******** PORTD CONNECTIONS *************** 
        //******************************************
        /* = PD0; */ //CANRX
        /* = PD1; */ //CANTX
        /* = PD2; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_CMD
        pinNumbers.pinVVT_2 = PD3; //
        pinNumbers.pinFlex = PD4;
        /* = PD5;*/ //TXD2
        /* = PD6; */ //RXD2
        pinNumbers.pinCoil1 = PD7; //
        /* = PD8; */ //
        pinNumbers.pinCoil5 = PD9;//
        /* = PD10; */ //
        /* = PD11; */ //
        pinNumbers.pinInjector1 = PD12; //
        pinNumbers.pinInjector2 = PD13; //
        pinNumbers.pinInjector3 = PD14; //
        pinNumbers.pinInjector4 = PD15; //

        //******************************************
        //******** PORTE CONNECTIONS *************** 
        //******************************************
        pinNumbers.pinTrigger = PE0; //
        pinNumbers.pinTrigger2 = PE1; //
        pinNumbers.pinStepperEnable = PE2; //
        /* = PE3; */ //ONBOARD KEY1
        /* = PE4; */ //ONBOARD KEY2
        pinNumbers.pinStepperStep = PE5; //
        pinNumbers.pinFan = PE6; //
        pinNumbers.pinStepperDir = PE7; //
        /* = PE8; */ //
        /* = PE9; */ //
        /* = PE10; */ //
        pinNumbers.pinInjector5 = PE11; //
        pinNumbers.pinInjector6 = PE12; //
        /* = PE13; */ //
        /* = PE14; */ //
        /* = PE15; */ //

      #elif defined(CORE_STM32)
        //https://github.com/stm32duino/Arduino_Core_STM32/blob/master/variants/Generic_F411Cx/variant.h#L28
        //pins PA12, PA11 are used for USB or CAN couldn't be used for GPIO
        //pins PB12, PB13, PB14 and PB15 are used to SPI FLASH
        //PB2 can't be used as input because it's the BOOT pin
        pinNumbers.pinInjector1 = PB7; //Output pin injector 1 is on
        pinNumbers.pinInjector2 = PB6; //Output pin injector 2 is on
        pinNumbers.pinInjector3 = PB5; //Output pin injector 3 is on
        pinNumbers.pinInjector4 = PB4; //Output pin injector 4 is on
        pinNumbers.pinCoil1 = PB9; //Pin for coil 1
        pinNumbers.pinCoil2 = PB8; //Pin for coil 2
        pinNumbers.pinCoil3 = PB3; //Pin for coil 3
        pinNumbers.pinCoil4 = PA15; //Pin for coil 4
        pinNumbers.pinTPS = A2;//TPS input pin
        pinNumbers.pinMAP = A3; //MAP sensor pin
        pinNumbers.pinIAT = A0; //IAT sensor pin
        pinNumbers.pinCLT = A1; //CLS sensor pin
        pinNumbers.pinO2 = A8; //O2 Sensor pin
        pinNumbers.pinBat = A4; //Battery reference voltage pin
        pinNumbers.pinBaro = pinNumbers.pinMAP;
        pinNumbers.pinTachOut = PB1; //Tacho output pin  (Goes to ULN2803)
        pinNumbers.pinIdle1 = PB2; //Single wire idle control
        pinNumbers.pinIdle2 = PB10; //2 wire idle control
        pinNumbers.pinBoost = PA6; //Boost control
        pinNumbers.pinStepperDir = PB10; //Direction pin  for DRV8825 driver
        pinNumbers.pinStepperStep = PB2; //Step pin for DRV8825 driver
        pinNumbers.pinFuelPump = PA8; //Fuel pump output
        pinNumbers.pinFan = PA5; //Pin for the fan output (Goes to ULN2803)
        //external interrupt enabled pins
        pinNumbers.pinFlex = PC14; // Flex sensor (Must be external interrupt enabled)
        pinNumbers.pinTrigger = PC13; //The CAS pin also led pin so bad idea
        pinNumbers.pinTrigger2 = PC15; //The Cam Sensor pin
      #endif
      break;

    case 6:
      #ifndef SMALL_FLASH_MODE
      //Pin mappings as per the 2001-05 MX5 PNP shield
      pinNumbers.pinInjector1 = 44; //Output pin injector 1 is on
      pinNumbers.pinInjector2 = 46; //Output pin injector 2 is on
      pinNumbers.pinInjector3 = 47; //Output pin injector 3 is on
      pinNumbers.pinInjector4 = 45; //Output pin injector 4 is on
      pinNumbers.pinInjector5 = 14; //Output pin injector 5 is on
      pinNumbers.pinCoil1 = 42; //Pin for coil 1
      pinNumbers.pinCoil2 = 43; //Pin for coil 2
      pinNumbers.pinCoil3 = 32; //Pin for coil 3
      pinNumbers.pinCoil4 = 33; //Pin for coil 4
      pinNumbers.pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinNumbers.pinTrigger = 19; //The CAS pin
      pinNumbers.pinTrigger2 = 18; //The Cam Sensor pin
      pinNumbers.pinTrigger3 = 2; //The Cam sensor 2 pin
      pinNumbers.pinTPS = A2;//TPS input pin
      pinNumbers.pinMAP = A5; //MAP sensor pin
      pinNumbers.pinIAT = A0; //IAT sensor pin
      pinNumbers.pinCLT = A1; //CLS sensor pin
      pinNumbers.pinO2 = A3; //O2 Sensor pin
      pinNumbers.pinBat = A4; //Battery reference voltage pin
      pinNumbers.pinDisplayReset = 48; // OLED reset pin
      pinNumbers.pinTachOut = 23; //Tacho output pin  (Goes to ULN2803)
      pinNumbers.pinIdle1 = 5; //Single wire idle control
      pinNumbers.pinBoost = 4;
      pinNumbers.pinVVT_1 = 11; //Default VVT output
      pinNumbers.pinVVT_2 = 48; //Default VVT2 output
      pinNumbers.pinIdle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
      pinNumbers.pinFuelPump = 40; //Fuel pump output
      pinNumbers.pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinNumbers.pinStepperStep = 17; //Step pin for DRV8825 driver
      pinNumbers.pinStepperEnable = 24;
      pinNumbers.pinFan = 41; //Pin for the fan output
      pinNumbers.pinLaunch = 12; //Can be overwritten below
      pinNumbers.pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
      pinNumbers.pinResetControl = 39; //Reset control output
      pinNumbers.pinVSS = 2;
      #endif
      //This is NOT correct. It has not yet been tested with this board
      #if defined(CORE_TEENSY35)
        pinNumbers.pinTrigger = 23;
        pinNumbers.pinTrigger2 = 36;
        pinNumbers.pinStepperDir = 34;
        pinNumbers.pinStepperStep = 35;
        pinNumbers.pinCoil1 = 33; //Done
        pinNumbers.pinCoil2 = 24; //Done
        pinNumbers.pinCoil3 = 51; //Won't work (No mapping for pin 32)
        pinNumbers.pinCoil4 = 52; //Won't work (No mapping for pin 33)
        pinNumbers.pinFuelPump = 26; //Requires PVT4 adapter or above
        pinNumbers.pinFan = 50; //Won't work (No mapping for pin 35)
        pinNumbers.pinTachOut = 28; //Done
      #endif
      break;

    case 8:
      #ifndef SMALL_FLASH_MODE
      //Pin mappings as per the 1996-97 MX5 PNP shield
      pinNumbers.pinInjector1 = 11; //Output pin injector 1 is on
      pinNumbers.pinInjector2 = 10; //Output pin injector 2 is on
      pinNumbers.pinInjector3 = 9; //Output pin injector 3 is on
      pinNumbers.pinInjector4 = 8; //Output pin injector 4 is on
      pinNumbers.pinInjector5 = 14; //Output pin injector 5 is on
      pinNumbers.pinCoil1 = 39; //Pin for coil 1
      pinNumbers.pinCoil2 = 41; //Pin for coil 2
      pinNumbers.pinCoil3 = 32; //Pin for coil 3
      pinNumbers.pinCoil4 = 33; //Pin for coil 4
      pinNumbers.pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinNumbers.pinTrigger = 19; //The CAS pin
      pinNumbers.pinTrigger2 = 18; //The Cam Sensor pin
      pinNumbers.pinTPS = A2;//TPS input pin
      pinNumbers.pinMAP = A5; //MAP sensor pin
      pinNumbers.pinIAT = A0; //IAT sensor pin
      pinNumbers.pinCLT = A1; //CLS sensor pin
      pinNumbers.pinO2 = A3; //O2 Sensor pin
      pinNumbers.pinBat = A4; //Battery reference voltage pin
      pinNumbers.pinDisplayReset = 48; // OLED reset pin
      pinNumbers.pinTachOut = A9; //Tacho output pin  (Goes to ULN2803)
      pinNumbers.pinIdle1 = 2; //Single wire idle control
      pinNumbers.pinBoost = 4;
      pinNumbers.pinIdle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
      pinNumbers.pinFuelPump = 49; //Fuel pump output
      pinNumbers.pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinNumbers.pinStepperStep = 17; //Step pin for DRV8825 driver
      pinNumbers.pinStepperEnable = 24;
      pinNumbers.pinFan = 35; //Pin for the fan output
      pinNumbers.pinLaunch = 37; //Can be overwritten below
      pinNumbers.pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
      pinNumbers.pinResetControl = 44; //Reset control output

      //This is NOT correct. It has not yet been tested with this board
      #if defined(CORE_TEENSY35)
        pinNumbers.pinTrigger = 23;
        pinNumbers.pinTrigger2 = 36;
        pinNumbers.pinStepperDir = 34;
        pinNumbers.pinStepperStep = 35;
        pinNumbers.pinCoil1 = 33; //Done
        pinNumbers.pinCoil2 = 24; //Done
        pinNumbers.pinCoil3 = 51; //Won't work (No mapping for pin 32)
        pinNumbers.pinCoil4 = 52; //Won't work (No mapping for pin 33)
        pinNumbers.pinFuelPump = 26; //Requires PVT4 adapter or above
        pinNumbers.pinFan = 50; //Won't work (No mapping for pin 35)
        pinNumbers.pinTachOut = 28; //Done
      #endif
      #endif
      break;

    case 9:
     #ifndef SMALL_FLASH_MODE
      //Pin mappings as per the 89-95 MX5 PNP shield
      pinNumbers.pinInjector1 = 11; //Output pin injector 1 is on
      pinNumbers.pinInjector2 = 10; //Output pin injector 2 is on
      pinNumbers.pinInjector3 = 9; //Output pin injector 3 is on
      pinNumbers.pinInjector4 = 8; //Output pin injector 4 is on
      pinNumbers.pinInjector5 = 14; //Output pin injector 5 is on
      pinNumbers.pinCoil1 = 39; //Pin for coil 1
      pinNumbers.pinCoil2 = 41; //Pin for coil 2
      pinNumbers.pinCoil3 = 32; //Pin for coil 3
      pinNumbers.pinCoil4 = 33; //Pin for coil 4
      pinNumbers.pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinNumbers.pinTrigger = 19; //The CAS pin
      pinNumbers.pinTrigger2 = 18; //The Cam Sensor pin
      pinNumbers.pinTPS = A2;//TPS input pin
      pinNumbers.pinMAP = A5; //MAP sensor pin
      pinNumbers.pinIAT = A0; //IAT sensor pin
      pinNumbers.pinCLT = A1; //CLS sensor pin
      pinNumbers.pinO2 = A3; //O2 Sensor pin
      pinNumbers.pinBat = A4; //Battery reference voltage pin
      pinNumbers.pinDisplayReset = 48; // OLED reset pin
      pinNumbers.pinTachOut = 49; //Tacho output pin  (Goes to ULN2803)
      pinNumbers.pinIdle1 = 2; //Single wire idle control
      pinNumbers.pinBoost = 4;
      pinNumbers.pinIdle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
      pinNumbers.pinFuelPump = 37; //Fuel pump output
      //Note that there is no stepper driver output on the PNP boards. These pins are unconnected and remain here just to prevent issues with random pin numbers occurring
      pinNumbers.pinStepperEnable = 15; //Enable pin for the DRV8825
      pinNumbers.pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinNumbers.pinStepperStep = 17; //Step pin for DRV8825 driver
      pinNumbers.pinFan = 35; //Pin for the fan output
      pinNumbers.pinLaunch = 12; //Can be overwritten below
      pinNumbers.pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
      pinNumbers.pinResetControl = 44; //Reset control output
      pinNumbers.pinVSS = 20;
      pinNumbers.pinIdleUp = 48;
      pinNumbers.pinCTPS = 47;
      #endif
      #if defined(CORE_TEENSY35)
        pinNumbers.pinTrigger = 23;
        pinNumbers.pinTrigger2 = 36;
        pinNumbers.pinStepperDir = 34;
        pinNumbers.pinStepperStep = 35;
        pinNumbers.pinCoil1 = 33; //Done
        pinNumbers.pinCoil2 = 24; //Done
        pinNumbers.pinCoil3 = 51; //Won't work (No mapping for pin 32)
        pinNumbers.pinCoil4 = 52; //Won't work (No mapping for pin 33)
        pinNumbers.pinFuelPump = 26; //Requires PVT4 adapter or above
        pinNumbers.pinFan = 50; //Won't work (No mapping for pin 35)
        pinNumbers.pinTachOut = 28; //Done
      #endif
      break;

    case 10:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings for user turtanas PCB
      pinNumbers.pinInjector1 = 4; //Output pin injector 1 is on
      pinNumbers.pinInjector2 = 5; //Output pin injector 2 is on
      pinNumbers.pinInjector3 = 6; //Output pin injector 3 is on
      pinNumbers.pinInjector4 = 7; //Output pin injector 4 is on
      pinNumbers.pinInjector5 = 8; //Placeholder only - NOT USED
      pinNumbers.pinInjector6 = 9; //Placeholder only - NOT USED
      pinNumbers.pinInjector7 = 10; //Placeholder only - NOT USED
      pinNumbers.pinInjector8 = 11; //Placeholder only - NOT USED
      pinNumbers.pinCoil1 = 24; //Pin for coil 1
      pinNumbers.pinCoil2 = 28; //Pin for coil 2
      pinNumbers.pinCoil3 = 36; //Pin for coil 3
      pinNumbers.pinCoil4 = 40; //Pin for coil 4
      pinNumbers.pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinNumbers.pinTrigger = 18; //The CAS pin
      pinNumbers.pinTrigger2 = 19; //The Cam Sensor pin
      pinNumbers.pinTPS = A2;//TPS input pin
      pinNumbers.pinMAP = A3; //MAP sensor pin
      pinNumbers.pinMAP2 = A8; //MAP2 sensor pin
      pinNumbers.pinIAT = A0; //IAT sensor pin
      pinNumbers.pinCLT = A1; //CLS sensor pin
      pinNumbers.pinO2 = A4; //O2 Sensor pin
      pinNumbers.pinBat = A7; //Battery reference voltage pin
      pinNumbers.pinDisplayReset = 48; // OLED reset pin
      pinNumbers.pinSpareTemp1 = A6;
      pinNumbers.pinSpareTemp2 = A5;
      pinNumbers.pinTachOut = 41; //Tacho output pin transistor is missing 2n2222 for this and 1k for 12v
      pinNumbers.pinFuelPump = 42; //Fuel pump output 2n2222
      pinNumbers.pinFan = 47; //Pin for the fan output
      pinNumbers.pinTachOut = 49; //Tacho output pin
      pinNumbers.pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
      pinNumbers.pinResetControl = 26; //Reset control output

    #endif
      break;

    case 14:
    // Pin mappings for the Levin board
    #if defined(STM32F407xx)
      pinNumbers.pinInjector1 = PB15;     // Output pin injector 1
      pinNumbers.pinInjector2 = PA8;      // Output pin injector 2
      pinNumbers.pinInjector3 = PB13;     // Output pin injector 3
      pinNumbers.pinInjector4 = PB14;     // Output pin injector 4
      pinNumbers.pinInjector5 = PE13;     // Output pin injector 5
      pinNumbers.pinInjector6 = PB12;     // Output pin injector 6
      pinNumbers.pinInjector7 = PE7;      // Output pin injector 7
      pinNumbers.pinInjector8 = PE10;     // Output pin injector 8
      pinNumbers.pinCoil1 = PC13;         // Pin for coil 1
      pinNumbers.pinCoil2 = PE6;          // Pin for coil 2
      pinNumbers.pinCoil3 = PE5;          // Pin for coil 3
      pinNumbers.pinCoil4 = PE4;          // Pin for coil 4
      pinNumbers.pinCoil5 = PE3;          // Pin for coil 5
      pinNumbers.pinCoil6 = PE2;          // Pin for coil 6
      pinNumbers.pinCoil7 = PB9;          // Pin for coil 7
      pinNumbers.pinCoil8 = PD12;         // Pin for coil 8
      pinNumbers.pinTrigger = PD3;        // The CAS pin
      pinNumbers.pinTrigger2 = PD4;       // The Cam Sensor pin
      pinNumbers.pinTPS = PA2;            // TPS input pin
      pinNumbers.pinMAP = PA3;            // MAP sensor pin
      pinNumbers.pinEMAP = PC5;           // EMAP sensor pin (placeholder)
      pinNumbers.pinIAT = PA0;            // IAT sensor pin
      pinNumbers.pinCLT = PA1;            // CLS sensor pin
      pinNumbers.pinO2 = PB0;             // O2 Sensor pin
      pinNumbers.pinBat = PA4;            // Battery reference voltage pin
      pinNumbers.pinBaro = PA5;           // Baro sensor pin
      pinNumbers.pinDisplayReset = PE12;  // OLED reset pin (placeholder)
      pinNumbers.pinTachOut = PE8;        // Tacho output pin  (Goes to UNL2803)
      pinNumbers.pinIdle1 = PD10;         // ICV pin1  (Goes to UNL2803)
      pinNumbers.pinIdle2 = PD9;          // ICV pin3  (Goes to UNL2803)
      pinNumbers.pinBoost = PD8;          // Boost control
      pinNumbers.pinVVT_1 = PD11;         // VVT1 output (intake vanos)
      pinNumbers.pinVVT_2 = PC6;          // VVT2 output (exhaust vanos)
      pinNumbers.pinFuelPump = PE11;      // Fuel pump output  (Goes to UNL2803)
      pinNumbers.pinStepperDir = PB10;    // Stepper valve isn't used with these
      pinNumbers.pinStepperStep = PB11;   // Stepper valve isn't used with these
      pinNumbers.pinStepperEnable = PA15; // Stepper valve isn't used with these
      pinNumbers.pinFan = PE9;            // Pin for the fan output (Goes to UNL2803)
      pinNumbers.pinLaunch = PB8;         // Launch control pin
      pinNumbers.pinFlex = PD7;           // Flex sensor
      pinNumbers.pinResetControl = PB7;   // Reset control output
      pinNumbers.pinVSS = PB6;            // VSS input pin
      pinNumbers.pinWMIEmpty = PA6;       //(placeholder)
      pinNumbers.pinWMIIndicator = PC3;   //(placeholder)
      pinNumbers.pinWMIEnabled = PE15;    //(placeholder)
      pinNumbers.pinIdleUp = PC7;         //(placeholder)
    #endif
      break;

    case 20:
    #if defined(CORE_AVR) && !defined(SMALL_FLASH_MODE) //No support for bluepill here anyway
      //Pin mappings as per the Plazomat In/Out shields Rev 0.1
      pinNumbers.pinInjector1 = 8; //Output pin injector 1 is on
      pinNumbers.pinInjector2 = 9; //Output pin injector 2 is on
      pinNumbers.pinInjector3 = 10; //Output pin injector 3 is on
      pinNumbers.pinInjector4 = 11; //Output pin injector 4 is on
      pinNumbers.pinInjector5 = 12; //Output pin injector 5 is on
      pinNumbers.pinCoil1 = 28; //Pin for coil 1
      pinNumbers.pinCoil2 = 24; //Pin for coil 2
      pinNumbers.pinCoil3 = 40; //Pin for coil 3
      pinNumbers.pinCoil4 = 36; //Pin for coil 4
      pinNumbers.pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinNumbers.pinSpareOut1 = 4; //Spare LSD Output 1(PWM)
      pinNumbers.pinSpareOut2 = 5; //Spare LSD Output 2(PWM)
      pinNumbers.pinSpareOut3 = 6; //Spare LSD Output 3(PWM)
      pinNumbers.pinSpareOut4 = 7; //Spare LSD Output 4(PWM)
      pinNumbers.pinSpareOut5 = 50; //Spare LSD Output 5(digital)
      pinNumbers.pinSpareOut6 = 52; //Spare LSD Output 6(digital)
      pinNumbers.pinTrigger = 20; //The CAS pin
      pinNumbers.pinTrigger2 = 21; //The Cam Sensor pin
      pinNumbers.pinSpareTemp2 = A15; //spare Analog input 2
      pinNumbers.pinSpareTemp1 = A14; //spare Analog input 1
      pinNumbers.pinO2 = A8; //O2 Sensor pin
      pinNumbers.pinBat = A4; //Battery reference voltage pin
      pinNumbers.pinMAP = A3; //MAP sensor pin
      pinNumbers.pinTPS = A2;//TPS input pin
      pinNumbers.pinCLT = A1; //CLS sensor pin
      pinNumbers.pinIAT = A0; //IAT sensor pin
      pinNumbers.pinFan = 47; //Pin for the fan output
      pinNumbers.pinFuelPump = 4; //Fuel pump output
      pinNumbers.pinTachOut = 49; //Tacho output pin
      pinNumbers.pinResetControl = 26; //Reset control output
    #endif
      break;

    case 30:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the dazv6 shield
      pinNumbers.pinInjector1 = 8; //Output pin injector 1 is on
      pinNumbers.pinInjector2 = 9; //Output pin injector 2 is on
      pinNumbers.pinInjector3 = 10; //Output pin injector 3 is on
      pinNumbers.pinInjector4 = 11; //Output pin injector 4 is on
      pinNumbers.pinInjector5 = 12; //Output pin injector 5 is on
      pinNumbers.pinCoil1 = 40; //Pin for coil 1
      pinNumbers.pinCoil2 = 38; //Pin for coil 2
      pinNumbers.pinCoil3 = 50; //Pin for coil 3
      pinNumbers.pinCoil4 = 52; //Pin for coil 4
      pinNumbers.pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinNumbers.pinTrigger = 19; //The CAS pin
      pinNumbers.pinTrigger2 = 18; //The Cam Sensor pin
      pinNumbers.pinTrigger3 = 17; // cam sensor 2 pin, pin17 isn't external trigger enabled in arduino mega??
      pinNumbers.pinTPS = A2;//TPS input pin
      pinNumbers.pinMAP = A3; //MAP sensor pin
      pinNumbers.pinIAT = A0; //IAT sensor pin
      pinNumbers.pinCLT = A1; //CLS sensor pin
      pinNumbers.pinO2 = A8; //O2 Sensor pin
      pinNumbers.pinO2_2 = A9; //O2 sensor pin (second sensor)
      pinNumbers.pinBat = A4; //Battery reference voltage pin
      pinNumbers.pinDisplayReset = 48; // OLED reset pin
      pinNumbers.pinTachOut = 49; //Tacho output pin
      pinNumbers.pinIdle1 = 5; //Single wire idle control
      pinNumbers.pinFuelPump = 45; //Fuel pump output
      pinNumbers.pinStepperDir = 20; //Direction pin  for DRV8825 driver
      pinNumbers.pinStepperStep = 21; //Step pin for DRV8825 driver
      pinNumbers.pinSpareHOut1 = 4; // high current output spare1
      pinNumbers.pinSpareHOut2 = 6; // high current output spare2
      pinNumbers.pinBoost = 7;
      pinNumbers.pinSpareLOut1 = 43; //low current output spare1
      pinNumbers.pinSpareLOut2 = 47;
      pinNumbers.pinSpareLOut3 = 49;
      pinNumbers.pinSpareLOut4 = 51;
      pinNumbers.pinSpareLOut5 = 53;
      pinNumbers.pinFan = 47; //Pin for the fan output
    #endif
      break;

   case 31:
      //Pin mappings for the BMW PnP PCBs by pazi88.
      #if defined(CORE_AVR)
      //This is the regular MEGA2560 pin mapping
      pinNumbers.pinInjector1 = 8; //Output pin injector 1
      pinNumbers.pinInjector2 = 9; //Output pin injector 2
      pinNumbers.pinInjector3 = 10; //Output pin injector 3
      pinNumbers.pinInjector4 = 11; //Output pin injector 4
      pinNumbers.pinInjector5 = 12; //Output pin injector 5
      pinNumbers.pinInjector6 = 50; //Output pin injector 6
      pinNumbers.pinInjector7 = 39; //Output pin injector 7
      pinNumbers.pinInjector8 = 42; //Output pin injector 8
      pinNumbers.pinCoil1 = 40; //Pin for coil 1
      pinNumbers.pinCoil2 = 38; //Pin for coil 2
      pinNumbers.pinCoil3 = 52; //Pin for coil 3
      pinNumbers.pinCoil4 = 48; //Pin for coil 4
      pinNumbers.pinCoil5 = 36; //Pin for coil 5
      pinNumbers.pinCoil6 = 34; //Pin for coil 6
      pinNumbers.pinCoil7 = 46; //Pin for coil 7
      pinNumbers.pinCoil8 = 53; //Pin for coil 8
      pinNumbers.pinTrigger = 19; //The CAS pin
      pinNumbers.pinTrigger2 = 18; //The Cam Sensor pin
      pinNumbers.pinTrigger3 = 20; //The Cam sensor 2 pin
      pinNumbers.pinTPS = A2;//TPS input pin
      pinNumbers.pinMAP = A3; //MAP sensor pin
      pinNumbers.pinEMAP = A15; //EMAP sensor pin
      pinNumbers.pinIAT = A0; //IAT sensor pin
      pinNumbers.pinCLT = A1; //CLT sensor pin
      pinNumbers.pinO2 = A8; //O2 Sensor pin
      pinNumbers.pinO2_2 = A12; //O2 Sensor pin
      pinNumbers.pinBat = A4; //Battery reference voltage pin
      pinNumbers.pinBaro = A5; //Baro sensor pin
      pinNumbers.pinDisplayReset = 41; // OLED reset pin
      pinNumbers.pinTachOut = 49; //Tacho output pin  (Goes to ULN2003)
      pinNumbers.pinIdle1 = 5; //ICV pin1
      pinNumbers.pinIdle2 = 6; //ICV pin3
      pinNumbers.pinBoost = 7; //Boost control
      pinNumbers.pinVVT_1 = 4; //VVT1 output (intake vanos)
      pinNumbers.pinVVT_2 = 26; //VVT2 output (exhaust vanos)
      pinNumbers.pinFuelPump = 45; //Fuel pump output  (Goes to ULN2003)
      pinNumbers.pinStepperDir = 16; //Stepper valve isn't used with these
      pinNumbers.pinStepperStep = 17; //Stepper valve isn't used with these
      pinNumbers.pinStepperEnable = 24; //Stepper valve isn't used with these
      pinNumbers.pinFan = 47; //Pin for the fan output (Goes to ULN2003)
      pinNumbers.pinLaunch = 51; //Launch control pin
      pinNumbers.pinFlex = 2; // Flex sensor
      pinNumbers.pinResetControl = 43; //Reset control output
      pinNumbers.pinVSS = 3; //VSS input pin
      pinNumbers.pinWMIEmpty = 31; //(placeholder)
      pinNumbers.pinWMIIndicator = 33; //(placeholder)
      pinNumbers.pinWMIEnabled = 35; //(placeholder)
      pinNumbers.pinIdleUp = 37; //(placeholder)
      pinNumbers.pinIdleUpOutput = 41; //(placeholder)
      pinNumbers.pinCTPS = A6; //(placeholder)
     #elif defined(STM32F407xx)
      pinNumbers.pinInjector1 = PB15; //Output pin injector 1
      pinNumbers.pinInjector2 = PB14; //Output pin injector 2
      pinNumbers.pinInjector3 = PB12; //Output pin injector 3
      pinNumbers.pinInjector4 = PB13; //Output pin injector 4
      pinNumbers.pinInjector5 = PA8; //Output pin injector 5
      pinNumbers.pinInjector6 = PE7; //Output pin injector 6
      pinNumbers.pinInjector7 = PE13; //Output pin injector 7
      pinNumbers.pinInjector8 = PE10; //Output pin injector 8
      pinNumbers.pinCoil1 = PE2; //Pin for coil 1
      pinNumbers.pinCoil2 = PE3; //Pin for coil 2
      pinNumbers.pinCoil3 = PC13; //Pin for coil 3
      pinNumbers.pinCoil4 = PE6; //Pin for coil 4
      pinNumbers.pinCoil5 = PE4; //Pin for coil 5
      pinNumbers.pinCoil6 = PE5; //Pin for coil 6
      pinNumbers.pinCoil7 = PE0; //Pin for coil 7
      pinNumbers.pinCoil8 = PB9; //Pin for coil 8
      pinNumbers.pinTrigger = PD3; //The CAS pin
      pinNumbers.pinTrigger2 = PD4; //The Cam Sensor pin
      pinNumbers.pinTPS = PA2;//TPS input pin
      pinNumbers.pinMAP = PA3; //MAP sensor pin
      pinNumbers.pinEMAP = PC5; //EMAP sensor pin
      pinNumbers.pinIAT = PA0; //IAT sensor pin
      pinNumbers.pinCLT = PA1; //CLS sensor pin
      pinNumbers.pinO2 = PB0; //O2 Sensor pin
      pinNumbers.pinO2_2 = PC2; //O2 Sensor pin
      pinNumbers.pinBat = PA4; //Battery reference voltage pin
      pinNumbers.pinBaro = PA5; //Baro sensor pin
      pinNumbers.pinDisplayReset = PE12; // OLED reset pin
      pinNumbers.pinTachOut = PE8; //Tacho output pin  (Goes to ULN2003)
      pinNumbers.pinIdle1 = PD10; //ICV pin1
      pinNumbers.pinIdle2 = PD9; //ICV pin3
      pinNumbers.pinBoost = PD8; //Boost control
      pinNumbers.pinVVT_1 = PD11; //VVT1 output (intake vanos)
      pinNumbers.pinVVT_2 = PC7; //VVT2 output (exhaust vanos)
      pinNumbers.pinFuelPump = PE11; //Fuel pump output  (Goes to ULN2003)
      pinNumbers.pinStepperDir = PB10; //Stepper valve isn't used with these
      pinNumbers.pinStepperStep = PB11; //Stepper valve isn't used with these
      pinNumbers.pinStepperEnable = PA15; //Stepper valve isn't used with these
      pinNumbers.pinFan = PE9; //Pin for the fan output (Goes to ULN2003)
      pinNumbers.pinLaunch = PB8; //Launch control pin
      pinNumbers.pinFlex = PD7; // Flex sensor
      pinNumbers.pinResetControl = PB7; //Reset control output
      pinNumbers.pinVSS = PB6; //VSS input pin
      pinNumbers.pinWMIEmpty = PD15; //(placeholder)
      pinNumbers.pinWMIIndicator = PD13; //(placeholder)
      pinNumbers.pinWMIEnabled = PE15; //(placeholder)
      pinNumbers.pinIdleUp = PE14; //(placeholder)
      pinNumbers.pinIdleUpOutput = PE12; //(placeholder)
      pinNumbers.pinCTPS = PA6; //(placeholder)
     #endif
      break;

    case 40:
     #ifndef SMALL_FLASH_MODE
      //Pin mappings as per the NO2C shield
      pinNumbers.pinInjector1 = 8; //Output pin injector 1 is on
      pinNumbers.pinInjector2 = 9; //Output pin injector 2 is on
      pinNumbers.pinInjector3 = 11; //Output pin injector 3 is on - NOT USED
      pinNumbers.pinInjector4 = 12; //Output pin injector 4 is on - NOT USED
      pinNumbers.pinInjector5 = 13; //Placeholder only - NOT USED
      pinNumbers.pinCoil1 = 23; //Pin for coil 1
      pinNumbers.pinCoil2 = 22; //Pin for coil 2
      pinNumbers.pinCoil3 = 2; //Pin for coil 3 - ONLY WITH DB2
      pinNumbers.pinCoil4 = 3; //Pin for coil 4 - ONLY WITH DB2
      pinNumbers.pinCoil5 = 46; //Placeholder only - NOT USED
      pinNumbers.pinTrigger = 19; //The CAS pin
      pinNumbers.pinTrigger2 = 18; //The Cam Sensor pin
      pinNumbers.pinTrigger3 = 21; //The Cam sensor 2 pin
      pinNumbers.pinTPS = A3; //TPS input pin
      pinNumbers.pinMAP = A0; //MAP sensor pin
      pinNumbers.pinIAT = A5; //IAT sensor pin
      pinNumbers.pinCLT = A4; //CLT sensor pin
      pinNumbers.pinO2 = A2; //O2 sensor pin
      pinNumbers.pinBat = A1; //Battery reference voltage pin
      pinNumbers.pinBaro = A6; //Baro sensor pin - ONLY WITH DB
      pinNumbers.pinSpareTemp1 = A7; //spare Analog input 1 - ONLY WITH DB
      pinNumbers.pinDisplayReset = 48; // OLED reset pin - NOT USED
      pinNumbers.pinTachOut = 38; //Tacho output pin
      pinNumbers.pinIdle1 = 5; //Single wire idle control
      pinNumbers.pinIdle2 = 47; //2 wire idle control - NOT USED
      pinNumbers.pinBoost = 7; //Boost control
      pinNumbers.pinVVT_1 = 6; //Default VVT output
      pinNumbers.pinVVT_2 = 48; //Default VVT2 output
      pinNumbers.pinFuelPump = 4; //Fuel pump output
      pinNumbers.pinStepperDir = 25; //Direction pin for DRV8825 driver
      pinNumbers.pinStepperStep = 24; //Step pin for DRV8825 driver
      pinNumbers.pinStepperEnable = 27; //Enable pin for DRV8825 driver
      pinNumbers.pinLaunch = 10; //Can be overwritten below
      pinNumbers.pinFlex = 20; // Flex sensor (Must be external interrupt enabled) - ONLY WITH DB
      pinNumbers.pinFan = 30; //Pin for the fan output - ONLY WITH DB
      pinNumbers.pinSpareLOut1 = 32; //low current output spare1 - ONLY WITH DB
      pinNumbers.pinSpareLOut2 = 34; //low current output spare2 - ONLY WITH DB
      pinNumbers.pinSpareLOut3 = 36; //low current output spare3 - ONLY WITH DB
      pinNumbers.pinResetControl = 26; //Reset control output
      #endif
      break;

    case 41:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the UA4C shield
      pinNumbers.pinInjector1 = 8; //Output pin injector 1 is on
      pinNumbers.pinInjector2 = 7; //Output pin injector 2 is on
      pinNumbers.pinInjector3 = 6; //Output pin injector 3 is on
      pinNumbers.pinInjector4 = 5; //Output pin injector 4 is on
      pinNumbers.pinInjector5 = 45; //Output pin injector 5 is on PLACEHOLDER value for now
      pinNumbers.pinCoil1 = 35; //Pin for coil 1
      pinNumbers.pinCoil2 = 36; //Pin for coil 2
      pinNumbers.pinCoil3 = 33; //Pin for coil 3
      pinNumbers.pinCoil4 = 34; //Pin for coil 4
      pinNumbers.pinCoil5 = 44; //Pin for coil 5 PLACEHOLDER value for now
      pinNumbers.pinTrigger = 19; //The CAS pin
      pinNumbers.pinTrigger2 = 18; //The Cam Sensor pin
      pinNumbers.pinTrigger3 = 3; //The Cam sensor 2 pin
      pinNumbers.pinFlex = 20; // Flex sensor
      pinNumbers.pinTPS = A3; //TPS input pin
      pinNumbers.pinMAP = A0; //MAP sensor pin
      pinNumbers.pinBaro = A7; //Baro sensor pin
      pinNumbers.pinIAT = A5; //IAT sensor pin
      pinNumbers.pinCLT = A4; //CLS sensor pin
      pinNumbers.pinO2 = A1; //O2 Sensor pin
      pinNumbers.pinO2_2 = A9; //O2 sensor pin (second sensor)
      pinNumbers.pinBat = A2; //Battery reference voltage pin
      pinNumbers.pinSpareTemp1 = A8; //spare Analog input 1
      pinNumbers.pinLaunch = 37; //Can be overwritten below
      pinNumbers.pinDisplayReset = 48; // OLED reset pin PLACEHOLDER value for now
      pinNumbers.pinTachOut = 22; //Tacho output pin
      pinNumbers.pinIdle1 = 9; //Single wire idle control
      pinNumbers.pinIdle2 = 10; //2 wire idle control
      pinNumbers.pinFuelPump = 23; //Fuel pump output
      pinNumbers.pinVVT_1 = 11; //Default VVT output
      pinNumbers.pinVVT_2 = 48; //Default VVT2 output
      pinNumbers.pinStepperDir = 32; //Direction pin  for DRV8825 driver
      pinNumbers.pinStepperStep = 31; //Step pin for DRV8825 driver
      pinNumbers.pinStepperEnable = 30; //Enable pin for DRV8825 driver
      pinNumbers.pinBoost = 12; //Boost control
      pinNumbers.pinSpareLOut1 = 26; //low current output spare1
      pinNumbers.pinSpareLOut2 = 27; //low current output spare2
      pinNumbers.pinSpareLOut3 = 28; //low current output spare3
      pinNumbers.pinSpareLOut4 = 29; //low current output spare4
      pinNumbers.pinFan = 24; //Pin for the fan output
      pinNumbers.pinResetControl = 46; //Reset control output PLACEHOLDER value for now
      pinNumbers.pinVSS = 2;
    #endif
      break;

    case 42:
      //Pin mappings for all BlitzboxBL49sp variants
      pinNumbers.pinInjector1 = 6; //Output pin injector 1
      pinNumbers.pinInjector2 = 7; //Output pin injector 2
      pinNumbers.pinInjector3 = 8; //Output pin injector 3
      pinNumbers.pinInjector4 = 9; //Output pin injector 4
      pinNumbers.pinCoil1 = 24; //Pin for coil 1
      pinNumbers.pinCoil2 = 25; //Pin for coil 2
      pinNumbers.pinCoil3 = 23; //Pin for coil 3
      pinNumbers.pinCoil4 = 22; //Pin for coil 4
      pinNumbers.pinTrigger = 19; //The CRANK Sensor pin
      pinNumbers.pinTrigger2 = 18; //The Cam Sensor pin
      pinNumbers.pinFlex = 20; // Flex sensor PLACEHOLDER value for now
      pinNumbers.pinTPS = A0; //TPS input pin
      pinNumbers.pinSpareTemp1 = A1; //LMM sensor pin
      pinNumbers.pinO2 = A2; //O2 Sensor pin
      pinNumbers.pinIAT = A3; //IAT sensor pin
      pinNumbers.pinCLT = A4; //CLT sensor pin
      pinNumbers.pinMAP = A7; //internal MAP sensor
      pinNumbers.pinBat = A6; //Battery reference voltage pin
      pinNumbers.pinBaro = A5; //external MAP/Baro sensor pin
      pinNumbers.pinO2_2 = A9; //O2 sensor pin (second sensor) PLACEHOLDER value for now
      pinNumbers.pinLaunch = 2; //Can be overwritten below
      pinNumbers.pinTachOut = 10; //Tacho output pin
      pinNumbers.pinIdle1 = 11; //Single wire idle control
      pinNumbers.pinIdle2 = 14; //2 wire idle control PLACEHOLDER value for now
      pinNumbers.pinFuelPump = 3; //Fuel pump output
      pinNumbers.pinVVT_1 = 15; //Default VVT output PLACEHOLDER value for now
      pinNumbers.pinBoost = 5; //Boost control
      pinNumbers.pinSpareLOut1 = 49; //enable Wideband Lambda Heater
      pinNumbers.pinSpareLOut2 = 16; //low current output spare2 PLACEHOLDER value for now
      pinNumbers.pinSpareLOut3 = 17; //low current output spare3 PLACEHOLDER value for now
      pinNumbers.pinSpareLOut4 = 21; //low current output spare4 PLACEHOLDER value for now
      pinNumbers.pinFan = 12; //Pin for the fan output
      pinNumbers.pinResetControl = 46; //Reset control output PLACEHOLDER value for now
    break;
    
    case 45:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings for the DIY-EFI CORE4 Module. This is an AVR only module
      #if defined(CORE_AVR)
      pinNumbers.pinInjector1 = 10; //Output pin injector 1 is on
      pinNumbers.pinInjector2 = 11; //Output pin injector 2 is on
      pinNumbers.pinInjector3 = 12; //Output pin injector 3 is on
      pinNumbers.pinInjector4 = 9; //Output pin injector 4 is on
      pinNumbers.pinCoil1 = 39; //Pin for coil 1
      pinNumbers.pinCoil2 = 29; //Pin for coil 2
      pinNumbers.pinCoil3 = 28; //Pin for coil 3
      pinNumbers.pinCoil4 = 27; //Pin for coil 4
      pinNumbers.pinCoil5 = 26; //Placeholder  for coil 5
      pinNumbers.pinTrigger = 19; //The CAS pin
      pinNumbers.pinTrigger2 = 18; //The Cam Sensor pin
      pinNumbers.pinTrigger3 = 21;// The Cam sensor 2 pin
      pinNumbers.pinFlex = 20; // Flex sensor
      pinNumbers.pinTPS = A3; //TPS input pin
      pinNumbers.pinMAP = A2; //MAP sensor pin
      pinNumbers.pinBaro = A15; //Baro sensor pin
      pinNumbers.pinIAT = A11; //IAT sensor pin
      pinNumbers.pinCLT = A4; //CLS sensor pin
      pinNumbers.pinO2 = A12; //O2 Sensor pin
      pinNumbers.pinO2_2 = A5; //O2 sensor pin (second sensor)
      pinNumbers.pinBat = A1; //Battery reference voltage pin
      pinNumbers.pinSpareTemp1 = A14; //spare Analog input 1
      pinNumbers.pinLaunch = 24; //Can be overwritten below
      pinNumbers.pinDisplayReset = 48; // OLED reset pin PLACEHOLDER value for now
      pinNumbers.pinTachOut = 38; //Tacho output pin
      pinNumbers.pinIdle1 = 42; //Single wire idle control
      pinNumbers.pinIdle2 = 43; //2 wire idle control
      pinNumbers.pinFuelPump = 41; //Fuel pump output
      pinNumbers.pinVVT_1 = 44; //Default VVT output
      pinNumbers.pinVVT_2 = 48; //Default VVT2 output
      pinNumbers.pinStepperDir = 32; //Direction pin  for DRV8825 driver
      pinNumbers.pinStepperStep = 31; //Step pin for DRV8825 driver
      pinNumbers.pinStepperEnable = 30; //Enable pin for DRV8825 driver
      pinNumbers.pinBoost = 45; //Boost control
      pinNumbers.pinSpareLOut1 = 37; //low current output spare1
      pinNumbers.pinSpareLOut2 = 36; //low current output spare2
      pinNumbers.pinSpareLOut3 = 35; //low current output spare3
      pinNumbers.pinInjector5 = 33; //Output pin injector 5 is on
      pinNumbers.pinInjector6 = 34; //Output pin injector 6 is on
      pinNumbers.pinFan = 40; //Pin for the fan output
      pinNumbers.pinResetControl = 46; //Reset control output PLACEHOLDER value for now
      #endif
    #endif
      break;

    #if defined(CORE_TEENSY35)
    case 50:
      //Pin mappings as per the teensy rev A shield
      pinNumbers.pinInjector1 = 2; //Output pin injector 1 is on
      pinNumbers.pinInjector2 = 10; //Output pin injector 2 is on
      pinNumbers.pinInjector3 = 6; //Output pin injector 3 is on
      pinNumbers.pinInjector4 = 9; //Output pin injector 4 is on
      //Placeholder only - NOT USED:
      //pinNumbers.pinInjector5 = 13;
      pinNumbers.pinCoil1 = 29; //Pin for coil 1
      pinNumbers.pinCoil2 = 30; //Pin for coil 2
      pinNumbers.pinCoil3 = 31; //Pin for coil 3 - ONLY WITH DB2
      pinNumbers.pinCoil4 = 32; //Pin for coil 4 - ONLY WITH DB2
      //Placeholder only - NOT USED:
      //pinNumbers.pinCoil5 = 46; 
      pinNumbers.pinTrigger = 23; //The CAS pin
      pinNumbers.pinTrigger2 = 36; //The Cam Sensor pin
      pinNumbers.pinTPS = 16; //TPS input pin
      pinNumbers.pinMAP = 17; //MAP sensor pin
      pinNumbers.pinIAT = 14; //IAT sensor pin
      pinNumbers.pinCLT = 15; //CLT sensor pin
      pinNumbers.pinO2 = A22; //O2 sensor pin
      pinNumbers.pinO2_2 = A21; //O2 sensor pin (second sensor)
      pinNumbers.pinBat = 18; //Battery reference voltage pin
      pinNumbers.pinTachOut = 20; //Tacho output pin
      pinNumbers.pinIdle1 = 5; //Single wire idle control
      pinNumbers.pinBoost = 11; //Boost control
      pinNumbers.pinFuelPump = 38; //Fuel pump output
      pinNumbers.pinStepperDir = 34; //Direction pin for DRV8825 driver
      pinNumbers.pinStepperStep = 35; //Step pin for DRV8825 driver
      pinNumbers.pinStepperEnable = 33; //Enable pin for DRV8825 driver
      pinNumbers.pinLaunch = 26; //Can be overwritten below
      pinNumbers.pinFan = 37; //Pin for the fan output - ONLY WITH DB
      pinNumbers.pinSpareHOut1 = 8; // high current output spare1
      pinNumbers.pinSpareHOut2 = 7; // high current output spare2
      pinNumbers.pinSpareLOut1 = 21; //low current output spare1
      break;

    case 51:
      //Pin mappings as per the teensy revB board shield
      pinNumbers.pinInjector1 = 2; //Output pin injector 1 is on
      pinNumbers.pinInjector2 = 10; //Output pin injector 2 is on
      pinNumbers.pinInjector3 = 6; //Output pin injector 3 is on - NOT USED
      pinNumbers.pinInjector4 = 9; //Output pin injector 4 is on - NOT USED
      pinNumbers.pinCoil1 = 29; //Pin for coil 1
      pinNumbers.pinCoil2 = 30; //Pin for coil 2
      pinNumbers.pinCoil3 = 31; //Pin for coil 3 - ONLY WITH DB2
      pinNumbers.pinCoil4 = 32; //Pin for coil 4 - ONLY WITH DB2
      pinNumbers.pinTrigger = 23; //The CAS pin
      pinNumbers.pinTrigger2 = 36; //The Cam Sensor pin
      pinNumbers.pinTPS = 16; //TPS input pin
      pinNumbers.pinMAP = 17; //MAP sensor pin
      pinNumbers.pinIAT = 14; //IAT sensor pin
      pinNumbers.pinCLT = 15; //CLT sensor pin
      pinNumbers.pinO2 = A22; //O2 sensor pin
      pinNumbers.pinO2_2 = A21; //O2 sensor pin (second sensor)
      pinNumbers.pinBat = 18; //Battery reference voltage pin
      pinNumbers.pinTachOut = 20; //Tacho output pin
      pinNumbers.pinIdle1 = 5; //Single wire idle control
      pinNumbers.pinBoost = 11; //Boost control
      pinNumbers.pinFuelPump = 38; //Fuel pump output
      pinNumbers.pinStepperDir = 34; //Direction pin for DRV8825 driver
      pinNumbers.pinStepperStep = 35; //Step pin for DRV8825 driver
      pinNumbers.pinStepperEnable = 33; //Enable pin for DRV8825 driver
      pinNumbers.pinLaunch = 26; //Can be overwritten below
      pinNumbers.pinFan = 37; //Pin for the fan output - ONLY WITH DB
      pinNumbers.pinSpareHOut1 = 8; // high current output spare1
      pinNumbers.pinSpareHOut2 = 7; // high current output spare2
      pinNumbers.pinSpareLOut1 = 21; //low current output spare1
      break;
    #endif

    #if defined(CORE_TEENSY35)
    case 53:
      //Pin mappings for the Juice Box (ignition only board)
      pinNumbers.pinInjector1 = 2; //Output pin injector 1 is on - NOT USED
      pinNumbers.pinInjector2 = 56; //Output pin injector 2 is on - NOT USED
      pinNumbers.pinInjector3 = 6; //Output pin injector 3 is on - NOT USED
      pinNumbers.pinInjector4 = 50; //Output pin injector 4 is on - NOT USED
      pinNumbers.pinCoil1 = 29; //Pin for coil 1
      pinNumbers.pinCoil2 = 30; //Pin for coil 2
      pinNumbers.pinCoil3 = 31; //Pin for coil 3
      pinNumbers.pinCoil4 = 32; //Pin for coil 4
      pinNumbers.pinTrigger = 37; //The CAS pin
      pinNumbers.pinTrigger2 = 38; //The Cam Sensor pin - NOT USED
      pinNumbers.pinTPS = A2; //TPS input pin
      pinNumbers.pinMAP = A7; //MAP sensor pin
      pinNumbers.pinIAT = A1; //IAT sensor pin
      pinNumbers.pinCLT = A5; //CLT sensor pin
      pinNumbers.pinO2 = A0; //O2 sensor pin
      pinNumbers.pinO2_2 = A21; //O2 sensor pin (second sensor) - NOT USED
      pinNumbers.pinBat = A6; //Battery reference voltage pin
      pinNumbers.pinTachOut = 28; //Tacho output pin
      pinNumbers.pinIdle1 = 5; //Single wire idle control - NOT USED
      pinNumbers.pinBoost = 11; //Boost control - NOT USED
      pinNumbers.pinFuelPump = 24; //Fuel pump output
      pinNumbers.pinStepperDir = 3; //Direction pin for DRV8825 driver - NOT USED
      pinNumbers.pinStepperStep = 4; //Step pin for DRV8825 driver - NOT USED
      pinNumbers.pinStepperEnable = 6; //Enable pin for DRV8825 driver - NOT USED
      pinNumbers.pinLaunch = 26; //Can be overwritten below
      pinNumbers.pinFan = 25; //Pin for the fan output
      pinNumbers.pinSpareHOut1 = 26; // high current output spare1
      pinNumbers.pinSpareHOut2 = 27; // high current output spare2
      pinNumbers.pinSpareLOut1 = 55; //low current output spare1 - NOT USED
      break;
    #endif

    case 55:
      #if defined(CORE_TEENSY)
      //Pin mappings for the DropBear
      injControlMode = InjIoControlMode::MC33810;
      ignControlMode = IgnIoControlMode::MC33810;

      //The injector pins below are not used directly as the control is via SPI through the MC33810s, however the pin numbers are set to be the SPI pins (SCLK, MOSI, MISO and CS) so that nothing else will set them as inputs
      pinNumbers.pinInjector1 = 13; //SCLK
      pinNumbers.pinInjector2 = 11; //MOSI
      pinNumbers.pinInjector3 = 12; //MISO
      pinNumbers.pinInjector4 = 10; //CS for MC33810 1
      pinNumbers.pinInjector5 = 9; //CS for MC33810 2
      pinNumbers.pinInjector6 = 9; //CS for MC33810 3

      //Dummy pins, without these pin 0 (Serial1 RX) gets overwritten
      pinNumbers.pinCoil1 = 40;
      pinNumbers.pinCoil2 = 41;
      /*
      pinNumbers.pinCoil3 = 55;
      pinNumbers.pinCoil4 = 55;
      pinNumbers.pinCoil5 = 55;
      pinNumbers.pinCoil6 = 55;
      */
      
      pinNumbers.pinTrigger = 19; //The CAS pin
      pinNumbers.pinTrigger2 = 18; //The Cam Sensor pin
      pinNumbers.pinTrigger3 = 22; //Uses one of the protected spare digital inputs. This must be set or Serial1 (Pin 0) gets broken
      pinNumbers.pinFlex = A16; // Flex sensor
      pinNumbers.pinMAP = A1; //MAP sensor pin
      pinNumbers.pinBaro = A0; //Baro sensor pin
      pinNumbers.pinBat = A14; //Battery reference voltage pin
      pinNumbers.pinSpareTemp1 = A17; //spare Analog input 1
      pinNumbers.pinLaunch = A15; //Can be overwritten below
      pinNumbers.pinTachOut = 5; //Tacho output pin
      pinNumbers.pinIdle1 = 27; //Single wire idle control
      pinNumbers.pinIdle2 = 29; //2 wire idle control. Shared with Spare 1 output
      pinNumbers.pinFuelPump = 8; //Fuel pump output
      pinNumbers.pinVVT_1 = 28; //Default VVT output
      pinNumbers.pinStepperDir = 32; //Direction pin  for DRV8825 driver
      pinNumbers.pinStepperStep = 31; //Step pin for DRV8825 driver
      pinNumbers.pinStepperEnable = 30; //Enable pin for DRV8825 driver
      pinNumbers.pinBoost = 24; //Boost control
      pinNumbers.pinSpareLOut1 = 29; //low current output spare1
      pinNumbers.pinSpareLOut2 = 26; //low current output spare2
      pinNumbers.pinSpareLOut3 = 28; //low current output spare3
      pinNumbers.pinSpareLOut4 = 29; //low current output spare4
      pinNumbers.pinFan = 25; //Pin for the fan output
      pinNumbers.pinResetControl = 46; //Reset control output PLACEHOLDER value for now
      pinNumbers.pinVSS = 22;

      pinNumbers.pinWMIEmpty = 23; //Spare digital input
      pinNumbers.pinWMIIndicator = pinNumbers.pinSpareLOut2; //Spare output
      pinNumbers.pinWMIEnabled = pinNumbers.pinSpareLOut1; //Spare output

      //CS pin number is now set in a compile flag. 
      // #ifdef USE_SPI_EEPROM
      //   pinSPIFlash_CS = 6;
      // #endif

      #if defined(CORE_TEENSY35)
        pinNumbers.pinTPS = A22; //TPS input pin
        pinNumbers.pinIAT = A19; //IAT sensor pin
        pinNumbers.pinCLT = A20; //CLS sensor pin
        pinNumbers.pinO2 = A21; //O2 Sensor pin
        pinNumbers.pinO2_2 = A18; //Spare 2
      #endif

      #if defined(CORE_TEENSY41)
        //New pins for the actual T4.1 version of the Dropbear
        pinNumbers.pinBaro = A4; 
        pinNumbers.pinMAP = A5;
        pinNumbers.pinTPS = A3; //TPS input pin
        pinNumbers.pinIAT = A0; //IAT sensor pin
        pinNumbers.pinCLT = A1; //CLS sensor pin
        pinNumbers.pinO2 = A2; //O2 Sensor pin
        pinNumbers.pinBat = A15; //Battery reference voltage pin. Needs Alpha4+
        pinNumbers.pinLaunch = 36;
        pinNumbers.pinFlex = 37; // Flex sensor
        pinNumbers.pinSpareTemp1 = A16; 
        pinNumbers.pinSpareTemp2 = A17;

        pinNumbers.pinTrigger = 20; //The CAS pin
        pinNumbers.pinTrigger2 = 21; //The Cam Sensor pin
        pinNumbers.pinTrigger3 = 34; //Uses one of the protected spare digital inputs.

        pinNumbers.pinFuelPump = 5; //Fuel pump output
        pinNumbers.pinTachOut = 0; //Tacho output pin

        pinNumbers.pinResetControl = 49; //PLaceholder only. Cannot use 42-47 as these are the SD card
        pinNumbers.pinWMIEmpty = 35; //Spare digital input
        pinNumbers.pinVSS = 34;

        //CS pin number is now set in a compile flag. 
        // #ifdef USE_SPI_EEPROM
        //   pinSPIFlash_CS = 33;
        // #endif

      #endif

        pinNumbers.pinMC33810_1_CS = 10;
        pinNumbers.pinMC33810_2_CS = 9;

      //Pin alignment to the MC33810 outputs
      pinNumbers.mc33810InjBits[0] = 3;
      pinNumbers.mc33810InjBits[1] = 1;
      pinNumbers.mc33810InjBits[2] = 0;
      pinNumbers.mc33810InjBits[3] = 2;
      pinNumbers.mc33810IgnBits[0] = 4;
      pinNumbers.mc33810IgnBits[1] = 5;
      pinNumbers.mc33810IgnBits[2] = 6;
      pinNumbers.mc33810IgnBits[3] = 7;

      pinNumbers.mc33810InjBits[4] = 3;
      pinNumbers.mc33810InjBits[5] = 1;
      pinNumbers.mc33810InjBits[6] = 0;
      pinNumbers.mc33810InjBits[7] = 2;
      pinNumbers.mc33810IgnBits[4] = 4;
      pinNumbers.mc33810IgnBits[5] = 5;
      pinNumbers.mc33810IgnBits[6] = 6;
      pinNumbers.mc33810IgnBits[7] = 7;



      #endif
      break;

    case 56:
      #if defined(CORE_TEENSY)
      //Pin mappings for the Bear Cub (Teensy 4.1)
      pinNumbers.pinInjector1 = 6;
      pinNumbers.pinInjector2 = 7;
      pinNumbers.pinInjector3 = 9;
      pinNumbers.pinInjector4 = 8;
      pinNumbers.pinInjector5 = 0; //Not used
      pinNumbers.pinCoil1 = 2;
      pinNumbers.pinCoil2 = 3;
      pinNumbers.pinCoil3 = 4;
      pinNumbers.pinCoil4 = 5;

      pinNumbers.pinTrigger = 20; //The CAS pin
      pinNumbers.pinTrigger2 = 21; //The Cam Sensor pin
      pinNumbers.pinFlex = 37; // Flex sensor
      pinNumbers.pinMAP = A5; //MAP sensor pin
      pinNumbers.pinBaro = A4; //Baro sensor pin
      pinNumbers.pinBat = A15; //Battery reference voltage pin
      pinNumbers.pinTPS = A3; //TPS input pin
      pinNumbers.pinIAT = A0; //IAT sensor pin
      pinNumbers.pinCLT = A1; //CLS sensor pin
      pinNumbers.pinO2 = A2; //O2 Sensor pin
      pinNumbers.pinLaunch = 36;

      pinNumbers.pinSpareTemp1 = A16; //spare Analog input 1
      pinNumbers.pinSpareTemp2 = A17; //spare Analog input 2
      pinNumbers.pinTachOut = 38; //Tacho output pin
      pinNumbers.pinIdle1 = 27; //Single wire idle control
      pinNumbers.pinIdle2 = 26; //2 wire idle control. Shared with Spare 1 output
      pinNumbers.pinFuelPump = 10; //Fuel pump output
      pinNumbers.pinVVT_1 = 28; //Default VVT output
      pinNumbers.pinStepperDir = 32; //Direction pin  for DRV8825 driver
      pinNumbers.pinStepperStep = 31; //Step pin for DRV8825 driver
      pinNumbers.pinStepperEnable = 30; //Enable pin for DRV8825 driver
      pinNumbers.pinBoost = 24; //Boost control
      pinNumbers.pinSpareLOut1 = 29; //low current output spare1
      pinNumbers.pinSpareLOut2 = 26; //low current output spare2
      pinNumbers.pinSpareLOut3 = 28; //low current output spare3
      pinNumbers.pinSpareLOut4 = 29; //low current output spare4
      pinNumbers.pinFan = 25; //Pin for the fan output
      pinNumbers.pinResetControl = 46; //Reset control output PLACEHOLDER value for now

      #endif
      break;
    
 
    case 60:
        #if defined(STM32F407xx)
        //Pin definitions for experimental board Tjeerd 
        //Black F407VE wiki.stm32duino.com/index.php?title=STM32F407
        //https://github.com/Tjeerdie/SPECTRE/tree/master/SPECTRE_V0.5
        
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
        pinNumbers.pinCoil3 = PA8;
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
        pinNumbers.pinBaro = PB1; //ADC12
        // = PB2;  //(DO NOT USE FOR SPEEDUINO) BOOT1 
        // = PB3;  //(DO NOT USE FOR SPEEDUINO) SPI1_SCK FLASH CHIP
        // = PB4;  //(DO NOT USE FOR SPEEDUINO) SPI1_MISO FLASH CHIP
        // = PB5;  //(DO NOT USE FOR SPEEDUINO) SPI1_MOSI FLASH CHIP
        // = PB6;  //NRF_CE
        pinNumbers.pinCoil6 = PB7;  //NRF_CS
        // = PB8;  //NRF_IRQ
        pinNumbers.pinCoil2 = PB9; //
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
        pinNumbers.pinIAT = PC0; //ADC123 
        pinNumbers.pinTPS = PC1; //ADC123
        pinNumbers.pinMAP = PC2; //ADC123 
        pinNumbers.pinCLT = PC3; //ADC123
        pinNumbers.pinO2 = PC4; //ADC12
        pinNumbers.pinBat = PC5;  //ADC12
        pinNumbers.pinBoost = PC6; //
        pinNumbers.pinIdle1 = PC7; //
        // = PC8;  //(DO NOT USE FOR SPEEDUINO) - SDIO_D0
        // = PC9;  //(DO NOT USE FOR SPEEDUINO) - SDIO_D1
        // = PC10;  //(DO NOT USE FOR SPEEDUINO) - SDIO_D2
        // = PC11;  //(DO NOT USE FOR SPEEDUINO) - SDIO_D3
        // = PC12;  //(DO NOT USE FOR SPEEDUINO) - SDIO_SCK
        pinNumbers.pinTachOut = PC13; //
        // = PC14;  //(DO NOT USE FOR SPEEDUINO) - OSC32_IN
        // = PC15;  //(DO NOT USE FOR SPEEDUINO) - OSC32_OUT

        //******************************************
        //******** PORTD CONNECTIONS *************** 
        //******************************************
        // = PD0;  //CANRX
        // = PD1;  //CANTX
        // = PD2;  //(DO NOT USE FOR SPEEDUINO) - SDIO_CMD
        pinNumbers.pinIdle2 = PD3; //
        // = PD4;  //
        pinNumbers.pinFlex = PD4;
        // = PD5; //TXD2
        // = PD6;  //RXD2
        pinNumbers.pinCoil1 = PD7; //
        // = PD7;  //
        // = PD8;  //
        pinNumbers.pinCoil5 = PD9;//
        pinNumbers.pinCoil4 = PD10;//
        // = PD11;  //
        pinNumbers.pinInjector1 = PD12; //
        pinNumbers.pinInjector2 = PD13; //
        pinNumbers.pinInjector3 = PD14; //
        pinNumbers.pinInjector4 = PD15; //

        //******************************************
        //******** PORTE CONNECTIONS *************** 
        //******************************************
        pinNumbers.pinTrigger = PE0; //
        pinNumbers.pinTrigger2 = PE1; //
        pinNumbers.pinStepperEnable = PE2; //
        pinNumbers.pinFuelPump = PE3; //ONBOARD KEY1
        // = PE4;  //ONBOARD KEY2
        pinNumbers.pinStepperStep = PE5; //
        pinNumbers.pinFan = PE6; //
        pinNumbers.pinStepperDir = PE7; //
        // = PE8;  //
        pinNumbers.pinInjector5 = PE9; //
        // = PE10;  //
        pinNumbers.pinInjector6 = PE11; //
        // = PE12; //
        pinNumbers.pinInjector8 = PE13; //
        pinNumbers.pinInjector7 = PE14; //
        // = PE15;  //
     #elif (defined(STM32F411xE) || defined(STM32F401xC))
        //pins PA12, PA11 are used for USB or CAN couldn't be used for GPIO
        //PB2 can't be used as input because is BOOT pin
        pinNumbers.pinInjector1 = PB7; //Output pin injector 1 is on
        pinNumbers.pinInjector2 = PB6; //Output pin injector 2 is on
        pinNumbers.pinInjector3 = PB5; //Output pin injector 3 is on
        pinNumbers.pinInjector4 = PB4; //Output pin injector 4 is on
        pinNumbers.pinCoil1 = PB9; //Pin for coil 1
        pinNumbers.pinCoil2 = PB8; //Pin for coil 2
        pinNumbers.pinCoil3 = PB3; //Pin for coil 3
        pinNumbers.pinCoil4 = PA15; //Pin for coil 4
        pinNumbers.pinTPS = A2;//TPS input pin
        pinNumbers.pinMAP = A3; //MAP sensor pin
        pinNumbers.pinIAT = A0; //IAT sensor pin
        pinNumbers.pinCLT = A1; //CLS sensor pin
        pinNumbers.pinO2 = A8; //O2 Sensor pin
        pinNumbers.pinBat = A4; //Battery reference voltage pin
        pinNumbers.pinBaro = pinNumbers.pinMAP;
        pinNumbers.pinTachOut = PB1; //Tacho output pin  (Goes to ULN2803)
        pinNumbers.pinIdle1 = PB2; //Single wire idle control
        pinNumbers.pinIdle2 = PB10; //2 wire idle control
        pinNumbers.pinBoost = PA6; //Boost control
        pinNumbers.pinStepperDir = PB10; //Direction pin  for DRV8825 driver
        pinNumbers.pinStepperStep = PB2; //Step pin for DRV8825 driver
        pinNumbers.pinFuelPump = PA8; //Fuel pump output
        pinNumbers.pinFan = PA5; //Pin for the fan output (Goes to ULN2803)

        //external interrupt enabled pins
        pinNumbers.pinFlex = PC14; // Flex sensor (Must be external interrupt enabled)
        pinNumbers.pinTrigger = PC13; //The CAS pin also led pin so bad idea
        pinNumbers.pinTrigger2 = PC15; //The Cam Sensor pin

     #elif defined(CORE_STM32)
        //blue pill wiki.stm32duino.com/index.php?title=Blue_Pill
        //Maple mini wiki.stm32duino.com/index.php?title=Maple_Mini
        //pins PA12, PA11 are used for USB or CAN couldn't be used for GPIO
        //PB2 can't be used as input because is BOOT pin
        pinNumbers.pinInjector1 = PB7; //Output pin injector 1 is on
        pinNumbers.pinInjector2 = PB6; //Output pin injector 2 is on
        pinNumbers.pinInjector3 = PB5; //Output pin injector 3 is on
        pinNumbers.pinInjector4 = PB4; //Output pin injector 4 is on
        pinNumbers.pinCoil1 = PB3; //Pin for coil 1
        pinNumbers.pinCoil2 = PA15; //Pin for coil 2
        pinNumbers.pinCoil3 = PA14; //Pin for coil 3
        pinNumbers.pinCoil4 = PA9; //Pin for coil 4
        pinNumbers.pinCoil5 = PA8; //Pin for coil 5
        pinNumbers.pinTPS = A0; //TPS input pin
        pinNumbers.pinMAP = A1; //MAP sensor pin
        pinNumbers.pinIAT = A2; //IAT sensor pin
        pinNumbers.pinCLT = A3; //CLS sensor pin
        pinNumbers.pinO2 = A4; //O2 Sensor pin
        pinNumbers.pinBat = A5; //Battery reference voltage pin
        pinNumbers.pinBaro = pinNumbers.pinMAP;
        pinNumbers.pinIdle1 = PB2; //Single wire idle control
        pinNumbers.pinIdle2 = PA2; //2 wire idle control
        pinNumbers.pinBoost = PA1; //Boost control
        pinNumbers.pinVVT_1 = PA0; //Default VVT output
        pinNumbers.pinVVT_2 = PA2; //Default VVT2 output
        pinNumbers.pinStepperDir = PC15; //Direction pin  for DRV8825 driver
        pinNumbers.pinStepperStep = PC14; //Step pin for DRV8825 driver
        pinNumbers.pinStepperEnable = PC13; //Enable pin for DRV8825
        pinNumbers.pinDisplayReset = PB2; // OLED reset pin
        pinNumbers.pinFan = PB1; //Pin for the fan output
        pinNumbers.pinFuelPump = PB11; //Fuel pump output
        pinNumbers.pinTachOut = PB10; //Tacho output pin
        //external interrupt enabled pins
        pinNumbers.pinFlex = PB8; // Flex sensor (Must be external interrupt enabled)
        pinNumbers.pinTrigger = PA10; //The CAS pin
        pinNumbers.pinTrigger2 = PA13; //The Cam Sensor pin
      
    #endif
      break;

    case 61:
    #if defined(BOARD_FCR_MICRO_F4)
        pinNumbers.pinIAT       = PA0;
        pinNumbers.pinCLT       = PA1;
        pinNumbers.pinTPS       = PA2;
        pinNumbers.pinBat       = PA4;
        pinNumbers.pinMAP       = PA5;
        pinNumbers.pinBaro      = PA6;
        pinNumbers.pinO2        = PB0;
        pinNumbers.pinTrigger   = PB1;
        pinNumbers.pinTrigger2  = PA7;
        pinNumbers.pinCoil1     = PE10;
        pinNumbers.pinCoil2     = PE11;
        pinNumbers.pinInjector1 = PC6;
        pinNumbers.pinInjector2 = PC7;
        pinNumbers.pinIdle1     = PD2;
        pinNumbers.pinFan       = PD3;
        pinNumbers.pinFuelPump  = PD4;
        pinNumbers.pinTachOut   = PC9;
    #endif
      break;

    default:
      #if defined(STM32F407xx)
      //Pin definitions for experimental board Tjeerd 
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
        pinNumbers.pinFuelPump = PA6; //ADC12 LED_BUILTIN_1
        /* = PA7; */ //ADC12 LED_BUILTIN_2
        pinNumbers.pinCoil3 = PA8;
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
        pinNumbers.pinBaro = PB1; //ADC12
        /* = PB2; */ //(DO NOT USE FOR SPEEDUINO) BOOT1 
        /* = PB3; */ //(DO NOT USE FOR SPEEDUINO) SPI1_SCK FLASH CHIP
        /* = PB4; */ //(DO NOT USE FOR SPEEDUINO) SPI1_MISO FLASH CHIP
        /* = PB5; */ //(DO NOT USE FOR SPEEDUINO) SPI1_MOSI FLASH CHIP
        /* = PB6; */ //NRF_CE
        /* = PB7; */ //NRF_CS
        /* = PB8; */ //NRF_IRQ
        pinNumbers.pinCoil2 = PB9; //
        /* = PB9; */ //
        pinNumbers.pinCoil4 = PB10; //TXD3
        pinNumbers.pinIdle1 = PB11; //RXD3
        pinNumbers.pinIdle2 = PB12; //
        /* pinNumbers.pinBoost = PB12; */ //
        /* = PB13; */ //SPI2_SCK
        /* = PB14; */ //SPI2_MISO
        /* = PB15; */ //SPI2_MOSI

        //******************************************
        //******** PORTC CONNECTIONS *************** 
        //******************************************
        pinNumbers.pinMAP = PC0; //ADC123 
        pinNumbers.pinTPS = PC1; //ADC123
        pinNumbers.pinIAT = PC2; //ADC123
        pinNumbers.pinCLT = PC3; //ADC123
        pinNumbers.pinO2 = PC4; //ADC12
        pinNumbers.pinBat = PC5; //ADC12
        /*pinNumbers.pinVVT_1 = PC6; */ //
        pinNumbers.pinDisplayReset = PC7; //
        /* = PC8; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D0
        /* = PC9; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D1
        /* = PC10; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D2
        /* = PC11; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D3
        /* = PC12; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_SCK
        pinNumbers.pinTachOut = PC13; //
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
        pinNumbers.pinFlex = PD4;
        /* = PD5;*/ //TXD2
        /* = PD6; */ //RXD2
        pinNumbers.pinCoil1 = PD7; //
        /* = PD7; */ //
        /* = PD8; */ //
        pinNumbers.pinCoil5 = PD9;//
        /* = PD10; */ //
        /* = PD11; */ //
        pinNumbers.pinInjector1 = PD12; //
        pinNumbers.pinInjector2 = PD13; //
        pinNumbers.pinInjector3 = PD14; //
        pinNumbers.pinInjector4 = PD15; //

        //******************************************
        //******** PORTE CONNECTIONS *************** 
        //******************************************
        pinNumbers.pinTrigger = PE0; //
        pinNumbers.pinTrigger2 = PE1; //
        pinNumbers.pinStepperEnable = PE2; //
        /* = PE3; */ //ONBOARD KEY1
        /* = PE4; */ //ONBOARD KEY2
        pinNumbers.pinStepperStep = PE5; //
        pinNumbers.pinFan = PE6; //
        pinNumbers.pinStepperDir = PE7; //
        /* = PE8; */ //
        /* = PE9; */ //
        /* = PE10; */ //
        pinNumbers.pinInjector5 = PE11; //
        pinNumbers.pinInjector6 = PE12; //
        /* = PE13; */ //
        /* = PE14; */ //
        /* = PE15; */ //
      #else
        #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
        //Pin mappings as per the v0.2 shield
        pinNumbers.pinInjector1 = 8; //Output pin injector 1 is on
        pinNumbers.pinInjector2 = 9; //Output pin injector 2 is on
        pinNumbers.pinInjector3 = 10; //Output pin injector 3 is on
        pinNumbers.pinInjector4 = 11; //Output pin injector 4 is on
        pinNumbers.pinInjector5 = 12; //Output pin injector 5 is on
        pinNumbers.pinCoil1 = 28; //Pin for coil 1
        pinNumbers.pinCoil2 = 24; //Pin for coil 2
        pinNumbers.pinCoil3 = 40; //Pin for coil 3
        pinNumbers.pinCoil4 = 36; //Pin for coil 4
        pinNumbers.pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
        pinNumbers.pinTrigger = 20; //The CAS pin
        pinNumbers.pinTrigger2 = 21; //The Cam Sensor pin
        pinNumbers.pinTPS = A2; //TPS input pin
        pinNumbers.pinMAP = A3; //MAP sensor pin
        pinNumbers.pinIAT = A0; //IAT sensor pin
        pinNumbers.pinCLT = A1; //CLS sensor pin
        #ifdef A8 //Bit hacky, but needed for the atmega2561
        pinNumbers.pinO2 = A8; //O2 Sensor pin
        #endif
        pinNumbers.pinBat = A4; //Battery reference voltage pin
        pinNumbers.pinStepperDir = 16; //Direction pin  for DRV8825 driver
        pinNumbers.pinStepperStep = 17; //Step pin for DRV8825 driver
        pinNumbers.pinDisplayReset = 48; // OLED reset pin
        pinNumbers.pinFan = 47; //Pin for the fan output
        pinNumbers.pinFuelPump = 4; //Fuel pump output
        pinNumbers.pinTachOut = 49; //Tacho output pin
        pinNumbers.pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
        pinNumbers.pinBoost = 5;
        pinNumbers.pinIdle1 = 6;
        pinNumbers.pinResetControl = 43; //Reset control output
        #endif
      #endif  
      break;
  }

  //Setup any devices that are using selectable pins

  if ( (configPage6.launchPin != 0) && (configPage6.launchPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinLaunch = pinTranslate(configPage6.launchPin); }
  if ( (configPage4.ignBypassPin != 0) && (configPage4.ignBypassPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinIgnBypass = pinTranslate(configPage4.ignBypassPin); }
  if ( (configPage2.tachoPin != 0) && (configPage2.tachoPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinTachOut = pinTranslate(configPage2.tachoPin); }
  if ( (configPage4.fuelPumpPin != 0) && (configPage4.fuelPumpPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinFuelPump = pinTranslate(configPage4.fuelPumpPin); }
  if ( (configPage6.fanPin != 0) && (configPage6.fanPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinFan = pinTranslate(configPage6.fanPin); }
  if ( (configPage6.boostPin != 0) && (configPage6.boostPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinBoost = pinTranslate(configPage6.boostPin); }
  if ( (configPage6.vvt1Pin != 0) && (configPage6.vvt1Pin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinVVT_1 = pinTranslate(configPage6.vvt1Pin); }
  if ( (configPage6.useExtBaro != 0) && (configPage6.baroPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinBaro = pinTranslateAnalog(configPage6.baroPin); }
  if ( (configPage6.useEMAP != 0) && (configPage10.EMAPPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinEMAP = pinTranslateAnalog(configPage10.EMAPPin); }
  if ( (configPage10.fuel2InputPin != 0) && (configPage10.fuel2InputPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinFuel2Input = pinTranslate(configPage10.fuel2InputPin); }
  if ( (configPage10.spark2InputPin != 0) && (configPage10.spark2InputPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinSpark2Input = pinTranslate(configPage10.spark2InputPin); }
  if ( (configPage2.vssPin != 0) && (configPage2.vssPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinVSS = pinTranslate(configPage2.vssPin); }
  if ( (configPage10.fuelPressureEnable) && (configPage10.fuelPressurePin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinFuelPressure = pinTranslateAnalog(configPage10.fuelPressurePin); }
  if ( (configPage10.oilPressureEnable) && (configPage10.oilPressurePin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinOilPressure = pinTranslateAnalog(configPage10.oilPressurePin); }
  
  if ( (configPage10.wmiEmptyPin != 0) && (configPage10.wmiEmptyPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinWMIEmpty = pinTranslate(configPage10.wmiEmptyPin); }
  if ( (configPage10.wmiIndicatorPin != 0) && (configPage10.wmiIndicatorPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinWMIIndicator = pinTranslate(configPage10.wmiIndicatorPin); }
  if ( (configPage10.wmiEnabledPin != 0) && (configPage10.wmiEnabledPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinWMIEnabled = pinTranslate(configPage10.wmiEnabledPin); }
  if ( (configPage10.vvt2Pin != 0) && (configPage10.vvt2Pin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinVVT_2 = pinTranslate(configPage10.vvt2Pin); }
  if ( (configPage13.onboard_log_trigger_Epin != 0 ) && (configPage13.onboard_log_trigger_Epin != 0) && (configPage13.onboard_log_tr5_Epin_pin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinSDEnable = pinTranslate(configPage13.onboard_log_tr5_Epin_pin); }
  

  //Currently there's no default pin for Idle Up
  
  pinNumbers.pinIdleUp = pinTranslate(configPage2.idleUpPin);

  //Currently there's no default pin for Idle Up Output
  pinNumbers.pinIdleUpOutput = pinTranslate(configPage2.idleUpOutputPin);

  //Currently there's no default pin for closed throttle position sensor
  pinNumbers.pinCTPS = pinTranslate(configPage2.CTPSPin);
  
  // Air conditioning control initialisation
  if ((configPage15.airConCompPin != 0) && (configPage15.airConCompPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinAirConComp = pinTranslate(configPage15.airConCompPin); }
  if ((configPage15.airConFanPin != 0) && (configPage15.airConFanPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinAirConFan = pinTranslate(configPage15.airConFanPin); }
  if ((configPage15.airConReqPin != 0) && (configPage15.airConReqPin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinAirConRequest = pinTranslate(configPage15.airConReqPin); }
    
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
  pinMode(pinNumbers.pinIdle1, OUTPUT);
  pinMode(pinNumbers.pinIdle2, OUTPUT);
  pinMode(pinNumbers.pinIdleUpOutput, OUTPUT);
  pinMode(pinNumbers.pinStepperDir, OUTPUT);
  pinMode(pinNumbers.pinStepperStep, OUTPUT);
  pinMode(pinNumbers.pinStepperEnable, OUTPUT);
  if(configPage4.ignBypassEnabled > 0) { pinMode(pinNumbers.pinIgnBypass, OUTPUT); }

  //This is a legacy mode option to revert the MAP reading behaviour to match what was in place prior to the 201905 firmware
  if(configPage2.legacyMAP > 0) { digitalWrite(pinNumbers.pinMAP, HIGH); }

  if(ignControlMode == IgnIoControlMode::Direct)
  {
    uint8_t ignPins[IGN_CHANNELS] = {
      pinNumbers.pinCoil1,
      #if (IGN_CHANNELS >= 2)
      pinNumbers.pinCoil2,
      #endif
      #if (IGN_CHANNELS >= 3)
      pinNumbers.pinCoil3,
      #endif
      #if (IGN_CHANNELS >= 4)
      pinNumbers.pinCoil4,
      #endif
      #if (IGN_CHANNELS >= 5)
      pinNumbers.pinCoil5,
      #endif
      #if (IGN_CHANNELS >= 6)
      pinNumbers.pinCoil6,
      #endif
      #if (IGN_CHANNELS >= 7)
      pinNumbers.pinCoil7,
      #endif
      #if (IGN_CHANNELS >= 8)
      pinNumbers.pinCoil8,
      #endif
    };
    initIgnDirectIO(configPage4, ignPins);
  } 

  if(injControlMode == InjIoControlMode::Direct)
  {
    uint8_t injPins[INJ_CHANNELS] = {
      pinNumbers.pinInjector1,
      #if (INJ_CHANNELS >= 2)
      pinNumbers.pinInjector2,
      #endif
      #if (INJ_CHANNELS >= 3)
      pinNumbers.pinInjector3,
      #endif
      #if (INJ_CHANNELS >= 4)
      pinNumbers.pinInjector4,
      #endif
      #if (INJ_CHANNELS >= 5)
      pinNumbers.pinInjector5,
      #endif
      #if (INJ_CHANNELS >= 6)
      pinNumbers.pinInjector6,
      #endif
      #if (INJ_CHANNELS >= 7)
      pinNumbers.pinInjector7,
      #endif
      #if (INJ_CHANNELS >= 8)
      pinNumbers.pinInjector8,
      #endif
    };
    initInjDirectIO(injPins);
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
      pinMode(pinNumbers.pinMAP, INPUT_ANALOG);
      pinMode(pinNumbers.pinO2, INPUT_ANALOG);
      pinMode(pinNumbers.pinO2_2, INPUT_ANALOG);
      pinMode(pinNumbers.pinTPS, INPUT_ANALOG);
      pinMode(pinNumbers.pinIAT, INPUT_ANALOG);
      pinMode(pinNumbers.pinCLT, INPUT_ANALOG);
      pinMode(pinNumbers.pinBat, INPUT_ANALOG);
      pinMode(pinNumbers.pinBaro, INPUT_ANALOG);
    #else
      pinMode(pinNumbers.pinMAP, INPUT);
      pinMode(pinNumbers.pinO2, INPUT);
      pinMode(pinNumbers.pinO2_2, INPUT);
      pinMode(pinNumbers.pinTPS, INPUT);
      pinMode(pinNumbers.pinIAT, INPUT);
      pinMode(pinNumbers.pinCLT, INPUT);
      pinMode(pinNumbers.pinBat, INPUT);
      pinMode(pinNumbers.pinBaro, INPUT);
    #endif
  #elif defined(CORE_TEENSY41)
    //Teensy 4.1 has a weak pull down resistor that needs to be disabled for all analog pinNumbers. 
    pinMode(pinNumbers.pinMAP, INPUT_DISABLE);
    pinMode(pinNumbers.pinO2, INPUT_DISABLE);
    pinMode(pinNumbers.pinO2_2, INPUT_DISABLE);
    pinMode(pinNumbers.pinTPS, INPUT_DISABLE);
    pinMode(pinNumbers.pinIAT, INPUT_DISABLE);
    pinMode(pinNumbers.pinCLT, INPUT_DISABLE);
    pinMode(pinNumbers.pinBat, INPUT_DISABLE);
    pinMode(pinNumbers.pinBaro, INPUT_DISABLE);
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
  if( (configPage2.idleUpEnabled > 0) && (!pinIsOutput(pinNumbers.pinIdleUp)) )
  {
    if (configPage2.idleUpPolarity == 0) { pinMode(pinNumbers.pinIdleUp, INPUT_PULLUP); } //Normal setting
    else { pinMode(pinNumbers.pinIdleUp, INPUT); } //inverted setting
  }
  if( (configPage2.CTPSEnabled > 0) && (!pinIsOutput(pinNumbers.pinCTPS)) )
  {
    if (configPage2.CTPSPolarity == 0) { pinMode(pinNumbers.pinCTPS, INPUT_PULLUP); } //Normal setting
    else { pinMode(pinNumbers.pinCTPS, INPUT); } //inverted setting
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
  if( (configPage10.fuelPressureEnable > 0)  && (!pinIsOutput(pinNumbers.pinFuelPressure)) )
  {
    pinMode(pinNumbers.pinFuelPressure, INPUT);
  }
  if( (configPage10.oilPressureEnable > 0) && (!pinIsOutput(pinNumbers.pinOilPressure)) )
  {
    pinMode(pinNumbers.pinOilPressure, INPUT);
  }
  if( (configPage13.onboard_log_trigger_Epin > 0) && (!pinIsOutput(pinNumbers.pinSDEnable)) )
  {
    pinMode(pinNumbers.pinSDEnable, INPUT);
  }
  if(configPage10.wmiEnabled > 0)
  {
    pinMode(pinNumbers.pinWMIEnabled, OUTPUT);
    if(configPage10.wmiIndicatorEnabled > 0)
    {
      pinMode(pinNumbers.pinWMIIndicator, OUTPUT);
      if (configPage10.wmiIndicatorPolarity > 0) { digitalWrite(pinNumbers.pinWMIIndicator, HIGH); }
    }
    if( (configPage10.wmiEmptyEnabled > 0) && (!pinIsOutput(pinNumbers.pinWMIEmpty)) )
    {
      if (configPage10.wmiEmptyPolarity == 0) { pinMode(pinNumbers.pinWMIEmpty, INPUT_PULLUP); } //Normal setting
      else { pinMode(pinNumbers.pinWMIEmpty, INPUT); } //inverted setting
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