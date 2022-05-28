
#include "globals.h"
#include "engineProtection.h"

byte checkEngineProtect()
{
  byte protectActive = 0;
  if(checkRevLimit() || checkBoostLimit() || checkOilPressureLimit() || checkAFRLimit() )
  {
    if( currentStatus.RPMdiv100 > configPage4.engineProtectMaxRPM ) { protectActive = 1; }
  }

  return protectActive;
}

byte checkRevLimit()
{
  //Hardcut RPM limit
  byte revLimiterActive = 0;
  BIT_CLEAR(currentStatus.engineProtectStatus, ENGINE_PROTECT_BIT_RPM);
  BIT_CLEAR(currentStatus.spark, BIT_SPARK_HRDLIM);
  BIT_CLEAR(currentStatus.engineProtectStatus, ENGINE_PROTECT_BIT_COOLANT);

  if (configPage6.engineProtectType != PROTECT_CUT_OFF) 
  {
    if(configPage9.hardRevMode == HARD_REV_FIXED)
    {
      if ( (currentStatus.RPMdiv100 >= configPage4.HardRevLim) || ((softLimitTime > configPage4.SoftLimMax) && (currentStatus.RPMdiv100 >= configPage4.SoftRevLim)) )
      { 
        BIT_SET(currentStatus.spark, BIT_SPARK_HRDLIM); //Legacy and likely to be removed at some point
        BIT_SET(currentStatus.engineProtectStatus, ENGINE_PROTECT_BIT_RPM);
        revLimiterActive = 1; 
      } 
      else { BIT_CLEAR(currentStatus.spark, BIT_SPARK_HRDLIM); }
    }
    else if(configPage9.hardRevMode == HARD_REV_COOLANT )
    {
      int8_t coolantLimit = (int16_t)(table2D_getValue(&coolantProtectTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET));
      if(currentStatus.RPMdiv100 > coolantLimit)
      {
        BIT_SET(currentStatus.engineProtectStatus, ENGINE_PROTECT_BIT_COOLANT);
        BIT_SET(currentStatus.spark, BIT_SPARK_HRDLIM); //Legacy and likely to be removed at some point
        BIT_SET(currentStatus.engineProtectStatus, ENGINE_PROTECT_BIT_RPM);
        revLimiterActive = 1; 
      } 
    }
  }

  return revLimiterActive;
}

byte checkBoostLimit()
{
  byte boostLimitActive = 0;
  BIT_CLEAR(currentStatus.engineProtectStatus, ENGINE_PROTECT_BIT_MAP);
  BIT_CLEAR(currentStatus.spark, BIT_SPARK_BOOSTCUT);
  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_BOOSTCUT);

  if (configPage6.engineProtectType != PROTECT_CUT_OFF) {
    //Boost cutoff is very similar to launchControl, but with a check against MAP rather than a switch
    if( (configPage6.boostCutEnabled > 0) && (currentStatus.MAP > (configPage6.boostLimit * 2)) ) //The boost limit is divided by 2 to allow a limit up to 511kPa
    {
      boostLimitActive = 1;
      BIT_SET(currentStatus.engineProtectStatus, ENGINE_PROTECT_BIT_MAP);
      /*
      switch(configPage6.boostCutType)
      {
        case 1:
          BIT_SET(currentStatus.spark, BIT_SPARK_BOOSTCUT);
          BIT_CLEAR(currentStatus.status1, BIT_STATUS1_BOOSTCUT);
          BIT_SET(currentStatus.engineProtectStatus, ENGINE_PROTECT_BIT_MAP);
          break;
        case 2:
          BIT_SET(currentStatus.status1, BIT_STATUS1_BOOSTCUT);
          BIT_CLEAR(currentStatus.spark, BIT_SPARK_BOOSTCUT);
          break;
        case 3:
          BIT_SET(currentStatus.spark, BIT_SPARK_BOOSTCUT);
          BIT_SET(currentStatus.status1, BIT_STATUS1_BOOSTCUT);
          break;
        default:
          //Shouldn't ever happen, but just in case, disable all cuts
          BIT_CLEAR(currentStatus.status1, BIT_STATUS1_BOOSTCUT);
          BIT_CLEAR(currentStatus.spark, BIT_SPARK_BOOSTCUT);
      }
      */
    }
  }

  return boostLimitActive;
}

byte checkOilPressureLimit()
{
  byte oilProtectActive = 0;
  BIT_CLEAR(currentStatus.engineProtectStatus, ENGINE_PROTECT_BIT_OIL); //Will be set true below if required

  if (configPage6.engineProtectType != PROTECT_CUT_OFF) {
    if( (configPage10.oilPressureProtEnbl == true) && (configPage10.oilPressureEnable == true) )
    {
      byte oilLimit = table2D_getValue(&oilPressureProtectTable, currentStatus.RPMdiv100);
      if(currentStatus.oilPressure < oilLimit)
      {
        BIT_SET(currentStatus.engineProtectStatus, ENGINE_PROTECT_BIT_OIL);
        oilProtectActive = 1;
      }
    }
  }

  return oilProtectActive;
}

byte checkAFRLimit()
{
  byte checkAFRLimitActive = 0;

  /*
    To use this function, a wideband sensor is required.

    First of all, check wether wideband sensor is used.
    After confirmation, the following conditions has to be met:
    - MAP above x kPa (Note: MAP is divided by 10 to be stored in byte, increments of 10 kPa)
    - RPM above x
    - TPS above x %
    - AFR threshold (AFR target + defined maximum deviation)
    - Time before cut

    When all of the conditions above are true, the engine protection
    function is activated using selected protection cut method.

    For reactivation, the following condition has to be met:
    - TPS below x %
  */
  
  /* Check if engine protection is enabled */
  if(configPage6.engineProtectType != PROTECT_CUT_OFF) {
    /* Check if wideband sensor is used */
    if(configPage6.egoType == 2) {
      /* Conditions */
      byte mapCondition = (currentStatus.MAP >= (configPage9.afrProtectMinMAP * 10)) ? 1 : 0;
      byte rpmCondition = (currentStatus.RPM >= (configPage9.afrProtectMinRPM)) ? 1 : 0;
      byte tpsCondition = (currentStatus.TPS >= configPage9.afrProtectMinTPS) ? 1 : 0;
      byte afrCondition = (currentStatus.O2 >= (currentStatus.afrTarget + configPage9.afrProtectDeviation)) ? 1 : 0;

      /* Check if conditions above are fulfilled */
      if(mapCondition && rpmCondition && tpsCondition && afrCondition) {
        /* All conditions fulfilled - start countdown before protection cut */
        if(!afrProtectCountEnabled) {
          afrProtectCountEnabled = true;
          afrProtectCount = millis();
        }

        /* Check if countdown has reached its target, if so then instruct to cut */
        if(millis() >= (afrProtectCount + configPage9.afrProtectCutTime * 10)) {
          checkAFRLimitActive = 1;
          BIT_SET(currentStatus.engineProtectStatus, ENGINE_PROTECT_BIT_AFR);
        }
      }

      /* Check if condition for reactivation is fulfilled */
      if(checkAFRLimitActive && (currentStatus.TPS <= configPage9.afrProtectReactivationTPS)) {
        checkAFRLimitActive = 0;
        BIT_CLEAR(currentStatus.engineProtectStatus, ENGINE_PROTECT_BIT_AFR);
      }
    }
  }

  return checkAFRLimitActive;
}

