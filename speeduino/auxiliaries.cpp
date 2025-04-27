/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
#include "globals.h"
#include "auxiliaries.h"
#include "maths.h"
#include "src/PID_v1/PID_v1.h"
#include "decoders.h"
#include "timers.h"
#include "utilities.h"

static long vvt1_pwm_value;
static long vvt2_pwm_value;
volatile unsigned int vvt1_pwm_cur_value;
volatile unsigned int vvt2_pwm_cur_value;
static long vvt_pid_target_angle;
static long vvt2_pid_target_angle;
static long vvt_pid_current_angle;
static long vvt2_pid_current_angle;
volatile bool vvt1_pwm_state;
volatile bool vvt2_pwm_state;
volatile bool vvt1_max_pwm;
volatile bool vvt2_max_pwm;
volatile char nextVVT;
byte boostCounter;
byte vvtCounter;

volatile PORT_TYPE *boost_pin_port;
volatile PINMASK_TYPE boost_pin_mask;
volatile PORT_TYPE *n2o_stage1_pin_port;
volatile PINMASK_TYPE n2o_stage1_pin_mask;
volatile PORT_TYPE *n2o_stage2_pin_port;
volatile PINMASK_TYPE n2o_stage2_pin_mask;
volatile PORT_TYPE *n2o_arming_pin_port;
volatile PINMASK_TYPE n2o_arming_pin_mask;
volatile PORT_TYPE *aircon_comp_pin_port;
volatile PINMASK_TYPE aircon_comp_pin_mask;
volatile PORT_TYPE *aircon_fan_pin_port;
volatile PINMASK_TYPE aircon_fan_pin_mask;
volatile PORT_TYPE *aircon_req_pin_port;
volatile PINMASK_TYPE aircon_req_pin_mask;
volatile PORT_TYPE *vvt1_pin_port;
volatile PINMASK_TYPE vvt1_pin_mask;
volatile PORT_TYPE *vvt2_pin_port;
volatile PINMASK_TYPE vvt2_pin_mask;
volatile PORT_TYPE *fan_pin_port;
volatile PINMASK_TYPE fan_pin_mask;

#if defined(PWM_FAN_AVAILABLE)//PWM fan not available on Arduino MEGA
volatile bool fan_pwm_state;
uint16_t fan_pwm_max_count; //Used for variable PWM frequency
volatile unsigned int fan_pwm_cur_value;
long fan_pwm_value;
#endif
static constexpr table2D fanPWMTable(_countof(configPage9.PWMFanDuty), configPage9.PWMFanDuty, configPage6.fanPWMBins);

bool acIsEnabled;
bool acStandAloneFanIsEnabled;
uint8_t acStartDelay;
uint8_t acTPSLockoutDelay;
uint8_t acRPMLockoutDelay;
uint8_t acAfterEngineStartDelay;
bool waitedAfterCranking; // This starts false and prevents the A/C from running until a few seconds after cranking

long boost_pwm_target_value;
volatile bool boost_pwm_state;
volatile unsigned int boost_pwm_cur_value = 0;

uint32_t vvtWarmTime;
bool vvtIsHot;
bool vvtTimeHold;
uint16_t vvt_pwm_max_count; //Used for variable PWM frequency
uint16_t boost_pwm_max_count; //Used for variable PWM frequency
static constexpr table2D flexBoostTable(_countof(configPage10.flexBoostAdj), configPage10.flexBoostAdj, configPage10.flexBoostBins);

//Old PID method. Retained in case the new one has issues
//integerPID boostPID(&MAPx100, &boost_pwm_target_value, &boostTargetx100, configPage6.boostKP, configPage6.boostKI, configPage6.boostKD, DIRECT);
integerPID_ideal boostPID(&currentStatus.MAP, &currentStatus.boostDuty , &currentStatus.boostTarget, &configPage10.boostSens, &configPage10.boostIntv, configPage6.boostKP, configPage6.boostKI, configPage6.boostKD, DIRECT); //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call
integerPID vvtPID(&vvt_pid_current_angle, &currentStatus.vvt1Duty, &vvt_pid_target_angle, configPage10.vvtCLKP, configPage10.vvtCLKI, configPage10.vvtCLKD, configPage6.vvtPWMdir); //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call
integerPID vvt2PID(&vvt2_pid_current_angle, &currentStatus.vvt2Duty, &vvt2_pid_target_angle, configPage10.vvtCLKP, configPage10.vvtCLKI, configPage10.vvtCLKD, configPage4.vvt2PWMdir); //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call

static inline void checkAirConCoolantLockout(void);
static inline void checkAirConTPSLockout(void);
static inline void checkAirConRPMLockout(void);

