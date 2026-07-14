/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
#include "auxiliaries.h"
#include "globals.h"
#include "elapsed_time.h"
#include "maths.h"
#include "src/PID/integerPID.h"
#include "src/PID/integerPID_ideal.h"
#include "decoders.h"
#include "timers.h"
#include "preprocessor.h"
#include "units.h"
#include "atomic.h"
#include "src/pins/fastInputPin.h"
#include "src/pins/boardOutputPin.h"
#include "scheduler_fuel_controller.h"

TESTABLE_STATIC long vvt1_pwm_value;
TESTABLE_STATIC long vvt2_pwm_value;
TESTABLE_STATIC volatile unsigned int vvt1_pwm_cur_value;
TESTABLE_STATIC volatile unsigned int vvt2_pwm_cur_value;
TESTABLE_STATIC volatile bool vvt1_pwm_state;
TESTABLE_STATIC volatile bool vvt2_pwm_state;
TESTABLE_STATIC volatile bool vvt1_max_pwm;
TESTABLE_STATIC volatile bool vvt2_max_pwm;
TESTABLE_STATIC volatile char nextVVT;
TESTABLE_STATIC byte boostCounter;
TESTABLE_STATIC byte vvtCounter;

TESTABLE_STATIC fastInputPin_t n2o_arming_pin;

static __attribute__((optimize("Os"))) uint8_t getN2oArmPinPolarity(const config10 &page10)
{
  if(page10.n2o_pin_polarity == 1U) 
  { 
    return INPUT_PULLUP; 
  }
  return INPUT;
}
static __attribute__((optimize("Os"))) void initialiseN2oArmPin(const config10 &page10)
{
  if(configPage10.n2o_enable!=0U && !pinIsReserved(page10.n2o_arming_pin))
  {
    // The pin modes are only set if the if n2o is enabled to prevent them conflicting 
    // with other inputs. 
    n2o_arming_pin.setPin(page10.n2o_arming_pin, getN2oArmPinPolarity(page10));
  }
}

TESTABLE_STATIC fastInputPin_t aircon_req_pin;

static __attribute__((optimize("Os"))) uint8_t getAirConRequestPinMode(const config15 &page15)
{
  if(page15.airConReqPol)
  {
    // Inverted
    // +5V is ON, Use external pull-down resistor for OFF
    return INPUT;
  }
  else
  {
    //Normal
    // Pin pulled to Ground is ON. Floating (internally pulled up to +5V) is OFF.
    return INPUT_PULLUP;
  }
}

TESTABLE_STATIC boardOutputPin_t boost_pin;
TESTABLE_STATIC boardOutputPin_t n2o_stage1_pin;
TESTABLE_STATIC boardOutputPin_t n2o_stage2_pin;
TESTABLE_STATIC boardOutputPin_t aircon_comp_pin;
TESTABLE_STATIC boardOutputPin_t aircon_fan_pin;

static __attribute__((optimize("Os"))) void initialiseN2oPins(const config10 &page10)
{
  n2o_stage1_pin.setPin(page10.n2o_stage1_pin, OUTPUT);
  n2o_stage2_pin.setPin(page10.n2o_stage2_pin, OUTPUT);
  initialiseN2oArmPin(page10);
}

TESTABLE_STATIC void airConOn(void)
{
  ATOMIC() { 
    if (configPage15.airConCompPol)
    {
      aircon_comp_pin.setPinLow();
    }
    else
    {
      aircon_comp_pin.setPinHigh();
    }
    currentStatus.airconCompressorOn = true; 
  }  
}
TESTABLE_STATIC void airConOff(void)
{
  ATOMIC() { 
    if (configPage15.airConCompPol)
    {
      aircon_comp_pin.setPinHigh();
    }
    else
    {
      aircon_comp_pin.setPinLow();
    }
    currentStatus.airconCompressorOn = false; 
  }
}
static void airConFanOn(void)
{
  ATOMIC() { 
    if (configPage15.airConFanPol)
    {
      aircon_fan_pin.setPinLow();
    }
    else
    {
      aircon_fan_pin.setPinHigh();
    }
    currentStatus.airconFanOn = true; 
  }
}
static void airConFanOff(void)
{
  ATOMIC() { 
    if (configPage15.airConFanPol)
    {
      aircon_fan_pin.setPinHigh();
    }
    else
    {
      aircon_fan_pin.setPinLow();
    }
    currentStatus.airconFanOn = false; 
  }
}

static bool isWmiTankEmpty(void)
{
  if (configPage10.wmiEmptyEnabled) 
  {
    return (configPage10.wmiEmptyPolarity) ? digitalRead(pinNumbers.pinWMIEmpty) : !digitalRead(pinNumbers.pinWMIEmpty);
  }
  return true;
}

TESTABLE_STATIC bool acIsEnabled;
TESTABLE_STATIC bool acStandAloneFanIsEnabled;
TESTABLE_STATIC uint8_t acStartDelay;
TESTABLE_STATIC uint8_t acTPSLockoutDelay;
TESTABLE_STATIC uint8_t acRPMLockoutDelay;
TESTABLE_STATIC uint8_t acAfterEngineStartDelay;
TESTABLE_STATIC bool waitedAfterCranking; // This starts false and prevents the A/C from running until a few seconds after cranking

TESTABLE_STATIC long boost_pwm_target_value;
TESTABLE_STATIC volatile bool boost_pwm_state;
TESTABLE_STATIC volatile unsigned int boost_pwm_cur_value = 0;

static uint32_t vvtWarmTime;
TESTABLE_STATIC bool vvtIsHot;
TESTABLE_STATIC bool vvtTimeHold;
TESTABLE_STATIC uint16_t vvt_pwm_max_count; //Used for variable PWM frequency
static uint16_t boost_pwm_max_count; //Used for variable PWM frequency
TESTABLE_CONSTEXPR table2D_u8_s16_6 flexBoostTable(&configPage10.flexBoostBins, &configPage10.flexBoostAdj);

//Old PID method. Retained in case the new one has issues
static integerPID_ideal boostPID; //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call
static integerPID vvtPID; //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call
static integerPID vvt2PID; //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call

