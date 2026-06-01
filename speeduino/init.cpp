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

TESTABLE_STATIC pinNumbers_t getDefaultPinMapping(void)
{
  pinNumbers_t pins;

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
  pins.pinFuelPump = PA6; //ADC12 LED_BUILTIN_1
  /* = PA7; */ //ADC12 LED_BUILTIN_2
  pins.coilPins[2] = PA8;
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
  pins.pinBaro = PB1; //ADC12
  /* = PB2; */ //(DO NOT USE FOR SPEEDUINO) BOOT1 
  /* = PB3; */ //(DO NOT USE FOR SPEEDUINO) SPI1_SCK FLASH CHIP
  /* = PB4; */ //(DO NOT USE FOR SPEEDUINO) SPI1_MISO FLASH CHIP
  /* = PB5; */ //(DO NOT USE FOR SPEEDUINO) SPI1_MOSI FLASH CHIP
  /* = PB6; */ //NRF_CE
  /* = PB7; */ //NRF_CS
  /* = PB8; */ //NRF_IRQ
  pins.coilPins[1] = PB9; //
  /* = PB9; */ //
  pins.coilPins[3] = PB10; //TXD3
  pins.pinIdle1 = PB11; //RXD3
  pins.pinIdle2 = PB12; //
  /* pins.pinBoost = PB12; */ //
  /* = PB13; */ //SPI2_SCK
  /* = PB14; */ //SPI2_MISO
  /* = PB15; */ //SPI2_MOSI

  //******************************************
  //******** PORTC CONNECTIONS *************** 
  //******************************************
  pins.pinMAP = PC0; //ADC123 
  pins.pinTPS = PC1; //ADC123
  pins.pinIAT = PC2; //ADC123
  pins.pinCLT = PC3; //ADC123
  pins.pinO2 = PC4; //ADC12
  pins.pinBat = PC5; //ADC12
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
  pins.pinFlex = PD4;
  /* = PD5;*/ //TXD2
  /* = PD6; */ //RXD2
  pins.coilPins[0] = PD7; //
  /* = PD7; */ //
  /* = PD8; */ //
  pins.coilPins[4] = PD9;//
  /* = PD10; */ //
  /* = PD11; */ //
  pins.injectorPins[0] = PD12; //
  pins.injectorPins[1] = PD13; //
  pins.injectorPins[2] = PD14; //
  pins.injectorPins[3] = PD15; //

  //******************************************
  //******** PORTE CONNECTIONS *************** 
  //******************************************
  pins.pinTrigger = PE0; //
  pins.pinTrigger2 = PE1; //
  pins.pinStepperEnable = PE2; //
  /* = PE3; */ //ONBOARD KEY1
  /* = PE4; */ //ONBOARD KEY2
  pins.pinStepperStep = PE5; //
  pins.pinFan = PE6; //
  pins.pinStepperDir = PE7; //
  /* = PE8; */ //
  /* = PE9; */ //
  /* = PE10; */ //
  pins.injectorPins[4] = PE11; //
  pins.injectorPins[5] = PE12; //
  /* = PE13; */ //
  /* = PE14; */ //
  /* = PE15; */ //
#else
  #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
  //Pin mappings as per the v0.2 shield
  pins.injectorPins[0] = 8; //Output pin injector 1 is on
  pins.injectorPins[1] = 9; //Output pin injector 2 is on
  pins.injectorPins[2] = 10; //Output pin injector 3 is on
  pins.injectorPins[3] = 11; //Output pin injector 4 is on
  pins.injectorPins[4] = 12; //Output pin injector 5 is on
  pins.coilPins[0] = 28; //Pin for coil 1
  pins.coilPins[1] = 24; //Pin for coil 2
  pins.coilPins[2] = 40; //Pin for coil 3
  pins.coilPins[3] = 36; //Pin for coil 4
  pins.coilPins[4] = 34; //Pin for coil 5 PLACEHOLDER value for now
  pins.pinTrigger = 20; //The CAS pin
  pins.pinTrigger2 = 21; //The Cam Sensor pin
  pins.pinTPS = A2; //TPS input pin
  pins.pinMAP = A3; //MAP sensor pin
  pins.pinIAT = A0; //IAT sensor pin
  pins.pinCLT = A1; //CLS sensor pin
  pins.pinO2 = A8; //O2 Sensor pin
  pins.pinBat = A4; //Battery reference voltage pin
  pins.pinStepperDir = 16; //Direction pin  for DRV8825 driver
  pins.pinStepperStep = 17; //Step pin for DRV8825 driver
  pins.pinFan = 47; //Pin for the fan output
  pins.pinFuelPump = 4; //Fuel pump output
  pins.pinTachOut = 49; //Tacho output pin
  pins.pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
  pins.pinBoost = 5;
  pins.pinIdle1 = 6;
  pins.pinResetControl = 43; //Reset control output
  #endif
#endif  

  return pins;
}