/*
Air Conditioning Control
*/
void initialiseAirCon(void)
{
  if( (configPage15.airConEnable) == 1 &&
      pinAirConRequest != 0 &&
      pinAirConComp != 0 )
  {
    // Hold the A/C off until a few seconds after cranking
    acAfterEngineStartDelay = 0;
    waitedAfterCranking = false;

    acStartDelay = 0;
    acTPSLockoutDelay = 0;
    acRPMLockoutDelay = 0;

    BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_REQUEST);     // Bit 0
    BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_COMPRESSOR);  // Bit 1
    BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_RPM_LOCKOUT); // Bit 2
    BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_TPS_LOCKOUT); // Bit 3
    BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_TURNING_ON);  // Bit 4
    BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_CLT_LOCKOUT); // Bit 5
    BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_FAN);         // Bit 6
    aircon_req_pin_port = portInputRegister(digitalPinToPort(pinAirConRequest));
    aircon_req_pin_mask = digitalPinToBitMask(pinAirConRequest);
    aircon_comp_pin_port = portOutputRegister(digitalPinToPort(pinAirConComp));
    aircon_comp_pin_mask = digitalPinToBitMask(pinAirConComp);

    AIRCON_OFF();

    if((configPage15.airConFanEnabled > 0) && (pinAirConFan != 0))
    {
      aircon_fan_pin_port = portOutputRegister(digitalPinToPort(pinAirConFan));
      aircon_fan_pin_mask = digitalPinToBitMask(pinAirConFan);
      AIRCON_FAN_OFF();
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

void airConControl(void)
{
  if(acIsEnabled == true)
  {
    // ------------------------------------------------------------------------------------------------------
    // Check that the engine has been running past the post-start delay period before enabling the compressor
    // ------------------------------------------------------------------------------------------------------
    if (BIT_CHECK(currentStatus.engine, BIT_ENGINE_RUN))
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
        BIT_CHECK(currentStatus.airConStatus, BIT_AIRCON_TPS_LOCKOUT) == false &&
        BIT_CHECK(currentStatus.airConStatus, BIT_AIRCON_RPM_LOCKOUT) == false &&
        BIT_CHECK(currentStatus.airConStatus, BIT_AIRCON_CLT_LOCKOUT) == false )
    {
      // Set the BIT_AIRCON_TURNING_ON bit to notify the idle system to idle up & the cooling fan to start (if enabled)
      BIT_SET(currentStatus.airConStatus, BIT_AIRCON_TURNING_ON);

      // Stand-alone fan operation
      if(acStandAloneFanIsEnabled == true)
      {
        AIRCON_FAN_ON();
      }

      // Start the A/C compressor after the "Compressor On" delay period
      if(acStartDelay >= configPage15.airConCompOnDelay)
      {
        AIRCON_ON();
      }
      else
      {
        acStartDelay++;
      }
    }
    else
    {
      BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_TURNING_ON);

      // Stand-alone fan operation
      if(acStandAloneFanIsEnabled == true)
      {
        AIRCON_FAN_OFF();
      }

      AIRCON_OFF();
      acStartDelay = 0;
    }
  }
}

bool READ_AIRCON_REQUEST(void)
{
  if(acIsEnabled == false)
  {
    return false;
  }
  // Read the status of the A/C request pin (A/C button), taking into account the pin's polarity
  bool acReqPinStatus = ( ((configPage15.airConReqPol)==1) ? 
                             !!(*aircon_req_pin_port & aircon_req_pin_mask) :
                             !(*aircon_req_pin_port & aircon_req_pin_mask));
  BIT_WRITE(currentStatus.airConStatus, BIT_AIRCON_REQUEST, acReqPinStatus);
  return acReqPinStatus;
}

static inline void checkAirConCoolantLockout(void)
{
  // ---------------------------
  // Coolant Temperature Lockout
  // ---------------------------
  int offTemp = (int)configPage15.airConClTempCut - CALIBRATION_TEMPERATURE_OFFSET;
  if (currentStatus.coolant > offTemp)
  {
    // A/C is cut off due to high coolant
    BIT_SET(currentStatus.airConStatus, BIT_AIRCON_CLT_LOCKOUT);
  }
  else if (currentStatus.coolant < (offTemp - 1))
  {
    // Adds a bit of hysteresis (2 degrees) to removing the lockout
    // Yes, it is 2 degrees (not 1 degree or 3 degrees) because we go "> offTemp" to enable and "< (offtemp-1)" to disable,
    // e.g. if offTemp is 100, it needs to go GREATER than 100 to enable, i.e. 101, and then 98 to disable,
    // because the coolant temp is an integer. So 98.5 degrees to 100.5 degrees is the analog null zone where nothing happens,
    // depending on sensor calibration and table interpolation.
    // Hopefully offTemp wasn't -40... otherwise underflow... but that would be ridiculous
    BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_CLT_LOCKOUT);
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
    BIT_SET(currentStatus.airConStatus, BIT_AIRCON_TPS_LOCKOUT);
    acTPSLockoutDelay = 0;
  }
  else if ( (BIT_CHECK(currentStatus.airConStatus, BIT_AIRCON_TPS_LOCKOUT) == true) &&
            (currentStatus.TPS <= configPage15.airConTPSCut) )
  {
    // No need for hysteresis as we have the stand-down delay period after the high TPS condition goes away.
    if (acTPSLockoutDelay >= configPage15.airConTPSCutTime)
    {
      BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_TPS_LOCKOUT);
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
    BIT_SET(currentStatus.airConStatus, BIT_AIRCON_RPM_LOCKOUT);
    acRPMLockoutDelay = 0;
  }
  else if ( (currentStatus.RPM >= (configPage15.airConMinRPMdiv10 * 10)) &&
            (currentStatus.RPMdiv100 <= configPage15.airConMaxRPMdiv100) )
  {
    // No need to add hysteresis as we have the stand-down delay period after the high/low RPM condition goes away.
    if (acRPMLockoutDelay >= configPage15.airConRPMCutTime)
    {
      BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_RPM_LOCKOUT);
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


/*
Fan control
*/
void initialiseFan(void)
{
  fan_pin_port = portOutputRegister(digitalPinToPort(pinFan));
  fan_pin_mask = digitalPinToBitMask(pinFan);
  FAN_OFF();  //Initialise program with the fan in the off state
  BIT_CLEAR(currentStatus.status4, BIT_STATUS4_FAN);
  currentStatus.fanDuty = 0;

  #if defined(PWM_FAN_AVAILABLE)
    DISABLE_FAN_TIMER(); //disable FAN timer if available
    if ( configPage2.fanEnable == 2 ) // PWM Fan control
    {
      #if defined(CORE_TEENSY)
        fan_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (32U * configPage6.fanFreq * 2U)); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
      #endif
      fan_pwm_value = 0;
    }
  #endif
}

