#include "pw_calcs.h"
#include "unit_testing.h"
#include "maths.h"

/** @name Staging
 * These values are a percentage of the total (Combined) req_fuel value that would be required for each injector channel to deliver that much fuel.   
 * 
 * Eg:
 *  - Pri injectors are 250cc
 *  - Sec injectors are 500cc
 *  - Total injector capacity = 750cc
 * 
 *  - stagedPriReqFuelPct = 300% (The primary injectors would have to run 3x the overall PW in order to be the equivalent of the full 750cc capacity
 *  - stagedSecReqFuelPct = 150% (The secondary injectors would have to run 1.5x the overall PW in order to be the equivalent of the full 750cc capacity
*/
static uint16_t stagedPriReqFuelPct = 0;
static uint16_t stagedSecReqFuelPct = 0; 

void initialisePWCalcs(void)
{
  if(configPage10.stagingEnabled == true)
  {
    uint32_t totalInjector = configPage10.stagedInjSizePri + configPage10.stagedInjSizeSec;
    /*
        These values are a percentage of the req_fuel value that would be required for each injector channel to deliver that much fuel.
        Eg:
        Pri injectors are 250cc
        Sec injectors are 500cc
        Total injector capacity = 750cc

        stagedPriReqFuelPct = 300% (The primary injectors would have to run 3x the overall PW in order to be the equivalent of the full 750cc capacity
        stagedSecReqFuelPct = 150% (The secondary injectors would have to run 1.5x the overall PW in order to be the equivalent of the full 750cc capacity
    */
    stagedPriReqFuelPct = udiv_32_16(100UL * totalInjector, configPage10.stagedInjSizePri);
    stagedSecReqFuelPct = udiv_32_16(100UL * totalInjector ,configPage10.stagedInjSizeSec);
  }
  else
  {
    stagedPriReqFuelPct = 0;
    stagedSecReqFuelPct = 0;
  }
}

TESTABLE_INLINE_STATIC uint16_t calculateRequiredFuel(const config2 &page2) {
  uint16_t reqFuel = page2.reqFuel * 100U; //Convert to uS and an int. This is the only variable to be used in calculations
  if ((page2.strokes == FOUR_STROKE) && ((page2.injLayout!= INJ_SEQUENTIAL) || (page2.nCylinders > INJ_CHANNELS)))
  {
    //Default is 1 squirt per revolution, so we halve the given req-fuel figure (Which would be over 2 revolutions)
    //The req_fuel calculation above gives the total required fuel (At VE 100%) in the full cycle.
    //If we're doing more than 1 squirt per cycle then we need to split the amount accordingly.
    //(Note that in a non-sequential 4-stroke setup you cannot have less than 2 squirts as you cannot determine the
    //stroke to make the single squirt on)
    reqFuel = reqFuel / 2U; 
  }

  return reqFuel;
}

static inline uint16_t calcNitrousStagePulseWidth(uint8_t minRPM, uint8_t maxRPM, uint8_t adderMin, uint8_t adderMax, const statuses &current)
{
  return (uint16_t)map(current.RPMdiv100, minRPM, maxRPM, adderMin, adderMax)*UINT16_C(100);
}

//Manual adder for nitrous. These are not in correctionsFuel() because they are direct adders to the ms value, not % based
TESTABLE_INLINE_STATIC uint16_t pwApplyNitrous(uint16_t pw, const config10 &page10, const statuses &current)
{
  if (current.nitrous_status!=NITROUS_OFF && pw!=0U)
  {
    if( (current.nitrous_status == NITROUS_STAGE1) || (current.nitrous_status == NITROUS_BOTH) )
    {
      pw = pw + calcNitrousStagePulseWidth(page10.n2o_stage1_minRPM, page10.n2o_stage1_maxRPM, page10.n2o_stage1_adderMin, page10.n2o_stage1_adderMax, current);
    }
    if( (current.nitrous_status == NITROUS_STAGE2) || (current.nitrous_status == NITROUS_BOTH) )
    {
      pw = pw + calcNitrousStagePulseWidth(page10.n2o_stage2_minRPM, page10.n2o_stage2_maxRPM, page10.n2o_stage2_adderMin, page10.n2o_stage2_adderMax, current);
    }
  }

  return pw;
}

static inline uint32_t pwApplyMapMode(uint32_t intermediate, uint16_t MAP, const statuses &current) {
  if ( configPage2.multiplyMAP == MULTIPLY_MAP_MODE_100) { 
    uint16_t mutiplier = div100((uint16_t)(MAP << 7U));
    return rshift<7U>(intermediate * (uint32_t)mutiplier); 
  }
  if( configPage2.multiplyMAP == MULTIPLY_MAP_MODE_BARO) { 
     uint16_t mutiplier = (MAP << 7U) / current.baro; 
    return rshift<7U>(intermediate * (uint32_t)mutiplier); 
  }
  return intermediate;
}

