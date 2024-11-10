#include "pw_calcs.h"
#include "unit_testing.h"
#include "maths.h"

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

// Manual adder for nitrous. These are not in correctionsFuel() because they are direct adders to the ms value, not % based
TESTABLE_INLINE_STATIC uint16_t pwApplyNitrous(uint16_t pw, const config10 &page10, const statuses &current)
{
  if (current.nitrous_status!=NITROUS_OFF)
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

static inline uint32_t pwApplyMapMode(uint32_t intermediate, const config2 &page2, const statuses &current) {
  if ( page2.multiplyMAP == MULTIPLY_MAP_MODE_100) { 
    uint16_t multiplier = div100((uint16_t)((uint16_t)current.MAP << 7U));
    return rshift<7U>(intermediate * (uint32_t)multiplier); 
  }
  if( page2.multiplyMAP == MULTIPLY_MAP_MODE_BARO) { 
     uint16_t multiplier = ((uint16_t)current.MAP << 7U) / current.baro; 
    return rshift<7U>(intermediate * (uint32_t)multiplier); 
  }
  return intermediate;
}

static inline uint32_t pwApplyAFRMultiplier(uint32_t intermediate, const config2 &page2, const config6 &page6, const statuses &current) {
  if (page2.includeAFR == true) {
    if ((page6.egoType == EGO_TYPE_WIDE) && (current.runSecs > page6.ego_sdelay) ) {
      uint16_t multiplier = ((uint16_t)current.O2 << 7U) / current.afrTarget;  //Include AFR (vs target) if enabled
      return rshift<7U>(intermediate * (uint32_t)multiplier); 
    }
  } else {
    if ( (page2.incorporateAFR == true) ) {
      uint16_t multiplier = ((uint16_t)page2.stoich << 7U) / current.afrTarget;  //Incorporate stoich vs target AFR, if enabled.
      return rshift<7U>(intermediate * (uint32_t)multiplier); 
    }
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
    return intermediate + injOpen; //Add the injector opening time
}

static inline uint32_t pwIncludeAe(uint32_t intermediate, uint16_t REQ_FUEL, const config2 &page2, const statuses &current) {
  // We need to add Acceleration Enrichment pct increase if the engine is accelerating
  if (BIT_CHECK(current.engine, BIT_ENGINE_ACC) && (page2.aeApplyMode == AE_MODE_ADDER) && (current.AEamount>100U)) {
    return intermediate + percentage(current.AEamount - 100U, REQ_FUEL);
  }
  return intermediate;
}

TESTABLE_INLINE_STATIC uint16_t computePrimaryPulseWidth(uint16_t REQ_FUEL, uint16_t injOpenTime, const config2 &page2, const config6 &page6, const config10 &page10, const statuses &current) {
  //Standard float version of the calculation
  //return (REQ_FUEL * (float)(VE/100.0) * (float)(MAP/100.0) * (float)(TPS/100.0) * (float)(corrections/100.0) + injOpenTime);
  //Note: The MAP and TPS portions are currently disabled, we use VE and corrections only
  uint32_t intermediate = 
    pwIncludeAe(
      pwIncludeOpenTime(
        pwApplyCorrections(
          pwApplyAFRMultiplier(
            pwApplyMapMode(
              pwComputeInitial(REQ_FUEL, current.VE), 
              page2, current),
            page2, page6, current), 
        current.corrections), 
      injOpenTime), 
      REQ_FUEL, page2, current);

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

static inline uint32_t calcTotalStagePw(uint16_t primaryPW, uint16_t injOpenTime, const config10 &page10) {
  // Subtract the opening time from PW1 as it needs to be multiplied out again by the pri/sec req_fuel values below. 
  // It is added on again after that calculation. 
  primaryPW = primaryPW - injOpenTime;
  uint32_t totalInjector = page10.stagedInjSizePri + page10.stagedInjSizeSec;
  return ((uint32_t)primaryPW)*totalInjector;
}

static inline uint32_t calcStagePrimaryPw(uint16_t primaryPW, uint16_t injOpenTime, const config10 &page10) {
  return fast_div(calcTotalStagePw(primaryPW, injOpenTime, page10), page10.stagedInjSizePri);
}
static inline uint32_t calcStageSecondaryPw(uint16_t primaryPW, uint16_t injOpenTime, const config10 &page10) {
  return fast_div(calcTotalStagePw(primaryPW, injOpenTime, page10), page10.stagedInjSizeSec);
}

static inline pulseWidths applyStagingModeTable(uint16_t primaryPW, uint16_t injOpenTime, const config10 &page10, const statuses &current) {
  uint32_t pwPrimaryStaged = calcStagePrimaryPw(primaryPW, injOpenTime, page10);

  uint8_t stagingSplit = get3DTableValue(&stagingTable, current.fuelLoad, current.RPM);
  if(stagingSplit > 0U) 
  { 
    uint32_t pwSecondaryStaged = calcStageSecondaryPw(primaryPW, injOpenTime, page10);
    uint32_t primary = percentage(100U - stagingSplit, pwPrimaryStaged) + injOpenTime;
    uint32_t secondary = percentage(stagingSplit, pwSecondaryStaged) + injOpenTime;
    return { 
      (uint16_t)min(primary, (uint32_t)UINT16_MAX),
      (uint16_t)min(secondary, (uint32_t)UINT16_MAX),
    };
  }

  return { (uint16_t)min(pwPrimaryStaged + injOpenTime, (uint32_t)UINT16_MAX), 0U};
}

static inline pulseWidths applyStagingModeAuto(uint16_t primaryPW, uint16_t pwLimit, uint16_t injOpenTime, const config10 &page10) {
  uint32_t pwPrimaryStaged = calcStagePrimaryPw(primaryPW, injOpenTime, page10);

  //If automatic mode, the primary injectors are used all the way up to their limit (Configured by the pulsewidth limit setting)
  //If they exceed their limit, the extra duty is passed to the secondaries
  if(pwPrimaryStaged > pwLimit)
  {
    uint32_t extraPW = pwPrimaryStaged - pwLimit + injOpenTime; //The open time must be added here AND below because pwPrimaryStaged does not include an open time. The addition of it here takes into account the fact that pwLlimit does not contain an allowance for an open time. 
    uint32_t secondary = fast_div(extraPW * page10.stagedInjSizePri, page10.stagedInjSizeSec) + injOpenTime;
    return { 
      pwLimit,
      (uint16_t)min(secondary, (uint32_t)UINT16_MAX),
    };
  }

  return { (uint16_t)min(pwPrimaryStaged + injOpenTime, (uint32_t)UINT16_MAX), 0U};
}

static inline bool canApplyStaging(const config2 &page2, const config10 &page10) {
    //To run staged injection, the number of cylinders must be less than or equal to the injector channels (ie Assuming you're running paired injection, you need at least as many injector channels as you have cylinders, half for the primaries and half for the secondaries)
 return  (page10.stagingEnabled == true) 
      && (page2.nCylinders <= INJ_CHANNELS || page2.injType == INJ_TYPE_TBODY); //Final check is to ensure that DFCO isn't active, which would cause an overflow below (See #267)  
}

TESTABLE_INLINE_STATIC pulseWidths applyStagingToPw(uint16_t primaryPW, uint16_t pwLimit, uint16_t injOpenTime, const config2 &page2, const config10 &page10, statuses &current) {
  pulseWidths widths = { primaryPW, 0U };

  // Apply the pwLimit if staging is disabled and engine is not cranking
  if (!canApplyStaging(page2, page10)) {
    if(!BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK)) { 
      // This will be the likely path in the majority of cases,
      // so we put this first in the 'if' chain
      widths = { min(primaryPW, pwLimit), 0U };
    } 
  } else { // Staging is go!
    //Scale the 'full' pulsewidth by each of the injector capacities
    if(page10.stagingMode == STAGING_MODE_TABLE) {
      widths = applyStagingModeTable(primaryPW, injOpenTime, page10, current);
    } else if(page10.stagingMode == STAGING_MODE_AUTO) {
      widths = applyStagingModeAuto(primaryPW, pwLimit, injOpenTime, page10);
    } else {
      // Unknown staging mode - accept default & keep MISRA checker happy.
    }
  }

  BIT_WRITE(current.status4, BIT_STATUS4_STAGING_ACTIVE, widths.secondary!=0U);

  return widths;
}

TESTABLE_INLINE_STATIC uint16_t calculateOpenTime(const config2 &page2, const statuses &current) {
  // Convert injector open time from tune to microseconds & apply voltage correction if required
  return page2.injOpen * (page2.battVCorMode == BATTV_COR_MODE_OPENTIME ? current.batCorrection : 100U); 
}

// This function is only required so the unit tests can easily control REQ_FUEL
TESTABLE_INLINE_STATIC pulseWidths computePulseWidths(uint16_t REQ_FUEL, const config2 &page2, const config6 &page6, const config10 &page10, statuses &current) {
  uint16_t injOpenTime = calculateOpenTime(page2, current);
  return applyStagingToPw(computePrimaryPulseWidth(REQ_FUEL, injOpenTime, page2, page6, page10, current), 
                          calculatePWLimit(page2, current), 
                          injOpenTime,
                          page2,
                          page10,
                          current);
}

pulseWidths computePulseWidths(const config2 &page2, const config6 &page6, const config10 &page10, statuses &current) {
  // Zero typically indicates that one of the full fuel cuts is active
  if (current.corrections!=0U) {
    return computePulseWidths(calculateRequiredFuel(page2), page2, page6, page10, current);
  }
  return { 0U, 0U };
}


void setFuelChannelPulseWidths(uint8_t maxFuelChannels, const pulseWidths &pulseWidths, const config2 &page2, statuses &current)
{
  current.PW1 = pulseWidths.primary;
  current.PW2 = current.PW3 = current.PW4 = current.PW5 = current.PW6 = current.PW7 = current.PW8 = 0U;
  
  if (pulseWidths.secondary!=0U && maxFuelChannels>1U) {
    //Allocate the primary and secondary pulse widths based on the fuel configuration
    switch (page2.nCylinders) 
    {
      case 1:
        //Nothing required for 1 cylinder, channels are correct already
        break;
      case 2:
        //Primary pulsewidth on channels 1 and 2, secondary on channels 3 and 4
        current.PW2 = pulseWidths.primary;
        current.PW3 = pulseWidths.secondary;
        current.PW4 = pulseWidths.secondary;
        break;
      case 3:
        current.PW2 = pulseWidths.primary;
        current.PW3 = pulseWidths.primary;

        //6 channels required for 'normal' 3 cylinder staging support
        #if INJ_CHANNELS >= 6
          //Primary pulsewidth on channels 1, 2 and 3, secondary on channels 4, 5 and 6
          current.PW4 = pulseWidths.secondary;
          current.PW5 = pulseWidths.secondary;
          current.PW6 = pulseWidths.secondary;
        #else
          //If there are not enough channels, then primary pulsewidth is on channels 1, 2 and 3, secondary on channel 4
          current.PW4 = pulseWidths.secondary;
        #endif
        break;
      case 4:
        if( (page2.injLayout == INJ_SEQUENTIAL) || (page2.injLayout == INJ_SEMISEQUENTIAL) )
        {
          //Staging with 4 cylinders semi/sequential requires 8 total channels
          #if INJ_CHANNELS >= 8
            current.PW2 = pulseWidths.primary;
            current.PW3 = pulseWidths.primary;
            current.PW4 = pulseWidths.primary;

            current.PW5 = pulseWidths.secondary;
            current.PW6 = pulseWidths.secondary;
            current.PW7 = pulseWidths.secondary;
            current.PW8 = pulseWidths.secondary;
          #else
            //This is an invalid config as there are not enough outputs to support sequential + staging
            //Put the staging output to the non-existent channel 5
            current.PW5 = pulseWidths.secondary;
          #endif
        }
        else
        {
          current.PW2 = pulseWidths.primary;
          current.PW3 = pulseWidths.secondary;
          current.PW4 = pulseWidths.secondary;
        }
        break;
        
      case 5:      
          current.PW2 = pulseWidths.primary;
          current.PW3 = pulseWidths.primary;
          current.PW4 = pulseWidths.primary;

        //No easily supportable 5 cylinder staging option unless there are at least 5 channels
        #if INJ_CHANNELS >= 5
          if (page2.injLayout != INJ_SEQUENTIAL)
          {
            current.PW5 = pulseWidths.secondary;
          }
          #if INJ_CHANNELS >= 6
            current.PW6 = pulseWidths.secondary;
          #endif
        #endif
        break;

      case 6:
        #if INJ_CHANNELS >= 6
          //8 cylinder staging only if not sequential
          if (page2.injLayout != INJ_SEQUENTIAL)
          {
            current.PW4 = pulseWidths.secondary;
            current.PW5 = pulseWidths.secondary;
            current.PW6 = pulseWidths.secondary;
          }
          #if INJ_CHANNELS >= 8
          else
            {
              current.PW4 = pulseWidths.primary;
              current.PW5 = pulseWidths.primary;
              current.PW6 = pulseWidths.primary;
              //If there are 8 channels, then the 6 cylinder sequential option is available by using channels 7 + 8 for staging
              current.PW7 = pulseWidths.secondary;
              current.PW8 = pulseWidths.secondary;
            }
          #endif
        #endif
        current.PW2 = pulseWidths.primary;
        current.PW3 = pulseWidths.primary;
        break;

      case 8:
        #if INJ_CHANNELS >= 8
          //8 cylinder staging only if not sequential
          if (page2.injLayout != INJ_SEQUENTIAL)
          {
            current.PW5 = pulseWidths.secondary;
            current.PW6 = pulseWidths.secondary;
            current.PW7 = pulseWidths.secondary;
            current.PW8 = pulseWidths.secondary;
          }
        #endif
        current.PW2 = pulseWidths.primary;
        current.PW3 = pulseWidths.primary;
        current.PW4 = pulseWidths.primary;
        break;

      default:
        //Assume 4 cylinder non-seq for default
        current.PW2 = pulseWidths.primary;
        current.PW3 = pulseWidths.secondary;
        current.PW4 = pulseWidths.secondary;
        break;
    }
  } else {
    if (pulseWidths.primary!=0U) {
      #define ASSIGN_PRIMARY_OR_ZERO(index) \
        current.PW ## index = ((maxFuelChannels) >= (uint8_t)(index)) ? pulseWidths.primary : 0U;
      ASSIGN_PRIMARY_OR_ZERO(2)
      ASSIGN_PRIMARY_OR_ZERO(3)
      ASSIGN_PRIMARY_OR_ZERO(4)
      ASSIGN_PRIMARY_OR_ZERO(5)
      ASSIGN_PRIMARY_OR_ZERO(6)
      ASSIGN_PRIMARY_OR_ZERO(7)
      ASSIGN_PRIMARY_OR_ZERO(8)
    }
  } 
}