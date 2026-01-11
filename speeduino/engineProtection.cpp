#include "globals.h"
#include "engineProtection.h"
#include "maths.h"
#include "units.h"
#include "unit_testing.h"
#include "preprocessor.h"
#include "decoders.h"
#include "units.h"

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
      engineProtectOil = (currMillis >= oilProtEndTime) || (current.engineProtectOil);
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

static inline uint16_t getAfrO2Limit(const statuses &current, const config9 &page9)
{
  if (page9.afrProtectEnabled==AFR_PROTECT_FIXED) {
    return page9.afrProtectDeviation;
  } if (page9.afrProtectEnabled==AFR_PROTECT_TABLE) {
    return current.afrTarget + page9.afrProtectDeviation;
  } else {
    return UINT16_MAX;
  }
}

static inline bool afrLimitAfrCondition(const statuses &current, const config9 &page9)
{
  /*
    Depending on selected mode, this could either be fixed AFR value or a
    value set to be the maximum deviation from AFR target table.

    1 = fixed value mode, 2 = target table mode
  */
  return (page9.afrProtectEnabled!=AFR_PROTECT_OFF)
      && (current.O2 >=getAfrO2Limit(current, page9));
}

TESTABLE_INLINE_STATIC bool checkAFRLimit(const statuses &current, const config6 &page6, const config9 &page9, uint32_t currMillis)
{
  static constexpr char X2_MULTIPLIER = 2;
  static constexpr char X100_MULTIPLIER = 100;

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

  /*
    Do 3 checks here;
    - whether engine protection is enabled
    - whether AFR protection is enabled
    - whether wideband sensor is used
  */
  if((page6.engineProtectType != PROTECT_CUT_OFF) && (page9.afrProtectEnabled!=AFR_PROTECT_OFF) && (page6.egoType == EGO_TYPE_WIDE)) {
    /* Conditions */
    bool mapCondition = (current.MAP >= (page9.afrProtectMinMAP * X2_MULTIPLIER));
    bool rpmCondition = (current.RPMdiv100 >= page9.afrProtectMinRPM);
    bool tpsCondition = (current.TPS >= page9.afrProtectMinTPS);
    bool afrCondition = afrLimitAfrCondition(current, page9);

    /* Check if conditions above are fulfilled */
    if(mapCondition && rpmCondition && tpsCondition && afrCondition) 
    {
      /* All conditions fulfilled - start counter for 'protection delay' */
      if(afrProtectedActivateTime==0U) 
      {
        afrProtectedActivateTime = currMillis + (page9.afrProtectCutTime * X100_MULTIPLIER);
      }

      /* Check if countdown has reached its target, if so then instruct to cut */
      checkAFRLimitActive = currMillis >= afrProtectedActivateTime;
    } 
    else 
    {
      /* Conditions have presumably changed - deactivate and reset counter */
      afrProtectedActivateTime = 0U;
    }

    /* Check if condition for reactivation is fulfilled */
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
  current.engineProtectBoostCut = checkBoostLimit(current, page6);
  current.engineProtectOil = checkOilPressureLimit(current, page6, page10, currMillis);
  current.engineProtectAfr = checkAFRLimit(current, page6, page9, currMillis);

  return (current.engineProtectBoostCut || current.engineProtectOil || current.engineProtectAfr)
      && ( current.RPMdiv100 > page4.engineProtectMaxRPM );
}

BEGIN_LTO_ALWAYS_INLINE(bool) checkEngineProtect(statuses &current, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10)
{
  return checkEngineProtect(current, page4, page6, page9, page10, millis());
}
END_LTO_INLINE();

uint8_t checkRevLimit(statuses &current, const config4 &page4, const config6 &page6, const config9 &page9)
{
  //Hardcut RPM limit
  uint8_t currentLimitRPM = UINT8_MAX; //Default to no limit (In case PROTECT_CUT_OFF is selected)
  current.engineProtectRpm = false;
  current.engineProtectClt = false;

  if (page6.engineProtectType != PROTECT_CUT_OFF) 
  {
    if(page9.hardRevMode == HARD_REV_FIXED)
    {
      currentLimitRPM = page4.HardRevLim;
      current.engineProtectRpm = (current.RPMdiv100 >= page4.HardRevLim) 
                              || ((softLimitTime > page4.SoftLimMax) && (current.RPMdiv100 >= page4.SoftRevLim));
    }
    else if(page9.hardRevMode == HARD_REV_COOLANT )
    {
      currentLimitRPM = table2D_getValue(&coolantProtectTable, temperatureAddOffset(current.coolant));
      if(current.RPMdiv100 > currentLimitRPM)
      {
        current.engineProtectClt = true;
        current.engineProtectRpm = true;
      } 
    }
  }

  return currentLimitRPM;
}

TESTABLE_STATIC uint32_t rollingCutLastRev = 0; /**< Tracks whether we're on the same or a different rev for the rolling cut */
TESTABLE_CONSTEXPR table2D_i8_u8_4 rollingCutTable(&configPage15.rollingProtRPMDelta, &configPage15.rollingProtCutPercent);

// Test-hookable RNG for rolling cut (defaults to existing random1to100)
TESTABLE_STATIC uint8_t (*rollingCutRandFunc)(void) = random1to100;

statuses::scheduler_cut_t calculateFuelIgnitionChannelCut(statuses &current, const config2 &page2, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10)
{
  if (getDecoderStatus().syncStatus==SyncStatus::None)
  {
    return { 0x0, 0x0, 0x0 };
  }

  statuses::scheduler_cut_t cutState = current.schedulerCutState;

  //Check for any of the engine protections or rev limiters being turned on
  uint16_t maxAllowedRPM = checkRevLimit(current, page4, page6, page9); //The maximum RPM allowed by all the potential limiters (Engine protection, 2-step, flat shift etc). Divided by 100. `checkRevLimit()` returns the current maximum RPM allow (divided by 100) based on either the fixed hard limit or the current coolant temp
  //Check each of the functions that has an RPM limit. Update the max allowed RPM if the function is active and has a lower RPM than already set
  if( checkEngineProtect(current, page4, page6, page9, page10) && (page4.engineProtectMaxRPM < maxAllowedRPM)) { maxAllowedRPM = page4.engineProtectMaxRPM; }
  if ( (current.launchingHard == true) && (page6.lnchHardLim < maxAllowedRPM) ) { maxAllowedRPM = page6.lnchHardLim; }
  maxAllowedRPM = maxAllowedRPM * 100U; //All of the above limits are divided by 100, convert back to RPM
  if ( (current.flatShiftingHard == true) && (current.clutchEngagedRPM < maxAllowedRPM) ) { maxAllowedRPM = current.clutchEngagedRPM; } //Flat shifting is a special case as the RPM limit is based on when the clutch was engaged. It is not divided by 100 as it is set with the actual RPM

  if(current.RPM >= maxAllowedRPM)
  {
    current.hardLimitActive = true;
  }
  else if(current.hardLimitActive)
  {
    current.hardLimitActive = false;
  }

  if( (page2.hardCutType == HARD_CUT_FULL) && current.hardLimitActive)
  {
    //Full hard cut turns outputs off completely. 
    switch(page6.engineProtectType)
    {
      case PROTECT_CUT_OFF:
        //Make sure all channels are turned on
        cutState.ignitionChannels = 0xFF;
        cutState.fuelChannels = 0xFF;
        resetEngineProtect(current);
        break;
      case PROTECT_CUT_IGN:
        cutState.ignitionChannels = 0;
        break;
      case PROTECT_CUT_FUEL:
        cutState.fuelChannels = 0;
        break;
      case PROTECT_CUT_BOTH:
        cutState.ignitionChannels = 0;
        cutState.fuelChannels = 0;
        break;
      default:
        cutState.ignitionChannels = 0;
        cutState.fuelChannels = 0;
        break;
    }
  } //Hard cut check
  else if( (page2.hardCutType == HARD_CUT_ROLLING) && (current.RPM > (maxAllowedRPM + (rollingCutTable.axis[0] * 10))) ) //Limit for rolling is the max allowed RPM minus the lowest value in the delta table (Delta values are negative!)
  { 
    uint8_t revolutionsToCut = 1;
    if(page2.strokes == FOUR_STROKE) { revolutionsToCut *= 2; } //4 stroke needs to cut for at least 2 revolutions
    if( (page4.sparkMode != IGN_MODE_SEQUENTIAL) || (page2.injLayout != INJ_SEQUENTIAL) ) { revolutionsToCut *= 2; } //4 stroke and non-sequential will cut for 4 revolutions minimum. This is to ensure no half fuel ignition cycles take place

    if(rollingCutLastRev == 0) { rollingCutLastRev = current.startRevolutions; } //First time check
    if ( (current.startRevolutions >= (rollingCutLastRev + revolutionsToCut)) || (current.RPM > maxAllowedRPM) ) //If current RPM is over the max allowed RPM always cut, otherwise check if the required number of revolutions have passed since the last cut
    { 
      uint8_t cutPercent = 0;
      int16_t rpmDelta = current.RPM - maxAllowedRPM;
      if(rpmDelta >= 0) { cutPercent = 100; } //If the current RPM is over the max allowed RPM then cut is full (100%)
      else { cutPercent = table2D_getValue(&rollingCutTable, (int8_t)(rpmDelta / 10) ); } //
      

      for(uint8_t x=0; x<max(current.maxIgnOutputs, current.maxInjOutputs); x++)
      {  
        if( (cutPercent == 100) || (rollingCutRandFunc() < cutPercent) )
        {
          switch(page6.engineProtectType)
          {
            case PROTECT_CUT_OFF:
              //Make sure all channels are turned on
              cutState.ignitionChannels = 0xFF;
              cutState.fuelChannels = 0xFF;
              break;
            case PROTECT_CUT_IGN:
              BIT_CLEAR(cutState.ignitionChannels, x); //Turn off this ignition channel
              break;
            case PROTECT_CUT_FUEL:
              BIT_CLEAR(cutState.fuelChannels, x); //Turn off this fuel channel
              break;
            case PROTECT_CUT_BOTH:
              BIT_CLEAR(cutState.ignitionChannels, x); //Turn off this ignition channel
              BIT_CLEAR(cutState.fuelChannels, x); //Turn off this fuel channel
              break;
            default:
              BIT_CLEAR(cutState.ignitionChannels, x); //Turn off this ignition channel
              BIT_CLEAR(cutState.fuelChannels, x); //Turn off this fuel channel
              break;
          }
        }
        else
        {
          //Turn fuel and ignition channels on

          //Special case for non-sequential, 4-stroke where both fuel and ignition are cut. The ignition pulses should wait 1 cycle after the fuel channels are turned back on before firing again
          if( (revolutionsToCut == 4) &&                          //4 stroke and non-sequential
              (BIT_CHECK(cutState.fuelChannels, x) == false) &&          //Fuel on this channel is currently off, meaning it is the first revolution after a cut
              (page6.engineProtectType == PROTECT_CUT_BOTH) //Both fuel and ignition are cut
            )
          { BIT_SET(cutState.ignitionChannelsPending, x); } //Set this ignition channel as pending
          else { BIT_SET(cutState.ignitionChannels, x); } //Turn on this ignition channel
            
          
          BIT_SET(cutState.fuelChannels, x); //Turn on this fuel channel
        }
      }
      rollingCutLastRev = current.startRevolutions;
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
    resetEngineProtect(current);
    //No engine protection active, so turn all the channels on
    if(current.startRevolutions >= page4.StgCycles)
    { 
      //Enable the fuel and ignition, assuming staging revolutions are complete 
      cutState.ignitionChannels = 0xff; 
      cutState.fuelChannels = 0xff; 
    } 
  }

  return cutState;
}