static inline void checkAirConCoolantLockout(void);
static inline void checkAirConTPSLockout(void);
static inline void checkAirConRPMLockout(void);

/*
Air Conditioning Control
*/
void __attribute__((optimize("Os"))) initialiseAirCon(void)
{
  if( (configPage15.airConEnable) &&
      !pinIsReserved(pinNumbers.pinAirConRequest) &&
      !pinIsReserved(pinNumbers.pinAirConComp) &&
      !pinIsOutput(pinNumbers.pinAirConRequest))
  {
    // Hold the A/C off until a few seconds after cranking
    acAfterEngineStartDelay = 0;
    waitedAfterCranking = false;

    acStartDelay = 0;
    acTPSLockoutDelay = 0;
    acRPMLockoutDelay = 0;

    currentStatus.airconRequested = false;
    currentStatus.airconCompressorOn = false;
    currentStatus.airconRpmLockout = false;
    currentStatus.airconTpsLockout = false;
    currentStatus.airconTurningOn = false;
    currentStatus.airconCltLockout = false;
    currentStatus.airconFanOn = false;
    aircon_req_pin.setPin(pinNumbers.pinAirConRequest, getAirConRequestPinMode(configPage15));
    aircon_comp_pin.setPin(pinNumbers.pinAirConComp, OUTPUT);
  
    airConOff();

    if((configPage15.airConFanEnabled) && (pinIsReserved(pinNumbers.pinAirConFan)))
    {
      aircon_fan_pin.setPin(pinNumbers.pinAirConFan, OUTPUT);
      airConFanOff();
      acStandAloneFanIsEnabled = true;
    }
    else
    {
      acStandAloneFanIsEnabled = false;
    }

    acIsEnabled = true;

  }
  else
  {
    acIsEnabled = false;
  }
}

static bool READ_AIRCON_REQUEST(void)
{
  if(acIsEnabled == false)
  {
    return false;
  }
  // Read the status of the A/C request pin (A/C button), taking into account the pin's polarity
  currentStatus.airconRequested = aircon_req_pin.isPinHigh()==configPage15.airConReqPol;
  return currentStatus.airconRequested;
}

void airConControl(void)
{
  if(acIsEnabled == true)
  {
    // ------------------------------------------------------------------------------------------------------
    // Check that the engine has been running past the post-start delay period before enabling the compressor
    // ------------------------------------------------------------------------------------------------------
    if (currentStatus.rotationStatus==EngineRotationStatus::Running)
    {
      if(acAfterEngineStartDelay >= configPage15.airConAfterStartDelay)
      {
        waitedAfterCranking = true;
      }
      else
      {
        acAfterEngineStartDelay++;
      }
    }
    else
    {
      acAfterEngineStartDelay = 0;
      waitedAfterCranking = false;
    }
    
    // --------------------------------------------------------------------
    // Determine the A/C lockouts based on the noted parameters
    // These functions set/clear the globl currentStatus.airConStatus bits.
    // --------------------------------------------------------------------
    checkAirConCoolantLockout();
    checkAirConTPSLockout();
    checkAirConRPMLockout();
    
    // -----------------------------------------
    // Check the A/C Request Signal (A/C Button)
    // -----------------------------------------
    if( READ_AIRCON_REQUEST() == true &&
        waitedAfterCranking == true &&
        currentStatus.airconTpsLockout == false &&
        currentStatus.airconRpmLockout == false &&
        currentStatus.airconCltLockout == false )
    {
      // Set the flag bit to notify the idle system to idle up & the cooling fan to start (if enabled)
      currentStatus.airconTurningOn = true;

      // Stand-alone fan operation
      if(acStandAloneFanIsEnabled == true)
      {
        airConFanOn();
      }

      // Start the A/C compressor after the "Compressor On" delay period
      if(acStartDelay >= configPage15.airConCompOnDelay)
      {
        airConOn();
      }
      else
      {
        acStartDelay++;
      }
    }
    else
    {
      currentStatus.airconTurningOn = false;

      // Stand-alone fan operation
      if(acStandAloneFanIsEnabled == true)
      {
        airConFanOff();
      }

      airConOff();
      acStartDelay = 0;
    }
  }
}

static inline void checkAirConCoolantLockout(void)
{
  // ---------------------------
  // Coolant Temperature Lockout
  // ---------------------------
  int offTemp = temperatureRemoveOffset(configPage15.airConClTempCut);
  if (currentStatus.coolant > offTemp)
  {
    // A/C is cut off due to high coolant
    currentStatus.airconCltLockout = true;
  }
  else if (currentStatus.coolant < (offTemp - 1))
  {
    // Adds a bit of hysteresis (2 degrees) to removing the lockout
    // Yes, it is 2 degrees (not 1 degree or 3 degrees) because we go "> offTemp" to enable and "< (offtemp-1)" to disable,
    // e.g. if offTemp is 100, it needs to go GREATER than 100 to enable, i.e. 101, and then 98 to disable,
    // because the coolant temp is an integer. So 98.5 degrees to 100.5 degrees is the analog null zone where nothing happens,
    // depending on sensor calibration and table interpolation.
    // Hopefully offTemp wasn't -40... otherwise underflow... but that would be ridiculous
    currentStatus.airconCltLockout = false;
  }
}

static inline void checkAirConTPSLockout(void)
{
  // ------------------------------
  // High Throttle Position Lockout
  // ------------------------------
  if (currentStatus.TPS > configPage15.airConTPSCut)
  {
    // A/C is cut off due to high TPS
    currentStatus.airconTpsLockout = true;
    acTPSLockoutDelay = 0;
  }
  else if ( (currentStatus.airconTpsLockout == true) &&
            (currentStatus.TPS <= configPage15.airConTPSCut) )
  {
    // No need for hysteresis as we have the stand-down delay period after the high TPS condition goes away.
    if (acTPSLockoutDelay >= configPage15.airConTPSCutTime)
    {
      currentStatus.airconTpsLockout = false;
    }
    else
    {
      acTPSLockoutDelay++;
    }
  }
  else
  {
    acTPSLockoutDelay = 0;
  }
}

