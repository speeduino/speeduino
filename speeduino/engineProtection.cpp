#include "globals.h"
#include "engineProtection.h"
#include "maths.h"
#include "units.h"
#include "unit_testing.h"
#include "preprocessor.h"
#include "decoders.h"
#include "units.h"
#include "preprocessor.h"

TESTABLE_STATIC uint32_t oilProtEndTime;
TESTABLE_CONSTEXPR table2D_u8_u8_4 oilPressureProtectTable(&configPage10.oilPressureProtRPM, &configPage10.oilPressureProtMins);
TESTABLE_CONSTEXPR table2D_u8_u8_6 coolantProtectTable(&configPage9.coolantProtTemp, &configPage9.coolantProtRPM);

/* AFR protection state moved to file scope so unit tests can control/reset it */
TESTABLE_STATIC bool checkAFRLimitActive = false;
TESTABLE_STATIC unsigned long afrProtectedActivateTime = 0;

TESTABLE_INLINE_STATIC bool checkOilPressureLimit(const statuses &current, const config6 &page6, const config10 &page10, uint32_t currMillis)
{
  bool engineProtectOil = false; //Will be set true below if required

  if ( (page6.engineProtectType != PROTECT_CUT_OFF) 
    && (page10.oilPressureProtEnbl == true) 
    && (page10.oilPressureEnable == true))
  {
    uint8_t oilLimit = table2D_getValue(&oilPressureProtectTable, current.RPMdiv100);
    if(current.oilPressure < oilLimit)
    {
      //Check if this is the first time we've been below the limit
      if(oilProtEndTime == 0U) { oilProtEndTime = currMillis + TIME_TEN_MILLIS.toUser(page10.oilPressureProtTime); }
      /* Check if countdown has reached its target, if so then instruct to cut */
      engineProtectOil = (currMillis >= oilProtEndTime) || (current.engineProtect.oil);
    }
    else 
    { 
      oilProtEndTime = 0; //Reset the timer
    }
  }

  return engineProtectOil;
}

TESTABLE_INLINE_STATIC bool checkBoostLimit(const statuses &current, const config6 &page6)
{
  return (page6.engineProtectType != PROTECT_CUT_OFF)
      && (page6.boostCutEnabled > 0) 
      && (current.MAP > ((long)page6.boostLimit * 2L));
}

static inline bool canApplyAfrLimit(const config6 &page6, const config9 &page9)
{
  return (page6.engineProtectType != PROTECT_CUT_OFF) 
      && (page9.afrProtectEnabled != AFR_PROTECT_OFF) 
      && (page6.egoType == EGO_TYPE_WIDE);
}

static inline uint16_t getAfrO2Limit(const statuses &current, const config9 &page9)
{
  if (page9.afrProtectEnabled==AFR_PROTECT_FIXED) {
    return page9.afrProtectDeviation;
  } 
  if (page9.afrProtectEnabled==AFR_PROTECT_TABLE) {
    return current.afrTarget + (uint16_t)page9.afrProtectDeviation;
  }
  
  return UINT16_MAX;
}

static inline bool isAfrLimitCondtionActive(const statuses &current, const config9 &page9)
{
    return (current.MAP >= (long)(page9.afrProtectMinMAP * UINT16_C(2)))
          && (current.RPMdiv100 >= page9.afrProtectMinRPM) 
          && (current.TPS >= page9.afrProtectMinTPS) 
          && (current.O2 >= getAfrO2Limit(current, page9)); 
}