void fanControl(void)
{
  if( configPage2.fanEnable == 1 ) // regular on/off fan control
  {
    int onTemp = (int)configPage6.fanSP - CALIBRATION_TEMPERATURE_OFFSET;
    int offTemp = onTemp - configPage6.fanHyster;
    bool fanPermit = false;

    
    if ( configPage2.fanWhenOff == true) { fanPermit = true; }
    else { fanPermit = BIT_CHECK(currentStatus.engine, BIT_ENGINE_RUN); }

    if ( (fanPermit == true) &&
         ((currentStatus.coolant >= onTemp) || 
           ((configPage15.airConTurnsFanOn) == 1 &&
           BIT_CHECK(currentStatus.airConStatus, BIT_AIRCON_TURNING_ON) == true)) )
    {
      //Fan needs to be turned on - either by high coolant temp, or from an A/C request (to ensure there is airflow over the A/C radiator).
      if(BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && (configPage2.fanWhenCranking == 0))
      {
        //If the user has elected to disable the fan during cranking, make sure it's off 
        FAN_OFF();
        BIT_CLEAR(currentStatus.status4, BIT_STATUS4_FAN);
      }
      else 
      {
        FAN_ON();
        BIT_SET(currentStatus.status4, BIT_STATUS4_FAN);
      }
    }
    else if ( (currentStatus.coolant <= offTemp) || (!fanPermit) )
    {
      //Fan needs to be turned off. 
      FAN_OFF();
      BIT_CLEAR(currentStatus.status4, BIT_STATUS4_FAN);
    }
  }
  else if( configPage2.fanEnable == 2 )// PWM Fan control
  {
    bool fanPermit = false;
    if ( configPage2.fanWhenOff == true) { fanPermit = true; }
    else { fanPermit = BIT_CHECK(currentStatus.engine, BIT_ENGINE_RUN); }
    if (fanPermit == true)
      {
      if(BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && (configPage2.fanWhenCranking == 0))
      {
        currentStatus.fanDuty = 0; //If the user has elected to disable the fan during cranking, make sure it's off 
        BIT_CLEAR(currentStatus.status4, BIT_STATUS4_FAN);
        #if defined(PWM_FAN_AVAILABLE)//PWM fan not available on Arduino MEGA
          DISABLE_FAN_TIMER();
        #endif
      }
      else
      {
        byte tempFanDuty = table2D_getValue(&fanPWMTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //In normal situation read PWM duty from the table
        if((configPage15.airConTurnsFanOn) == 1 &&
           BIT_CHECK(currentStatus.airConStatus, BIT_AIRCON_TURNING_ON) == true)
        {
          // Clamp the fan duty to airConPwmFanMinDuty or above, to ensure there is airflow over the A/C radiator
          if(tempFanDuty < configPage15.airConPwmFanMinDuty)
          {
            tempFanDuty = configPage15.airConPwmFanMinDuty;
          }
        }
        currentStatus.fanDuty = tempFanDuty;
        #if defined(PWM_FAN_AVAILABLE)
          fan_pwm_value = halfPercentage(currentStatus.fanDuty, fan_pwm_max_count); //update FAN PWM value last
          if (currentStatus.fanDuty > 0)
          {
            ENABLE_FAN_TIMER();
            BIT_SET(currentStatus.status4, BIT_STATUS4_FAN);
          }
        #endif
      }
    }
    else if (!fanPermit)
    {
      currentStatus.fanDuty = 0; ////If the user has elected to disable the fan when engine is not running, make sure it's off 
      BIT_CLEAR(currentStatus.status4, BIT_STATUS4_FAN);
    }

    #if defined(PWM_FAN_AVAILABLE)
      if(currentStatus.fanDuty == 0)
      {
        //Make sure fan has 0% duty)
        FAN_OFF();
        BIT_CLEAR(currentStatus.status4, BIT_STATUS4_FAN);
        DISABLE_FAN_TIMER();
      }
      else if (currentStatus.fanDuty == 200)
      {
        //Make sure fan has 100% duty
        FAN_ON();
        BIT_SET(currentStatus.status4, BIT_STATUS4_FAN);
        DISABLE_FAN_TIMER();
      }
    #else //Just in case if user still has selected PWM fan in TS, even though it warns that it doesn't work on mega.
      if(currentStatus.fanDuty == 0)
      {
        //Make sure fan has 0% duty)
        FAN_OFF();
        BIT_CLEAR(currentStatus.status4, BIT_STATUS4_FAN);
      }
      else if (currentStatus.fanDuty > 0)
      {
        //Make sure fan has 100% duty
        FAN_ON();
        BIT_SET(currentStatus.status4, BIT_STATUS4_FAN);
      }
    #endif
  }
}

void initialiseAuxPWM(void)
{
  boost_pin_port = portOutputRegister(digitalPinToPort(pinBoost));
  boost_pin_mask = digitalPinToBitMask(pinBoost);
  vvt1_pin_port = portOutputRegister(digitalPinToPort(pinVVT_1));
  vvt1_pin_mask = digitalPinToBitMask(pinVVT_1);
  vvt2_pin_port = portOutputRegister(digitalPinToPort(pinVVT_2));
  vvt2_pin_mask = digitalPinToBitMask(pinVVT_2);
  n2o_stage1_pin_port = portOutputRegister(digitalPinToPort(configPage10.n2o_stage1_pin));
  n2o_stage1_pin_mask = digitalPinToBitMask(configPage10.n2o_stage1_pin);
  n2o_stage2_pin_port = portOutputRegister(digitalPinToPort(configPage10.n2o_stage2_pin));
  n2o_stage2_pin_mask = digitalPinToBitMask(configPage10.n2o_stage2_pin);
  n2o_arming_pin_port = portInputRegister(digitalPinToPort(configPage10.n2o_arming_pin));
  n2o_arming_pin_mask = digitalPinToBitMask(configPage10.n2o_arming_pin);

  //This is a safety check that will be true if the board is uninitialised. This prevents hangs on a new board that could otherwise try to write to an invalid pin port/mask (Without this a new Teensy 4.x hangs on startup)
  //The n2o_minTPS variable is capped at 100 by TS, so 255 indicates a new board.
  if(configPage10.n2o_minTPS == 255) { configPage10.n2o_enable = 0; }

  if(configPage10.n2o_enable > 0)
  {
    //The pin modes are only set if the if n2o is enabled to prevent them conflicting with other outputs. 
    if(configPage10.n2o_pin_polarity == 1) { pinMode(configPage10.n2o_arming_pin, INPUT_PULLUP); }
    else { pinMode(configPage10.n2o_arming_pin, INPUT); }
  }

  boostPID.SetOutputLimits(configPage2.boostMinDuty, configPage2.boostMaxDuty);
  if(configPage6.boostMode == BOOST_MODE_SIMPLE) { boostPID.SetTunings(SIMPLE_BOOST_P, SIMPLE_BOOST_I, SIMPLE_BOOST_D); }
  else { boostPID.SetTunings(configPage6.boostKP, configPage6.boostKI, configPage6.boostKD); }

  if( configPage6.vvtEnabled > 0)
  {
    currentStatus.vvt1Angle = 0;
    currentStatus.vvt2Angle = 0;

    #if defined(CORE_AVR)
      vvt_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (16U * configPage6.vvtFreq * 2U)); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
    #elif defined(CORE_TEENSY35)
      vvt_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (32U * configPage6.vvtFreq * 2U)); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
    #elif defined(CORE_TEENSY41)
      vvt_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (2U * configPage6.vvtFreq * 2U)); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
    #endif

    if(configPage6.vvtMode == VVT_MODE_CLOSED_LOOP)
    {
      vvtPID.SetOutputLimits(configPage10.vvtCLminDuty, configPage10.vvtCLmaxDuty);
      vvtPID.SetTunings(configPage10.vvtCLKP, configPage10.vvtCLKI, configPage10.vvtCLKD);
      vvtPID.SetSampleTime(33); //30Hz is 33,33ms
      vvtPID.SetMode(AUTOMATIC); //Turn PID on
      if (configPage10.vvt2Enabled == 1) // same for VVT2 if it's enabled
      {
        vvt2PID.SetOutputLimits(configPage10.vvtCLminDuty, configPage10.vvtCLmaxDuty);
        vvt2PID.SetTunings(configPage10.vvtCLKP, configPage10.vvtCLKI, configPage10.vvtCLKD);
        vvt2PID.SetSampleTime(33); //30Hz is 33,33ms
        vvt2PID.SetMode(AUTOMATIC); //Turn PID on
      }
    }

    vvt1_pwm_value = 0;
    vvt2_pwm_value = 0;
    ENABLE_VVT_TIMER(); //Turn on the B compare unit (ie turn on the interrupt)
    BIT_CLEAR(currentStatus.status4, BIT_STATUS4_VVT1_ERROR);
    BIT_CLEAR(currentStatus.status4, BIT_STATUS4_VVT2_ERROR);
    vvtTimeHold = false;
    if (currentStatus.coolant >= (int)(configPage4.vvtMinClt - CALIBRATION_TEMPERATURE_OFFSET)) { vvtIsHot = true; } //Checks to see if coolant's already at operating temperature
  }
  
  if( (configPage6.vvtEnabled == 0) && (configPage10.wmiEnabled >= 1) )
  {
    // config wmi pwm output to use vvt output
    #if defined(CORE_AVR)
      vvt_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (16U * configPage6.vvtFreq * 2U)); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
    #elif defined(CORE_TEENSY35)
      vvt_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (32U * configPage6.vvtFreq * 2U)); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
    #elif defined(CORE_TEENSY41)
      vvt_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (2U * configPage6.vvtFreq * 2U)); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
    #endif
    BIT_CLEAR(currentStatus.status4, BIT_STATUS4_WMI_EMPTY);
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