static inline void checkAirConRPMLockout(void)
{
  // --------------------
  // High/Low RPM Lockout
  // --------------------
  if ( (currentStatus.RPM < (configPage15.airConMinRPMdiv10 * 10)) ||
       (currentStatus.RPMdiv100 > configPage15.airConMaxRPMdiv100) )
  {
    // A/C is cut off due to high/low RPM
    currentStatus.airconRpmLockout = true;
    acRPMLockoutDelay = 0;
  }
  else if ( (currentStatus.RPM >= (configPage15.airConMinRPMdiv10 * 10)) &&
            (currentStatus.RPMdiv100 <= configPage15.airConMaxRPMdiv100) )
  {
    // No need to add hysteresis as we have the stand-down delay period after the high/low RPM condition goes away.
    if (acRPMLockoutDelay >= configPage15.airConRPMCutTime)
    {
      currentStatus.airconRpmLockout = false;
    }
    else
    {
      acRPMLockoutDelay++;
    }
  }
  else
  {
    acRPMLockoutDelay = 0;
  }
}

TESTABLE_STATIC boardOutputPin_t vvt1_pin;
TESTABLE_STATIC boardOutputPin_t vvt2_pin;

static __attribute__((optimize("Os"))) void initialiseVvtPins(uint8_t pin1, uint8_t pin2) 
{ 
  vvt1_pin.setPin(pin1, OUTPUT);
  vvt2_pin.setPin(pin2, OUTPUT);
}

static void setBoostPidTunings(const config2 &page2, const config6 &page6, const config10 &page10)
{
  if(page6.boostMode == BOOST_MODE_SIMPLE)
  {
    boostPID.setTunings(PidTuningParameters());
  }
  else
  {
    boostPID.setTunings(PidTuningParameters(page6.boostKP, page6.boostKI, page6.boostKD));
  }
  boostPID.setOutputLimits(page2.boostMinDuty, page2.boostMaxDuty);
  boostPID.setSampleTime(millis(), page10.boostIntv);
  boostPID.setSensitivity(page10.boostSens);
}

static void setVvtPidTunings(integerPID &pid, const config10 &page10, bool isReverse)
{
  int8_t multiplier = isReverse ? 1 : -1;
  pid.setTunings(PidTuningParameters(page10.vvtCLKP, page10.vvtCLKI, page10.vvtCLKD) * multiplier, millis(), 33);
}

static void initialiseVvtPid(integerPID &pid, const config10 &page10, bool isReverse, int16_t currentAngle)
{
  pid.setOutputLimits(page10.vvtCLminDuty, page10.vvtCLmaxDuty);
  setVvtPidTunings(pid, page10, isReverse);
  pid.activate(currentAngle); //Turn PID on
}

void __attribute__((optimize("Os"))) initialiseAuxPWM(void)
{
  boost_pin.setPin(pinNumbers.pinBoost, OUTPUT);
  initialiseVvtPins(pinNumbers.pinVVT_1, pinNumbers.pinVVT_2);
  initialiseN2oPins(configPage10);

  //This is a safety check that will be true if the board is uninitialised. This prevents hangs on a new board that could otherwise try to write to an invalid pin port/mask (Without this a new Teensy 4.x hangs on startup)
  //The n2o_minTPS variable is capped at 100 by TS, so 255 indicates a new board.
  if(configPage10.n2o_minTPS == 255) { configPage10.n2o_enable = 0; }

  setBoostPidTunings(configPage2, configPage6, configPage10);
  boost_pwm_max_count = pwmFreqToTicks(FREQUENCY.toUser(configPage6.boostFreq));

  if( configPage6.vvtEnabled > 0)
  {
    currentStatus.vvt1Angle = 0;
    currentStatus.vvt2Angle = 0;
    vvt_pwm_max_count = pwmFreqToTicks(FREQUENCY.toUser(configPage6.vvtFreq));

    if(configPage6.vvtMode == VVT_MODE_CLOSED_LOOP)
    {
      initialiseVvtPid(vvtPID, configPage10, configPage6.vvtPWMdir, currentStatus.vvt1Angle);
      if (configPage10.vvt2Enabled == 1) // same for VVT2 if it's enabled
      {
        initialiseVvtPid(vvt2PID, configPage10, configPage4.vvt2PWMdir, currentStatus.vvt2Angle);
      }
    }

    vvt1_pwm_value = 0;
    vvt2_pwm_value = 0;
    ENABLE_VVT_TIMER(); //Turn on the B compare unit (ie turn on the interrupt)
    currentStatus.vvt1AngleError = false;
    currentStatus.vvt2AngleError = false;
    vvtTimeHold = false;
    if (currentStatus.coolant >= temperatureRemoveOffset(configPage4.vvtMinClt)) { vvtIsHot = true; } //Checks to see if coolant's already at operating temperature
  }
  
  if( (configPage6.vvtEnabled == 0) && (configPage10.wmiEnabled >= 1) )
  {
    // config wmi pwm output to use vvt output
    vvt_pwm_max_count = pwmFreqToTicks(FREQUENCY.toUser(configPage6.vvtFreq));
    currentStatus.wmiTankEmpty = false;
    currentStatus.wmiPW = 0;
    vvt1_pwm_value = 0;
    vvt2_pwm_value = 0;
    ENABLE_VVT_TIMER(); //Turn on the B compare unit (ie turn on the interrupt)
  }

  currentStatus.boostDuty = 0;
  boostCounter = 0;
  currentStatus.vvt1Duty = 0;
  currentStatus.vvt2Duty = 0;
  vvtCounter = 0;

  currentStatus.nitrous_status = NITROUS_OFF;
}