TESTABLE_INLINE_STATIC bool checkAFRLimit(const statuses &current, const config6 &page6, const config9 &page9, uint32_t currMillis)
{
  /*
    To use this function, a wideband sensor is required.

    First of all, check whether engine protection is enabled,
    thereafter check whether AFR protection is enabled and at last
    if wideband sensor is used.
    
    After confirmation, the following conditions has to be met:
    - MAP above x kPa
    - RPM above x
    - TPS above x %
    - AFR threshold (AFR target + defined maximum deviation)
    - Time before cut

    See afrProtect variables in globals.h for more information.

    If all conditions above are true, a specified time delay is starting
    to count down in which leads to the engine protection function
    to be activated using selected protection cut method (e.g. ignition,
    fuel or both).

    For reactivation, the following condition has to be met:
    - TPS below x %
  */
  if ( canApplyAfrLimit(page6, page9) )
  {
    if (isAfrLimitCondtionActive(current, page9))
    {
      // All conditions fulfilled - start counter for 'protection delay'
      if(afrProtectedActivateTime==0U) 
      {
        afrProtectedActivateTime = currMillis + (page9.afrProtectCutTime * UINT16_C(100));
      }

      // Check if countdown has reached its target, if so then instruct to cut
      checkAFRLimitActive = currMillis >= afrProtectedActivateTime;
    } 
    else 
    {
      // NOTE: we deliberately do not reset checkAFRLimitActive here
      // Once AFR protection is in effect, user must reduce throttle
      // to below the reactivation limit to reset manually (below)

      // Do nothing
    }

    // Check if condition for reactivation is fulfilled
    if(current.TPS <= page9.afrProtectReactivationTPS)
    {
      checkAFRLimitActive = false;
      afrProtectedActivateTime = 0U;
    }
  }
  else
  {
    checkAFRLimitActive = false;
  }
  return checkAFRLimitActive;
}


TESTABLE_INLINE_STATIC bool checkEngineProtect(statuses &current, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10, uint32_t currMillis)
{
  current.engineProtect.boostCut = checkBoostLimit(current, page6);
  current.engineProtect.oil = checkOilPressureLimit(current, page6, page10, currMillis);
  current.engineProtect.afr = checkAFRLimit(current, page6, page9, currMillis);

  return (current.engineProtect.boostCut || current.engineProtect.oil || current.engineProtect.afr)
      && ( current.RPMdiv100 > page4.engineProtectMaxRPM );
}

TESTABLE_INLINE_STATIC bool checkRpmLimit(const statuses &current, const config4 &page4, const config6 &page6, const config9 &page9)
{
  return (page6.engineProtectType != PROTECT_CUT_OFF) 
      && (page9.hardRevMode == HARD_REV_FIXED)
      && ((current.RPMdiv100 >= page4.HardRevLim) 
        || ((softLimitTime > page4.SoftLimMax) && (current.RPMdiv100 >= page4.SoftRevLim)));
}

TESTABLE_INLINE_STATIC bool checkCoolantLimit(const statuses &current, const config6 &page6, const config9 &page9)
{
  return (page6.engineProtectType != PROTECT_CUT_OFF) 
      && (page9.hardRevMode == HARD_REV_COOLANT)
      && (current.RPMdiv100 > table2D_getValue(&coolantProtectTable, temperatureAddOffset(current.coolant)));
}

TESTABLE_INLINE_STATIC uint8_t checkRevLimit(statuses &current, const config4 &page4, const config6 &page6, const config9 &page9)
{
  //Hardcut RPM limit
  uint8_t currentLimitRPM = UINT8_MAX; //Default to no limit (In case PROTECT_CUT_OFF is selected)
  current.engineProtect.rpm = false;
  current.engineProtect.coolant = false;

  if (page6.engineProtectType != PROTECT_CUT_OFF) 
  {
    if(page9.hardRevMode == HARD_REV_FIXED)
    {
      currentLimitRPM = page4.HardRevLim;
      current.engineProtect.rpm = (current.RPMdiv100 >= page4.HardRevLim) 
                              || ((softLimitTime > page4.SoftLimMax) && (current.RPMdiv100 >= page4.SoftRevLim));
    }
    else if(page9.hardRevMode == HARD_REV_COOLANT )
    {
      currentLimitRPM = table2D_getValue(&coolantProtectTable, temperatureAddOffset(current.coolant));
      if(current.RPMdiv100 > currentLimitRPM)
      {
        current.engineProtect.coolant = true;
        current.engineProtect.rpm = true;
      } 
    }
  }

  return currentLimitRPM;
}