TESTABLE_STATIC pinNumbers_t getPinMapping(uint8_t boardID)
{
  pinNumbers_t pins = getDefaultPinMapping();

  switch (boardID)
  {
    //Note: Case 0 (Speeduino v0.1) was removed in Nov 2020 to handle default case for blank FRAM modules

    case 1:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the v0.2 shield
      pins.injectorPins[0] = 8; //Output pin injector 1 is on
      pins.injectorPins[1] = 9; //Output pin injector 2 is on
      pins.injectorPins[2] = 10; //Output pin injector 3 is on
      pins.injectorPins[3] = 11; //Output pin injector 4 is on
      pins.injectorPins[4] = 12; //Output pin injector 5 is on
      pins.coilPins[0] = 28; //Pin for coil 1
      pins.coilPins[1] = 24; //Pin for coil 2
      pins.coilPins[2] = 40; //Pin for coil 3
      pins.coilPins[3] = 36; //Pin for coil 4
      pins.coilPins[4] = 34; //Pin for coil 5 PLACEHOLDER value for now
      pins.pinTrigger = 20; //The CAS pin
      pins.pinTrigger2 = 21; //The Cam Sensor pin
      pins.pinTrigger3 = 3; //The Cam sensor 2 pin
      pins.pinTPS = A2; //TPS input pin
      pins.pinMAP = A3; //MAP sensor pin
      pins.pinIAT = A0; //IAT sensor pin
      pins.pinCLT = A1; //CLS sensor pin
      pins.pinO2 = A8; //O2 Sensor pin
      pins.pinBat = A4; //Battery reference voltage pin
      pins.pinTachOut = 49; //Tacho output pin
      pins.pinIdle1 = 30; //Single wire idle control
      pins.pinIdle2 = 31; //2 wire idle control
      pins.pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pins.pinStepperStep = 17; //Step pin for DRV8825 driver
      pins.pinFan = 47; //Pin for the fan output
      pins.pinFuelPump = 4; //Fuel pump output
      pins.pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
      pins.pinResetControl = 43; //Reset control output
      break;
    #endif
    case 2:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the v0.3 shield
      pins.injectorPins[0] = 8; //Output pin injector 1 is on
      pins.injectorPins[1] = 9; //Output pin injector 2 is on
      pins.injectorPins[2] = 10; //Output pin injector 3 is on
      pins.injectorPins[3] = 11; //Output pin injector 4 is on
      pins.injectorPins[4] = 12; //Output pin injector 5 is on
      pins.coilPins[0] = 28; //Pin for coil 1
      pins.coilPins[1] = 24; //Pin for coil 2
      pins.coilPins[2] = 40; //Pin for coil 3
      pins.coilPins[3] = 36; //Pin for coil 4
      pins.coilPins[4] = 34; //Pin for coil 5 PLACEHOLDER value for now
      pins.pinTrigger = 19; //The CAS pin
      pins.pinTrigger2 = 18; //The Cam Sensor pin
      pins.pinTrigger3 = 3; //The Cam sensor 2 pin
      pins.pinTPS = A2;//TPS input pin
      pins.pinMAP = A3; //MAP sensor pin
      pins.pinIAT = A0; //IAT sensor pin
      pins.pinCLT = A1; //CLS sensor pin
      pins.pinO2 = A8; //O2 Sensor pin
      pins.pinBat = A4; //Battery reference voltage pin
      pins.pinTachOut = 49; //Tacho output pin
      pins.pinIdle1 = 5; //Single wire idle control
      pins.pinIdle2 = 53; //2 wire idle control
      pins.pinBoost = 7; //Boost control
      pins.pinVVT_1 = 6; //Default VVT output
      pins.pinVVT_2 = 48; //Default VVT2 output
      pins.pinFuelPump = 4; //Fuel pump output
      pins.pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pins.pinStepperStep = 17; //Step pin for DRV8825 driver
      pins.pinStepperEnable = 26; //Enable pin for DRV8825
      pins.pinFan = A13; //Pin for the fan output
      pins.pinLaunch = 51; //Can be overwritten below
      pins.pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
      pins.pinResetControl = 50; //Reset control output
      pins.pinBaro = A5;
      pins.pinVSS = 20;

      #if defined(CORE_TEENSY35)
        pins.pinTrigger = 23;
        pins.pinStepperDir = 33;
        pins.pinStepperStep = 34;
        pins.coilPins[0] = 31;
        pins.pinTachOut = 28;
        pins.pinFan = 27;
        pins.coilPins[3] = 21;
        pins.coilPins[2] = 30;
        pins.pinO2 = A22;
      #endif
    #endif
      break;

    case 3:
      //Pin mappings as per the v0.4 shield
      pins.injectorPins[0] = 8; //Output pin injector 1 is on
      pins.injectorPins[1] = 9; //Output pin injector 2 is on
      pins.injectorPins[2] = 10; //Output pin injector 3 is on
      pins.injectorPins[3] = 11; //Output pin injector 4 is on
      pins.injectorPins[4] = 12; //Output pin injector 5 is on
      pins.injectorPins[5] = 50; //CAUTION: Uses the same as Coil 4 below. 
      pins.coilPins[0] = 40; //Pin for coil 1
      pins.coilPins[1] = 38; //Pin for coil 2
      pins.coilPins[2] = 52; //Pin for coil 3
      pins.coilPins[3] = 50; //Pin for coil 4
      pins.coilPins[4] = 34; //Pin for coil 5 PLACEHOLDER value for now
      pins.pinTrigger = 19; //The CAS pin
      pins.pinTrigger2 = 18; //The Cam Sensor pin
      pins.pinTrigger3 = 3; //The Cam sensor 2 pin
      pins.pinTPS = A2;//TPS input pin
      pins.pinMAP = A3; //MAP sensor pin
      pins.pinIAT = A0; //IAT sensor pin
      pins.pinCLT = A1; //CLS sensor pin
      pins.pinO2 = A8; //O2 Sensor pin
      pins.pinBat = A4; //Battery reference voltage pin
      pins.pinTachOut = 49; //Tacho output pin  (Goes to ULN2803)
      pins.pinIdle1 = 5; //Single wire idle control
      pins.pinIdle2 = 6; //2 wire idle control
      pins.pinBoost = 7; //Boost control
      pins.pinVVT_1 = 4; //Default VVT output
      pins.pinVVT_2 = 48; //Default VVT2 output
      pins.pinFuelPump = 45; //Fuel pump output  (Goes to ULN2803)
      pins.pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pins.pinStepperStep = 17; //Step pin for DRV8825 driver
      pins.pinStepperEnable = 24; //Enable pin for DRV8825
      pins.pinFan = 47; //Pin for the fan output (Goes to ULN2803)
      pins.pinLaunch = 51; //Can be overwritten below
      pins.pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
      pins.pinResetControl = 43; //Reset control output
      pins.pinBaro = A5;
      pins.pinVSS = 20;
      pins.pinWMIEmpty = 46;
      pins.pinWMIIndicator = 44;
      pins.pinWMIEnabled = 42;

      #if defined(CORE_TEENSY35)
        pins.injectorPins[5] = 51;

        pins.pinTrigger = 23;
        pins.pinTrigger2 = 36;
        pins.pinStepperDir = 34;
        pins.pinStepperStep = 35;
        pins.coilPins[0] = 31;
        pins.coilPins[1] = 32;
        pins.pinTachOut = 28;
        pins.pinFan = 27;
        pins.coilPins[3] = 29;
        pins.coilPins[2] = 30;
        pins.pinO2 = A22;

        //Make sure the CAN pins aren't overwritten
        pins.pinTrigger3 = 54;
        pins.pinVVT_1 = 55;

      #elif defined(CORE_TEENSY41)
        //These are only to prevent lockups or weird behaviour on T4.1 when this board is used as the default
        pins.pinBaro = A4; 
        pins.pinMAP = A5;
        pins.pinTPS = A3; //TPS input pin
        pins.pinIAT = A0; //IAT sensor pin
        pins.pinCLT = A1; //CLS sensor pin
        pins.pinO2 = A2; //O2 Sensor pin
        pins.pinBat = A15; //Battery reference voltage pin. Needs Alpha4+
        pins.pinLaunch = 34; //Can be overwritten below
        pins.pinVSS = 35;

        pins.pinTrigger = 20; //The CAS pin
        pins.pinTrigger2 = 21; //The Cam Sensor pin
        pins.pinTrigger3 = 24;

        pins.pinStepperDir = 34;
        pins.pinStepperStep = 35;
        
        pins.coilPins[0] = 31;
        pins.coilPins[1] = 32;
        pins.coilPins[3] = 29;
        pins.coilPins[2] = 30;

        pins.pinTachOut = 28;
        pins.pinFan = 27;
        pins.pinFuelPump = 33;
        pins.pinWMIEmpty = 34;
        pins.pinWMIIndicator = 35;
        pins.pinWMIEnabled = 36;
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
        pins.pinFuelPump = PA7; //ADC12 LED_BUILTIN_2
        pins.coilPins[2] = PA8;
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
        pins.pinBaro = PB1; //ADC12
        /* = PB2; */ //(DO NOT USE FOR SPEEDUINO) BOOT1 
        /* = PB3; */ //(DO NOT USE FOR SPEEDUINO) SPI1_SCK FLASH CHIP
        /* = PB4; */ //(DO NOT USE FOR SPEEDUINO) SPI1_MISO FLASH CHIP
        /* = PB5; */ //(DO NOT USE FOR SPEEDUINO) SPI1_MOSI FLASH CHIP
        /* = PB6; */ //NRF_CE
        /* = PB7; */ //NRF_CS
        /* = PB8; */ //NRF_IRQ
        pins.coilPins[1] = PB9; //
        /* = PB9; */ //
        pins.coilPins[3] = PB10; //TXD3
        pins.pinIdle1 = PB11; //RXD3
        pins.pinIdle2 = PB12; //
        pins.pinBoost = PB12; //
        /* = PB13; */ //SPI2_SCK
        /* = PB14; */ //SPI2_MISO
        /* = PB15; */ //SPI2_MOSI

        //******************************************
        //******** PORTC CONNECTIONS *************** 
        //******************************************
        pins.pinMAP = PC0; //ADC123 
        pins.pinTPS = PC1; //ADC123
        pins.pinIAT = PC2; //ADC123
        pins.pinCLT = PC3; //ADC123
        pins.pinO2 = PC4;  //ADC12
        pins.pinBat = PC5; //ADC12
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
        pins.pinFlex = PD4;
        /* = PD5;*/ //TXD2
        /* = PD6; */ //RXD2
        pins.coilPins[0] = PD7; //
        /* = PD8; */ //
        pins.coilPins[4] = PD9;//
        /* = PD10; */ //
        /* = PD11; */ //
        pins.injectorPins[0] = PD12; //
        pins.injectorPins[1] = PD13; //
        pins.injectorPins[2] = PD14; //
        pins.injectorPins[3] = PD15; //

        //******************************************
        //******** PORTE CONNECTIONS *************** 
        //******************************************
        pins.pinTrigger = PE0; //
        pins.pinTrigger2 = PE1; //
        pins.pinStepperEnable = PE2; //
        /* = PE3; */ //ONBOARD KEY1
        /* = PE4; */ //ONBOARD KEY2
        pins.pinStepperStep = PE5; //
        pins.pinFan = PE6; //
        pins.pinStepperDir = PE7; //
        /* = PE8; */ //
        /* = PE9; */ //
        /* = PE10; */ //
        pins.injectorPins[4] = PE11; //
        pins.injectorPins[5] = PE12; //
        /* = PE13; */ //
        /* = PE14; */ //
        /* = PE15; */ //

      #elif defined(CORE_STM32)
        //https://github.com/stm32duino/Arduino_Core_STM32/blob/master/variants/Generic_F411Cx/variant.h#L28
        //pins PA12, PA11 are used for USB or CAN couldn't be used for GPIO
        //pins PB12, PB13, PB14 and PB15 are used to SPI FLASH
        //PB2 can't be used as input because it's the BOOT pin
        pins.injectorPins[0] = PB7; //Output pin injector 1 is on
        pins.injectorPins[1] = PB6; //Output pin injector 2 is on
        pins.injectorPins[2] = PB5; //Output pin injector 3 is on
        pins.injectorPins[3] = PB4; //Output pin injector 4 is on
        pins.coilPins[0] = PB9; //Pin for coil 1
        pins.coilPins[1] = PB8; //Pin for coil 2
        pins.coilPins[2] = PB3; //Pin for coil 3
        pins.coilPins[3] = PA15; //Pin for coil 4
        pins.pinTPS = A2;//TPS input pin
        pins.pinMAP = A3; //MAP sensor pin
        pins.pinIAT = A0; //IAT sensor pin
        pins.pinCLT = A1; //CLS sensor pin
        pins.pinO2 = A8; //O2 Sensor pin
        pins.pinBat = A4; //Battery reference voltage pin
        pins.pinBaro = pins.pinMAP;
        pins.pinTachOut = PB1; //Tacho output pin  (Goes to ULN2803)
        pins.pinIdle1 = PB2; //Single wire idle control
        pins.pinIdle2 = PB10; //2 wire idle control
        pins.pinBoost = PA6; //Boost control
        pins.pinStepperDir = PB10; //Direction pin  for DRV8825 driver
        pins.pinStepperStep = PB2; //Step pin for DRV8825 driver
        pins.pinFuelPump = PA8; //Fuel pump output
        pins.pinFan = PA5; //Pin for the fan output (Goes to ULN2803)
        //external interrupt enabled pins
        pins.pinFlex = PC14; // Flex sensor (Must be external interrupt enabled)
        pins.pinTrigger = PC13; //The CAS pin also led pin so bad idea
        pins.pinTrigger2 = PC15; //The Cam Sensor pin
      #endif
      break;

    case 6:
      #ifndef SMALL_FLASH_MODE
      //Pin mappings as per the 2001-05 MX5 PNP shield
      pins.injectorPins[0] = 44; //Output pin injector 1 is on
      pins.injectorPins[1] = 46; //Output pin injector 2 is on
      pins.injectorPins[2] = 47; //Output pin injector 3 is on
      pins.injectorPins[3] = 45; //Output pin injector 4 is on
      pins.injectorPins[4] = 14; //Output pin injector 5 is on
      pins.coilPins[0] = 42; //Pin for coil 1
      pins.coilPins[1] = 43; //Pin for coil 2
      pins.coilPins[2] = 32; //Pin for coil 3
      pins.coilPins[3] = 33; //Pin for coil 4
      pins.coilPins[4] = 34; //Pin for coil 5 PLACEHOLDER value for now
      pins.pinTrigger = 19; //The CAS pin
      pins.pinTrigger2 = 18; //The Cam Sensor pin
      pins.pinTrigger3 = 2; //The Cam sensor 2 pin
      pins.pinTPS = A2;//TPS input pin
      pins.pinMAP = A5; //MAP sensor pin
      pins.pinIAT = A0; //IAT sensor pin
      pins.pinCLT = A1; //CLS sensor pin
      pins.pinO2 = A3; //O2 Sensor pin
      pins.pinBat = A4; //Battery reference voltage pin
      pins.pinTachOut = 23; //Tacho output pin  (Goes to ULN2803)
      pins.pinIdle1 = 5; //Single wire idle control
      pins.pinBoost = 4;
      pins.pinVVT_1 = 11; //Default VVT output
      pins.pinVVT_2 = 48; //Default VVT2 output
      pins.pinIdle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
      pins.pinFuelPump = 40; //Fuel pump output
      pins.pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pins.pinStepperStep = 17; //Step pin for DRV8825 driver
      pins.pinStepperEnable = 24;
      pins.pinFan = 41; //Pin for the fan output
      pins.pinLaunch = 12; //Can be overwritten below
      pins.pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
      pins.pinResetControl = 39; //Reset control output
      pins.pinVSS = 2;
      #endif
      //This is NOT correct. It has not yet been tested with this board
      #if defined(CORE_TEENSY35)
        pins.pinTrigger = 23;
        pins.pinTrigger2 = 36;
        pins.pinStepperDir = 34;
        pins.pinStepperStep = 35;
        pins.coilPins[0] = 33; //Done
        pins.coilPins[1] = 24; //Done
        pins.coilPins[2] = 51; //Won't work (No mapping for pin 32)
        pins.coilPins[3] = 52; //Won't work (No mapping for pin 33)
        pins.pinFuelPump = 26; //Requires PVT4 adapter or above
        pins.pinFan = 50; //Won't work (No mapping for pin 35)
        pins.pinTachOut = 28; //Done
      #endif
      break;

    case 8:
      #ifndef SMALL_FLASH_MODE
      //Pin mappings as per the 1996-97 MX5 PNP shield
      pins.injectorPins[0] = 11; //Output pin injector 1 is on
      pins.injectorPins[1] = 10; //Output pin injector 2 is on
      pins.injectorPins[2] = 9; //Output pin injector 3 is on
      pins.injectorPins[3] = 8; //Output pin injector 4 is on
      pins.injectorPins[4] = 14; //Output pin injector 5 is on
      pins.coilPins[0] = 39; //Pin for coil 1
      pins.coilPins[1] = 41; //Pin for coil 2
      pins.coilPins[2] = 32; //Pin for coil 3
      pins.coilPins[3] = 33; //Pin for coil 4
      pins.coilPins[4] = 34; //Pin for coil 5 PLACEHOLDER value for now
      pins.pinTrigger = 19; //The CAS pin
      pins.pinTrigger2 = 18; //The Cam Sensor pin
      pins.pinTPS = A2;//TPS input pin
      pins.pinMAP = A5; //MAP sensor pin
      pins.pinIAT = A0; //IAT sensor pin
      pins.pinCLT = A1; //CLS sensor pin
      pins.pinO2 = A3; //O2 Sensor pin
      pins.pinBat = A4; //Battery reference voltage pin
      pins.pinTachOut = A9; //Tacho output pin  (Goes to ULN2803)
      pins.pinIdle1 = 2; //Single wire idle control
      pins.pinBoost = 4;
      pins.pinIdle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
      pins.pinFuelPump = 49; //Fuel pump output
      pins.pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pins.pinStepperStep = 17; //Step pin for DRV8825 driver
      pins.pinStepperEnable = 24;
      pins.pinFan = 35; //Pin for the fan output
      pins.pinLaunch = 37; //Can be overwritten below
      pins.pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
      pins.pinResetControl = 44; //Reset control output

      //This is NOT correct. It has not yet been tested with this board
      #if defined(CORE_TEENSY35)
        pins.pinTrigger = 23;
        pins.pinTrigger2 = 36;
        pins.pinStepperDir = 34;
        pins.pinStepperStep = 35;
        pins.coilPins[0] = 33; //Done
        pins.coilPins[1] = 24; //Done
        pins.coilPins[2] = 51; //Won't work (No mapping for pin 32)
        pins.coilPins[3] = 52; //Won't work (No mapping for pin 33)
        pins.pinFuelPump = 26; //Requires PVT4 adapter or above
        pins.pinFan = 50; //Won't work (No mapping for pin 35)
        pins.pinTachOut = 28; //Done
      #endif
      #endif
      break;

    case 9:
     #ifndef SMALL_FLASH_MODE
      //Pin mappings as per the 89-95 MX5 PNP shield
      pins.injectorPins[0] = 11; //Output pin injector 1 is on
      pins.injectorPins[1] = 10; //Output pin injector 2 is on
      pins.injectorPins[2] = 9; //Output pin injector 3 is on
      pins.injectorPins[3] = 8; //Output pin injector 4 is on
      pins.injectorPins[4] = 14; //Output pin injector 5 is on
      pins.coilPins[0] = 39; //Pin for coil 1
      pins.coilPins[1] = 41; //Pin for coil 2
      pins.coilPins[2] = 32; //Pin for coil 3
      pins.coilPins[3] = 33; //Pin for coil 4
      pins.coilPins[4] = 34; //Pin for coil 5 PLACEHOLDER value for now
      pins.pinTrigger = 19; //The CAS pin
      pins.pinTrigger2 = 18; //The Cam Sensor pin
      pins.pinTPS = A2;//TPS input pin
      pins.pinMAP = A5; //MAP sensor pin
      pins.pinIAT = A0; //IAT sensor pin
      pins.pinCLT = A1; //CLS sensor pin
      pins.pinO2 = A3; //O2 Sensor pin
      pins.pinBat = A4; //Battery reference voltage pin
      pins.pinTachOut = 49; //Tacho output pin  (Goes to ULN2803)
      pins.pinIdle1 = 2; //Single wire idle control
      pins.pinBoost = 4;
      pins.pinIdle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
      pins.pinFuelPump = 37; //Fuel pump output
      //Note that there is no stepper driver output on the PNP boards. These pins are unconnected and remain here just to prevent issues with random pin numbers occurring
      pins.pinStepperEnable = 15; //Enable pin for the DRV8825
      pins.pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pins.pinStepperStep = 17; //Step pin for DRV8825 driver
      pins.pinFan = 35; //Pin for the fan output
      pins.pinLaunch = 12; //Can be overwritten below
      pins.pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
      pins.pinResetControl = 44; //Reset control output
      pins.pinVSS = 20;
      pins.pinIdleUp = 48;
      pins.pinCTPS = 47;
      #endif
      #if defined(CORE_TEENSY35)
        pins.pinTrigger = 23;
        pins.pinTrigger2 = 36;
        pins.pinStepperDir = 34;
        pins.pinStepperStep = 35;
        pins.coilPins[0] = 33; //Done
        pins.coilPins[1] = 24; //Done
        pins.coilPins[2] = 51; //Won't work (No mapping for pin 32)
        pins.coilPins[3] = 52; //Won't work (No mapping for pin 33)
        pins.pinFuelPump = 26; //Requires PVT4 adapter or above
        pins.pinFan = 50; //Won't work (No mapping for pin 35)
        pins.pinTachOut = 28; //Done
      #endif
      break;

    case 10:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings for user turtanas PCB
      pins.injectorPins[0] = 4; //Output pin injector 1 is on
      pins.injectorPins[1] = 5; //Output pin injector 2 is on
      pins.injectorPins[2] = 6; //Output pin injector 3 is on
      pins.injectorPins[3] = 7; //Output pin injector 4 is on
      pins.injectorPins[4] = 8; //Placeholder only - NOT USED
      pins.injectorPins[5] = 9; //Placeholder only - NOT USED
      pins.injectorPins[6] = 10; //Placeholder only - NOT USED
      pins.injectorPins[7] = 11; //Placeholder only - NOT USED
      pins.coilPins[0] = 24; //Pin for coil 1
      pins.coilPins[1] = 28; //Pin for coil 2
      pins.coilPins[2] = 36; //Pin for coil 3
      pins.coilPins[3] = 40; //Pin for coil 4
      pins.coilPins[4] = 34; //Pin for coil 5 PLACEHOLDER value for now
      pins.pinTrigger = 18; //The CAS pin
      pins.pinTrigger2 = 19; //The Cam Sensor pin
      pins.pinTPS = A2;//TPS input pin
      pins.pinMAP = A3; //MAP sensor pin
      pins.pinIAT = A0; //IAT sensor pin
      pins.pinCLT = A1; //CLS sensor pin
      pins.pinO2 = A4; //O2 Sensor pin
      pins.pinBat = A7; //Battery reference voltage pin
      pins.pinTachOut = 41; //Tacho output pin transistor is missing 2n2222 for this and 1k for 12v
      pins.pinFuelPump = 42; //Fuel pump output 2n2222
      pins.pinFan = 47; //Pin for the fan output
      pins.pinTachOut = 49; //Tacho output pin
      pins.pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
      pins.pinResetControl = 26; //Reset control output

    #endif
      break;

    case 14:
    // Pin mappings for the Levin board
    #if defined(STM32F407xx)
      pins.injectorPins[0] = PB15;     // Output pin injector 1
      pins.injectorPins[1] = PA8;      // Output pin injector 2
      pins.injectorPins[2] = PB13;     // Output pin injector 3
      pins.injectorPins[3] = PB14;     // Output pin injector 4
      pins.injectorPins[4] = PE13;     // Output pin injector 5
      pins.injectorPins[5] = PB12;     // Output pin injector 6
      pins.injectorPins[6] = PE7;      // Output pin injector 7
      pins.injectorPins[7] = PE10;     // Output pin injector 8
      pins.coilPins[0] = PC13;         // Pin for coil 1
      pins.coilPins[1] = PE6;          // Pin for coil 2
      pins.coilPins[2] = PE5;          // Pin for coil 3
      pins.coilPins[3] = PE4;          // Pin for coil 4
      pins.coilPins[4] = PE3;          // Pin for coil 5
      pins.coilPins[5] = PE2;          // Pin for coil 6
      pins.coilPins[6] = PB9;          // Pin for coil 7
      pins.coilPins[7] = PD12;         // Pin for coil 8
      pins.pinTrigger = PD3;        // The CAS pin
      pins.pinTrigger2 = PD4;       // The Cam Sensor pin
      pins.pinTPS = PA2;            // TPS input pin
      pins.pinMAP = PA3;            // MAP sensor pin
      pins.pinEMAP = PC5;           // EMAP sensor pin (placeholder)
      pins.pinIAT = PA0;            // IAT sensor pin
      pins.pinCLT = PA1;            // CLS sensor pin
      pins.pinO2 = PB0;             // O2 Sensor pin
      pins.pinBat = PA4;            // Battery reference voltage pin
      pins.pinBaro = PA5;           // Baro sensor pin
      pins.pinTachOut = PE8;        // Tacho output pin  (Goes to UNL2803)
      pins.pinIdle1 = PD10;         // ICV pin1  (Goes to UNL2803)
      pins.pinIdle2 = PD9;          // ICV pin3  (Goes to UNL2803)
      pins.pinBoost = PD8;          // Boost control
      pins.pinVVT_1 = PD11;         // VVT1 output (intake vanos)
      pins.pinVVT_2 = PC6;          // VVT2 output (exhaust vanos)
      pins.pinFuelPump = PE11;      // Fuel pump output  (Goes to UNL2803)
      pins.pinStepperDir = PB10;    // Stepper valve isn't used with these
      pins.pinStepperStep = PB11;   // Stepper valve isn't used with these
      pins.pinStepperEnable = PA15; // Stepper valve isn't used with these
      pins.pinFan = PE9;            // Pin for the fan output (Goes to UNL2803)
      pins.pinLaunch = PB8;         // Launch control pin
      pins.pinFlex = PD7;           // Flex sensor
      pins.pinResetControl = PB7;   // Reset control output
      pins.pinVSS = PB6;            // VSS input pin
      pins.pinWMIEmpty = PA6;       //(placeholder)
      pins.pinWMIIndicator = PC3;   //(placeholder)
      pins.pinWMIEnabled = PE15;    //(placeholder)
      pins.pinIdleUp = PC7;         //(placeholder)
    #endif
      break;

    case 20:
    #if defined(CORE_AVR) && !defined(SMALL_FLASH_MODE) //No support for bluepill here anyway
      //Pin mappings as per the Plazomat In/Out shields Rev 0.1
      pins.injectorPins[0] = 8; //Output pin injector 1 is on
      pins.injectorPins[1] = 9; //Output pin injector 2 is on
      pins.injectorPins[2] = 10; //Output pin injector 3 is on
      pins.injectorPins[3] = 11; //Output pin injector 4 is on
      pins.injectorPins[4] = 12; //Output pin injector 5 is on
      pins.coilPins[0] = 28; //Pin for coil 1
      pins.coilPins[1] = 24; //Pin for coil 2
      pins.coilPins[2] = 40; //Pin for coil 3
      pins.coilPins[3] = 36; //Pin for coil 4
      pins.coilPins[4] = 34; //Pin for coil 5 PLACEHOLDER value for now
      pins.pinTrigger = 20; //The CAS pin
      pins.pinTrigger2 = 21; //The Cam Sensor pin
      pins.pinO2 = A8; //O2 Sensor pin
      pins.pinBat = A4; //Battery reference voltage pin
      pins.pinMAP = A3; //MAP sensor pin
      pins.pinTPS = A2;//TPS input pin
      pins.pinCLT = A1; //CLS sensor pin
      pins.pinIAT = A0; //IAT sensor pin
      pins.pinFan = 47; //Pin for the fan output
      pins.pinFuelPump = 4; //Fuel pump output
      pins.pinTachOut = 49; //Tacho output pin
      pins.pinResetControl = 26; //Reset control output
    #endif
      break;

    case 30:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the dazv6 shield
      pins.injectorPins[0] = 8; //Output pin injector 1 is on
      pins.injectorPins[1] = 9; //Output pin injector 2 is on
      pins.injectorPins[2] = 10; //Output pin injector 3 is on
      pins.injectorPins[3] = 11; //Output pin injector 4 is on
      pins.injectorPins[4] = 12; //Output pin injector 5 is on
      pins.coilPins[0] = 40; //Pin for coil 1
      pins.coilPins[1] = 38; //Pin for coil 2
      pins.coilPins[2] = 50; //Pin for coil 3
      pins.coilPins[3] = 52; //Pin for coil 4
      pins.coilPins[4] = 34; //Pin for coil 5 PLACEHOLDER value for now
      pins.pinTrigger = 19; //The CAS pin
      pins.pinTrigger2 = 18; //The Cam Sensor pin
      pins.pinTrigger3 = 17; // cam sensor 2 pin, pin17 isn't external trigger enabled in arduino mega??
      pins.pinTPS = A2;//TPS input pin
      pins.pinMAP = A3; //MAP sensor pin
      pins.pinIAT = A0; //IAT sensor pin
      pins.pinCLT = A1; //CLS sensor pin
      pins.pinO2 = A8; //O2 Sensor pin
      pins.pinO2_2 = A9; //O2 sensor pin (second sensor)
      pins.pinBat = A4; //Battery reference voltage pin
      pins.pinTachOut = 49; //Tacho output pin
      pins.pinIdle1 = 5; //Single wire idle control
      pins.pinFuelPump = 45; //Fuel pump output
      pins.pinStepperDir = 20; //Direction pin  for DRV8825 driver
      pins.pinStepperStep = 21; //Step pin for DRV8825 driver
      pins.pinBoost = 7;
      pins.pinFan = 47; //Pin for the fan output
    #endif
      break;

   case 31:
      //Pin mappings for the BMW PnP PCBs by pazi88.
      #if defined(CORE_AVR)
      //This is the regular MEGA2560 pin mapping
      pins.injectorPins[0] = 8; //Output pin injector 1
      pins.injectorPins[1] = 9; //Output pin injector 2
      pins.injectorPins[2] = 10; //Output pin injector 3
      pins.injectorPins[3] = 11; //Output pin injector 4
      pins.injectorPins[4] = 12; //Output pin injector 5
      pins.injectorPins[5] = 50; //Output pin injector 6
      pins.injectorPins[6] = 39; //Output pin injector 7
      pins.injectorPins[7] = 42; //Output pin injector 8
      pins.coilPins[0] = 40; //Pin for coil 1
      pins.coilPins[1] = 38; //Pin for coil 2
      pins.coilPins[2] = 52; //Pin for coil 3
      pins.coilPins[3] = 48; //Pin for coil 4
      pins.coilPins[4] = 36; //Pin for coil 5
      pins.coilPins[5] = 34; //Pin for coil 6
      pins.coilPins[6] = 46; //Pin for coil 7
      pins.coilPins[7] = 53; //Pin for coil 8
      pins.pinTrigger = 19; //The CAS pin
      pins.pinTrigger2 = 18; //The Cam Sensor pin
      pins.pinTrigger3 = 20; //The Cam sensor 2 pin
      pins.pinTPS = A2;//TPS input pin
      pins.pinMAP = A3; //MAP sensor pin
      pins.pinEMAP = A15; //EMAP sensor pin
      pins.pinIAT = A0; //IAT sensor pin
      pins.pinCLT = A1; //CLT sensor pin
      pins.pinO2 = A8; //O2 Sensor pin
      pins.pinO2_2 = A12; //O2 Sensor pin
      pins.pinBat = A4; //Battery reference voltage pin
      pins.pinBaro = A5; //Baro sensor pin
      pins.pinTachOut = 49; //Tacho output pin  (Goes to ULN2003)
      pins.pinIdle1 = 5; //ICV pin1
      pins.pinIdle2 = 6; //ICV pin3
      pins.pinBoost = 7; //Boost control
      pins.pinVVT_1 = 4; //VVT1 output (intake vanos)
      pins.pinVVT_2 = 26; //VVT2 output (exhaust vanos)
      pins.pinFuelPump = 45; //Fuel pump output  (Goes to ULN2003)
      pins.pinStepperDir = 16; //Stepper valve isn't used with these
      pins.pinStepperStep = 17; //Stepper valve isn't used with these
      pins.pinStepperEnable = 24; //Stepper valve isn't used with these
      pins.pinFan = 47; //Pin for the fan output (Goes to ULN2003)
      pins.pinLaunch = 51; //Launch control pin
      pins.pinFlex = 2; // Flex sensor
      pins.pinResetControl = 43; //Reset control output
      pins.pinVSS = 3; //VSS input pin
      pins.pinWMIEmpty = 31; //(placeholder)
      pins.pinWMIIndicator = 33; //(placeholder)
      pins.pinWMIEnabled = 35; //(placeholder)
      pins.pinIdleUp = 37; //(placeholder)
      pins.pinIdleUpOutput = 41; //(placeholder)
      pins.pinCTPS = A6; //(placeholder)
     #elif defined(STM32F407xx)
      pins.injectorPins[0] = PB15; //Output pin injector 1
      pins.injectorPins[1] = PB14; //Output pin injector 2
      pins.injectorPins[2] = PB12; //Output pin injector 3
      pins.injectorPins[3] = PB13; //Output pin injector 4
      pins.injectorPins[4] = PA8; //Output pin injector 5
      pins.injectorPins[5] = PE7; //Output pin injector 6
      pins.injectorPins[6] = PE13; //Output pin injector 7
      pins.injectorPins[7] = PE10; //Output pin injector 8
      pins.coilPins[0] = PE2; //Pin for coil 1
      pins.coilPins[1] = PE3; //Pin for coil 2
      pins.coilPins[2] = PC13; //Pin for coil 3
      pins.coilPins[3] = PE6; //Pin for coil 4
      pins.coilPins[4] = PE4; //Pin for coil 5
      pins.coilPins[5] = PE5; //Pin for coil 6
      pins.coilPins[6] = PE0; //Pin for coil 7
      pins.coilPins[7] = PB9; //Pin for coil 8
      pins.pinTrigger = PD3; //The CAS pin
      pins.pinTrigger2 = PD4; //The Cam Sensor pin
      pins.pinTPS = PA2;//TPS input pin
      pins.pinMAP = PA3; //MAP sensor pin
      pins.pinEMAP = PC5; //EMAP sensor pin
      pins.pinIAT = PA0; //IAT sensor pin
      pins.pinCLT = PA1; //CLS sensor pin
      pins.pinO2 = PB0; //O2 Sensor pin
      pins.pinO2_2 = PC2; //O2 Sensor pin
      pins.pinBat = PA4; //Battery reference voltage pin
      pins.pinBaro = PA5; //Baro sensor pin
      pins.pinTachOut = PE8; //Tacho output pin  (Goes to ULN2003)
      pins.pinIdle1 = PD10; //ICV pin1
      pins.pinIdle2 = PD9; //ICV pin3
      pins.pinBoost = PD8; //Boost control
      pins.pinVVT_1 = PD11; //VVT1 output (intake vanos)
      pins.pinVVT_2 = PC7; //VVT2 output (exhaust vanos)
      pins.pinFuelPump = PE11; //Fuel pump output  (Goes to ULN2003)
      pins.pinStepperDir = PB10; //Stepper valve isn't used with these
      pins.pinStepperStep = PB11; //Stepper valve isn't used with these
      pins.pinStepperEnable = PA15; //Stepper valve isn't used with these
      pins.pinFan = PE9; //Pin for the fan output (Goes to ULN2003)
      pins.pinLaunch = PB8; //Launch control pin
      pins.pinFlex = PD7; // Flex sensor
      pins.pinResetControl = PB7; //Reset control output
      pins.pinVSS = PB6; //VSS input pin
      pins.pinWMIEmpty = PD15; //(placeholder)
      pins.pinWMIIndicator = PD13; //(placeholder)
      pins.pinWMIEnabled = PE15; //(placeholder)
      pins.pinIdleUp = PE14; //(placeholder)
      pins.pinIdleUpOutput = PE12; //(placeholder)
      pins.pinCTPS = PA6; //(placeholder)
     #endif
      break;

    case 40:
     #ifndef SMALL_FLASH_MODE
      //Pin mappings as per the NO2C shield
      pins.injectorPins[0] = 8; //Output pin injector 1 is on
      pins.injectorPins[1] = 9; //Output pin injector 2 is on
      pins.injectorPins[2] = 11; //Output pin injector 3 is on - NOT USED
      pins.injectorPins[3] = 12; //Output pin injector 4 is on - NOT USED
      pins.injectorPins[4] = 13; //Placeholder only - NOT USED
      pins.coilPins[0] = 23; //Pin for coil 1
      pins.coilPins[1] = 22; //Pin for coil 2
      pins.coilPins[2] = 2; //Pin for coil 3 - ONLY WITH DB2
      pins.coilPins[3] = 3; //Pin for coil 4 - ONLY WITH DB2
      pins.coilPins[4] = 46; //Placeholder only - NOT USED
      pins.pinTrigger = 19; //The CAS pin
      pins.pinTrigger2 = 18; //The Cam Sensor pin
      pins.pinTrigger3 = 21; //The Cam sensor 2 pin
      pins.pinTPS = A3; //TPS input pin
      pins.pinMAP = A0; //MAP sensor pin
      pins.pinIAT = A5; //IAT sensor pin
      pins.pinCLT = A4; //CLT sensor pin
      pins.pinO2 = A2; //O2 sensor pin
      pins.pinBat = A1; //Battery reference voltage pin
      pins.pinBaro = A6; //Baro sensor pin - ONLY WITH DB
      pins.pinTachOut = 38; //Tacho output pin
      pins.pinIdle1 = 5; //Single wire idle control
      pins.pinIdle2 = 47; //2 wire idle control - NOT USED
      pins.pinBoost = 7; //Boost control
      pins.pinVVT_1 = 6; //Default VVT output
      pins.pinVVT_2 = 48; //Default VVT2 output
      pins.pinFuelPump = 4; //Fuel pump output
      pins.pinStepperDir = 25; //Direction pin for DRV8825 driver
      pins.pinStepperStep = 24; //Step pin for DRV8825 driver
      pins.pinStepperEnable = 27; //Enable pin for DRV8825 driver
      pins.pinLaunch = 10; //Can be overwritten below
      pins.pinFlex = 20; // Flex sensor (Must be external interrupt enabled) - ONLY WITH DB
      pins.pinFan = 30; //Pin for the fan output - ONLY WITH DB
      pins.pinResetControl = 26; //Reset control output
      #endif
      break;

    case 41:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the UA4C shield
      pins.injectorPins[0] = 8; //Output pin injector 1 is on
      pins.injectorPins[1] = 7; //Output pin injector 2 is on
      pins.injectorPins[2] = 6; //Output pin injector 3 is on
      pins.injectorPins[3] = 5; //Output pin injector 4 is on
      pins.injectorPins[4] = 45; //Output pin injector 5 is on PLACEHOLDER value for now
      pins.coilPins[0] = 35; //Pin for coil 1
      pins.coilPins[1] = 36; //Pin for coil 2
      pins.coilPins[2] = 33; //Pin for coil 3
      pins.coilPins[3] = 34; //Pin for coil 4
      pins.coilPins[4] = 44; //Pin for coil 5 PLACEHOLDER value for now
      pins.pinTrigger = 19; //The CAS pin
      pins.pinTrigger2 = 18; //The Cam Sensor pin
      pins.pinTrigger3 = 3; //The Cam sensor 2 pin
      pins.pinFlex = 20; // Flex sensor
      pins.pinTPS = A3; //TPS input pin
      pins.pinMAP = A0; //MAP sensor pin
      pins.pinBaro = A7; //Baro sensor pin
      pins.pinIAT = A5; //IAT sensor pin
      pins.pinCLT = A4; //CLS sensor pin
      pins.pinO2 = A1; //O2 Sensor pin
      pins.pinO2_2 = A9; //O2 sensor pin (second sensor)
      pins.pinBat = A2; //Battery reference voltage pin
      pins.pinLaunch = 37; //Can be overwritten below
      pins.pinTachOut = 22; //Tacho output pin
      pins.pinIdle1 = 9; //Single wire idle control
      pins.pinIdle2 = 10; //2 wire idle control
      pins.pinFuelPump = 23; //Fuel pump output
      pins.pinVVT_1 = 11; //Default VVT output
      pins.pinVVT_2 = 48; //Default VVT2 output
      pins.pinStepperDir = 32; //Direction pin  for DRV8825 driver
      pins.pinStepperStep = 31; //Step pin for DRV8825 driver
      pins.pinStepperEnable = 30; //Enable pin for DRV8825 driver
      pins.pinBoost = 12; //Boost control
      pins.pinFan = 24; //Pin for the fan output
      pins.pinResetControl = 46; //Reset control output PLACEHOLDER value for now
      pins.pinVSS = 2;
    #endif
      break;

    case 42:
      //Pin mappings for all BlitzboxBL49sp variants
      pins.injectorPins[0] = 6; //Output pin injector 1
      pins.injectorPins[1] = 7; //Output pin injector 2
      pins.injectorPins[2] = 8; //Output pin injector 3
      pins.injectorPins[3] = 9; //Output pin injector 4
      pins.coilPins[0] = 24; //Pin for coil 1
      pins.coilPins[1] = 25; //Pin for coil 2
      pins.coilPins[2] = 23; //Pin for coil 3
      pins.coilPins[3] = 22; //Pin for coil 4
      pins.pinTrigger = 19; //The CRANK Sensor pin
      pins.pinTrigger2 = 18; //The Cam Sensor pin
      pins.pinFlex = 20; // Flex sensor PLACEHOLDER value for now
      pins.pinTPS = A0; //TPS input pin
      pins.pinO2 = A2; //O2 Sensor pin
      pins.pinIAT = A3; //IAT sensor pin
      pins.pinCLT = A4; //CLT sensor pin
      pins.pinMAP = A7; //internal MAP sensor
      pins.pinBat = A6; //Battery reference voltage pin
      pins.pinBaro = A5; //external MAP/Baro sensor pin
      pins.pinO2_2 = A9; //O2 sensor pin (second sensor) PLACEHOLDER value for now
      pins.pinLaunch = 2; //Can be overwritten below
      pins.pinTachOut = 10; //Tacho output pin
      pins.pinIdle1 = 11; //Single wire idle control
      pins.pinIdle2 = 14; //2 wire idle control PLACEHOLDER value for now
      pins.pinFuelPump = 3; //Fuel pump output
      pins.pinVVT_1 = 15; //Default VVT output PLACEHOLDER value for now
      pins.pinBoost = 5; //Boost control
      pins.pinFan = 12; //Pin for the fan output
      pins.pinResetControl = 46; //Reset control output PLACEHOLDER value for now
    break;
    
    case 45:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings for the DIY-EFI CORE4 Module. This is an AVR only module
      #if defined(CORE_AVR)
      pins.injectorPins[0] = 10; //Output pin injector 1 is on
      pins.injectorPins[1] = 11; //Output pin injector 2 is on
      pins.injectorPins[2] = 12; //Output pin injector 3 is on
      pins.injectorPins[3] = 9; //Output pin injector 4 is on
      pins.coilPins[0] = 39; //Pin for coil 1
      pins.coilPins[1] = 29; //Pin for coil 2
      pins.coilPins[2] = 28; //Pin for coil 3
      pins.coilPins[3] = 27; //Pin for coil 4
      pins.coilPins[4] = 26; //Placeholder  for coil 5
      pins.pinTrigger = 19; //The CAS pin
      pins.pinTrigger2 = 18; //The Cam Sensor pin
      pins.pinTrigger3 = 21;// The Cam sensor 2 pin
      pins.pinFlex = 20; // Flex sensor
      pins.pinTPS = A3; //TPS input pin
      pins.pinMAP = A2; //MAP sensor pin
      pins.pinBaro = A15; //Baro sensor pin
      pins.pinIAT = A11; //IAT sensor pin
      pins.pinCLT = A4; //CLS sensor pin
      pins.pinO2 = A12; //O2 Sensor pin
      pins.pinO2_2 = A5; //O2 sensor pin (second sensor)
      pins.pinBat = A1; //Battery reference voltage pin
      pins.pinLaunch = 24; //Can be overwritten below
      pins.pinTachOut = 38; //Tacho output pin
      pins.pinIdle1 = 42; //Single wire idle control
      pins.pinIdle2 = 43; //2 wire idle control
      pins.pinFuelPump = 41; //Fuel pump output
      pins.pinVVT_1 = 44; //Default VVT output
      pins.pinVVT_2 = 48; //Default VVT2 output
      pins.pinStepperDir = 32; //Direction pin  for DRV8825 driver
      pins.pinStepperStep = 31; //Step pin for DRV8825 driver
      pins.pinStepperEnable = 30; //Enable pin for DRV8825 driver
      pins.pinBoost = 45; //Boost control
      pins.injectorPins[4] = 33; //Output pin injector 5 is on
      pins.injectorPins[5] = 34; //Output pin injector 6 is on
      pins.pinFan = 40; //Pin for the fan output
      pins.pinResetControl = 46; //Reset control output PLACEHOLDER value for now
      #endif
    #endif
      break;

    #if defined(CORE_TEENSY35)
    case 50:
      //Pin mappings as per the teensy rev A shield
      pins.injectorPins[0] = 2; //Output pin injector 1 is on
      pins.injectorPins[1] = 10; //Output pin injector 2 is on
      pins.injectorPins[2] = 6; //Output pin injector 3 is on
      pins.injectorPins[3] = 9; //Output pin injector 4 is on
      //Placeholder only - NOT USED:
      //pins.injectorPins[4] = 13;
      pins.coilPins[0] = 29; //Pin for coil 1
      pins.coilPins[1] = 30; //Pin for coil 2
      pins.coilPins[2] = 31; //Pin for coil 3 - ONLY WITH DB2
      pins.coilPins[3] = 32; //Pin for coil 4 - ONLY WITH DB2
      //Placeholder only - NOT USED:
      //pins.coilPins[4] = 46; 
      pins.pinTrigger = 23; //The CAS pin
      pins.pinTrigger2 = 36; //The Cam Sensor pin
      pins.pinTPS = 16; //TPS input pin
      pins.pinMAP = 17; //MAP sensor pin
      pins.pinIAT = 14; //IAT sensor pin
      pins.pinCLT = 15; //CLT sensor pin
      pins.pinO2 = A22; //O2 sensor pin
      pins.pinO2_2 = A21; //O2 sensor pin (second sensor)
      pins.pinBat = 18; //Battery reference voltage pin
      pins.pinTachOut = 20; //Tacho output pin
      pins.pinIdle1 = 5; //Single wire idle control
      pins.pinBoost = 11; //Boost control
      pins.pinFuelPump = 38; //Fuel pump output
      pins.pinStepperDir = 34; //Direction pin for DRV8825 driver
      pins.pinStepperStep = 35; //Step pin for DRV8825 driver
      pins.pinStepperEnable = 33; //Enable pin for DRV8825 driver
      pins.pinLaunch = 26; //Can be overwritten below
      pins.pinFan = 37; //Pin for the fan output - ONLY WITH DB
      break;

    case 51:
      //Pin mappings as per the teensy revB board shield
      pins.injectorPins[0] = 2; //Output pin injector 1 is on
      pins.injectorPins[1] = 10; //Output pin injector 2 is on
      pins.injectorPins[2] = 6; //Output pin injector 3 is on - NOT USED
      pins.injectorPins[3] = 9; //Output pin injector 4 is on - NOT USED
      pins.coilPins[0] = 29; //Pin for coil 1
      pins.coilPins[1] = 30; //Pin for coil 2
      pins.coilPins[2] = 31; //Pin for coil 3 - ONLY WITH DB2
      pins.coilPins[3] = 32; //Pin for coil 4 - ONLY WITH DB2
      pins.pinTrigger = 23; //The CAS pin
      pins.pinTrigger2 = 36; //The Cam Sensor pin
      pins.pinTPS = 16; //TPS input pin
      pins.pinMAP = 17; //MAP sensor pin
      pins.pinIAT = 14; //IAT sensor pin
      pins.pinCLT = 15; //CLT sensor pin
      pins.pinO2 = A22; //O2 sensor pin
      pins.pinO2_2 = A21; //O2 sensor pin (second sensor)
      pins.pinBat = 18; //Battery reference voltage pin
      pins.pinTachOut = 20; //Tacho output pin
      pins.pinIdle1 = 5; //Single wire idle control
      pins.pinBoost = 11; //Boost control
      pins.pinFuelPump = 38; //Fuel pump output
      pins.pinStepperDir = 34; //Direction pin for DRV8825 driver
      pins.pinStepperStep = 35; //Step pin for DRV8825 driver
      pins.pinStepperEnable = 33; //Enable pin for DRV8825 driver
      pins.pinLaunch = 26; //Can be overwritten below
      pins.pinFan = 37; //Pin for the fan output - ONLY WITH DB
      break;
    #endif

    #if defined(CORE_TEENSY35)
    case 53:
      //Pin mappings for the Juice Box (ignition only board)
      pins.injectorPins[0] = 2; //Output pin injector 1 is on - NOT USED
      pins.injectorPins[1] = 56; //Output pin injector 2 is on - NOT USED
      pins.injectorPins[2] = 6; //Output pin injector 3 is on - NOT USED
      pins.injectorPins[3] = 50; //Output pin injector 4 is on - NOT USED
      pins.coilPins[0] = 29; //Pin for coil 1
      pins.coilPins[1] = 30; //Pin for coil 2
      pins.coilPins[2] = 31; //Pin for coil 3
      pins.coilPins[3] = 32; //Pin for coil 4
      pins.pinTrigger = 37; //The CAS pin
      pins.pinTrigger2 = 38; //The Cam Sensor pin - NOT USED
      pins.pinTPS = A2; //TPS input pin
      pins.pinMAP = A7; //MAP sensor pin
      pins.pinIAT = A1; //IAT sensor pin
      pins.pinCLT = A5; //CLT sensor pin
      pins.pinO2 = A0; //O2 sensor pin
      pins.pinO2_2 = A21; //O2 sensor pin (second sensor) - NOT USED
      pins.pinBat = A6; //Battery reference voltage pin
      pins.pinTachOut = 28; //Tacho output pin
      pins.pinIdle1 = 5; //Single wire idle control - NOT USED
      pins.pinBoost = 11; //Boost control - NOT USED
      pins.pinFuelPump = 24; //Fuel pump output
      pins.pinStepperDir = 3; //Direction pin for DRV8825 driver - NOT USED
      pins.pinStepperStep = 4; //Step pin for DRV8825 driver - NOT USED
      pins.pinStepperEnable = 6; //Enable pin for DRV8825 driver - NOT USED
      pins.pinLaunch = 26; //Can be overwritten below
      pins.pinFan = 25; //Pin for the fan output
      break;
    #endif

    case 55:
      #if defined(CORE_TEENSY)
      //Pin mappings for the DropBear
      //The injector pins below are not used directly as the control is via SPI through the MC33810s, however the pin numbers are set to be the SPI pins (SCLK, MOSI, MISO and CS) so that nothing else will set them as inputs
      pins.injectorPins[0] = 13; //SCLK
      pins.injectorPins[1] = 11; //MOSI
      pins.injectorPins[2] = 12; //MISO
      pins.injectorPins[3] = 10; //CS for MC33810 1
      pins.injectorPins[4] = 9; //CS for MC33810 2
      pins.injectorPins[5] = 9; //CS for MC33810 3

      //Dummy pins, without these pin 0 (Serial1 RX) gets overwritten
      pins.coilPins[0] = 40;
      pins.coilPins[1] = 41;
      /*
      pins.coilPins[2] = 55;
      pins.coilPins[3] = 55;
      pins.coilPins[4] = 55;
      pins.coilPins[5] = 55;
      */
      
      pins.pinTrigger = 19; //The CAS pin
      pins.pinTrigger2 = 18; //The Cam Sensor pin
      pins.pinTrigger3 = 22; //Uses one of the protected spare digital inputs. This must be set or Serial1 (Pin 0) gets broken
      pins.pinFlex = A16; // Flex sensor
      pins.pinMAP = A1; //MAP sensor pin
      pins.pinBaro = A0; //Baro sensor pin
      pins.pinBat = A14; //Battery reference voltage pin
      pins.pinLaunch = A15; //Can be overwritten below
      pins.pinTachOut = 5; //Tacho output pin
      pins.pinIdle1 = 27; //Single wire idle control
      pins.pinIdle2 = 29; //2 wire idle control. Shared with Spare 1 output
      pins.pinFuelPump = 8; //Fuel pump output
      pins.pinVVT_1 = 28; //Default VVT output
      pins.pinStepperDir = 32; //Direction pin  for DRV8825 driver
      pins.pinStepperStep = 31; //Step pin for DRV8825 driver
      pins.pinStepperEnable = 30; //Enable pin for DRV8825 driver
      pins.pinBoost = 24; //Boost control
      pins.pinFan = 25; //Pin for the fan output
      pins.pinResetControl = 46; //Reset control output PLACEHOLDER value for now
      pins.pinVSS = 22;

      pins.pinWMIEmpty = 23; //Spare digital input
      pins.pinWMIIndicator = 26; //Spare output
      pins.pinWMIEnabled = 29; //Spare output

      //CS pin number is now set in a compile flag. 
      // #ifdef USE_SPI_EEPROM
      //   pinSPIFlash_CS = 6;
      // #endif

      #if defined(CORE_TEENSY35)
        pins.pinTPS = A22; //TPS input pin
        pins.pinIAT = A19; //IAT sensor pin
        pins.pinCLT = A20; //CLS sensor pin
        pins.pinO2 = A21; //O2 Sensor pin
        pins.pinO2_2 = A18; //Spare 2
      #endif

      #if defined(CORE_TEENSY41)
        //New pins for the actual T4.1 version of the Dropbear
        pins.pinBaro = A4; 
        pins.pinMAP = A5;
        pins.pinTPS = A3; //TPS input pin
        pins.pinIAT = A0; //IAT sensor pin
        pins.pinCLT = A1; //CLS sensor pin
        pins.pinO2 = A2; //O2 Sensor pin
        pins.pinBat = A15; //Battery reference voltage pin. Needs Alpha4+
        pins.pinLaunch = 36;
        pins.pinFlex = 37; // Flex sensor

        pins.pinTrigger = 20; //The CAS pin
        pins.pinTrigger2 = 21; //The Cam Sensor pin
        pins.pinTrigger3 = 34; //Uses one of the protected spare digital inputs.

        pins.pinFuelPump = 5; //Fuel pump output
        pins.pinTachOut = 0; //Tacho output pin

        pins.pinResetControl = 49; //PLaceholder only. Cannot use 42-47 as these are the SD card
        pins.pinWMIEmpty = 35; //Spare digital input
        pins.pinVSS = 34;

        //CS pin number is now set in a compile flag. 
        // #ifdef USE_SPI_EEPROM
        //   pinSPIFlash_CS = 33;
        // #endif

      #endif

        pins.pinMC33810_1_CS = 10;
        pins.pinMC33810_2_CS = 9;

      //Pin alignment to the MC33810 outputs
      pins.mc33810InjBits[0] = 3;
      pins.mc33810InjBits[1] = 1;
      pins.mc33810InjBits[2] = 0;
      pins.mc33810InjBits[3] = 2;
      pins.mc33810IgnBits[0] = 4;
      pins.mc33810IgnBits[1] = 5;
      pins.mc33810IgnBits[2] = 6;
      pins.mc33810IgnBits[3] = 7;

      pins.mc33810InjBits[4] = 3;
      pins.mc33810InjBits[5] = 1;
      pins.mc33810InjBits[6] = 0;
      pins.mc33810InjBits[7] = 2;
      pins.mc33810IgnBits[4] = 4;
      pins.mc33810IgnBits[5] = 5;
      pins.mc33810IgnBits[6] = 6;
      pins.mc33810IgnBits[7] = 7;



      #endif
      break;

    case 56:
      #if defined(CORE_TEENSY)
      //Pin mappings for the Bear Cub (Teensy 4.1)
      pins.injectorPins[0] = 6;
      pins.injectorPins[1] = 7;
      pins.injectorPins[2] = 9;
      pins.injectorPins[3] = 8;
      pins.injectorPins[4] = 0; //Not used
      pins.coilPins[0] = 2;
      pins.coilPins[1] = 3;
      pins.coilPins[2] = 4;
      pins.coilPins[3] = 5;

      pins.pinTrigger = 20; //The CAS pin
      pins.pinTrigger2 = 21; //The Cam Sensor pin
      pins.pinFlex = 37; // Flex sensor
      pins.pinMAP = A5; //MAP sensor pin
      pins.pinBaro = A4; //Baro sensor pin
      pins.pinBat = A15; //Battery reference voltage pin
      pins.pinTPS = A3; //TPS input pin
      pins.pinIAT = A0; //IAT sensor pin
      pins.pinCLT = A1; //CLS sensor pin
      pins.pinO2 = A2; //O2 Sensor pin
      pins.pinLaunch = 36;

      pins.pinTachOut = 38; //Tacho output pin
      pins.pinIdle1 = 27; //Single wire idle control
      pins.pinIdle2 = 26; //2 wire idle control. Shared with Spare 1 output
      pins.pinFuelPump = 10; //Fuel pump output
      pins.pinVVT_1 = 28; //Default VVT output
      pins.pinStepperDir = 32; //Direction pin  for DRV8825 driver
      pins.pinStepperStep = 31; //Step pin for DRV8825 driver
      pins.pinStepperEnable = 30; //Enable pin for DRV8825 driver
      pins.pinBoost = 24; //Boost control
      pins.pinFan = 25; //Pin for the fan output
      pins.pinResetControl = 46; //Reset control output PLACEHOLDER value for now

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
        pins.coilPins[2] = PA8;
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
        pins.pinBaro = PB1; //ADC12
        // = PB2;  //(DO NOT USE FOR SPEEDUINO) BOOT1 
        // = PB3;  //(DO NOT USE FOR SPEEDUINO) SPI1_SCK FLASH CHIP
        // = PB4;  //(DO NOT USE FOR SPEEDUINO) SPI1_MISO FLASH CHIP
        // = PB5;  //(DO NOT USE FOR SPEEDUINO) SPI1_MOSI FLASH CHIP
        // = PB6;  //NRF_CE
        pins.coilPins[5] = PB7;  //NRF_CS
        // = PB8;  //NRF_IRQ
        pins.coilPins[1] = PB9; //
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
        pins.pinIAT = PC0; //ADC123 
        pins.pinTPS = PC1; //ADC123
        pins.pinMAP = PC2; //ADC123 
        pins.pinCLT = PC3; //ADC123
        pins.pinO2 = PC4; //ADC12
        pins.pinBat = PC5;  //ADC12
        pins.pinBoost = PC6; //
        pins.pinIdle1 = PC7; //
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
        pins.pinIdle2 = PD3; //
        // = PD4;  //
        pins.pinFlex = PD4;
        // = PD5; //TXD2
        // = PD6;  //RXD2
        pins.coilPins[0] = PD7; //
        // = PD7;  //
        // = PD8;  //
        pins.coilPins[4] = PD9;//
        pins.coilPins[3] = PD10;//
        // = PD11;  //
        pins.injectorPins[0] = PD12; //
        pins.injectorPins[1] = PD13; //
        pins.injectorPins[2] = PD14; //
        pins.injectorPins[3] = PD15; //

        //******************************************
        //******** PORTE CONNECTIONS *************** 
        //******************************************
        pins.pinTrigger = PE0; //
        pins.pinTrigger2 = PE1; //
        pins.pinStepperEnable = PE2; //
        pins.pinFuelPump = PE3; //ONBOARD KEY1
        // = PE4;  //ONBOARD KEY2
        pins.pinStepperStep = PE5; //
        pins.pinFan = PE6; //
        pins.pinStepperDir = PE7; //
        // = PE8;  //
        pins.injectorPins[4] = PE9; //
        // = PE10;  //
        pins.injectorPins[5] = PE11; //
        // = PE12; //
        pins.injectorPins[7] = PE13; //
        pins.injectorPins[6] = PE14; //
        // = PE15;  //
     #elif (defined(STM32F411xE) || defined(STM32F401xC))
        //pins PA12, PA11 are used for USB or CAN couldn't be used for GPIO
        //PB2 can't be used as input because is BOOT pin
        pins.injectorPins[0] = PB7; //Output pin injector 1 is on
        pins.injectorPins[1] = PB6; //Output pin injector 2 is on
        pins.injectorPins[2] = PB5; //Output pin injector 3 is on
        pins.injectorPins[3] = PB4; //Output pin injector 4 is on
        pins.coilPins[0] = PB9; //Pin for coil 1
        pins.coilPins[1] = PB8; //Pin for coil 2
        pins.coilPins[2] = PB3; //Pin for coil 3
        pins.coilPins[3] = PA15; //Pin for coil 4
        pins.pinTPS = A2;//TPS input pin
        pins.pinMAP = A3; //MAP sensor pin
        pins.pinIAT = A0; //IAT sensor pin
        pins.pinCLT = A1; //CLS sensor pin
        pins.pinO2 = A8; //O2 Sensor pin
        pins.pinBat = A4; //Battery reference voltage pin
        pins.pinBaro = pins.pinMAP;
        pins.pinTachOut = PB1; //Tacho output pin  (Goes to ULN2803)
        pins.pinIdle1 = PB2; //Single wire idle control
        pins.pinIdle2 = PB10; //2 wire idle control
        pins.pinBoost = PA6; //Boost control
        pins.pinStepperDir = PB10; //Direction pin  for DRV8825 driver
        pins.pinStepperStep = PB2; //Step pin for DRV8825 driver
        pins.pinFuelPump = PA8; //Fuel pump output
        pins.pinFan = PA5; //Pin for the fan output (Goes to ULN2803)

        //external interrupt enabled pins
        pins.pinFlex = PC14; // Flex sensor (Must be external interrupt enabled)
        pins.pinTrigger = PC13; //The CAS pin also led pin so bad idea
        pins.pinTrigger2 = PC15; //The Cam Sensor pin

     #elif defined(CORE_STM32)
        //blue pill wiki.stm32duino.com/index.php?title=Blue_Pill
        //Maple mini wiki.stm32duino.com/index.php?title=Maple_Mini
        //pins PA12, PA11 are used for USB or CAN couldn't be used for GPIO
        //PB2 can't be used as input because is BOOT pin
        pins.injectorPins[0] = PB7; //Output pin injector 1 is on
        pins.injectorPins[1] = PB6; //Output pin injector 2 is on
        pins.injectorPins[2] = PB5; //Output pin injector 3 is on
        pins.injectorPins[3] = PB4; //Output pin injector 4 is on
        pins.coilPins[0] = PB3; //Pin for coil 1
        pins.coilPins[1] = PA15; //Pin for coil 2
        pins.coilPins[2] = PA14; //Pin for coil 3
        pins.coilPins[3] = PA9; //Pin for coil 4
        pins.coilPins[4] = PA8; //Pin for coil 5
        pins.pinTPS = A0; //TPS input pin
        pins.pinMAP = A1; //MAP sensor pin
        pins.pinIAT = A2; //IAT sensor pin
        pins.pinCLT = A3; //CLS sensor pin
        pins.pinO2 = A4; //O2 Sensor pin
        pins.pinBat = A5; //Battery reference voltage pin
        pins.pinBaro = pins.pinMAP;
        pins.pinIdle1 = PB2; //Single wire idle control
        pins.pinIdle2 = PA2; //2 wire idle control
        pins.pinBoost = PA1; //Boost control
        pins.pinVVT_1 = PA0; //Default VVT output
        pins.pinVVT_2 = PA2; //Default VVT2 output
        pins.pinStepperDir = PC15; //Direction pin  for DRV8825 driver
        pins.pinStepperStep = PC14; //Step pin for DRV8825 driver
        pins.pinStepperEnable = PC13; //Enable pin for DRV8825
        pins.pinFan = PB1; //Pin for the fan output
        pins.pinFuelPump = PB11; //Fuel pump output
        pins.pinTachOut = PB10; //Tacho output pin
        //external interrupt enabled pins
        pins.pinFlex = PB8; // Flex sensor (Must be external interrupt enabled)
        pins.pinTrigger = PA10; //The CAS pin
        pins.pinTrigger2 = PA13; //The Cam Sensor pin
      
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
        pinNumbers.coilPins[0]  = PE10;
        pinNumbers.coilPins[1]  = PE11;
        pinNumbers.injectorPins[0] = PC6;
        pinNumbers.injectorPins[1] = PC7;
        pinNumbers.pinIdle1     = PD2;
        pinNumbers.pinFan       = PD3;
        pinNumbers.pinFuelPump  = PD4;
        pinNumbers.pinTachOut   = PC9;
    #endif
      break;

    default:
      return getDefaultPinMapping();
      break;
  }

  return pins;
}