static void boostByGear(void)
{
  if(configPage4.boostType == OPEN_LOOP_BOOST)
  {
    if( configPage9.boostByGearEnabled == 1 )
    {
      uint16_t combinedBoost = 0;
      switch (currentStatus.gear)
      {
        case 1:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear1 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM))  ) << 2;
          if( combinedBoost <= 10000 ){ currentStatus.boostDuty = combinedBoost; }
          else{ currentStatus.boostDuty = 10000; }
          break;
        case 2:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear2 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM))  ) << 2;
          if( combinedBoost <= 10000 ){ currentStatus.boostDuty = combinedBoost; }
          else{ currentStatus.boostDuty = 10000; }
          break;
        case 3:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear3 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM))  ) << 2;
          if( combinedBoost <= 10000 ){ currentStatus.boostDuty = combinedBoost; }
          else{ currentStatus.boostDuty = 10000; }
          break;
        case 4:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear4 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM))  ) << 2;
          if( combinedBoost <= 10000 ){ currentStatus.boostDuty = combinedBoost; }
          else{ currentStatus.boostDuty = 10000; }
          break;
        case 5:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear5 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM))  ) << 2;
          if( combinedBoost <= 10000 ){ currentStatus.boostDuty = combinedBoost; }
          else{ currentStatus.boostDuty = 10000; }
          break;
        case 6:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear6 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM))  ) << 2;
          if( combinedBoost <= 10000 ){ currentStatus.boostDuty = combinedBoost; }
          else{ currentStatus.boostDuty = 10000; }
          break;
        default:
          break;
      }
    }
    else if( configPage9.boostByGearEnabled == 2 ) 
    {
      switch (currentStatus.gear)
      {
        case 1:
          currentStatus.boostDuty = configPage9.boostByGear1 * 2 * 100;
          break;
        case 2:
          currentStatus.boostDuty = configPage9.boostByGear2 * 2 * 100;
          break;
        case 3:
          currentStatus.boostDuty = configPage9.boostByGear3 * 2 * 100;
          break;
        case 4:
          currentStatus.boostDuty = configPage9.boostByGear4 * 2 * 100;
          break;
        case 5:
          currentStatus.boostDuty = configPage9.boostByGear5 * 2 * 100;
          break;
        case 6:
          currentStatus.boostDuty = configPage9.boostByGear6 * 2 * 100;
          break;
        default:
          break;
      }
    }
  }
  else if (configPage4.boostType == CLOSED_LOOP_BOOST)
  {
    if( configPage9.boostByGearEnabled == 1 )
    {
      uint16_t combinedBoost = 0;
      switch (currentStatus.gear)
      {
        case 1:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear1 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM)) / 100 ) << 2;
          if( combinedBoost <= 511 ){ currentStatus.boostTarget = combinedBoost; }
          else{ currentStatus.boostTarget = 511; }
          break;
        case 2:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear2 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM)) / 100 ) << 2;
          if( combinedBoost <= 511 ){ currentStatus.boostTarget = combinedBoost; }
          else{ currentStatus.boostTarget = 511; }
          break;
        case 3:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear3 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM)) / 100 ) << 2;
          if( combinedBoost <= 511 ){ currentStatus.boostTarget = combinedBoost; }
          else{ currentStatus.boostTarget = 511; }
          break;
        case 4:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear4 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM)) / 100 ) << 2;
          if( combinedBoost <= 511 ){ currentStatus.boostTarget = combinedBoost; }
          else{ currentStatus.boostTarget = 511; }
          break;
        case 5:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear5 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM)) / 100 ) << 2;
          if( combinedBoost <= 511 ){ currentStatus.boostTarget = combinedBoost; }
          else{ currentStatus.boostTarget = 511; }
          break;
        case 6:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear6 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM)) / 100 ) << 2;
          if( combinedBoost <= 511 ){ currentStatus.boostTarget = combinedBoost; }
          else{ currentStatus.boostTarget = 511; }
          break;
        default:
          break;
      }
    }
    else if( configPage9.boostByGearEnabled == 2 ) 
    {
      switch (currentStatus.gear)
      {
        case 1:
          currentStatus.boostTarget = (configPage9.boostByGear1 << 1);
          break;
        case 2:
          currentStatus.boostTarget = (configPage9.boostByGear2 << 1);
          break;
        case 3:
          currentStatus.boostTarget = (configPage9.boostByGear3 << 1);
          break;
        case 4:
          currentStatus.boostTarget = (configPage9.boostByGear4 << 1);
          break;
        case 5:
          currentStatus.boostTarget = (configPage9.boostByGear5 << 1);
          break;
        case 6:
          currentStatus.boostTarget = (configPage9.boostByGear6 << 1);
          break;
        default:
          break;
      }
    }
  }
}

