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

__attribute__((optimize("Os"))) void initialiseBoost(uint8_t boostPin)
{
  boostOutput = PwmOutputChannel(boostPin, FREQUENCY.toUser(configPage6.boostFreq));
  setBoostPidTunings(configPage2, configPage6, configPage10);
  currentStatus.boostDuty = 0;
}

void boostDisable(void)
{
  boostPID.initialize(currentStatus.MAP); //This resets the ITerm value to prevent rubber banding
  currentStatus.boostDuty = 0;
  DISABLE_BOOST_TIMER(); //Turn off timer
  boostOutput.pin.setPinLow(); //Make sure solenoid is off (0% duty)
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
    target = get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM) << 1;
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

static uint16_t convertTargetToDuty(const statuses &current)
{
  uint16_t duty = 0;
  if(current.boostTarget > 0)
  {
    if( BIT_CHECK(current.LOOP_TIMER, BIT_TIMER_1HZ) )
    {
      setBoostPidTunings(configPage2, configPage6, configPage10);
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

void boostControl(void)
{
  if(isBoostActive(currentStatus, configPage6) )
  {
    if(configPage4.boostType == OPEN_LOOP_BOOST)
    {
      currentStatus.boostDuty = getBoostDuty(currentStatus, configPage2, configPage9);
    }
    else if (configPage4.boostType == CLOSED_LOOP_BOOST)
    {
      if( BIT_CHECK(currentStatus.LOOP_TIMER, BIT_TIMER_4HZ) )
      { 
        currentStatus.flexBoostCorrection = getFlexCorrection(currentStatus, configPage2);
        currentStatus.boostTarget = getBoostTarget(currentStatus, configPage2, configPage9);
      } 

      if(((configPage15.boostControlEnable == EN_BOOST_CONTROL_BARO) && (currentStatus.MAP >= currentStatus.baro)) || ((configPage15.boostControlEnable == EN_BOOST_CONTROL_FIXED) && (currentStatus.MAP >= configPage15.boostControlEnableThreshold))) //Only enables boost control above baro pressure or above user defined threshold (User defined level is usually set to boost with wastegate actuator only boost level)
      {
        currentStatus.boostDuty = convertTargetToDuty(currentStatus);
      }
      else
      {
        boostPID.initialize(currentStatus.MAP); //This resets the ITerm value to prevent rubber banding
        //Boost control needs to have a high duty cycle if control is below threshold (baro or fixed value). This ensures the waste gate is closed as much as possible, this build boost as fast as possible.
        currentStatus.boostDuty = configPage15.boostDCWhenDisabled*100;
      } //MAP above boost + hyster
    } //Open / Cloosed loop   
  }
  else { // Disable timer channel and zero the flex boost correction status
    boostPID.initialize(currentStatus.MAP); //This resets the ITerm value to prevent rubber banding
    currentStatus.boostTarget = 0;
    currentStatus.boostDuty = 0;
    currentStatus.flexBoostCorrection = 0;
  }
  applyDutyToPwm(currentStatus);
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