statuses::engine_protect_flags_t checkEngineProtection(const statuses &current, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10)
{
  statuses::engine_protect_flags_t flags = { false, false, false, false, false };

  if (page6.engineProtectType != PROTECT_CUT_OFF) 
  {
    flags.boostCut = checkBoostLimit(current, page6);
    flags.oil = checkOilPressureLimit(current, page6, page10, millis());
    flags.afr = checkAFRLimit(current, page6, page9, millis());
    flags.coolant = checkCoolantLimit(current, page6, page9);
    flags.rpm = flags.coolant || checkRpmLimit(current, page4, page6, page9);
  }

  return flags;
};

TESTABLE_INLINE_STATIC uint8_t getHardRevLimit(const statuses &current, const config4 &page4, const config9 &page9)
{
  if (page9.hardRevMode == HARD_REV_FIXED)
  {
    return page4.HardRevLim;
  }
  if (page9.hardRevMode == HARD_REV_COOLANT)
  {
    return table2D_getValue(&coolantProtectTable, temperatureAddOffset(current.coolant));
  }
  return UINT8_MAX;
}

TESTABLE_INLINE_STATIC uint8_t applyEngineProtectionRevLimit(uint8_t curLimit, const statuses &current, const config4 &page4)
{
  if ((current.engineProtect.boostCut) || (current.engineProtect.oil) || (current.engineProtect.afr))
  {
    return min(curLimit, page4.engineProtectMaxRPM);
  }

  return curLimit;
}

TESTABLE_INLINE_STATIC uint8_t applyHardLaunchRevLimit(uint8_t curLimit, const statuses &current, const config6 &page6)
{
  if (current.launchingHard)
  {
    return min(curLimit, page6.lnchHardLim);
  }

  return curLimit;
}

TESTABLE_INLINE_STATIC uint16_t applyFlatShiftRevLimit(uint16_t curLimit, const statuses &current)
{
  if ( current.flatShiftingHard ) 
  {
    return min(curLimit, (uint16_t)current.clutchEngagedRPM);
  }
  return curLimit;
}

TESTABLE_INLINE_STATIC uint16_t getMaxRpm(const statuses &current, const config4 &page4, const config6 &page6, const config9 &page9)
{
  // The maximum RPM allowed by all the potential limiters (Engine protection, 2-step, flat shift etc).
  // Divided by 100.
  return applyFlatShiftRevLimit(
          (uint16_t)applyHardLaunchRevLimit(
              applyEngineProtectionRevLimit(
                getHardRevLimit(current, page4, page9), 
                current, page4),
              current, page6) * 100U,
            current);
  
}

TESTABLE_STATIC uint32_t rollingCutLastRev = 0; /**< Tracks whether we're on the same or a different rev for the rolling cut */

// Test-hookable RNG for rolling cut (defaults to existing random1to100)
TESTABLE_STATIC uint8_t (*rollingCutRandFunc)(void) = random1to100;

static inline statuses::scheduler_cut_t applyFullCut(const config6 &page6)
{
  //Full hard cut turns outputs off completely. 
  switch(page6.engineProtectType)
  {
    case PROTECT_CUT_IGN:
      return { .ignitionChannelsPending = 0x00, .ignitionChannels = 0x00, .fuelChannels = 0xFF, .status = SchedulerCutStatus::Full };
      break;
    case PROTECT_CUT_FUEL:
      return { .ignitionChannelsPending = 0x00, .ignitionChannels = 0xFF, .fuelChannels = 0x00, .status = SchedulerCutStatus::Full };
      break;
    case PROTECT_CUT_BOTH:
    default:
      return { .ignitionChannelsPending = 0x00, .ignitionChannels = 0x00, .fuelChannels = 0x00, .status = SchedulerCutStatus::Full };
      break;
  }
}