void boostControl(void)
{
  if( configPage6.boostEnabled==1 )
  {
    if(configPage4.boostType == OPEN_LOOP_BOOST)
    {
      //Open loop
      if ( (configPage9.boostByGearEnabled > 0) && isExternalVssMode(configPage2) ){ boostByGear(); }
      else{ currentStatus.boostDuty = get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM) * 2 * 100; }

      if(currentStatus.boostDuty > 10000) { currentStatus.boostDuty = 10000; } //Safety check
      if(currentStatus.boostDuty == 0) { DISABLE_BOOST_TIMER(); boost_pin.setPinLow(); } //If boost duty is 0, shut everything down
      else
      {
        boost_pwm_target_value = ((unsigned long)(currentStatus.boostDuty) * boost_pwm_max_count) / 10000; //Convert boost duty (Which is a % multiplied by 100) to a pwm count
      }
    }
    else if (configPage4.boostType == CLOSED_LOOP_BOOST)
    {
      if( (boostCounter & 7) == 1) 
      { 
        if ( (configPage9.boostByGearEnabled > 0) && isExternalVssMode(configPage2) ){ boostByGear(); }
        else{ currentStatus.boostTarget = get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM) << 1; } //Boost target table is in kpa and divided by 2

        //If flex fuel is enabled, there can be an adder to the boost target based on ethanol content
        if( configPage2.flexEnabled == 1 )
        {
          currentStatus.flexBoostCorrection = table2D_getValue(&flexBoostTable, currentStatus.ethanolPct);
          currentStatus.boostTarget += currentStatus.flexBoostCorrection;
          currentStatus.boostTarget = min(currentStatus.boostTarget, (uint16_t)511U);
        }
        else
        {
          currentStatus.flexBoostCorrection = 0;
        }
      } 

      if(((configPage15.boostControlEnable == EN_BOOST_CONTROL_BARO) && (currentStatus.MAP >= currentStatus.baro)) || ((configPage15.boostControlEnable == EN_BOOST_CONTROL_FIXED) && (currentStatus.MAP >= configPage15.boostControlEnableThreshold))) //Only enables boost control above baro pressure or above user defined threshold (User defined level is usually set to boost with wastegate actuator only boost level)
      {
        if(currentStatus.boostTarget > 0)
        {
          //This only needs to be run very infrequently, once every 16 calls to boostControl(). This is approx. once per second
          if( (boostCounter & 15) == 1)
          {
            setBoostPidTunings(configPage2, configPage6, configPage10);
          }

          boostPID.setSetPoint(currentStatus.boostTarget);
          boostPID.setFeedForwardTerm(get3DTableValue(&boostTableLookupDuty, currentStatus.boostTarget, currentStatus.RPM) * 100/2);
          //Compute() returns false if the required interval has not yet passed.
          bool PIDcomputed = boostPID.compute(millis(), 
                                              currentStatus.MAP,
                                              &currentStatus.boostDuty);
          
          if(currentStatus.boostDuty == 0) { DISABLE_BOOST_TIMER(); boost_pin.setPinLow(); } //If boost duty is 0, shut everything down
          else
          {
            if(PIDcomputed == true)
            {
              boost_pwm_target_value = ((unsigned long)(currentStatus.boostDuty) * boost_pwm_max_count) / 10000; //Convert boost duty (Which is a % multiplied by 100) to a pwm count
            }
          }
        }
        else
        {
          //If boost target is 0, turn everything off
          boostDisable();
        }
      }
      else
      {
        boostPID.initialize(currentStatus.MAP); //This resets the ITerm value to prevent rubber banding
        //Boost control needs to have a high duty cycle if control is below threshold (baro or fixed value). This ensures the waste gate is closed as much as possible, this build boost as fast as possible.
        currentStatus.boostDuty = configPage15.boostDCWhenDisabled*100;
        boost_pwm_target_value = ((unsigned long)(currentStatus.boostDuty) * boost_pwm_max_count) / 10000; //Convert boost duty (Which is a % multiplied by 100) to a pwm count
        ENABLE_BOOST_TIMER(); //Turn on the compare unit (ie turn on the interrupt) if boost duty >0
        if(currentStatus.boostDuty == 0) { boostDisable(); } //If boost control does nothing disable PWM completely
      } //MAP above boost + hyster
    } //Open / Cloosed loop

    //Check for 100% duty cycle
    if(currentStatus.boostDuty >= 10000)
    {
      DISABLE_BOOST_TIMER(); //Turn off the compare unit (ie turn off the interrupt) if boost duty is 100%
      boost_pin.setPinHigh(); //Turn on boost pin if duty is 100%
    }
    else if(currentStatus.boostDuty > 0)
    {
      ENABLE_BOOST_TIMER(); //Turn on the compare unit (ie turn on the interrupt) if boost duty is > 0
    }
    
  }
  else { // Disable timer channel and zero the flex boost correction status
    DISABLE_BOOST_TIMER();
    currentStatus.flexBoostCorrection = 0;
  }

  boostCounter++;
}

void vvt1On(void)
{
  vvt1_pin.setPinHigh();
}
void vvt1Off(void)
{
  vvt1_pin.setPinLow();
}
void vvt2On(void)
{
  vvt2_pin.setPinHigh();
}
void vvt2Off(void)
{
  vvt2_pin.setPinLow();
}

