
#include "globals.h"
#include "engineProtection.h"

uint8_t checkEngineProtect()
{
  uint8_t protectActive = 0;
  if(checkRevLimit() || checkBoostLimit() || checkOilPressureLimit() || checkAFRLimit())
  {
    if(currentStatus.RPM > (configPage4.engineProtectMaxRPM*100U)) { protectActive = 1; }
  }

  return protectActive;
}

uint8_t checkRevLimit()
{
  //Hardcut RPM limit
  uint8_t revLimiterActive = 0;
  BIT_CLEAR(currentStatus.engineProtectStatus, ENGINE_PROTECT_BIT_RPM);

  if (currentStatus.RPM > ((unsigned int)(configPage4.HardRevLim) * 100) )
  { 
    BIT_SET(currentStatus.spark, BIT_SPARK_HRDLIM); //Legacy and likely to be removed at some point
    BIT_SET(currentStatus.engineProtectStatus, ENGINE_PROTECT_BIT_RPM);
    revLimiterActive = 1; 
  } 
  else { BIT_CLEAR(currentStatus.spark, BIT_SPARK_HRDLIM); }

  return revLimiterActive;
}

uint8_t checkBoostLimit()
{
  uint8_t boostLimitActive = 0;
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
  else
  {
    BIT_CLEAR(currentStatus.spark, BIT_SPARK_BOOSTCUT);
    BIT_CLEAR(currentStatus.status1, BIT_STATUS1_BOOSTCUT);
  }

  return boostLimitActive;
}

uint8_t checkOilPressureLimit()
{
  uint8_t oilProtectActive = 0;
  BIT_CLEAR(currentStatus.engineProtectStatus, ENGINE_PROTECT_BIT_OIL); //Will be set true below if required

  if( (configPage10.oilPressureProtEnbl == true) && (configPage10.oilPressureEnable == true) )
  {
    uint8_t oilLimit = table2D_getValue(&oilPressureProtectTable, currentStatus.RPMdiv100);
    if(currentStatus.oilPressure < oilLimit)
    {
      BIT_SET(currentStatus.engineProtectStatus, ENGINE_PROTECT_BIT_OIL);
      oilProtectActive = 1;
    }
  }

  return oilProtectActive;
}

uint8_t checkAFRLimit()
{
  uint8_t checkAFRLimitActive = 0;

  return checkAFRLimitActive;
}

