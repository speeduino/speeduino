#include "../../pins/boardOutputPin.h"
#include "../../../units.h"
#include "../../../unit_testing.h"
#include "../../../globals.h"
#include "src/pins/invertableOutputPin.h"
#include "src/pins/outputPin.h"
#include "src/pwm/PwmOutputChannel.h"
#include "src/pwm/interruptHandlers.h"

TESTABLE_CONSTEXPR table2D_u8_u8_4 fanPWMTable(&configPage6.fanPWMBins, &configPage9.PWMFanDuty);
using fanPwmChannel_t = PwmOutputChannel<invertableOutputPinAdaper_t<outputPin_t>>;
TESTABLE_STATIC fanPwmChannel_t _fanPwm;

static void applyDutyToPwm(const statuses &current)
{
  _fanPwm.setTargetDuty(current.fanDuty);

#if defined(PWM_FAN_AVAILABLE)
  if (_fanPwm.isPartialDuty())
  { 
    ENABLE_FAN_TIMER(); //Turn on the compare unit (ie turn on the interrupt) if boost duty >0
  }
  else
  {
    DISABLE_FAN_TIMER(); 
  }
#endif
}

void __attribute__((optimize("Os"))) initialiseFan(uint8_t fanPin)
{
#if !defined(PWM_FAN_AVAILABLE)
  // PWM is unavailable, but the user selected it anyway...
  if ( configPage2.fanEnable == FANMODE_PWM )
  {
    // ...force on/off mode
    configPage2.fanEnable = FANMODE_ONOFF;
  }  
#endif

  _fanPwm = fanPwmChannel_t(fanPin, FREQUENCY.toUser(configPage6.fanFreq));
  _fanPwm.pin.setInverted(configPage6.fanInv);
  currentStatus.fanDuty = 0;
  applyDutyToPwm(currentStatus);
}

static bool airConTurnsFanOn(const statuses &current, const config15 &page15)
{
  return page15.airConTurnsFanOn && current.acStatus.turningOn;
}

static bool isFanPermitted(const statuses &current, const config2 &page2)
{
  return (page2.fanWhenOff || current.rotationStatus == EngineRotationStatus::Running)
      && (current.rotationStatus != EngineRotationStatus::Cranking || page2.fanWhenCranking);
}

static uint8_t calculateDutyOnOffMode(const statuses &current, const config2 &page2, const config6 &page6, const config15 &page15)
{
  int16_t onTemp = temperatureRemoveOffset(page6.fanSP);
  int16_t offTemp = onTemp - page6.fanHyster;
  // Cranking inhibition must also override a held-on state in the hysteresis band.

  uint8_t duty = current.fanDuty;
  const bool fanPermit = isFanPermitted(current, page2);
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

#if defined(PWM_FAN_AVAILABLE)
static uint8_t calculateDutyPwmMode(const statuses &current, const config2 &page2, const config15 &page15)
{
  uint8_t duty = 0;

  if(isFanPermitted(current, page2))
  {
    duty = table2D_getValue(&fanPWMTable, temperatureAddOffset(current.coolant)); //In normal situation read PWM duty from the table
    if(airConTurnsFanOn(current, page15))
    {
      // Clamp the fan duty to airConPwmFanMinDuty or above, to ensure there is airflow over the A/C radiator
      duty = (std::max)(duty, page15.airConPwmFanMinDuty);
    }
  }

  return duty;
}
#endif

static uint8_t calculateDuty(const statuses &current, const config2 &page2, const config6 &page6, const config15 &page15)
{
  uint8_t duty = 0;
  if( page2.fanEnable == FANMODE_ONOFF)
  {
    duty = calculateDutyOnOffMode(current, page2, page6, page15);
  }
#if defined(PWM_FAN_AVAILABLE)
  else if( page2.fanEnable == FANMODE_PWM )
  {
    duty = calculateDutyPwmMode(current, page2, page15);
  }
  else
  {
    // Unknown mode
  }
#endif

  return duty;
}

void fanControl(void)
{
  currentStatus.fanDuty = calculateDuty(currentStatus, configPage2, configPage6, configPage15);
  applyDutyToPwm(currentStatus);
}

//The interrupt to control the FAN PWM. Mega2560 doesn't have enough timers, so this is only for the ARM chip ones
void fanInterrupt(void)
{
#if defined(PWM_FAN_AVAILABLE)
  auto setTimerCallback =[](uint16_t tickDelta) {
    SET_COMPARE(FAN_TIMER_COMPARE, FAN_TIMER_COUNTER + tickDelta );
  };
  pwmISR(_fanPwm, setTimerCallback);
#endif
}