void vvtControl(void)
{
  if( (configPage6.vvtEnabled == 1) && (currentStatus.coolant >= temperatureRemoveOffset(configPage4.vvtMinClt)) && (currentStatus.rotationStatus==EngineRotationStatus::Running))
  {
    if(vvtTimeHold == false) 
    {
      vvtWarmTime = runSecsX10;
      vvtTimeHold = true;
    }

    //Calculate the current cam angle for miata trigger
    if( configPage4.TrigPattern == 9 ) { currentStatus.vvt1Angle = getCamAngle_Miata9905(); }

    constexpr uint32_t VVT_TIME_DELAY_MULTIPLIER = 50;
    if( (vvtIsHot == true) || hasIntervalElapsed(runSecsX10, vvtWarmTime, configPage4.vvtDelay * VVT_TIME_DELAY_MULTIPLIER) )
    {
      vvtIsHot = true;

      if( (configPage6.vvtMode == VVT_MODE_OPEN_LOOP) || (configPage6.vvtMode == VVT_MODE_ONOFF) )
      {
        //Lookup VVT duty based on either MAP or TPS
        if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentStatus.vvt1Duty = get3DTableValue(&vvtTable, (currentStatus.TPS * 2U), currentStatus.RPM); }
        else { currentStatus.vvt1Duty = get3DTableValue(&vvtTable, currentStatus.MAP, currentStatus.RPM); }

        //VVT table can be used for controlling on/off switching. If this is turned on, then disregard any interpolation or non-binary values
        if( (configPage6.vvtMode == VVT_MODE_ONOFF) && (currentStatus.vvt1Duty < 200) ) { currentStatus.vvt1Duty = 0; }

        vvt1_pwm_value = halfPercentage(currentStatus.vvt1Duty, vvt_pwm_max_count);

        if (configPage10.vvt2Enabled == 1) // same for VVT2 if it's enabled
        {
          //Lookup VVT duty based on either MAP or TPS
          if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentStatus.vvt2Duty = get3DTableValue(&vvt2Table, (currentStatus.TPS * 2U), currentStatus.RPM); }
          else { currentStatus.vvt2Duty = get3DTableValue(&vvt2Table, currentStatus.MAP, currentStatus.RPM); }

          //VVT table can be used for controlling on/off switching. If this is turned on, then disregard any interpolation or non-binary values
          if( (configPage6.vvtMode == VVT_MODE_ONOFF) && (currentStatus.vvt2Duty < 200) ) { currentStatus.vvt2Duty = 0; }

          vvt2_pwm_value = halfPercentage(currentStatus.vvt2Duty, vvt_pwm_max_count);
        }

      } //Open loop
      else if( (configPage6.vvtMode == VVT_MODE_CLOSED_LOOP) )
      {
        //Lookup VVT duty based on either MAP or TPS
        if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentStatus.vvt1TargetAngle = get3DTableValue(&vvtTable, (currentStatus.TPS * 2U), currentStatus.RPM); }
        else { currentStatus.vvt1TargetAngle = get3DTableValue(&vvtTable, currentStatus.MAP, currentStatus.RPM); }

        if( (vvtCounter & 31) == 1) { //This only needs to be run very infrequently, once every 32 calls to vvtControl(). This is approx. once per second
          setVvtPidTunings(vvtPID, configPage10, configPage6.vvtPWMdir);  
        }

        // safety check that the cam angles are ok. The engine will be totally undriveable if the cam sensor is faulty and giving wrong cam angles, so if that happens, default to 0 duty.
        // This also prevents using zero or negative current angle values for PID adjustment, because those don't work in integer PID.
        if ( currentStatus.vvt1Angle <=  configPage10.vvtCLMinAng || currentStatus.vvt1Angle > configPage10.vvtCLMaxAng )
        {
          currentStatus.vvt1Duty = 0;
          vvt1_pwm_value = halfPercentage(currentStatus.vvt1Duty, vvt_pwm_max_count);
          currentStatus.vvt1AngleError = true;
        }
        //Check that we're not already at the angle we want to be
        else if((configPage6.vvtCLUseHold > 0) && (currentStatus.vvt1TargetAngle == currentStatus.vvt1Angle) )
        {
          currentStatus.vvt1Duty = configPage10.vvtCLholdDuty;
          vvt1_pwm_value = halfPercentage(currentStatus.vvt1Duty, vvt_pwm_max_count);
          vvtPID.reset(currentStatus.vvt1Angle);
          currentStatus.vvt1AngleError = false;
        }
        else
        {
          //If not already at target angle, calculate new value from PID
          int32_t pidOutput = 0;
          vvtPID.setSetPoint(currentStatus.vvt1TargetAngle);
          bool PID_compute = vvtPID.compute(millis(), currentStatus.vvt1Angle, &pidOutput);
          if(PID_compute == true) 
          { 
            currentStatus.vvt1Duty = (uint8_t)pidOutput;
            vvt1_pwm_value = halfPercentage(currentStatus.vvt1Duty, vvt_pwm_max_count); 
          }
          currentStatus.vvt1AngleError = false;
        }

        if (configPage10.vvt2Enabled == 1) // same for VVT2 if it's enabled
        {
          if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentStatus.vvt2TargetAngle = get3DTableValue(&vvt2Table, (currentStatus.TPS * 2U), currentStatus.RPM); }
          else { currentStatus.vvt2TargetAngle = get3DTableValue(&vvt2Table, currentStatus.MAP, currentStatus.RPM); }

          if( (vvtCounter & 31) == 1) { //This only needs to be run very infrequently, once every 32 calls to vvtControl(). This is approx. once per second
            setVvtPidTunings(vvt2PID, configPage10, configPage4.vvt2PWMdir);
        }

          // safety check that the cam angles are ok. The engine will be totally undriveable if the cam sensor is faulty and giving wrong cam angles, so if that happens, default to 0 duty.
          // This also prevents using zero or negative current angle values for PID adjustment, because those don't work in integer PID.
          if ( currentStatus.vvt2Angle <= configPage10.vvtCLMinAng || currentStatus.vvt2Angle > configPage10.vvtCLMaxAng )
          {
            currentStatus.vvt2Duty = 0;
            vvt2_pwm_value = halfPercentage(currentStatus.vvt2Duty, vvt_pwm_max_count);
            currentStatus.vvt2AngleError = true;
          }
          //Check that we're not already at the angle we want to be
          else if((configPage6.vvtCLUseHold > 0) && (currentStatus.vvt2TargetAngle == currentStatus.vvt2Angle) )
          {
            currentStatus.vvt2Duty = configPage10.vvtCLholdDuty;
            vvt2_pwm_value = halfPercentage(currentStatus.vvt2Duty, vvt_pwm_max_count);
            vvt2PID.reset(currentStatus.vvt2Angle);
            currentStatus.vvt2AngleError = false;
          }
          else
          {
            vvt2PID.setSetPoint(currentStatus.vvt2TargetAngle);
            //If not already at target angle, calculate new value from PID
            int32_t pidOutput = 0;
            bool PID_compute = vvt2PID.compute(millis(), currentStatus.vvt2Angle, &pidOutput);
            if(PID_compute == true) 
            { 
              currentStatus.vvt2Duty = (uint8_t)pidOutput;
              vvt2_pwm_value = halfPercentage(currentStatus.vvt2Duty, vvt_pwm_max_count); 
            }
            currentStatus.vvt2AngleError = false;
          }
        }
        vvtCounter++;
      }

      //Set the PWM state based on the above lookups
      if( configPage10.wmiEnabled == 0 ) //Added possibility to use vvt and wmi at the same time
      {
        if( (currentStatus.vvt1Duty == 0) && (currentStatus.vvt2Duty == 0) )
        {
          //Make sure solenoid is off (0% duty)
          vvt1Off();
          vvt2Off();
          vvt1_pwm_state = false;
          vvt1_max_pwm = false;
          vvt2_pwm_state = false;
          vvt2_max_pwm = false;
          DISABLE_VVT_TIMER();
        }
        else if( (currentStatus.vvt1Duty >= 200) && (currentStatus.vvt2Duty >= 200) )
        {
          //Make sure solenoid is on (100% duty)
          vvt1On();
          vvt2On();
          vvt1_pwm_state = true;
          vvt1_max_pwm = true;
          vvt2_pwm_state = true;
          vvt2_max_pwm = true;
          DISABLE_VVT_TIMER();
        }
        else
        {
          //Duty cycle is between 0 and 100. Make sure the timer is enabled
          ENABLE_VVT_TIMER();
          if(currentStatus.vvt1Duty < 200) { vvt1_max_pwm = false; }
          if(currentStatus.vvt2Duty < 200) { vvt2_max_pwm = false; }
        }
      }
      else
      {
        if( currentStatus.vvt1Duty == 0 )
        {
          //Make sure solenoid is off (0% duty)
          vvt1Off();
          vvt1_pwm_state = false;
          vvt1_max_pwm = false;
        }
        else if( currentStatus.vvt1Duty >= 200 )
        {
          //Make sure solenoid is on (100% duty)
          vvt1On();
          vvt1_pwm_state = true;
          vvt1_max_pwm = true;
        }
        else
        {
          //Duty cycle is between 0 and 100. Make sure the timer is enabled
          ENABLE_VVT_TIMER();
          if(currentStatus.vvt1Duty < 200) { vvt1_max_pwm = false; }
        }
      }
    }
  }
  else 
  { 
    if (configPage10.wmiEnabled == 0)
    {
      // Disable timer channel
      DISABLE_VVT_TIMER();
      currentStatus.vvt2Duty = 0;
      vvt2_pwm_value = 0;
      vvt2_pwm_state = false;
      vvt2_max_pwm = false;
    }
    currentStatus.vvt1Duty = 0;
    vvt1_pwm_value = 0;
    vvt1_pwm_state = false;
    vvt1_max_pwm = false;
    vvtTimeHold = false;
  } 
}

