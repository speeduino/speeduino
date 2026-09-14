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

// Convert a percentage to half percentage (percentage * 2)
static constexpr uint16_t percentToHalfPct(uint16_t percent)
{
  return percent * 2U;
}

// Convert a percentage to duty cycle (percentage * 100)
static constexpr uint16_t percentToDuty(uint16_t percent)
{
  return percent * 100U;
}

static __attribute__((optimize("Os"))) void setBoostPidTunings(const config2 &page2, const config6 &page6, const config10 &page10) noexcept
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

static uint16_t getBoostByGearFactor(const statuses &current, const config9 &page9) noexcept
{
  if ((current.gear>0U) && (current.gear-1U)<_countof(page9.boostByGear))
  {
    return PRESSURE.toUser(page9.boostByGear[current.gear-1U]);
  }
  return 0U;
}

enum class BoostByGearMode : uint8_t
{
  Off,
  Percent,
  Constant
};

static inline BoostByGearMode getBoostByGearMode(const config2 &page2, const config9 &page9)
{
  if( (page9.boostByGearEnabled == BOOST_BY_GEAR_PERCENT) && isExternalVssMode(page2) )
  {
    return BoostByGearMode::Percent;
  }
  else if( (page9.boostByGearEnabled == BOOST_BY_GEAR_CONSTANT) && isExternalVssMode(page2) )
  {
    return BoostByGearMode::Constant;
  }
  else
  {
    return BoostByGearMode::Off;
  }
}

static inline uint16_t lookupBoostTable(const statuses &current) noexcept
{
  // In open loop mode, the values in this table are duty cycle %
  // In closed loop mode, the values in this table are boost targets in kPa
  // In both cases, the values are stored in the table as kPa/2 (i.e. 1kPa = 2 in the table)
  return PRESSURE.toUser(get3DTableValue(&boostTable, percentToHalfPct(current.TPS), current.RPM));
}

static uint16_t getBoostDuty(const statuses &current, const config2 &page2, const config9 &page9)
{
  uint16_t duty = 0;
  if (getBoostByGearMode(page2, page9) == BoostByGearMode::Percent)
  {
    duty = getBoostByGearFactor(current, page9) * lookupBoostTable(current);
  }
  else if (getBoostByGearMode(page2, page9) == BoostByGearMode::Constant)
  {
    duty = percentToDuty(getBoostByGearFactor(current, page9));
  }
  else
  {
    duty = percentToDuty(lookupBoostTable(current));
  }
  return clamp(duty, (uint16_t)0, percentToDuty(100U));
}

static uint16_t getBoostTarget(const statuses &current, const config2 &page2, const config9 &page9)
{
  uint16_t target = 0;
  if (getBoostByGearMode(page2, page9) == BoostByGearMode::Percent)
  {
    target = percentage(lookupBoostTable(current), getBoostByGearFactor(current, page9));
  }
  else if (getBoostByGearMode(page2, page9) == BoostByGearMode::Constant)
  {
    target = getBoostByGearFactor(current, page9);
  }
  else
  {
    // LCOV_EXCL_BR_START
    INTERNAL_TEST_ASSERT(BoostByGearMode::Off == getBoostByGearMode(page2, page9));
    // LCOV_EXCL_BR_STOP

    //Boost target table is in kpa and divided by 2
    target = lookupBoostTable(current);
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
    // LCOV_EXCL_BR_START
    // The timer check *MUST* be a multiple of the boost control interval
    // otherwise branch will NEVER be taken.
    if( BIT_CHECK(current.LOOP_TIMER, BIT_TIMER_1HZ) )
    // LCOV_EXCL_BR_STOP
    {
      setBoostPidTunings(page2, page6, page10);
    }

    boostPID.setSetPoint(current.boostTarget);
    boostPID.setFeedForwardTerm(get3DTableValue(&boostTableLookupDuty, current.boostTarget, current.RPM) * 50U);
    // LCOV_EXCL_BR_START
    (void)boostPID.compute(millis(), current.MAP, &duty);
    // LCOV_EXCL_BR_STOP
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

static inline bool isBoostControlBaroActive(const statuses &current, const config15 &page15)
{
  return (page15.boostControlEnable == EN_BOOST_CONTROL_BARO) 
      && (current.MAP >= current.baro)
      ;
}

static inline bool isBoostControlFixedActive(const statuses &current, const config15 &page15)
{
  return (page15.boostControlEnable == EN_BOOST_CONTROL_FIXED) 
      && (current.MAP >= page15.boostControlEnableThreshold)
      ;
}

TESTABLE_STATIC void boostControlCore(statuses &current, const config2 &page2, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10, const config15 &page15)
{
  if(isBoostActive(current, page6) )
  {
    if(page4.boostType == OPEN_LOOP_BOOST)
    {
      current.boostDuty = getBoostDuty(current, page2, page9);
    }
    else // CLOSED_LOOP_BOOST
    {
      // LCOV_EXCL_BR_START
      // The timer check *MUST* be a multiple of the boost control interval
      // otherwise branch will NEVER be taken.
      if( BIT_CHECK(current.LOOP_TIMER, BIT_TIMER_10HZ) )
      // LCOV_EXCL_BR_STOP
      { 
        current.flexBoostCorrection = getFlexCorrection(current, page2);
        current.boostTarget = getBoostTarget(current, page2, page9);
        current.boostDuty = convertTargetToDuty(current, page2, page6, page10);
      } 

      // Only enables boost control above baro pressure or above user defined threshold 
      // (User defined level is usually set to boost with wastegate actuator only boost level)
      if(!isBoostControlBaroActive(current, page15) && !isBoostControlFixedActive(current, page15)) 
      {
        boostPID.initialize(current.MAP); //This resets the ITerm value to prevent rubber banding
        // Boost control needs to have a high duty cycle if control is below threshold (baro or fixed value). 
        // This ensures the waste gate is closed as much as possible, this build boost as fast as possible.
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

// LCOV_EXCL_START
void boostControl(statuses &current, const config2 &page2, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10, const config15 &page15)
{
  if( BIT_CHECK(current.LOOP_TIMER, BIT_TIMER_30HZ) )
  {
    boostControlCore(current, page2, page4, page6, page9, page10, page15);
  }
}
// LCOV_EXCL_STOP

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
