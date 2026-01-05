#include "globals.h"
#include "engineProtection.h"
#include "maths.h"
#include "units.h"
#include "unit_testing.h"

TESTABLE_STATIC uint8_t oilProtStartTime = 0;
TESTABLE_STATIC table2D_u8_u8_4 oilPressureProtectTable(&configPage10.oilPressureProtRPM, &configPage10.oilPressureProtMins);
TESTABLE_STATIC table2D_u8_u8_6 coolantProtectTable(&configPage9.coolantProtTemp, &configPage9.coolantProtRPM);

TESTABLE_INLINE_STATIC bool checkOilPressureLimit(statuses &current, const config6 &page6, const config10 &page10, uint32_t currMillis)
{
  bool alreadyActive = current.engineProtectOil;
  
  current.engineProtectOil = false; //Will be set true below if required

  if (page6.engineProtectType != PROTECT_CUT_OFF) 
  {
    if( (page10.oilPressureProtEnbl == true) && (page10.oilPressureEnable == true) )
    {
      uint8_t oilLimit = table2D_getValue(&oilPressureProtectTable, current.RPMdiv100);
      if(current.oilPressure < oilLimit)
      {
        //Check if this is the first time we've been below the limit
        if(oilProtStartTime == 0U) { oilProtStartTime = div100(currMillis); }
        /* Check if countdown has reached its target, if so then instruct to cut */
        if( (uint8_t(div100(currMillis)) >= (uint16_t(oilProtStartTime + page10.oilPressureProtTime)) ) || (alreadyActive > 0) )
        {
          current.engineProtectOil = true;
        }
      }
      else 
      { 
        oilProtStartTime = 0; //Reset the timer
      }
    }
  }

  return current.engineProtectOil;
}

TESTABLE_INLINE_STATIC bool checkBoostLimit(statuses &current, const config6 &page6)
{
  current.engineProtectBoostCut = (page6.engineProtectType != PROTECT_CUT_OFF)
                                   &&  ((page6.boostCutEnabled > 0) && (current.MAP > ((long)page6.boostLimit * 2L)) );

  return current.engineProtectBoostCut;
}

static byte checkAFRLimit(void)
{
  static bool checkAFRLimitActive = false;
  static bool afrProtectCountEnabled = false;
  static unsigned long afrProtectCount = 0;
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
  if(configPage6.engineProtectType != PROTECT_CUT_OFF && configPage9.afrProtectEnabled && configPage6.egoType == EGO_TYPE_WIDE) {
    /* Conditions */
    bool mapCondition = (currentStatus.MAP >= (configPage9.afrProtectMinMAP * X2_MULTIPLIER)) ? true : false;
    bool rpmCondition = (currentStatus.RPMdiv100 >= configPage9.afrProtectMinRPM) ? true : false;
    bool tpsCondition = (currentStatus.TPS >= configPage9.afrProtectMinTPS) ? true : false;

    /*
      Depending on selected mode, this could either be fixed AFR value or a
      value set to be the maximum deviation from AFR target table.

      1 = fixed value mode, 2 = target table mode
    */
    bool afrCondition;
    switch(configPage9.afrProtectEnabled)
    {
      case 1: afrCondition = (currentStatus.O2 >= configPage9.afrProtectDeviation) ? true : false; break; /* Fixed value */
      case 2: afrCondition = (currentStatus.O2 >= (currentStatus.afrTarget + configPage9.afrProtectDeviation)) ? true : false; break; /* Deviation from target table */
      default: afrCondition = false; /* Unknown mode. Shouldn't even get here */
    }

    /* Check if conditions above are fulfilled */
    if(mapCondition && rpmCondition && tpsCondition && afrCondition) 
    {
      /* All conditions fulfilled - start counter for 'protection delay' */
      if(!afrProtectCountEnabled) 
      {
        afrProtectCountEnabled = true;
        afrProtectCount = millis();
      }

      /* Check if countdown has reached its target, if so then instruct to cut */
      if(millis() >= (afrProtectCount + (configPage9.afrProtectCutTime * X100_MULTIPLIER))) 
      {
        checkAFRLimitActive = true;
        currentStatus.engineProtectAfr = true;
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
    if(checkAFRLimitActive && (currentStatus.TPS <= configPage9.afrProtectReactivationTPS)) 
    {
      checkAFRLimitActive = false;
      afrProtectCountEnabled = false;
      currentStatus.engineProtectAfr = false;
    }
  }

  return checkAFRLimitActive;
}


byte checkEngineProtect(void)
{
  byte protectActive = 0;
  if(checkBoostLimit(currentStatus, configPage6) || checkOilPressureLimit(currentStatus, configPage6, configPage10, millis()) || checkAFRLimit() )
  {
    if( currentStatus.RPMdiv100 > configPage4.engineProtectMaxRPM ) { protectActive = 1; }
  }

  return protectActive;
}

byte checkRevLimit(void)
{
  //Hardcut RPM limit
  byte currentLimitRPM = UINT8_MAX; //Default to no limit (In case PROTECT_CUT_OFF is selected)
  currentStatus.engineProtectRpm = false;
  currentStatus.engineProtectClt = false;

  if (configPage6.engineProtectType != PROTECT_CUT_OFF) 
  {
    if(configPage9.hardRevMode == HARD_REV_FIXED)
    {
      currentLimitRPM = configPage4.HardRevLim;
      if ( (currentStatus.RPMdiv100 >= configPage4.HardRevLim) || ((softLimitTime > configPage4.SoftLimMax) && (currentStatus.RPMdiv100 >= configPage4.SoftRevLim)) )
      { 
        currentStatus.engineProtectRpm = true;
      } 
      else 
      { 
        currentStatus.engineProtectRpm = false;
      }
    }
    else if(configPage9.hardRevMode == HARD_REV_COOLANT )
    {
      currentLimitRPM = (int16_t)(table2D_getValue(&coolantProtectTable, temperatureAddOffset(currentStatus.coolant)));
      if(currentStatus.RPMdiv100 > currentLimitRPM)
      {
        currentStatus.engineProtectClt = true;
        currentStatus.engineProtectRpm = true;
      } 
    }
  }

  return currentLimitRPM;
}