void nitrousControl(void)
{
  currentStatus.nitrousActive = false;
  currentStatus.nitrous_status = NITROUS_OFF; //Reset the current state

  if(configPage10.n2o_enable > 0)
  {
    bool isArmed = n2o_arming_pin.isPinHigh();
    if (configPage10.n2o_pin_polarity == 1) { isArmed = !isArmed; } //If nitrous is active when pin is low, flip the reading (n2o_pin_polarity = 0 = active when High)

    //Perform the main checks to see if nitrous is ready
    if( (isArmed == true) && (currentStatus.coolant > temperatureRemoveOffset(configPage10.n2o_minCLT)) && (currentStatus.TPS > configPage10.n2o_minTPS) && (currentStatus.O2 < configPage10.n2o_maxAFR) && (currentStatus.MAP < (uint16_t)(configPage10.n2o_maxMAP * 2U)) )
    {
      //Config page values are divided by 100 to fit within a byte. Multiply them back out to real values. 
      uint16_t realStage1MinRPM = (uint16_t)configPage10.n2o_stage1_minRPM * 100;
      uint16_t realStage1MaxRPM = (uint16_t)configPage10.n2o_stage1_maxRPM * 100;
      uint16_t realStage2MinRPM = (uint16_t)configPage10.n2o_stage2_minRPM * 100;
      uint16_t realStage2MaxRPM = (uint16_t)configPage10.n2o_stage2_maxRPM * 100;

      //The nitrous state is set to 0 and then the subsequent stages are added
      // OFF    = 0
      // STAGE1 = 1
      // STAGE2 = 2
      // BOTH   = 3 (ie STAGE1 + STAGE2 = BOTH)
      if( (currentStatus.RPM > realStage1MinRPM) && (currentStatus.RPM < realStage1MaxRPM) )
      {
        currentStatus.nitrous_status += NITROUS_STAGE1;
        currentStatus.nitrousActive = true;
        n2o_stage1_pin.setPinHigh();
      }
      if(configPage10.n2o_enable == NITROUS_STAGE2) //This is really just a sanity check
      {
        if( (currentStatus.RPM > realStage2MinRPM) && (currentStatus.RPM < realStage2MaxRPM) )
        {
          currentStatus.nitrous_status += NITROUS_STAGE2;
          currentStatus.nitrousActive = true;
          n2o_stage2_pin.setPinHigh();
        }
      }
    }
  }

  if (currentStatus.nitrousActive == false)
  {
    if(configPage10.n2o_enable > 0)
    {
      n2o_stage1_pin.setPinLow();
      n2o_stage2_pin.setPinLow();
    }
  }
}

// Water methanol injection control
void wmiControl(void)
{
  int wmiPW = 0;
  
  // wmi can only work when vvt2 is disabled 
  if( (configPage10.vvt2Enabled == 0) && (configPage10.wmiEnabled >= 1) )
  {
    if( isWmiTankEmpty() )
    {
     currentStatus.wmiTankEmpty = false;
      if( (currentStatus.TPS >= configPage10.wmiTPS) && (currentStatus.RPMdiv100 >= configPage10.wmiRPM) && ( (currentStatus.MAP / 2U) >= configPage10.wmiMAP) && ( temperatureAddOffset(currentStatus.IAT) >= configPage10.wmiIAT) )
      {
        switch(configPage10.wmiMode)
        {
        case WMI_MODE_SIMPLE:
          // Simple mode - Output is turned on when preset boost level is reached
          wmiPW = 200;
          break;
        case WMI_MODE_PROPORTIONAL:
          // Proportional Mode - Output PWM is proportionally controlled between two MAP values - MAP Value 1 = PWM:0% / MAP Value 2 = PWM:100%
          wmiPW = map(currentStatus.MAP/2U, configPage10.wmiMAP, configPage10.wmiMAP2, 0, 200);
          break;
        case WMI_MODE_OPENLOOP:
          //  Mapped open loop - Output PWM follows 2D map value (RPM vs MAP) Cell value contains desired PWM% [range 0-100%]
          wmiPW = get3DTableValue(&wmiTable, currentStatus.MAP, currentStatus.RPM);
          break;
        case WMI_MODE_CLOSEDLOOP:
          // Mapped closed loop - Output PWM follows injector duty cycle with 2D correction map applied (RPM vs MAP). Cell value contains correction value% [nom 100%] 
          wmiPW = max(0, ((int)fuelSchedule1.pw + configPage10.wmiOffset)) * get3DTableValue(&wmiTable, currentStatus.MAP, currentStatus.RPM) / 200;
          break;
        default:
          // Wrong mode
          wmiPW = 0;
          break;
        }
        if (wmiPW > 200) { wmiPW = 200; } //without this the duty can get beyond 100%
      }
    }
    else { currentStatus.wmiTankEmpty = true; }

    currentStatus.wmiPW = wmiPW;
    vvt2_pwm_value = halfPercentage(currentStatus.wmiPW, vvt_pwm_max_count);

    if(wmiPW == 0)
    {
      // Make sure water pump is off
      vvt2Off();
      vvt2_pwm_state = false;
      vvt2_max_pwm = false;
      if( configPage6.vvtEnabled == 0 ) { DISABLE_VVT_TIMER(); }
      digitalWrite(pinNumbers.pinWMIEnabled, LOW);
    }
    else
    {
      digitalWrite(pinNumbers.pinWMIEnabled, HIGH);
      if (wmiPW >= 200)
      {
        // Make sure water pump is on (100% duty)
        vvt2On();
        vvt2_pwm_state = true;
        vvt2_max_pwm = true;
        if( configPage6.vvtEnabled == 0 ) { DISABLE_VVT_TIMER(); }
      }
      else
      {
        vvt2_max_pwm = false;
        ENABLE_VVT_TIMER();
      }
    }
  }
}

