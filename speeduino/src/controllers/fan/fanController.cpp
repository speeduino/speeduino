#include "../../pins/boardOutputPin.h"
#include "../../../units.h"
#include "../../../unit_testing.h"
#include "../../../globals.h"

#if defined(PWM_FAN_AVAILABLE)//PWM fan not available on Arduino MEGA
TESTABLE_STATIC volatile bool fan_pwm_state;
static uint16_t fan_pwm_max_count; //Used for variable PWM frequency
static volatile unsigned int fan_pwm_cur_value;
TESTABLE_STATIC long fan_pwm_value;
#endif
TESTABLE_CONSTEXPR table2D_u8_u8_4 fanPWMTable(&configPage6.fanPWMBins, &configPage9.PWMFanDuty);

TESTABLE_STATIC boardOutputPin_t fan_pin;

void fanOn(void) 
{
  ATOMIC() { 
    ((configPage6.fanInv) ? fan_pin.setPinLow() : fan_pin.setPinHigh()); 
  }
}
void fanOff(void)
{
  ATOMIC() { 
    ((configPage6.fanInv) ? fan_pin.setPinHigh() : fan_pin.setPinLow()); 
  }
}

void __attribute__((optimize("Os"))) initialiseFan(uint8_t fanPin)
{
  fan_pin.setPin(fanPin, OUTPUT);
  fanOff();  //Initialise program with the fan in the off state
  currentStatus.fanOn = false;
  currentStatus.fanDuty = 0;

#if defined(PWM_FAN_AVAILABLE)
  DISABLE_FAN_TIMER(); //disable FAN timer if available
  if ( configPage2.fanEnable == 2 ) // PWM Fan control
  {
    fan_pwm_max_count = pwmFreqToTicks(FREQUENCY.toUser(configPage6.fanFreq));
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
    else { fanPermit = currentStatus.rotationStatus==EngineRotationStatus::Running; }

    if ( (fanPermit == true) &&
         ((currentStatus.coolant >= onTemp) || 
           ((configPage15.airConTurnsFanOn) == 1 &&
           currentStatus.airconTurningOn == true)) )
    {
      //Fan needs to be turned on - either by high coolant temp, or from an A/C request (to ensure there is airflow over the A/C radiator).
      if((currentStatus.rotationStatus==EngineRotationStatus::Cranking) && (configPage2.fanWhenCranking == 0))
      {
        //If the user has elected to disable the fan during cranking, make sure it's off 
        fanOff();
        currentStatus.fanOn = false;
      }
      else 
      {
        fanOn();
        currentStatus.fanOn = true;
      }
    }
    else if ( (currentStatus.coolant <= offTemp) || (!fanPermit) )
    {
      //Fan needs to be turned off. 
      fanOff();
      currentStatus.fanOn = false;
    }
  }
  else if( configPage2.fanEnable == 2 )// PWM Fan control
  {
    bool fanPermit = false;
    if ( configPage2.fanWhenOff == true) { fanPermit = true; }
    else { fanPermit = currentStatus.rotationStatus==EngineRotationStatus::Running; }
    if (fanPermit == true)
      {
      if((currentStatus.rotationStatus==EngineRotationStatus::Cranking) && (configPage2.fanWhenCranking == 0))
      {
        currentStatus.fanDuty = 0; //If the user has elected to disable the fan during cranking, make sure it's off 
        currentStatus.fanOn = false;
        #if defined(PWM_FAN_AVAILABLE)//PWM fan not available on Arduino MEGA
          DISABLE_FAN_TIMER();
        #endif
      }
      else
      {
        byte tempFanDuty = table2D_getValue(&fanPWMTable, temperatureAddOffset(currentStatus.coolant)); //In normal situation read PWM duty from the table
        if((configPage15.airConTurnsFanOn) == 1 &&
           currentStatus.airconTurningOn == true)
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
            currentStatus.fanOn = true;
          }
        #endif
      }
    }
    else if (!fanPermit)
    {
      currentStatus.fanDuty = 0; ////If the user has elected to disable the fan when engine is not running, make sure it's off 
      currentStatus.fanOn = false;
    }

    #if defined(PWM_FAN_AVAILABLE)
      if(currentStatus.fanDuty == 0)
      {
        //Make sure fan has 0% duty)
        fanOff();
        currentStatus.fanOn = false;
        DISABLE_FAN_TIMER();
      }
      else if (currentStatus.fanDuty == 200)
      {
        //Make sure fan has 100% duty
        fanOn();
        currentStatus.fanOn = true;
        DISABLE_FAN_TIMER();
      }
    #else //Just in case if user still has selected PWM fan in TS, even though it warns that it doesn't work on mega.
      if(currentStatus.fanDuty == 0)
      {
        //Make sure fan has 0% duty)
        fanOff();
        currentStatus.fanOn = false;
      }
      else if (currentStatus.fanDuty > 0)
      {
        //Make sure fan has 100% duty
        fanOn();
        currentStatus.fanOn = true;
      }
    #endif
  }
}

//The interrupt to control the FAN PWM. Mega2560 doesn't have enough timers, so this is only for the ARM chip ones
void fanInterrupt(void)
{
#if defined(PWM_FAN_AVAILABLE)
  if (fan_pwm_state == true)
  {
    fanOff();
    FAN_TIMER_COMPARE = FAN_TIMER_COUNTER + (fan_pwm_max_count - fan_pwm_cur_value);
    fan_pwm_state = false;
  }
  else
  {
    fanOn();
    FAN_TIMER_COMPARE = FAN_TIMER_COUNTER + fan_pwm_value;
    fan_pwm_cur_value = fan_pwm_value;
    fan_pwm_state = true;
  }
#endif
}