TESTABLE_CONSTEXPR table2D_i8_u8_4 rollingCutTable(&configPage15.rollingProtRPMDelta, &configPage15.rollingProtCutPercent);

TESTABLE_INLINE_STATIC bool useRollingCut(const statuses &current, const config2 &page2, uint16_t maxAllowedRPM)
{
  //Limit for rolling is the max allowed RPM minus the lowest value in the delta table (Delta values are negative!)
  return (page2.hardCutType == HARD_CUT_ROLLING) 
      && (current.RPM < maxAllowedRPM)
      && (current.RPM > (maxAllowedRPM + (rollingCutTable.axis[0] * 10)));
}

static inline bool isNonSequential(const config2 &page2, const config4 &page4)
{
  return (page4.sparkMode != IGN_MODE_SEQUENTIAL) || (page2.injLayout != INJ_SEQUENTIAL);
}

static inline uint8_t calcBaseCutRevolutions(const config2 &page2)
{
  return (page2.strokes == FOUR_STROKE) ? 2U : 1U;
}

TESTABLE_INLINE_STATIC uint8_t calcRollingCutRevolutions(const config2 &page2, const config4 &page4)
{
  // 4 stroke and non-sequential will cut for 4 revolutions minimum. This is to ensure no half fuel ignition cycles take place
  if ( isNonSequential(page2, page4) ) 
  { 
    return calcBaseCutRevolutions(page2) * 2U; 
  } 
  return calcBaseCutRevolutions(page2);
}

TESTABLE_INLINE_STATIC uint8_t calcRollingCutPercentage(const statuses &current, uint16_t maxAllowedRPM)
{
  int16_t rpmDelta = current.RPM - maxAllowedRPM;
  //If the current RPM is over the max allowed RPM then cut is full (100%)
  if (rpmDelta >= 0) { 
    // 101 is used to make the comparison below simpler
    return 101U; 
  }
  // Avoid underflow
  if (rpmDelta<((int16_t)INT8_MIN*10)) {
    return rollingCutTable.values[0];
  }
  
  return table2D_getValue(&rollingCutTable, (int8_t)(rpmDelta / 10) ); 
}

static inline statuses::scheduler_cut_t channelOff(statuses::scheduler_cut_t cutState, const config6 &page6, uint8_t channel)
{
  switch(page6.engineProtectType)
  {
    case PROTECT_CUT_IGN:
      BIT_CLEAR(cutState.ignitionChannels, channel); //Turn off this ignition channel
      break;
    case PROTECT_CUT_FUEL:
      BIT_CLEAR(cutState.fuelChannels, channel); //Turn off this fuel channel
      break;
    case PROTECT_CUT_BOTH:
    default:
      BIT_CLEAR(cutState.ignitionChannels, channel); //Turn off this ignition channel
      BIT_CLEAR(cutState.fuelChannels, channel); //Turn off this fuel channel
      break;
  }
  return cutState;
}

TESTABLE_INLINE_STATIC statuses::scheduler_cut_t channelOn(statuses::scheduler_cut_t cutState, bool pendingIgnitionCut, uint8_t channel)
{
  // Special case for non-sequential, 4-stroke where both fuel and ignition are cut.
  // The ignition pulses should wait 1 cycle after the fuel channels are turned back on before firing again
  if( pendingIgnitionCut &&
      //Fuel on this channel is currently off, meaning it is the first revolution after a cut
      (BIT_CHECK(cutState.fuelChannels, channel) == false))
  { 
    BIT_SET(cutState.ignitionChannelsPending, channel); //Set this ignition channel as pending
  }
  else 
  { 
    BIT_SET(cutState.ignitionChannels, channel); //Turn on this ignition channel
  }
          
  BIT_SET(cutState.fuelChannels, channel); //Turn on this fuel channel

  return cutState;
}