static inline uint32_t pwApplyAFRMultiplier(uint32_t intermediate, const statuses &current) {
  if ( (configPage2.includeAFR == true) && (configPage6.egoType == EGO_TYPE_WIDE) && (current.runSecs > configPage6.ego_sdelay) ) {
    uint16_t mutiplier = ((uint16_t)current.O2 << 7U) / current.afrTarget;  //Include AFR (vs target) if enabled
    return rshift<7U>(intermediate * (uint32_t)mutiplier); 
  }
  if ( (configPage2.incorporateAFR == true) && (configPage2.includeAFR == false) ) {
    uint16_t mutiplier = ((uint16_t)configPage2.stoich << 7U) / current.afrTarget;  //Incorporate stoich vs target AFR, if enabled.
    return rshift<7U>(intermediate * (uint32_t)mutiplier); 
  }
  return intermediate;
}

static inline uint32_t pwApplyCorrections(uint32_t intermediate, uint16_t corrections) {
  return percentageApprox(corrections, intermediate); 
}

static inline uint32_t pwComputeInitial(uint16_t REQ_FUEL, uint8_t VE) {
  return percentageApprox(VE, REQ_FUEL); 
}

static inline uint32_t pwIncludeOpenTime(uint32_t intermediate, uint16_t injOpen) {
  // If intermediate is not 0, we need to add the opening time (0 typically indicates that one of the full fuel cuts is active)
  if (intermediate != 0U) {
    return intermediate + injOpen; //Add the injector opening time
  }
  return intermediate;
}

static inline uint32_t pwIncludeAe(uint32_t intermediate, uint16_t REQ_FUEL, const statuses &current) {
  // If intermediate is not 0, we need to add Acceleration Enrichment pct increase if the engine
  // is accelerating (0 typically indicates that one of the full fuel cuts is active)
  if ((intermediate != 0U) && BIT_CHECK(current.engine, BIT_ENGINE_ACC) && (configPage2.aeApplyMode == AE_MODE_ADDER)) {
    return intermediate + percentage(current.AEamount - 100U, REQ_FUEL);
  }
  return intermediate;
}

static inline uint16_t computePrimaryPulseWidth(uint16_t REQ_FUEL, uint8_t VE, uint16_t MAP, uint16_t corrections, uint16_t injOpenTime, const config10 &page10, const statuses &current) {
  //Standard float version of the calculation
  //return (REQ_FUEL * (float)(VE/100.0) * (float)(MAP/100.0) * (float)(TPS/100.0) * (float)(corrections/100.0) + injOpenTime);
  //Note: The MAP and TPS portions are currently disabled, we use VE and corrections only
  uint32_t intermediate = 
    pwIncludeAe(
      pwIncludeOpenTime(
        pwApplyCorrections(
          pwApplyAFRMultiplier(
            pwApplyMapMode(
              pwComputeInitial(REQ_FUEL, VE), 
              MAP, current),
            current), 
        corrections), 
      injOpenTime), 
      REQ_FUEL, current);

  // Make sure this won't overflow when we convert to uInt. This means the maximum pulsewidth possible is 65.535mS
  return pwApplyNitrous((uint16_t)(intermediate>UINT16_MAX ? UINT16_MAX : intermediate), page10, current);
}  

TESTABLE_INLINE_STATIC uint16_t calculatePWLimit(const config2 &page2, const statuses &current)
{
  uint32_t tempLimit = percentageApprox(page2.dutyLim, current.revolutionTime); //The pulsewidth limit is determined to be the duty cycle limit (Eg 85%) by the total time it takes to perform 1 revolution
  //Handle multiple squirts per rev
  if (page2.strokes == FOUR_STROKE) { tempLimit = tempLimit * 2U; }

  //Optimise for power of two divisions where possible
  switch(current.nSquirts)
  {
    case 1:
      //No action needed
      break;
    case 2:
      tempLimit = tempLimit / 2U;
      break;
    case 4:
      tempLimit = tempLimit / 4U;
      break;
    case 8:
      tempLimit = tempLimit / 8U;
      break;
    default:
      //Non-PoT squirts value. Perform (slow) uint32_t division
      tempLimit = tempLimit / current.nSquirts;
      break;
  }
  // Make sure this won't overflow when we convert to uInt. This means the maximum pulsewidth possible is 65.535mS
  return (uint16_t)min(tempLimit, (uint32_t)UINT16_MAX);
}