/** Set board / microcontroller specific pin mappings / assignments.
 * The boardID is switch-case compared against raw boardID integers (not enum or defined label, and probably no need for that either)
 * which are originated from tuning SW (e.g. TS) set values and are available in reference/speeduino.ini (See pinLayout, note also that
 * numbering is not contiguous here).
 */
void setPinMapping(byte boardID)
{
#if defined(MC33810_SUPPORT)
  InjIoControlMode injControlMode = boardID==55 ? InjIoControlMode::MC33810 : InjIoControlMode::Direct;
  IgnIoControlMode ignControlMode = boardID==55 ? IgnIoControlMode::MC33810 : IgnIoControlMode::Direct;
#else
  InjIoControlMode injControlMode = InjIoControlMode::Direct;
  IgnIoControlMode ignControlMode = IgnIoControlMode::Direct;
#endif

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
#ifdef SD_LOGGING
  if ( (configPage13.onboard_log_trigger_Epin != 0) && (configPage13.onboard_log_tr5_Epin_pin < BOARD_MAX_IO_PINS) ) { pinNumbers.pinSDEnable = pinTranslate(configPage13.onboard_log_tr5_Epin_pin); }
#endif
  

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
#ifdef SD_LOGGING
  if( (configPage13.onboard_log_trigger_Epin > 0) && (!pinIsOutput(pinNumbers.pinSDEnable)) )
  {
    pinMode(pinNumbers.pinSDEnable, INPUT);
  }
#endif
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