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

TESTABLE_STATIC void fanOn(void) 
{
  ATOMIC() { 
    ((configPage6.fanInv) ? fan_pin.setPinLow() : fan_pin.setPinHigh()); 
  }
}
TESTABLE_STATIC void fanOff(void)
{
  ATOMIC() { 
    ((configPage6.fanInv) ? fan_pin.setPinHigh() : fan_pin.setPinLow()); 
  }
}

void __attribute__((optimize("Os"))) initialiseFan(uint8_t fanPin)
{
  fan_pin.setPin(fanPin, OUTPUT);
  fanOff();  //Initialise program with the fan in the off state
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

static void matchFanStateToDuty(const statuses &current)
{
  if (current.fanDuty==0)
  {
    fanOff();
  }
  else
  {
    fanOn();
  }
}

static bool airConTurnsFanOn(const statuses &current, const config15 &page15)
{
  return page15.airConTurnsFanOn && current.acStatus.turningOn;
}

static uint8_t getDutyOnOffMode(const statuses &current, const config2 &page2, const config6 &page6, const config15 &page15)
{
  int16_t onTemp = temperatureRemoveOffset(page6.fanSP);
  int16_t offTemp = onTemp - page6.fanHyster;
  // Cranking inhibition must also override a held-on state in the hysteresis band.
  const bool fanPermit = (page2.fanWhenOff || current.rotationStatus == EngineRotationStatus::Running)
                      && (current.rotationStatus != EngineRotationStatus::Cranking || page2.fanWhenCranking);

  uint8_t duty = current.fanDuty;
  if ( fanPermit &&
       ((current.coolant >= onTemp) || airConTurnsFanOn(current, page15)) )
  {
    //Fan needs to be turned on - either by high coolant temp, or from an A/C request (to ensure there is airflow over the A/C radiator).
    duty = 200;
  }
  else if ( (current.coolant <= offTemp) || (!fanPermit) )
  {
    //Fan needs to be turned off. 
    duty = 0;
  }
  else
  {
    // No change - send back current duty
  }

  return duty;
}

static void fanControlOnOffMode(statuses &current, const config2 &page2, const config6 &page6, const config15 &page15)
{
  current.fanDuty = getDutyOnOffMode(current, page2, page6, page15);
  matchFanStateToDuty(current);
}

void fanControl(void)
{
  if( configPage2.fanEnable == 1 ) // regular on/off fan control
  {
    fanControlOnOffMode(currentStatus, configPage2, configPage6, configPage15);
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
        #if defined(PWM_FAN_AVAILABLE)//PWM fan not available on Arduino MEGA
          DISABLE_FAN_TIMER();
        #endif
      }
      else
      {
        byte tempFanDuty = table2D_getValue(&fanPWMTable, temperatureAddOffset(currentStatus.coolant)); //In normal situation read PWM duty from the table
        if((configPage15.airConTurnsFanOn) == 1 &&
           currentStatus.acStatus.turningOn == true)
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
          }
        #endif
      }
    }
    else if (!fanPermit)
    {
      currentStatus.fanDuty = 0; ////If the user has elected to disable the fan when engine is not running, make sure it's off 
    }

    #if defined(PWM_FAN_AVAILABLE)
      if(currentStatus.fanDuty == 0)
      {
        //Make sure fan has 0% duty)
        fanOff();
        DISABLE_FAN_TIMER();
      }
      else if (currentStatus.fanDuty == 200)
      {
        //Make sure fan has 100% duty
        fanOn();
        DISABLE_FAN_TIMER();
      }
    #else //Just in case if user still has selected PWM fan in TS, even though it warns that it doesn't work on mega.
      if(currentStatus.fanDuty == 0)
      {
        //Make sure fan has 0% duty)
        fanOff();
      }
      else if (currentStatus.fanDuty > 0)
      {
        //Make sure fan has 100% duty
        fanOn();
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