void boostDisable(void)
{
  boostPID.initialize(currentStatus.MAP); //This resets the ITerm value to prevent rubber banding
  currentStatus.boostDuty = 0;
  DISABLE_BOOST_TIMER(); //Turn off timer
  boost_pin.setPinLow(); //Make sure solenoid is off (0% duty)
}

//The interrupt to control the Boost PWM
void boostInterrupt(void)
{
  if (boost_pwm_state == true)
  {
    #if defined(CORE_TEENSY41) //PIT TIMERS count down and have opposite effect on PWM
    boost_pin.setPinHigh();
    #else
    boost_pin.setPinLow();  // Switch pin to low
    #endif
    SET_COMPARE(BOOST_TIMER_COMPARE, BOOST_TIMER_COUNTER + (boost_pwm_max_count - boost_pwm_cur_value) );
    boost_pwm_state = false;
  }
  else
  {
    #if defined(CORE_TEENSY41) //PIT TIMERS count down and have opposite effect on PWM
    boost_pin.setPinLow();
    #else
    boost_pin.setPinHigh();  // Switch pin high
    #endif
    SET_COMPARE(BOOST_TIMER_COMPARE, BOOST_TIMER_COUNTER + boost_pwm_target_value);
    boost_pwm_cur_value = boost_pwm_target_value;
    boost_pwm_state = true;
  }
}

//The interrupt to control the VVT PWM
void vvtInterrupt(void)
{
  if ( ((vvt1_pwm_state == false) || (vvt1_max_pwm == true)) && ((vvt2_pwm_state == false) || (vvt2_max_pwm == true)) )
  {
    if( (vvt1_pwm_value > 0) && (vvt1_max_pwm == false) ) //Don't toggle if at 0%
    {
      #if defined(CORE_TEENSY41)
      vvt1Off();
      #else
      vvt1On();
      #endif
      vvt1_pwm_state = true;
    }
    if( (vvt2_pwm_value > 0) && (vvt2_max_pwm == false) ) //Don't toggle if at 0%
    {
      #if defined(CORE_TEENSY41)
      vvt2Off();
      #else
      vvt2On();
      #endif
      vvt2_pwm_state = true;
    }

    if( (vvt1_pwm_state == true) && ((vvt1_pwm_value <= vvt2_pwm_value) || (vvt2_pwm_state == false)) )
    {
      SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + vvt1_pwm_value);
      vvt1_pwm_cur_value = vvt1_pwm_value;
      vvt2_pwm_cur_value = vvt2_pwm_value;
      if (vvt1_pwm_value == vvt2_pwm_value) { nextVVT = 2; } //Next event is for both PWM
      else { nextVVT = 0; } //Next event is for PWM0
    }
    else if( vvt2_pwm_state == true )
    {
      SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + vvt2_pwm_value);
      vvt1_pwm_cur_value = vvt1_pwm_value;
      vvt2_pwm_cur_value = vvt2_pwm_value;
      nextVVT = 1; //Next event is for PWM1
    }
    else { SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + vvt_pwm_max_count); } //Shouldn't ever get here
  }
  else
  {
    if(nextVVT == 0)
    {
      if(vvt1_pwm_value < (long)vvt_pwm_max_count) //Don't toggle if at 100%
      {
        #if defined(CORE_TEENSY41)
        vvt1On();
        #else
        vvt1Off();
        #endif
        vvt1_pwm_state = false;
        vvt1_max_pwm = false;
      }
      else { vvt1_max_pwm = true; }
      nextVVT = 1; //Next event is for PWM1
      if(vvt2_pwm_state == true){ SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt2_pwm_cur_value - vvt1_pwm_cur_value) ); }
      else
      { 
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvt1_pwm_cur_value) );
        nextVVT = 2; //Next event is for both PWM
      }
    }
    else if (nextVVT == 1)
    {
      if(vvt2_pwm_value < (long)vvt_pwm_max_count) //Don't toggle if at 100%
      {
        #if defined(CORE_TEENSY41)
        vvt2On();
        #else
        vvt2Off();
        #endif
        vvt2_pwm_state = false;
        vvt2_max_pwm = false;
      }
      else { vvt2_max_pwm = true; }
      nextVVT = 0; //Next event is for PWM0
      if(vvt1_pwm_state == true) { SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt1_pwm_cur_value - vvt2_pwm_cur_value) ); }
      else
      { 
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvt2_pwm_cur_value) );
        nextVVT = 2; //Next event is for both PWM
      }
    }
    else
    {
      if(vvt1_pwm_value < (long)vvt_pwm_max_count) //Don't toggle if at 100%
      {
       #if defined(CORE_TEENSY41)
        vvt1On();
        #else
        vvt1Off();
        #endif
        vvt1_pwm_state = false;
        vvt1_max_pwm = false;
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvt1_pwm_cur_value) );
      }
      else { vvt1_max_pwm = true; }
      if(vvt2_pwm_value < (long)vvt_pwm_max_count) //Don't toggle if at 100%
      {
        #if defined(CORE_TEENSY41)
        vvt2On();
        #else
        vvt2Off();
        #endif
        vvt2_pwm_state = false;
        vvt2_max_pwm = false;
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvt2_pwm_cur_value) );
      }
      else { vvt2_max_pwm = true; }
    }
  }
}