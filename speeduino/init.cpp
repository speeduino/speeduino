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
#include "scheduler_fuel_controller.h"
#include "src/controllers/vvt/vvtController.h"
#include "sensors.h"
#include "decoders.h"
#include "corrections.h"
#include "src/controllers/idle/idle.h"
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
#include "src/pins/pinMapping.h"
#include "resetControl.h"
#include "scheduler_ignition_controller.h"
#include "maths.h"
#include "elapsed_time.h"
#include "src/controllers/fuelPump/fuelPumpController.h"
#include "src/controllers/fan/fanController.h"
#include "src/controllers/boost/boostController.h"
#include "src/controllers/aircon/airconController.h"
#include "src/controllers/nitrous/nitrousController.h"
#include "src/controllers/progammableIO/programmableIOControl.h"
#include "src/controllers/tacho/tachoController.h"
#include "src/controllers/launch/launchController.h"
#include "src/controllers/ignBypass/ignBypassControl.h"
#include "src/controllers/auxChannels/auxChannelController.h"

#if defined(CORE_AVR)
#pragma GCC push_options
// This minimizes RAM usage at no performance cost
#pragma GCC optimize ("Os") 
#endif

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
  while (digitalRead(EEPROM_RESET_PIN) != HIGH && !hasIntervalElapsed(millis(), start_time, START_RESET_INTERVAL))
  {
    //make sure the key is pressed for at least 0.5 second 
    if (hasIntervalElapsed(millis(), start_time, MIN_BUTTON_PRESSED_INTERVAL)) {
      //if key is pressed afterboot for 0.5 second make led turn off
      digitalWrite(LED_BUILTIN, HIGH);

      //see if the user reacts to the led turned off with removing the keypress within 1 second
      while ((!hasIntervalElapsed(millis(), start_time, MAX_BUTTON_RELEASE_INTERVAL)) && (exit_erase_loop!=true)){

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
 * - Initialise triggers 
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

    //Perform all initialisations
    initialiseIgnitionSchedules(currentStatus, configPage2, configPage4, configPage10, configPage13, pinNumbers);
    initialiseFuelSchedules(currentStatus, configPage2, configPage4, configPage6, configPage10, pinNumbers);
    initialiseIdle(true);
    initialiseFan(currentStatus, configPage2, configPage6, pinNumbers);
    initialiseBoost(currentStatus, configPage2, configPage6, configPage10, pinNumbers);
    initialiseAirCon(currentStatus, configPage15, pinNumbers);
    initialiseNitrous(currentStatus, configPage10);
    initialiseVvtWmi(currentStatus, pinNumbers, configPage4, configPage6, configPage10);
    initialiseCorrections();
    currentStatus.ioError = false; //Clear the I/O error bit. The bit will be set in initialiseADC() if there is problem in there.
    initialiseADC();
    initAuxChannels(currentStatus, configPage9);
    initialiseMAPBaro();
    initialiseProgrammableIO(configPage13);
    initialiseFlexSensor(configPage2, currentStatus, pinNumbers.pinFlex);
    initialiseLaunchControl(currentStatus, configPage6, pinNumbers);
    initialiseIgnBypass(currentStatus, configPage4, pinNumbers);

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
    currentStatus.setRevolutionTime(0U);
    currentStatus.runSecs = 0;
    currentStatus.secl = 0;
    //currentStatus.seclx10 = 0;
    currentStatus.startRevolutions = 0;
    currentStatus.syncLossCounter = 0;
    currentStatus.crankRPM = ((unsigned int)configPage4.crankRPM * 10); //Crank RPM limit (Saves us calculating this over and over again. It's updated once per second in timers.ino)
    currentStatus.engineProtect.reset();
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

     
    initialiseFuelPump(currentStatus, configPage2, pinNumbers);

    interrupts();
    initialiseCLT();
    initialiseTPS();
    initialiseTachoControl(pinNumbers.pinTachOut, configPage2, configPage6, currentStatus);

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
  //This is a legacy mode option to revert the MAP reading behaviour to match what was in place prior to the 201905 firmware
  if(configPage2.legacyMAP > 0) { digitalWrite(pinNumbers.pinMAP, HIGH); }

  // Select the board-specific mode once for the analog inputs configured here.
  #if defined(CORE_STM32) && defined(INPUT_ANALOG)
    constexpr auto analogInputMode = INPUT_ANALOG;
  #elif defined(CORE_TEENSY41)
    constexpr auto analogInputMode = INPUT_DISABLE;
  #else
    constexpr auto analogInputMode = INPUT;
  #endif

  #if defined(CORE_STM32) || defined(CORE_TEENSY41)
    pinMode(pinNumbers.pinMAP, analogInputMode);
    pinMode(pinNumbers.pinO2, analogInputMode);
    pinMode(pinNumbers.pinO2_2, analogInputMode);
    pinMode(pinNumbers.pinTPS, analogInputMode);
    pinMode(pinNumbers.pinIAT, analogInputMode);
    pinMode(pinNumbers.pinCLT, analogInputMode);
    pinMode(pinNumbers.pinBat, analogInputMode);
    pinMode(pinNumbers.pinBaro, analogInputMode);
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
    pinMode(pinNumbers.pinFuelPressure, analogInputMode);
  }
  if( (configPage10.oilPressureEnable > 0) && (!pinIsOutput(pinNumbers.pinOilPressure)) )
  {
    pinMode(pinNumbers.pinOilPressure, analogInputMode);
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
void initialiseTriggers(void)
{
  byte triggerInterrupt = 0; // By default, use the first interrupt
  byte triggerInterrupt2 = 1;
  byte triggerInterrupt3 = 2;

  #if defined(CORE_AVR)
    switch (pinTrigger) {
      //Arduino Mega 2560 mapping
      case 2:
        triggerInterrupt = 0; break;
      case 3:
        triggerInterrupt = 1; break;
      case 18:
        triggerInterrupt = 5; break;
      case 19:
        triggerInterrupt = 4; break;
      case 20:
        triggerInterrupt = 3; break;
      case 21:
        triggerInterrupt = 2; break;
      default:
        triggerInterrupt = 0; break; //This should NEVER happen
    }
  #else
    triggerInterrupt = pinTrigger;
  #endif

  #if defined(CORE_AVR)
    switch (pinTrigger2) {
      //Arduino Mega 2560 mapping
      case 2:
        triggerInterrupt2 = 0; break;
      case 3:
        triggerInterrupt2 = 1; break;
      case 18:
        triggerInterrupt2 = 5; break;
      case 19:
        triggerInterrupt2 = 4; break;
      case 20:
        triggerInterrupt2 = 3; break;
      case 21:
        triggerInterrupt2 = 2; break;
      default:
        triggerInterrupt2 = 0; break; //This should NEVER happen
    }
  #else
    triggerInterrupt2 = pinTrigger2;
  #endif

  #if defined(CORE_AVR)
    switch (pinTrigger3) {
      //Arduino Mega 2560 mapping
      case 2:
        triggerInterrupt3 = 0; break;
      case 3:
        triggerInterrupt3 = 1; break;
      case 18:
        triggerInterrupt3 = 5; break;
      case 19:
        triggerInterrupt3 = 4; break;
      case 20:
        triggerInterrupt3 = 3; break;
      case 21:
        triggerInterrupt3 = 2; break;
      default:
        triggerInterrupt3 = 0; break; //This should NEVER happen
    }
  #else
    triggerInterrupt3 = pinTrigger3;
  #endif

  pinMode(pinTrigger, INPUT);
  pinMode(pinTrigger2, INPUT);
  pinMode(pinTrigger3, INPUT);

  detachInterrupt(triggerInterrupt);
  detachInterrupt(triggerInterrupt2);
  detachInterrupt(triggerInterrupt3);
  //The default values for edges
  primaryTriggerEdge = 0; //This should ALWAYS be changed below
  secondaryTriggerEdge = 0; //This is optional and may not be changed below, depending on the decoder in use
  tertiaryTriggerEdge = 0; //This is even more optional and may not be changed below, depending on the decoder in use

  //Set the trigger function based on the decoder in the config
  switch (configPage4.TrigPattern)
  {
    case DECODER_MISSING_TOOTH:
      //Missing tooth decoder
      triggerSetup_missingTooth();
      triggerHandler = triggerPri_missingTooth;
      triggerSecondaryHandler = triggerSec_missingTooth;
      triggerTertiaryHandler = triggerThird_missingTooth;
      
      getRPM = getRPM_missingTooth;
      getCrankAngle = getCrankAngle_missingTooth;
      triggerSetEndTeeth = triggerSetEndTeeth_missingTooth;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      if(configPage4.TrigEdgeSec == 0) { secondaryTriggerEdge = RISING; }
      else { secondaryTriggerEdge = FALLING; }
      if(configPage10.TrigEdgeThrd == 0) { tertiaryTriggerEdge = RISING; }
      else { tertiaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);

      if(BIT_CHECK(decoderState, BIT_DECODER_HAS_SECONDARY)) { attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge); }
      if(configPage10.vvt2Enabled > 0) { attachInterrupt(triggerInterrupt3, triggerTertiaryHandler, tertiaryTriggerEdge); } // we only need this for vvt2, so not really needed if it's not used

      break;

    case DECODER_BASIC_DISTRIBUTOR:
      // Basic distributor
      triggerSetup_BasicDistributor();
      triggerHandler = triggerPri_BasicDistributor;
      getRPM = getRPM_BasicDistributor;
      getCrankAngle = getCrankAngle_BasicDistributor;
      triggerSetEndTeeth = triggerSetEndTeeth_BasicDistributor;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      break;

    case 2:
      triggerSetup_DualWheel();
      triggerHandler = triggerPri_DualWheel;
      triggerSecondaryHandler = triggerSec_DualWheel;
      getRPM = getRPM_DualWheel;
      getCrankAngle = getCrankAngle_DualWheel;
      triggerSetEndTeeth = triggerSetEndTeeth_DualWheel;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      if(configPage4.TrigEdgeSec == 0) { secondaryTriggerEdge = RISING; }
      else { secondaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case DECODER_GM7X:
      triggerSetup_GM7X();
      triggerHandler = triggerPri_GM7X;
      getRPM = getRPM_GM7X;
      getCrankAngle = getCrankAngle_GM7X;
      triggerSetEndTeeth = triggerSetEndTeeth_GM7X;

      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, triggerHandler, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, triggerHandler, FALLING); }

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      break;

    case DECODER_4G63:
      triggerSetup_4G63();
      triggerHandler = triggerPri_4G63;
      triggerSecondaryHandler = triggerSec_4G63;
      getRPM = getRPM_4G63;
      getCrankAngle = getCrankAngle_4G63;
      triggerSetEndTeeth = triggerSetEndTeeth_4G63;

      primaryTriggerEdge = CHANGE;
      secondaryTriggerEdge = FALLING;

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case DECODER_24X:
      triggerSetup_24X();
      triggerHandler = triggerPri_24X;
      triggerSecondaryHandler = triggerSec_24X;
      getRPM = getRPM_24X;
      getCrankAngle = getCrankAngle_24X;
      triggerSetEndTeeth = triggerSetEndTeeth_24X;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = CHANGE; //Secondary is always on every change

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case DECODER_JEEP2000:
      triggerSetup_Jeep2000();
      triggerHandler = triggerPri_Jeep2000;
      triggerSecondaryHandler = triggerSec_Jeep2000;
      getRPM = getRPM_Jeep2000;
      getCrankAngle = getCrankAngle_Jeep2000;
      triggerSetEndTeeth = triggerSetEndTeeth_Jeep2000;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = CHANGE;

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case DECODER_AUDI135:
      triggerSetup_Audi135();
      triggerHandler = triggerPri_Audi135;
      triggerSecondaryHandler = triggerSec_Audi135;
      getRPM = getRPM_Audi135;
      getCrankAngle = getCrankAngle_Audi135;
      triggerSetEndTeeth = triggerSetEndTeeth_Audi135;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = RISING; //always rising for this trigger

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case DECODER_HONDA_D17:
      triggerSetup_HondaD17();
      triggerHandler = triggerPri_HondaD17;
      triggerSecondaryHandler = triggerSec_HondaD17;
      getRPM = getRPM_HondaD17;
      getCrankAngle = getCrankAngle_HondaD17;
      triggerSetEndTeeth = triggerSetEndTeeth_HondaD17;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = CHANGE;

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case DECODER_HONDA_J32:
      triggerSetup_HondaJ32();
      triggerHandler = triggerPri_HondaJ32;
      triggerSecondaryHandler = triggerSec_HondaJ32;
      getRPM = getRPM_HondaJ32;
      getCrankAngle = getCrankAngle_HondaJ32;
      triggerSetEndTeeth = triggerSetEndTeeth_HondaJ32;

      primaryTriggerEdge = RISING; // Don't honor the config, always use rising edge 
      secondaryTriggerEdge = RISING; // Unused

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);  // Suspect this line is not needed
      break;

    case DECODER_MIATA_9905:
      triggerSetup_Miata9905();
      triggerHandler = triggerPri_Miata9905;
      triggerSecondaryHandler = triggerSec_Miata9905;
      getRPM = getRPM_Miata9905;
      getCrankAngle = getCrankAngle_Miata9905;
      triggerSetEndTeeth = triggerSetEndTeeth_Miata9905;

      //These may both need to change, not sure
      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      if(configPage4.TrigEdgeSec == 0) { secondaryTriggerEdge = RISING; }
      else { secondaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case DECODER_MAZDA_AU:
      triggerSetup_MazdaAU();
      triggerHandler = triggerPri_MazdaAU;
      triggerSecondaryHandler = triggerSec_MazdaAU;
      getRPM = getRPM_MazdaAU;
      getCrankAngle = getCrankAngle_MazdaAU;
      triggerSetEndTeeth = triggerSetEndTeeth_MazdaAU;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = FALLING;

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case DECODER_NON360:
      triggerSetup_non360();
      triggerHandler = triggerPri_DualWheel; //Is identical to the dual wheel decoder, so that is used. Same goes for the secondary below
      triggerSecondaryHandler = triggerSec_DualWheel; //Note the use of the Dual Wheel trigger function here. No point in having the same code in twice.
      getRPM = getRPM_non360;
      getCrankAngle = getCrankAngle_non360;
      triggerSetEndTeeth = triggerSetEndTeeth_non360;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = FALLING;

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case DECODER_NISSAN_360:
      triggerSetup_Nissan360();
      triggerHandler = triggerPri_Nissan360;
      triggerSecondaryHandler = triggerSec_Nissan360;
      getRPM = getRPM_Nissan360;
      getCrankAngle = getCrankAngle_Nissan360;
      triggerSetEndTeeth = triggerSetEndTeeth_Nissan360;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = CHANGE;

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case DECODER_SUBARU_67:
      triggerSetup_Subaru67();
      triggerHandler = triggerPri_Subaru67;
      triggerSecondaryHandler = triggerSec_Subaru67;
      getRPM = getRPM_Subaru67;
      getCrankAngle = getCrankAngle_Subaru67;
      triggerSetEndTeeth = triggerSetEndTeeth_Subaru67;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = FALLING;

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case DECODER_DAIHATSU_PLUS1:
      triggerSetup_Daihatsu();
      triggerHandler = triggerPri_Daihatsu;
      getRPM = getRPM_Daihatsu;
      getCrankAngle = getCrankAngle_Daihatsu;
      triggerSetEndTeeth = triggerSetEndTeeth_Daihatsu;

      //No secondary input required for this pattern
      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      break;

    case DECODER_HARLEY:
      triggerSetup_Harley();
      triggerHandler = triggerPri_Harley;
      //triggerSecondaryHandler = triggerSec_Harley;
      getRPM = getRPM_Harley;
      getCrankAngle = getCrankAngle_Harley;
      triggerSetEndTeeth = triggerSetEndTeeth_Harley;

      primaryTriggerEdge = RISING; //Always rising
      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      break;

    case DECODER_36_2_2_2:
      //36-2-2-2
      triggerSetup_ThirtySixMinus222();
      triggerHandler = triggerPri_ThirtySixMinus222;
      triggerSecondaryHandler = triggerSec_ThirtySixMinus222;
      getRPM = getRPM_ThirtySixMinus222;
      getCrankAngle = getCrankAngle_missingTooth; //This uses the same function as the missing tooth decoder, so no need to duplicate code
      triggerSetEndTeeth = triggerSetEndTeeth_ThirtySixMinus222;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      if(configPage4.TrigEdgeSec == 0) { secondaryTriggerEdge = RISING; }
      else { secondaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case DECODER_36_2_1:
      //36-2-1
      triggerSetup_ThirtySixMinus21();
      triggerHandler = triggerPri_ThirtySixMinus21;
      triggerSecondaryHandler = triggerSec_missingTooth;
      getRPM = getRPM_ThirtySixMinus21;
      getCrankAngle = getCrankAngle_missingTooth; //This uses the same function as the missing tooth decoder, so no need to duplicate code
      triggerSetEndTeeth = triggerSetEndTeeth_ThirtySixMinus21;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      if(configPage4.TrigEdgeSec == 0) { secondaryTriggerEdge = RISING; }
      else { secondaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case DECODER_420A:
      //DSM 420a
      triggerSetup_420a();
      triggerHandler = triggerPri_420a;
      triggerSecondaryHandler = triggerSec_420a;
      getRPM = getRPM_420a;
      getCrankAngle = getCrankAngle_420a;
      triggerSetEndTeeth = triggerSetEndTeeth_420a;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = FALLING; //Always falling edge

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case DECODER_WEBER:
      //Weber-Marelli
      triggerSetup_DualWheel();
      triggerHandler = triggerPri_Webber;
      triggerSecondaryHandler = triggerSec_Webber;
      getRPM = getRPM_DualWheel;
      getCrankAngle = getCrankAngle_DualWheel;
      triggerSetEndTeeth = triggerSetEndTeeth_DualWheel;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      if(configPage4.TrigEdgeSec == 0) { secondaryTriggerEdge = RISING; }
      else { secondaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case DECODER_ST170:
      //Ford ST170
      triggerSetup_FordST170();
      triggerHandler = triggerPri_missingTooth;
      triggerSecondaryHandler = triggerSec_FordST170;
      getRPM = getRPM_FordST170;
      getCrankAngle = getCrankAngle_FordST170;
      triggerSetEndTeeth = triggerSetEndTeeth_FordST170;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      if(configPage4.TrigEdgeSec == 0) { secondaryTriggerEdge = RISING; }
      else { secondaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);

      break;
	  
    case DECODER_DRZ400:
      triggerSetup_DRZ400();
      triggerHandler = triggerPri_DualWheel;
      triggerSecondaryHandler = triggerSec_DRZ400;
      getRPM = getRPM_DualWheel;
      getCrankAngle = getCrankAngle_DualWheel;
      triggerSetEndTeeth = triggerSetEndTeeth_DualWheel;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      if(configPage4.TrigEdgeSec == 0) { secondaryTriggerEdge = RISING; }
      else { secondaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case DECODER_NGC:
      //Chrysler NGC - 4, 6 and 8 cylinder
      triggerSetup_NGC();
      triggerHandler = triggerPri_NGC;
      getRPM = getRPM_NGC;
      getCrankAngle = getCrankAngle_missingTooth;
      triggerSetEndTeeth = triggerSetEndTeeth_NGC;

      primaryTriggerEdge = CHANGE;
      if (configPage2.nCylinders == 4) {
        triggerSecondaryHandler = triggerSec_NGC4;
        secondaryTriggerEdge = CHANGE;
      }
      else {
        triggerSecondaryHandler = triggerSec_NGC68;
        secondaryTriggerEdge = FALLING;
      }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case DECODER_VMAX:
      triggerSetup_Vmax();
      triggerHandler = triggerPri_Vmax;
      getRPM = getRPM_Vmax;
      getCrankAngle = getCrankAngle_Vmax;
      triggerSetEndTeeth = triggerSetEndTeeth_Vmax;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = true; } // set as boolean so we can directly use it in decoder.
      else { primaryTriggerEdge = false; }
      
      attachInterrupt(triggerInterrupt, triggerHandler, CHANGE); //Hardcoded change, the primaryTriggerEdge will be used in the decoder to select if it`s an inverted or non-inverted signal.
      break;

    case DECODER_RENIX:
      //Renault 44 tooth decoder
      triggerSetup_Renix();
      triggerHandler = triggerPri_Renix;
      getRPM = getRPM_missingTooth;
      getCrankAngle = getCrankAngle_missingTooth;
      triggerSetEndTeeth = triggerSetEndTeeth_Renix;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt 
      else { primaryTriggerEdge = FALLING; }
      if(configPage4.TrigEdgeSec == 0) { secondaryTriggerEdge = RISING; }
      else { secondaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      break;

    case DECODER_ROVERMEMS:
      //Rover MEMs - covers multiple flywheel trigger combinations.
      triggerSetup_RoverMEMS();
      triggerHandler = triggerPri_RoverMEMS;
      getRPM = getRPM_RoverMEMS;
      triggerSetEndTeeth = triggerSetEndTeeth_RoverMEMS;
            
      triggerSecondaryHandler = triggerSec_RoverMEMS; 
      getCrankAngle = getCrankAngle_missingTooth;   

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      if(configPage4.TrigEdgeSec == 0) { secondaryTriggerEdge = RISING; }
      else { secondaryTriggerEdge = FALLING; }
      
      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;   

    case DECODER_SUZUKI_K6A:
      triggerSetup_SuzukiK6A();
      triggerHandler = triggerPri_SuzukiK6A; // only primary, no secondary, trigger pattern is over 720 degrees
      getRPM = getRPM_SuzukiK6A;
      getCrankAngle = getCrankAngle_SuzukiK6A;
      triggerSetEndTeeth = triggerSetEndTeeth_SuzukiK6A;


      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      
      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      break;

      case DECODER_FORD_TFI:
      // Ford TFI
      triggerSetup_FordTFI();
      triggerHandler = triggerPri_FordTFI;
      triggerSecondaryHandler = triggerSec_FordTFI;
      getRPM = getRPM_FordTFI;
      getCrankAngle = getCrankAngle_FordTFI;
      triggerSetEndTeeth = triggerSetEndTeeth_FordTFI;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      if(configPage4.TrigEdgeSec == 0) { secondaryTriggerEdge = RISING; }
      else { secondaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case DECODER_SUBARU_7_C_ONLY:
      // Subaru 7 teeth crank ONLY
      triggerSetup_Subaru7crankOnly();
      triggerHandler = triggerPri_Subaru7crankOnly;
      triggerSecondaryHandler = triggerSec_Subaru7crankOnly;
      getRPM = getRPM_Subaru7crankOnly;
      getCrankAngle = getCrankAngle_Subaru7crankOnly;
      triggerSetEndTeeth = triggerSetEndTeeth_Subaru7crankOnly;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = FALLING;

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      break;

    default:
      triggerHandler = triggerPri_missingTooth;
      getRPM = getRPM_missingTooth;
      getCrankAngle = getCrankAngle_missingTooth;

      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, triggerHandler, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, triggerHandler, FALLING); }
      break;
  }

  #if defined(CORE_TEENSY41)
    //Teensy 4 requires a HYSTERESIS flag to be set on any external interrupt pins to prevent false interrupts
    setTeensy41PinsHysteresis();
  #endif
}

static inline bool isAnyFuelScheduleRunning(void) {
  return fuelSchedule1.Status==RUNNING
      || fuelSchedule2.Status==RUNNING
      || fuelSchedule3.Status==RUNNING
      || fuelSchedule4.Status==RUNNING
#if INJ_CHANNELS >= 5      
      || fuelSchedule5.Status==RUNNING
#endif
#if INJ_CHANNELS >= 6
      || fuelSchedule6.Status==RUNNING
#endif
#if INJ_CHANNELS >= 7
      || fuelSchedule7.Status==RUNNING
#endif
#if INJ_CHANNELS >= 8
      || fuelSchedule8.Status==RUNNING
#endif
      ;
}

static inline bool isAnyIgnScheduleRunning(void) {
  return ignitionSchedule1.Status==RUNNING      
#if IGN_CHANNELS >= 2 
      || ignitionSchedule2.Status==RUNNING
#endif      
#if IGN_CHANNELS >= 3 
      || ignitionSchedule3.Status==RUNNING
#endif      
#if IGN_CHANNELS >= 4       
      || ignitionSchedule4.Status==RUNNING
#endif      
#if IGN_CHANNELS >= 5      
      || ignitionSchedule5.Status==RUNNING
#endif
#if IGN_CHANNELS >= 6
      || ignitionSchedule6.Status==RUNNING
#endif
#if IGN_CHANNELS >= 7
      || ignitionSchedule7.Status==RUNNING
#endif
#if IGN_CHANNELS >= 8
      || ignitionSchedule8.Status==RUNNING
#endif
      ;
}

/** Change injectors or/and ignition angles to 720deg.
 * Roll back req_fuel size and set number of outputs equal to cylinder count.
* */
void changeHalfToFullSync(void)
{
  //Need to do another check for injLayout as this function can be called from ignition
  noInterrupts();
  if( (configPage2.injLayout == INJ_SEQUENTIAL) && (CRANK_ANGLE_MAX_INJ != 720) && (!isAnyFuelScheduleRunning()))
  {
    CRANK_ANGLE_MAX_INJ = 720;
    req_fuel_uS *= 2;
    
    fuelSchedule1.pStartFunction = openInjector1;
    fuelSchedule1.pEndFunction = closeInjector1;
    fuelSchedule2.pStartFunction = openInjector2;
    fuelSchedule2.pEndFunction = closeInjector2;
    fuelSchedule3.pStartFunction = openInjector3;
    fuelSchedule3.pEndFunction = closeInjector3;
    fuelSchedule4.pStartFunction = openInjector4;
    fuelSchedule4.pEndFunction = closeInjector4;
#if INJ_CHANNELS >= 5
    fuelSchedule5.pStartFunction = openInjector5;
    fuelSchedule5.pEndFunction = closeInjector5;
#endif
#if INJ_CHANNELS >= 6
    fuelSchedule6.pStartFunction = openInjector6;
    fuelSchedule6.pEndFunction = closeInjector6;
#endif
#if INJ_CHANNELS >= 7
    fuelSchedule7.pStartFunction = openInjector7;
    fuelSchedule7.pEndFunction = closeInjector7;
#endif
#if INJ_CHANNELS >= 8
    fuelSchedule8.pStartFunction = openInjector8;
     fuelSchedule8.pEndFunction = closeInjector8;
#endif

    switch (configPage2.nCylinders)
    {
      case 4:
        maxInjOutputs = 4;
        break;
            
      case 6:
        maxInjOutputs = 6;
        break;

      case 8:
        maxInjOutputs = 8;
        break;

      default:
        break; //No actions required for other cylinder counts

    }
  }
  interrupts();

  //Need to do another check for sparkMode as this function can be called from injection
  if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && (CRANK_ANGLE_MAX_IGN != 720) && (!isAnyIgnScheduleRunning()) )
  {
    CRANK_ANGLE_MAX_IGN = 720;
    maxIgnOutputs = configPage2.nCylinders;
    switch (configPage2.nCylinders)
    {
    case 4:
      ignitionSchedule1.pStartCallback = beginCoil1Charge;
      ignitionSchedule1.pEndCallback = endCoil1Charge;
      ignitionSchedule2.pStartCallback = beginCoil2Charge;
      ignitionSchedule2.pEndCallback = endCoil2Charge;
      break;

    case 6:
      ignitionSchedule1.pStartCallback = beginCoil1Charge;
      ignitionSchedule1.pEndCallback = endCoil1Charge;
      ignitionSchedule2.pStartCallback = beginCoil2Charge;
      ignitionSchedule2.pEndCallback = endCoil2Charge;
      ignitionSchedule3.pStartCallback = beginCoil3Charge;
      ignitionSchedule3.pEndCallback = endCoil3Charge;
      break;

    case 8:
      ignitionSchedule1.pStartCallback = beginCoil1Charge;
      ignitionSchedule1.pEndCallback = endCoil1Charge;
      ignitionSchedule2.pStartCallback = beginCoil2Charge;
      ignitionSchedule2.pEndCallback = endCoil2Charge;
      ignitionSchedule3.pStartCallback = beginCoil3Charge;
      ignitionSchedule3.pEndCallback = endCoil3Charge;
      ignitionSchedule4.pStartCallback = beginCoil4Charge;
      ignitionSchedule4.pEndCallback = endCoil4Charge;
      break;

    default:
      break; //No actions required for other cylinder counts
      
    }
  }
}

/** Change injectors or/and ignition angles to 360deg.
 * In semi sequentiol mode req_fuel size is half.
 * Set number of outputs equal to half cylinder count.
* */
void changeFullToHalfSync(void)
{
  if(configPage2.injLayout == INJ_SEQUENTIAL)
  {
    CRANK_ANGLE_MAX_INJ = 360;
    req_fuel_uS /= 2;
    switch (configPage2.nCylinders)
    {
      case 4:
        if(configPage4.inj4cylPairing == INJ_PAIR_13_24)
        {
          fuelSchedule1.pStartFunction = openInjector1and3;
          fuelSchedule1.pEndFunction = closeInjector1and3;
          fuelSchedule2.pStartFunction = openInjector2and4;
          fuelSchedule2.pEndFunction = closeInjector2and4;
        }
        else
        {
          fuelSchedule1.pStartFunction = openInjector1and4;
          fuelSchedule1.pEndFunction = closeInjector1and4;
          fuelSchedule2.pStartFunction = openInjector2and3;
          fuelSchedule2.pEndFunction = closeInjector2and3;
        }
        maxInjOutputs = 2;
        break;
            
      case 6:
        fuelSchedule1.pStartFunction = openInjector1and4;
        fuelSchedule1.pEndFunction = closeInjector1and4;
        fuelSchedule2.pStartFunction = openInjector2and5;
        fuelSchedule2.pEndFunction = closeInjector2and5;
        fuelSchedule3.pStartFunction = openInjector3and6;
        fuelSchedule3.pEndFunction = closeInjector3and6;
        maxInjOutputs = 3;
        break;

      case 8:
        fuelSchedule1.pStartFunction = openInjector1and5;
        fuelSchedule1.pEndFunction = closeInjector1and5;
        fuelSchedule2.pStartFunction = openInjector2and6;
        fuelSchedule2.pEndFunction = closeInjector2and6;
        fuelSchedule3.pStartFunction = openInjector3and7;
        fuelSchedule3.pEndFunction = closeInjector3and7;
        fuelSchedule4.pStartFunction = openInjector4and8;
        fuelSchedule4.pEndFunction = closeInjector4and8;
        maxInjOutputs = 4;
        break;
    }
  }

  if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
  {
    CRANK_ANGLE_MAX_IGN = 360;
    maxIgnOutputs = configPage2.nCylinders / 2;
    switch (configPage2.nCylinders)
    {
      case 4:
        ignitionSchedule1.pStartCallback = beginCoil1and3Charge;
        ignitionSchedule1.pEndCallback = endCoil1and3Charge;
        ignitionSchedule2.pStartCallback = beginCoil2and4Charge;
        ignitionSchedule2.pEndCallback = endCoil2and4Charge;
        break;
            
      case 6:
        ignitionSchedule1.pStartCallback = beginCoil1and4Charge;
        ignitionSchedule1.pEndCallback = endCoil1and4Charge;
        ignitionSchedule2.pStartCallback = beginCoil2and5Charge;
        ignitionSchedule2.pEndCallback = endCoil2and5Charge;
        ignitionSchedule3.pStartCallback = beginCoil3and6Charge;
        ignitionSchedule3.pEndCallback = endCoil3and6Charge;
        break;

      case 8:
        ignitionSchedule1.pStartCallback = beginCoil1and5Charge;
        ignitionSchedule1.pEndCallback = endCoil1and5Charge;
        ignitionSchedule2.pStartCallback = beginCoil2and6Charge;
        ignitionSchedule2.pEndCallback = endCoil2and6Charge;
        ignitionSchedule3.pStartCallback = beginCoil3and7Charge;
        ignitionSchedule3.pEndCallback = endCoil3and7Charge;
        ignitionSchedule4.pStartCallback = beginCoil4and8Charge;
        ignitionSchedule4.pEndCallback = endCoil4and8Charge;
        break;
    }
  }
}

#if defined(CORE_AVR)
#pragma GCC pop_options
#endif