void boostByGear(void)
{
  if(configPage4.boostType == OPEN_LOOP_BOOST)
  {
    if( configPage9.boostByGearEnabled == 1 )
    {
      uint16_t combinedBoost = 0;
      switch (currentStatus.gear)
      {
        case 1:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear1 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2), currentStatus.RPM))  ) << 2;
          if( combinedBoost <= 10000 ){ currentStatus.boostDuty = combinedBoost; }
          else{ currentStatus.boostDuty = 10000; }
          break;
        case 2:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear2 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2), currentStatus.RPM))  ) << 2;
          if( combinedBoost <= 10000 ){ currentStatus.boostDuty = combinedBoost; }
          else{ currentStatus.boostDuty = 10000; }
          break;
        case 3:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear3 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2), currentStatus.RPM))  ) << 2;
          if( combinedBoost <= 10000 ){ currentStatus.boostDuty = combinedBoost; }
          else{ currentStatus.boostDuty = 10000; }
          break;
        case 4:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear4 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2), currentStatus.RPM))  ) << 2;
          if( combinedBoost <= 10000 ){ currentStatus.boostDuty = combinedBoost; }
          else{ currentStatus.boostDuty = 10000; }
          break;
        case 5:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear5 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2), currentStatus.RPM))  ) << 2;
          if( combinedBoost <= 10000 ){ currentStatus.boostDuty = combinedBoost; }
          else{ currentStatus.boostDuty = 10000; }
          break;
        case 6:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear6 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2), currentStatus.RPM))  ) << 2;
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
          combinedBoost = ( ((uint16_t)configPage9.boostByGear1 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2), currentStatus.RPM)) / 100 ) << 2;
          if( combinedBoost <= 511 ){ currentStatus.boostTarget = combinedBoost; }
          else{ currentStatus.boostTarget = 511; }
          break;
        case 2:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear2 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2), currentStatus.RPM)) / 100 ) << 2;
          if( combinedBoost <= 511 ){ currentStatus.boostTarget = combinedBoost; }
          else{ currentStatus.boostTarget = 511; }
          break;
        case 3:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear3 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2), currentStatus.RPM)) / 100 ) << 2;
          if( combinedBoost <= 511 ){ currentStatus.boostTarget = combinedBoost; }
          else{ currentStatus.boostTarget = 511; }
          break;
        case 4:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear4 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2), currentStatus.RPM)) / 100 ) << 2;
          if( combinedBoost <= 511 ){ currentStatus.boostTarget = combinedBoost; }
          else{ currentStatus.boostTarget = 511; }
          break;
        case 5:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear5 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2), currentStatus.RPM)) / 100 ) << 2;
          if( combinedBoost <= 511 ){ currentStatus.boostTarget = combinedBoost; }
          else{ currentStatus.boostTarget = 511; }
          break;
        case 6:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear6 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2), currentStatus.RPM)) / 100 ) << 2;
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
      if ( (configPage9.boostByGearEnabled > 0) && (configPage2.vssMode > 1) ){ boostByGear(); }
      else{ currentStatus.boostDuty = get3DTableValue(&boostTable, (currentStatus.TPS * 2), currentStatus.RPM) * 2 * 100; }

      if(currentStatus.boostDuty > 10000) { currentStatus.boostDuty = 10000; } //Safety check
      if(currentStatus.boostDuty == 0) { DISABLE_BOOST_TIMER(); BOOST_PIN_LOW(); } //If boost duty is 0, shut everything down
      else
      {
        boost_pwm_target_value = ((unsigned long)(currentStatus.boostDuty) * boost_pwm_max_count) / 10000; //Convert boost duty (Which is a % multiplied by 100) to a pwm count
      }
    }
    else if (configPage4.boostType == CLOSED_LOOP_BOOST)
    {
      if( (boostCounter & 7) == 1) 
      { 
        if ( (configPage9.boostByGearEnabled > 0) && (configPage2.vssMode > 1) ){ boostByGear(); }
        else{ currentStatus.boostTarget = get3DTableValue(&boostTable, (currentStatus.TPS * 2), currentStatus.RPM) << 1; } //Boost target table is in kpa and divided by 2

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
            boostPID.SetOutputLimits(configPage2.boostMinDuty, configPage2.boostMaxDuty);

            if(configPage6.boostMode == BOOST_MODE_SIMPLE) { boostPID.SetTunings(SIMPLE_BOOST_P, SIMPLE_BOOST_I, SIMPLE_BOOST_D); }
            else { boostPID.SetTunings(configPage6.boostKP, configPage6.boostKI, configPage6.boostKD); }
          }

          bool PIDcomputed = boostPID.Compute(get3DTableValue(&boostTableLookupDuty, currentStatus.boostTarget, currentStatus.RPM) * 100/2); //Compute() returns false if the required interval has not yet passed.
          if(currentStatus.boostDuty == 0) { DISABLE_BOOST_TIMER(); BOOST_PIN_LOW(); } //If boost duty is 0, shut everything down
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
        boostPID.Initialize(); //This resets the ITerm value to prevent rubber banding
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
      BOOST_PIN_HIGH(); //Turn on boost pin if duty is 100%
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

void vvtControl(void)
{
  if( (configPage6.vvtEnabled == 1) && (currentStatus.coolant >= (int)(configPage4.vvtMinClt - CALIBRATION_TEMPERATURE_OFFSET)) && (BIT_CHECK(currentStatus.engine, BIT_ENGINE_RUN)))
  {
    if(vvtTimeHold == false) 
    {
      vvtWarmTime = runSecsX10;
      vvtTimeHold = true;
    }

    //Calculate the current cam angle for miata trigger
    if( configPage4.TrigPattern == 9 ) { currentStatus.vvt1Angle = getCamAngle_Miata9905(); }

    if( (vvtIsHot == true) || ((runSecsX10 - vvtWarmTime) >= (configPage4.vvtDelay * VVT_TIME_DELAY_MULTIPLIER)) ) 
    {
      vvtIsHot = true;

      if( (configPage6.vvtMode == VVT_MODE_OPEN_LOOP) || (configPage6.vvtMode == VVT_MODE_ONOFF) )
      {
        //Lookup VVT duty based on either MAP or TPS
        if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentStatus.vvt1Duty = get3DTableValue(&vvtTable, (currentStatus.TPS * 2), currentStatus.RPM); }
        else { currentStatus.vvt1Duty = get3DTableValue(&vvtTable, (currentStatus.MAP), currentStatus.RPM); }

        //VVT table can be used for controlling on/off switching. If this is turned on, then disregard any interpolation or non-binary values
        if( (configPage6.vvtMode == VVT_MODE_ONOFF) && (currentStatus.vvt1Duty < 200) ) { currentStatus.vvt1Duty = 0; }

        vvt1_pwm_value = halfPercentage(currentStatus.vvt1Duty, vvt_pwm_max_count);

        if (configPage10.vvt2Enabled == 1) // same for VVT2 if it's enabled
        {
          //Lookup VVT duty based on either MAP or TPS
          if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentStatus.vvt2Duty = get3DTableValue(&vvt2Table, (currentStatus.TPS * 2), currentStatus.RPM); }
          else { currentStatus.vvt2Duty = get3DTableValue(&vvt2Table, (currentStatus.MAP), currentStatus.RPM); }

          //VVT table can be used for controlling on/off switching. If this is turned on, then disregard any interpolation or non-binary values
          if( (configPage6.vvtMode == VVT_MODE_ONOFF) && (currentStatus.vvt2Duty < 200) ) { currentStatus.vvt2Duty = 0; }

          vvt2_pwm_value = halfPercentage(currentStatus.vvt2Duty, vvt_pwm_max_count);
        }

      } //Open loop
      else if( (configPage6.vvtMode == VVT_MODE_CLOSED_LOOP) )
      {
        //Lookup VVT duty based on either MAP or TPS
        if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentStatus.vvt1TargetAngle = get3DTableValue(&vvtTable, (currentStatus.TPS * 2), currentStatus.RPM); }
        else { currentStatus.vvt1TargetAngle = get3DTableValue(&vvtTable, currentStatus.MAP, currentStatus.RPM); }

        if( (vvtCounter & 31) == 1) { vvtPID.SetTunings(configPage10.vvtCLKP, configPage10.vvtCLKI, configPage10.vvtCLKD);  //This only needs to be run very infrequently, once every 32 calls to vvtControl(). This is approx. once per second
        vvtPID.SetControllerDirection(configPage6.vvtPWMdir); }

        // safety check that the cam angles are ok. The engine will be totally undriveable if the cam sensor is faulty and giving wrong cam angles, so if that happens, default to 0 duty.
        // This also prevents using zero or negative current angle values for PID adjustment, because those don't work in integer PID.
        if ( currentStatus.vvt1Angle <=  configPage10.vvtCLMinAng || currentStatus.vvt1Angle > configPage10.vvtCLMaxAng )
        {
          currentStatus.vvt1Duty = 0;
          vvt1_pwm_value = halfPercentage(currentStatus.vvt1Duty, vvt_pwm_max_count);
          BIT_SET(currentStatus.status4, BIT_STATUS4_VVT1_ERROR);
        }
        //Check that we're not already at the angle we want to be
        else if((configPage6.vvtCLUseHold > 0) && (currentStatus.vvt1TargetAngle == currentStatus.vvt1Angle) )
        {
          currentStatus.vvt1Duty = configPage10.vvtCLholdDuty;
          vvt1_pwm_value = halfPercentage(currentStatus.vvt1Duty, vvt_pwm_max_count);
          vvtPID.Initialize();
          BIT_CLEAR(currentStatus.status4, BIT_STATUS4_VVT1_ERROR);
        }
        else
        {
          //This is dumb, but need to convert the current angle into a long pointer.
          vvt_pid_target_angle = (unsigned long)currentStatus.vvt1TargetAngle;
          vvt_pid_current_angle = (long)currentStatus.vvt1Angle;

          //If not already at target angle, calculate new value from PID
          bool PID_compute = vvtPID.Compute(true);
          //vvtPID.Compute2(currentStatus.vvt1TargetAngle, currentStatus.vvt1Angle, false);
          //vvt_pwm_target_value = percentage(40, vvt_pwm_max_count);
          //if (currentStatus.vvt1Angle > currentStatus.vvt1TargetAngle) { vvt_pwm_target_value = 0; }
          if(PID_compute == true) { vvt1_pwm_value = halfPercentage(currentStatus.vvt1Duty, vvt_pwm_max_count); }
          BIT_CLEAR(currentStatus.status4, BIT_STATUS4_VVT1_ERROR);
        }

        if (configPage10.vvt2Enabled == 1) // same for VVT2 if it's enabled
        {
          if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentStatus.vvt2TargetAngle = get3DTableValue(&vvt2Table, (currentStatus.TPS * 2), currentStatus.RPM); }
          else { currentStatus.vvt2TargetAngle = get3DTableValue(&vvt2Table, currentStatus.MAP, currentStatus.RPM); }

          if( (vvtCounter & 31) == 1) { vvt2PID.SetTunings(configPage10.vvtCLKP, configPage10.vvtCLKI, configPage10.vvtCLKD);  //This only needs to be run very infrequently, once every 32 calls to vvtControl(). This is approx. once per second
          vvt2PID.SetControllerDirection(configPage4.vvt2PWMdir); }

          // safety check that the cam angles are ok. The engine will be totally undriveable if the cam sensor is faulty and giving wrong cam angles, so if that happens, default to 0 duty.
          // This also prevents using zero or negative current angle values for PID adjustment, because those don't work in integer PID.
          if ( currentStatus.vvt2Angle <= configPage10.vvtCLMinAng || currentStatus.vvt2Angle > configPage10.vvtCLMaxAng )
          {
            currentStatus.vvt2Duty = 0;
            vvt2_pwm_value = halfPercentage(currentStatus.vvt2Duty, vvt_pwm_max_count);
            BIT_SET(currentStatus.status4, BIT_STATUS4_VVT2_ERROR);
          }
          //Check that we're not already at the angle we want to be
          else if((configPage6.vvtCLUseHold > 0) && (currentStatus.vvt2TargetAngle == currentStatus.vvt2Angle) )
          {
            currentStatus.vvt2Duty = configPage10.vvtCLholdDuty;
            vvt2_pwm_value = halfPercentage(currentStatus.vvt2Duty, vvt_pwm_max_count);
            vvt2PID.Initialize();
            BIT_CLEAR(currentStatus.status4, BIT_STATUS4_VVT2_ERROR);
          }
          else
          {
            //This is dumb, but need to convert the current angle into a long pointer.
            vvt2_pid_target_angle = (unsigned long)currentStatus.vvt2TargetAngle;
            vvt2_pid_current_angle = (long)currentStatus.vvt2Angle;
            //If not already at target angle, calculate new value from PID
            bool PID_compute = vvt2PID.Compute(true);
            if(PID_compute == true) { vvt2_pwm_value = halfPercentage(currentStatus.vvt2Duty, vvt_pwm_max_count); }
            BIT_CLEAR(currentStatus.status4, BIT_STATUS4_VVT2_ERROR);
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
          VVT1_PIN_OFF();
          VVT2_PIN_OFF();
          vvt1_pwm_state = false;
          vvt1_max_pwm = false;
          vvt2_pwm_state = false;
          vvt2_max_pwm = false;
          DISABLE_VVT_TIMER();
        }
        else if( (currentStatus.vvt1Duty >= 200) && (currentStatus.vvt2Duty >= 200) )
        {
          //Make sure solenoid is on (100% duty)
          VVT1_PIN_ON();
          VVT2_PIN_ON();
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
          VVT1_PIN_OFF();
          vvt1_pwm_state = false;
          vvt1_max_pwm = false;
        }
        else if( currentStatus.vvt1Duty >= 200 )
        {
          //Make sure solenoid is on (100% duty)
          VVT1_PIN_ON();
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
  bool nitrousOn = false; //This tracks whether the control gets turned on at any point. 
  if(configPage10.n2o_enable > 0)
  {
    bool isArmed = READ_N2O_ARM_PIN();
    if (configPage10.n2o_pin_polarity == 1) { isArmed = !isArmed; } //If nitrous is active when pin is low, flip the reading (n2o_pin_polarity = 0 = active when High)

    //Perform the main checks to see if nitrous is ready
    if( (isArmed == true) && (currentStatus.coolant > (configPage10.n2o_minCLT - CALIBRATION_TEMPERATURE_OFFSET)) && (currentStatus.TPS > configPage10.n2o_minTPS) && (currentStatus.O2 < configPage10.n2o_maxAFR) && (currentStatus.MAP < (uint16_t)(configPage10.n2o_maxMAP * 2)) )
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
      currentStatus.nitrous_status = NITROUS_OFF; //Reset the current state
      if( (currentStatus.RPM > realStage1MinRPM) && (currentStatus.RPM < realStage1MaxRPM) )
      {
        currentStatus.nitrous_status += NITROUS_STAGE1;
        BIT_SET(currentStatus.status3, BIT_STATUS3_NITROUS);
        N2O_STAGE1_PIN_HIGH();
        nitrousOn = true;
      }
      if(configPage10.n2o_enable == NITROUS_STAGE2) //This is really just a sanity check
      {
        if( (currentStatus.RPM > realStage2MinRPM) && (currentStatus.RPM < realStage2MaxRPM) )
        {
          currentStatus.nitrous_status += NITROUS_STAGE2;
          BIT_SET(currentStatus.status3, BIT_STATUS3_NITROUS);
          N2O_STAGE2_PIN_HIGH();
          nitrousOn = true;
        }
      }
    }
  }

  if (nitrousOn == false)
  {
    currentStatus.nitrous_status = NITROUS_OFF;
    BIT_CLEAR(currentStatus.status3, BIT_STATUS3_NITROUS);

    if(configPage10.n2o_enable > 0)
    {
      N2O_STAGE1_PIN_LOW();
      N2O_STAGE2_PIN_LOW();
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
    if( WMI_TANK_IS_EMPTY() )
    {
      BIT_CLEAR(currentStatus.status4, BIT_STATUS4_WMI_EMPTY);
      if( (currentStatus.TPS >= configPage10.wmiTPS) && (currentStatus.RPMdiv100 >= configPage10.wmiRPM) && ( (currentStatus.MAP / 2) >= configPage10.wmiMAP) && ( (currentStatus.IAT + CALIBRATION_TEMPERATURE_OFFSET) >= configPage10.wmiIAT) )
      {
        switch(configPage10.wmiMode)
        {
        case WMI_MODE_SIMPLE:
          // Simple mode - Output is turned on when preset boost level is reached
          wmiPW = 200;
          break;
        case WMI_MODE_PROPORTIONAL:
          // Proportional Mode - Output PWM is proportionally controlled between two MAP values - MAP Value 1 = PWM:0% / MAP Value 2 = PWM:100%
          wmiPW = map(currentStatus.MAP/2, configPage10.wmiMAP, configPage10.wmiMAP2, 0, 200);
          break;
        case WMI_MODE_OPENLOOP:
          //  Mapped open loop - Output PWM follows 2D map value (RPM vs MAP) Cell value contains desired PWM% [range 0-100%]
          wmiPW = get3DTableValue(&wmiTable, currentStatus.MAP, currentStatus.RPM);
          break;
        case WMI_MODE_CLOSEDLOOP:
          // Mapped closed loop - Output PWM follows injector duty cycle with 2D correction map applied (RPM vs MAP). Cell value contains correction value% [nom 100%] 
          wmiPW = max(0, ((int)currentStatus.PW1 + configPage10.wmiOffset)) * get3DTableValue(&wmiTable, currentStatus.MAP, currentStatus.RPM) / 200;
          break;
        default:
          // Wrong mode
          wmiPW = 0;
          break;
        }
        if (wmiPW > 200) { wmiPW = 200; } //without this the duty can get beyond 100%
      }
    }
    else { BIT_SET(currentStatus.status4, BIT_STATUS4_WMI_EMPTY); }

    currentStatus.wmiPW = wmiPW;
    vvt2_pwm_value = halfPercentage(currentStatus.wmiPW, vvt_pwm_max_count);

    if(wmiPW == 0)
    {
      // Make sure water pump is off
      VVT2_PIN_LOW();
      vvt2_pwm_state = false;
      vvt2_max_pwm = false;
      if( configPage6.vvtEnabled == 0 ) { DISABLE_VVT_TIMER(); }
      digitalWrite(pinWMIEnabled, LOW);
    }
    else
    {
      digitalWrite(pinWMIEnabled, HIGH);
      if (wmiPW >= 200)
      {
        // Make sure water pump is on (100% duty)
        VVT2_PIN_HIGH();
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
  boostPID.Initialize(); //This resets the ITerm value to prevent rubber banding
  currentStatus.boostDuty = 0;
  DISABLE_BOOST_TIMER(); //Turn off timer
  BOOST_PIN_LOW(); //Make sure solenoid is off (0% duty)
}

//The interrupt to control the Boost PWM
#if defined(CORE_AVR)
  ISR(TIMER1_COMPA_vect) //cppcheck-suppress misra-c2012-8.2
#else
  void boostInterrupt(void) //Most ARM chips can simply call a function
#endif
{
  if (boost_pwm_state == true)
  {
    #if defined(CORE_TEENSY41) //PIT TIMERS count down and have opposite effect on PWM
    BOOST_PIN_HIGH();
    #else
    BOOST_PIN_LOW();  // Switch pin to low
    #endif
    SET_COMPARE(BOOST_TIMER_COMPARE, BOOST_TIMER_COUNTER + (boost_pwm_max_count - boost_pwm_cur_value) );
    boost_pwm_state = false;
  }
  else
  {
    #if defined(CORE_TEENSY41) //PIT TIMERS count down and have opposite effect on PWM
    BOOST_PIN_LOW();
    #else
    BOOST_PIN_HIGH();  // Switch pin high
    #endif
    SET_COMPARE(BOOST_TIMER_COMPARE, BOOST_TIMER_COUNTER + boost_pwm_target_value);
    boost_pwm_cur_value = boost_pwm_target_value;
    boost_pwm_state = true;
  }
}

//The interrupt to control the VVT PWM
#if defined(CORE_AVR)
  ISR(TIMER1_COMPB_vect) //cppcheck-suppress misra-c2012-8.2
#else
  void vvtInterrupt(void) //Most ARM chips can simply call a function
#endif
{
  if ( ((vvt1_pwm_state == false) || (vvt1_max_pwm == true)) && ((vvt2_pwm_state == false) || (vvt2_max_pwm == true)) )
  {
    if( (vvt1_pwm_value > 0) && (vvt1_max_pwm == false) ) //Don't toggle if at 0%
    {
      #if defined(CORE_TEENSY41)
      VVT1_PIN_OFF();
      #else
      VVT1_PIN_ON();
      #endif
      vvt1_pwm_state = true;
    }
    if( (vvt2_pwm_value > 0) && (vvt2_max_pwm == false) ) //Don't toggle if at 0%
    {
      #if defined(CORE_TEENSY41)
      VVT2_PIN_OFF();
      #else
      VVT2_PIN_ON();
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
        VVT1_PIN_ON();
        #else
        VVT1_PIN_OFF();
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
        VVT2_PIN_ON();
        #else
        VVT2_PIN_OFF();
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
        VVT1_PIN_ON();
        #else
        VVT1_PIN_OFF();
        #endif
        vvt1_pwm_state = false;
        vvt1_max_pwm = false;
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvt1_pwm_cur_value) );
      }
      else { vvt1_max_pwm = true; }
      if(vvt2_pwm_value < (long)vvt_pwm_max_count) //Don't toggle if at 100%
      {
        #if defined(CORE_TEENSY41)
        VVT2_PIN_ON();
        #else
        VVT2_PIN_OFF();
        #endif
        vvt2_pwm_state = false;
        vvt2_max_pwm = false;
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvt2_pwm_cur_value) );
      }
      else { vvt2_max_pwm = true; }
    }
  }
}

#if defined(PWM_FAN_AVAILABLE)
//The interrupt to control the FAN PWM. Mega2560 doesn't have enough timers, so this is only for the ARM chip ones
  void fanInterrupt(void)
{
  if (fan_pwm_state == true)
  {
    FAN_OFF();
    FAN_TIMER_COMPARE = FAN_TIMER_COUNTER + (fan_pwm_max_count - fan_pwm_cur_value);
    fan_pwm_state = false;
  }
  else
  {
    FAN_ON();
    FAN_TIMER_COMPARE = FAN_TIMER_COUNTER + fan_pwm_value;
    fan_pwm_cur_value = fan_pwm_value;
    fan_pwm_state = true;
  }
}
#endif