static inline pulseWidths applyStagingModeTable(uint16_t primaryPW, uint16_t injOpenTime) {
  //Subtract the opening time from PW1 as it needs to be multiplied out again by the pri/sec req_fuel values below. It is added on again after that calculation. 
  uint32_t pwPrimaryStaged = percentage(stagedPriReqFuelPct, primaryPW - injOpenTime);

  uint8_t stagingSplit = get3DTableValue(&stagingTable, currentStatus.fuelLoad, currentStatus.RPM);
  if(stagingSplit > 0U) 
  { 
    uint32_t pwSecondaryStaged = percentage(stagedSecReqFuelPct, primaryPW - injOpenTime); //This is ONLY needed in in table mode. Auto mode only calculates the difference.
    uint32_t primary = percentage(100U - stagingSplit, pwPrimaryStaged) + injOpenTime;
    uint32_t secondary = percentage(stagingSplit, pwSecondaryStaged) + injOpenTime;
    return { 
      (uint16_t)min(primary, (uint32_t)UINT16_MAX),
      (uint16_t)min(secondary, (uint32_t)UINT16_MAX),
    };
  }

  return { (uint16_t)min(pwPrimaryStaged + injOpenTime, (uint32_t)UINT16_MAX), 0U};
}

static inline pulseWidths applyStagingModeAuto(uint16_t primaryPW, uint16_t pwLimit, uint16_t injOpenTime) {
  //Subtract the opening time from PW1 as it needs to be multiplied out again by the pri/sec req_fuel values below. It is added on again after that calculation. 
  uint32_t pwPrimaryStaged = percentage(stagedPriReqFuelPct, primaryPW - injOpenTime);

  //If automatic mode, the primary injectors are used all the way up to their limit (Configured by the pulsewidth limit setting)
  //If they exceed their limit, the extra duty is passed to the secondaries
  if(pwPrimaryStaged > pwLimit)
  {
    uint32_t extraPW = pwPrimaryStaged - pwLimit + injOpenTime; //The open time must be added here AND below because pwPrimaryStaged does not include an open time. The addition of it here takes into account the fact that pwLlimit does not contain an allowance for an open time. 
    uint32_t secondary = udiv_32_16(extraPW * stagedSecReqFuelPct, stagedPriReqFuelPct) + injOpenTime;
    return { 
      pwLimit,
      (uint16_t)min(secondary, (uint32_t)UINT16_MAX),
    };
  }

  return { (uint16_t)min(pwPrimaryStaged + injOpenTime, (uint32_t)UINT16_MAX), 0U};
}

static inline pulseWidths applyStagingToPw(uint16_t primaryPW, uint16_t pwLimit, uint16_t injOpenTime) {
  pulseWidths widths = { primaryPW, 0U };

  if (primaryPW!=0U) {
    //To run staged injection, the number of cylinders must be less than or equal to the injector channels (ie Assuming you're running paired injection, you need at least as many injector channels as you have cylinders, half for the primaries and half for the secondaries)
    if ( (configPage10.stagingEnabled == true) && (configPage2.nCylinders <= INJ_CHANNELS || configPage2.injType == INJ_TYPE_TBODY)) //Final check is to ensure that DFCO isn't active, which would cause an overflow below (See #267)
    {
      //Scale the 'full' pulsewidth by each of the injector capacities
      if(configPage10.stagingMode == STAGING_MODE_TABLE) {
        widths = applyStagingModeTable(primaryPW, injOpenTime);
      } else if(configPage10.stagingMode == STAGING_MODE_AUTO) {
        widths = applyStagingModeAuto(primaryPW, pwLimit, injOpenTime);
      } else {
        // Unknown staging mode - accept default & keep MISRA checker happy.
      }
    }
    //Apply the pwLimit if staging is disabled and engine is not cranking
    else if( (!BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK))) { 
      widths = { min(primaryPW, pwLimit), 0U };
    } else {
      // No staging needed - accept default & keep MISRA checker happy.
    }
  }

  BIT_WRITE(currentStatus.status4, BIT_STATUS4_STAGING_ACTIVE, widths.secondary!=0U);

  return widths;
}

TESTABLE_INLINE_STATIC uint16_t calculateOpenTime(const config2 &page2, const statuses &current) {
  // Convert injector open time from tune to microseconds & apply voltage correction if required
  return page2.injOpen * (page2.battVCorMode == BATTV_COR_MODE_OPENTIME ? current.batCorrection : 100U); 
}

TESTABLE_INLINE_STATIC pulseWidths computePulseWidths(uint16_t REQ_FUEL, uint8_t VE, uint16_t MAP, uint16_t corrections) {
  uint16_t injOpenTime = calculateOpenTime(configPage2, currentStatus);
  return applyStagingToPw(computePrimaryPulseWidth(REQ_FUEL, VE, MAP, corrections, injOpenTime, configPage10, currentStatus), calculatePWLimit(configPage2, currentStatus), injOpenTime);
}

pulseWidths computePulseWidths(const config2 &page2, statuses &current) {
  return computePulseWidths(calculateRequiredFuel(page2), current.VE, current.MAP, current.corrections);
}
