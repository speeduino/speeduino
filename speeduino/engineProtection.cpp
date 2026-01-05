#include "globals.h"
#include "engineProtection.h"
#include "maths.h"
#include "units.h"
#include "unit_testing.h"

TESTABLE_STATIC uint8_t oilProtStartTime = 0;
TESTABLE_STATIC table2D_u8_u8_4 oilPressureProtectTable(&configPage10.oilPressureProtRPM, &configPage10.oilPressureProtMins);
TESTABLE_STATIC table2D_u8_u8_6 coolantProtectTable(&configPage9.coolantProtTemp, &configPage9.coolantProtRPM);

/* AFR protection state moved to file scope so unit tests can control/reset it */
TESTABLE_STATIC bool checkAFRLimitActive = false;
TESTABLE_STATIC bool afrProtectCountEnabled = false;
TESTABLE_STATIC unsigned long afrProtectCount = 0;

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
      if(oilProtStartTime == 0U) { oilProtStartTime = div100(currMillis); }
      /* Check if countdown has reached its target, if so then instruct to cut */
      if( (uint8_t(div100(currMillis)) >= (uint16_t(oilProtStartTime + page10.oilPressureProtTime)) ) || (current.engineProtectOil) )
      {
        engineProtectOil = true;
      }
    }
    else 
    { 
      oilProtStartTime = 0; //Reset the timer
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

static inline uint8_t getAfrO2Limit(const statuses &current, const config9 &page9)
{
  if (page9.afrProtectEnabled==AFR_PROTECT_FIXED) {
    return page9.afrProtectDeviation;
  } if (page9.afrProtectEnabled==AFR_PROTECT_TABLE) {
    return current.afrTarget + page9.afrProtectDeviation;
  } else {
    return 0U;
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
      if(!afrProtectCountEnabled) 
      {
        afrProtectCountEnabled = true;
        afrProtectCount = currMillis;
      }

      /* Check if countdown has reached its target, if so then instruct to cut */
      if(currMillis >= (afrProtectCount + (page9.afrProtectCutTime * X100_MULTIPLIER))) 
      {
        checkAFRLimitActive = true;
      }
    } 
    else 
    {
      /* Conditions have presumably changed - deactivate and reset counter */
      if(afrProtectCountEnabled) 
      {
        afrProtectCountEnabled = false;
        afrProtectCount = 0;
      }
    }

    /* Check if condition for reactivation is fulfilled */
    if(checkAFRLimitActive && (current.TPS <= page9.afrProtectReactivationTPS)) 
    {
      checkAFRLimitActive = false;
      afrProtectCountEnabled = false;
    }
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

bool checkEngineProtect(statuses &current, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10)
{
  return checkEngineProtect(current, page4, page6, page9, page10, millis());
}

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
