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
#include "units.h"


volatile PORT_TYPE *n2o_stage1_pin_port;
volatile PINMASK_TYPE n2o_stage1_pin_mask;
volatile PORT_TYPE *n2o_stage2_pin_port;
volatile PINMASK_TYPE n2o_stage2_pin_mask;
volatile PORT_TYPE *n2o_arming_pin_port;
volatile PINMASK_TYPE n2o_arming_pin_mask;


volatile PORT_TYPE *fan_pin_port;
volatile PINMASK_TYPE fan_pin_mask;

#if defined(PWM_FAN_AVAILABLE)//PWM fan not available on Arduino MEGA
volatile bool fan_pwm_state;
uint16_t fan_pwm_max_count; //Used for variable PWM frequency
volatile unsigned int fan_pwm_cur_value;
long fan_pwm_value;
#endif
static table2D_u8_u8_4 fanPWMTable(&configPage6.fanPWMBins, &configPage9.PWMFanDuty);


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
    int onTemp = temperatureRemoveOffset(configPage6.fanSP);
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
        byte tempFanDuty = table2D_getValue(&fanPWMTable, temperatureAddOffset(currentStatus.coolant)); //In normal situation read PWM duty from the table
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


  currentStatus.nitrous_status = NITROUS_OFF;

}





void nitrousControl(void)
{
  bool nitrousOn = false; //This tracks whether the control gets turned on at any point. 
  if(configPage10.n2o_enable > 0)
  {
    bool isArmed = READ_N2O_ARM_PIN();
    if (configPage10.n2o_pin_polarity == 1) { isArmed = !isArmed; } //If nitrous is active when pin is low, flip the reading (n2o_pin_polarity = 0 = active when High)

    //Perform the main checks to see if nitrous is ready
    if( (isArmed == true) && (currentStatus.coolant > temperatureRemoveOffset(configPage10.n2o_minCLT)) && (currentStatus.TPS > configPage10.n2o_minTPS) && (currentStatus.O2 < configPage10.n2o_maxAFR) && (currentStatus.MAP < (uint16_t)(configPage10.n2o_maxMAP * 2)) )
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
