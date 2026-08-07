#include "../../pins/boardOutputPin.h"
#include "../../../globals.h"
#include "../../../unit_testing.h"
#include "../../../units.h"
#include "../../PID/integerPID_ideal.h"
#include "../../../timers.h"
#include "../../pwm/PwmOutputChannel.h"

TESTABLE_STATIC PwmOutputChannel boostOutput;
TESTABLE_STATIC integerPID_ideal boostPID; //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call

TESTABLE_CONSTEXPR table2D_u8_s16_6 flexBoostTable(&configPage10.flexBoostBins, &configPage10.flexBoostAdj);

static __attribute__((optimize("Os"))) void setBoostPidTunings(const config2 &page2, const config6 &page6, const config10 &page10)
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

__attribute__((optimize("Os"))) void initialiseBoost(statuses &current, const config2 &page2, const config6 &page6, const config10 &page10, const pinNumbers_t &pins)
{
  boostOutput = PwmOutputChannel(pins.pinBoost, FREQUENCY.toUser(page6.boostFreq));
  setBoostPidTunings(page2, page6, page10);
  current.boostDuty = 0;
}

static uint8_t getBoostByGearFactor(const statuses &current, const config9 &page9)
{
  if ((current.gear-1U)<_countof(page9.boostByGear))
  {
    return page9.boostByGear[current.gear-1U];
  }
  return 0U;
}

static uint16_t getBoostDuty(const statuses &current, const config2 &page2, const config9 &page9)
{
  uint16_t duty = 0;
  bool usingExternalVss = isExternalVssMode(page2);
  if( (page9.boostByGearEnabled == BOOST_BY_GEAR_PERCENT) && usingExternalVss )
  {
    duty = ( ((uint16_t)getBoostByGearFactor(current, page9) * (uint16_t)get3DTableValue(&boostTable, (current.TPS * 2U), current.RPM))  ) << 2;
  }
  else if( (page9.boostByGearEnabled == BOOST_BY_GEAR_CONSTANT) && usingExternalVss) 
  {
    duty = (uint16_t)getBoostByGearFactor(current, page9) * 2U * 100U;
  }
  else
  {
    duty = (uint16_t)get3DTableValue(&boostTable, (current.TPS * 2U), current.RPM) * 2U * 100U;
  }
  return clamp(duty, (uint16_t)0, (uint16_t)10000U);
}

static uint16_t getBoostTarget(const statuses &current, const config2 &page2, const config9 &page9)
{
  uint16_t target = 0;
  bool usingExternalVss = isExternalVssMode(page2);
  if( (page9.boostByGearEnabled == BOOST_BY_GEAR_PERCENT) && usingExternalVss )
  {
    target = ( ((uint16_t)getBoostByGearFactor(current, page9) * (uint16_t)get3DTableValue(&boostTable, (current.TPS * 2U), current.RPM)) / 100 ) << 2;
  }
  else if( (page9.boostByGearEnabled == BOOST_BY_GEAR_CONSTANT) && usingExternalVss) 
  {
    target = (uint16_t)getBoostByGearFactor(current, page9) * 2U;
  }
  else
  {
    //Boost target table is in kpa and divided by 2
    target = get3DTableValue(&boostTable, (current.TPS * 2U), current.RPM) << 1;
  }
  // flexBoostCorrection is int16_t; beware of conversion under-/over-flow
  int16_t correctedTarget = (int16_t)target+current.flexBoostCorrection;
  return clamp(correctedTarget, (int16_t)0, (int16_t)511);
}

static int16_t getFlexCorrection(const statuses &current, const config2 &page2)
{
  //If flex fuel is enabled, there can be an adder to the boost target based on ethanol content
  if( page2.flexEnabled )
  {
    return table2D_getValue(&flexBoostTable, current.ethanolPct);
  }
  return 0U;
}

static uint16_t convertTargetToDuty(const statuses &current, const config2 &page2, const config6 &page6, const config10 &page10)
{
  uint16_t duty = 0;
  if(current.boostTarget > 0)
  {
    if( BIT_CHECK(current.LOOP_TIMER, BIT_TIMER_1HZ) )
    {
      setBoostPidTunings(page2, page6, page10);
    }

    boostPID.setSetPoint(current.boostTarget);
    boostPID.setFeedForwardTerm(get3DTableValue(&boostTableLookupDuty, current.boostTarget, current.RPM) * 100/2);
    (void)boostPID.compute(millis(), current.MAP, &duty);
  }

  return duty;
}

static void applyDutyToPwm(const statuses &current)
{
  // Convert boost duty (Which is a % multiplied by 100) to half percentage. I.e 0-200
  boostOutput.setTargetDuty(fast_div_closest(current.boostDuty, 50U));

  if (boostOutput.isPartialDuty())
  { 
    ENABLE_BOOST_TIMER(); //Turn on the compare unit (ie turn on the interrupt) if boost duty >0
  }
  // Check for 100% duty cycle
  else
  {
    DISABLE_BOOST_TIMER(); 
  }
}

static bool isBoostActive(const statuses &current, const config6 &page6)
{
  return (page6.boostEnabled)
      && (current.rotationStatus==EngineRotationStatus::Running)
  ;
}

void boostControl(statuses &current, const config2 &page2, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10, const config15 &page15)
{
  if(isBoostActive(current, page6) )
  {
    if(page4.boostType == OPEN_LOOP_BOOST)
    {
      current.boostDuty = getBoostDuty(current, page2, page9);
    }
    else if (page4.boostType == CLOSED_LOOP_BOOST)
    {
      if( BIT_CHECK(current.LOOP_TIMER, BIT_TIMER_4HZ) )
      { 
        current.flexBoostCorrection = getFlexCorrection(current, page2);
        current.boostTarget = getBoostTarget(current, page2, page9);
      } 

      if(((page15.boostControlEnable == EN_BOOST_CONTROL_BARO) && (current.MAP >= current.baro)) || ((page15.boostControlEnable == EN_BOOST_CONTROL_FIXED) && (current.MAP >= page15.boostControlEnableThreshold))) //Only enables boost control above baro pressure or above user defined threshold (User defined level is usually set to boost with wastegate actuator only boost level)
      {
        current.boostDuty = convertTargetToDuty(current, page2, page6, page10);
      }
      else
      {
        boostPID.initialize(current.MAP); //This resets the ITerm value to prevent rubber banding
        //Boost control needs to have a high duty cycle if control is below threshold (baro or fixed value). This ensures the waste gate is closed as much as possible, this build boost as fast as possible.
        current.boostDuty = page15.boostDCWhenDisabled*100;
      } //MAP above boost + hyster
    } //Open / Cloosed loop   
  }
  else { // Disable timer channel and zero the flex boost correction status
    boostPID.initialize(current.MAP); //This resets the ITerm value to prevent rubber banding
    current.boostTarget = 0;
    current.boostDuty = 0;
    current.flexBoostCorrection = 0;
  }
  applyDutyToPwm(current);
}

//The interrupt to control the Boost PWM
void boostInterrupt(void)
{
  if (boostOutput.isPartialDuty())
  {
    if (boostOutput.pin.isPinHigh())
    {
      boostOutput.pin.setPinLow();
      SET_COMPARE(BOOST_TIMER_COMPARE, BOOST_TIMER_COUNTER + (boostOutput.maxDuty - boostOutput.targetDuty) );
    }
    else
    {
      boostOutput.pin.setPinHigh();
      SET_COMPARE(BOOST_TIMER_COMPARE, BOOST_TIMER_COUNTER + boostOutput.targetDuty);
    }
  }
}