static inline bool supportPendingIgnitionCut(const config2 &page2, const config4 &page4)
{
  return (page2.strokes == FOUR_STROKE) && isNonSequential(page2, page4);
}

static inline statuses::scheduler_cut_t applyRollingCutPercentage(const statuses &current, const config2 &page2, const config4 &page4, const config6 &page6, uint8_t cutPercent)
{
  bool pendingIgnitionCut = supportPendingIgnitionCut(page2, page4) && (page6.engineProtectType == PROTECT_CUT_BOTH);

  statuses::scheduler_cut_t cutState = current.schedulerCutState;
  for(uint8_t channel=0; channel<max(current.maxIgnOutputs, current.maxInjOutputs); ++channel)
  {  
    if( rollingCutRandFunc() < cutPercent )
    {
      cutState = channelOff(cutState, page6, channel);
    }
    else
    {
      cutState = channelOn(cutState, pendingIgnitionCut, channel);
    }
  }

  cutState.status = SchedulerCutStatus::Rolling;
  return cutState;
}

BEGIN_LTO_ALWAYS_INLINE(statuses::scheduler_cut_t) calculateFuelIgnitionChannelCut(statuses &current, const config2 &page2, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10)
{
  if ((getDecoderStatus().syncStatus==SyncStatus::None) || (current.startRevolutions < page4.StgCycles))
  {
      return { .ignitionChannelsPending = 0x00, .ignitionChannels = 0x00, .fuelChannels = 0x00, .status = SchedulerCutStatus::Full };
  }
  if (page6.engineProtectType==PROTECT_CUT_OFF)
  {
    //Make sure all channels are turned on
    current.engineProtect.reset();
    return { .ignitionChannelsPending = 0x00, .ignitionChannels = 0xFF, .fuelChannels = 0xFF, .status = SchedulerCutStatus::None };
  }

  statuses::scheduler_cut_t cutState = current.schedulerCutState;

  //Check for any of the engine protections or rev limiters being turned on
  uint16_t maxAllowedRPM = getMaxRpm(current, page4, page6, page9);

  // Full cut is always applied if RPM exceeds max allowed
  // regardless of page2.hardCutType
  if (current.RPM >= maxAllowedRPM)
  {
    return applyFullCut(page6);
  }
  else if (useRollingCut(current, page2, maxAllowedRPM))
  { 
    cutState.status = SchedulerCutStatus::Rolling; 
    if(rollingCutLastRev == 0) { rollingCutLastRev = current.startRevolutions; } //First time check

    uint8_t revolutionsToCut = calcRollingCutRevolutions(page2, page4);
    if ( (current.startRevolutions >= (rollingCutLastRev + revolutionsToCut))) //Check if the required number of revolutions have passed since the last cut
    { 
      rollingCutLastRev = current.startRevolutions;
      cutState = applyRollingCutPercentage(current, page2, page4, page6, calcRollingCutPercentage(current, maxAllowedRPM));
    }

    //Check whether there are any ignition channels that are waiting for injection pulses to occur before being turned back on. This can only occur when at least 2 revolutions have taken place since the fuel was turned back on
    //Note that ignitionChannelsPending can only be >0 on 4 stroke, non-sequential fuel when protect type is Both
    if( (cutState.ignitionChannelsPending > 0) && (current.startRevolutions >= (rollingCutLastRev + 2)) )
    {
      cutState.ignitionChannels = cutState.fuelChannels;
      cutState.ignitionChannelsPending = 0;
    }
  } //Rolling cut check
  else
  {
    current.engineProtect.reset();
    return { .ignitionChannelsPending = 0x00, .ignitionChannels = 0xFF, .fuelChannels = 0xFF, .status = SchedulerCutStatus::None };
  }

  return cutState;
}
END_LTO_INLINE()