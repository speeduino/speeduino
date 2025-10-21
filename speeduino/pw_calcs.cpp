#include "pw_calcs.h"
#include "maths.h"
#include "unit_testing.h"

TESTABLE_INLINE_STATIC uint16_t calculateRequiredFuel(const config2 &page2, const statuses &current) {
  uint16_t reqFuel = page2.reqFuel * 100U; //Convert to uS and an int. This is the only variable to be used in calculations
  if ((page2.strokes == FOUR_STROKE) && ((page2.injLayout != INJ_SEQUENTIAL) || BIT_CHECK(current.status3, BIT_STATUS3_HALFSYNC)))
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

static inline uint8_t calcNitrousStagePercent(uint8_t minRPMDiv100, uint8_t maxRPMDiv100, const statuses &current) {
  uint16_t minRPM = minRPMDiv100*UINT16_C(100);
  uint16_t maxRPM = maxRPMDiv100*UINT16_C(100);
  uint16_t rpmRange = maxRPM - minRPM;
  uint8_t adderPercent = (uint8_t)udiv_32_16((current.RPM - minRPM) * UINT32_C(100), rpmRange); //The percentage of the way through the RPM range
  return 100U - adderPercent; //Flip the percentage as we go from a higher adder to a lower adder as the RPMs rise
}

static inline uint16_t calcNitrousStagePulseWidth(uint8_t minRPMDiv100, uint8_t maxRPMDiv100, uint8_t adderMinDiv100, uint8_t adderMaxDiv100, const statuses &current)
{
  uint16_t adderMin = adderMinDiv100 * UINT16_C(100);
  uint16_t adderMax = adderMaxDiv100 * UINT16_C(100);
  uint16_t adderRange = adderMin - adderMax;
  return adderMax + percentage(calcNitrousStagePercent(minRPMDiv100, maxRPMDiv100, current), adderRange); //Calculate the above percentage of the calculated ms value.
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

TESTABLE_INLINE_STATIC uint16_t calculatePWLimit(const config2 &page2, const statuses &current)
{
  uint32_t tempLimit = percentageApprox(page2.dutyLim, current.revolutionTime); //The pulsewidth limit is determined to be the duty cycle limit (Eg 85%) by the total time it takes to perform 1 revolution
  //Handle multiple squirts per rev
  if (page2.strokes == FOUR_STROKE) { tempLimit = tempLimit * 2U; }
  //Optimise for power of two divisions where possible
  switch(current.nSquirts)  {
    case 1:
      //No action needed
      break;
    case 2:
      tempLimit = tempLimit >> 1U;
      break;
    case 4:
      tempLimit = tempLimit >> 2U;
      break;
    case 8:
      tempLimit = tempLimit >> 3U;
      break;
    default:
      //Non-PoT squirts value. Perform (slow) uint32_t division
      tempLimit = tempLimit / current.nSquirts;
      break;
  }
  return (uint16_t)min(tempLimit, (uint32_t)UINT16_MAX);
}


static inline uint32_t applyMapMode(uint32_t intermediate, const config2 &page2, const statuses &current) {
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

static inline uint32_t applyAFRMultiplier(uint32_t intermediate, const config2 &page2, const config6 &page6, const statuses &current) {
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

static inline uint32_t applyCorrections(uint32_t intermediate, uint16_t corrections) {
  return percentageApprox(corrections, intermediate); 
}

static inline uint16_t computeInitialPw(uint16_t REQ_FUEL, uint8_t VE) {
  // REQ_FUEL max is 255*100 = 25500
  // VE max is 255
  // Therefore max result = 65025. So just fits in a uint16_t.
  return percentageApprox(VE, REQ_FUEL); 
}

static inline uint32_t includeOpenTime(uint32_t intermediate, uint16_t injOpen) {
    return intermediate + injOpen; //Add the injector opening time
}

static inline uint32_t includeAe(uint32_t intermediate, uint16_t REQ_FUEL, const config2 &page2, const statuses &current) {
  // We need to add Acceleration Enrichment pct increase if the engine is accelerating
  if (BIT_CHECK(current.engine, BIT_ENGINE_ACC) && (page2.aeApplyMode == AE_MODE_ADDER) && (current.AEamount>100U)) {
    return intermediate + percentage(current.AEamount - 100U, REQ_FUEL);
  }
  return intermediate;
}

TESTABLE_INLINE_STATIC uint16_t calcPrimaryPulseWidth(uint16_t injOpenTime, const config2 &page2, const config6 &page6, const config10 &page10, const statuses &current) {
  uint16_t REQ_FUEL = calculateRequiredFuel(page2, current);

  //Standard float version of the calculation
  //return (REQ_FUEL * (float)(VE/100.0) * (float)(MAP/100.0) * (float)(TPS/100.0) * (float)(corrections/100.0) + injOpenTime);
  //Note: The MAP and TPS portions are currently disabled, we use VE and corrections only
  uint32_t intermediate = 
    includeAe(
      includeOpenTime(
        applyCorrections(
          applyAFRMultiplier(
            applyMapMode(
              computeInitialPw(REQ_FUEL, current.VE), 
              page2, current),
            page2, page6, current), 
        current.corrections), 
      injOpenTime), 
      REQ_FUEL, page2, current);

  // Make sure this won't overflow when we convert to uInt. This means the maximum pulsewidth possible is 65.535mS
  return pwApplyNitrous((uint16_t)min(intermediate, (uint32_t)UINT16_MAX), page10, current);
}

// Apply the pwLimit if staging is disabled and engine is not cranking
TESTABLE_INLINE_STATIC uint16_t applyPwLimits(uint16_t pw, uint16_t pwLimit, const config10 &page10, const statuses &current) {
  if( (!BIT_CHECK(current.engine, BIT_ENGINE_CRANK)) && (page10.stagingEnabled == false) ) { 
    return min(pw, pwLimit);
  }
  return pw;
}

static inline bool canApplyStaging(const config2 &page2, const config10 &page10) {
    //To run staged injection, the number of cylinders must be less than the injector channels (ie Assuming you're running paired injection, you need at least as many injector channels as you have cylinders, half for the primaries and half for the secondaries)
 return  (page10.stagingEnabled == true) 
      && (page2.nCylinders <= INJ_CHANNELS || page2.injType == INJ_TYPE_TBODY); //Final check is to ensure that DFCO isn't active, which would cause an overflow below (See #267)  
}

static inline uint32_t calcTotalStagePw(uint16_t primaryPW, uint16_t injOpenTime, const config10 &page10) {
  // Subtract the opening time from PW1 as it needs to be multiplied out again by the pri/sec req_fuel values below. 
  // It is added on again after that calculation. 
  primaryPW = primaryPW - injOpenTime;
  uint32_t totalInjector = page10.stagedInjSizePri + page10.stagedInjSizeSec;
  return ((uint32_t)primaryPW)*totalInjector;
}

static inline uint32_t calcStagePrimaryPw(uint32_t totalPw, const config10 &page10) {
  return fastDiv(totalPw, page10.stagedInjSizePri);
}
static inline uint32_t calcStageSecondaryPw(uint32_t totalPw, const config10 &page10) {
  return fastDiv(totalPw, page10.stagedInjSizeSec);
}

static inline pulseWidths applyStagingModeTable(uint16_t primaryPW, uint16_t injOpenTime, const config10 &page10, const statuses &current) {
  uint32_t totalPw = calcTotalStagePw(primaryPW, injOpenTime, page10);
  //Subtract the opening time from PW1 as it needs to be multiplied out again by the pri/sec req_fuel values below. It is added on again after that calculation. 
  uint32_t pwPrimaryStaged = calcStagePrimaryPw(totalPw, page10);

  uint8_t stagingSplit = get3DTableValue(&stagingTable, current.fuelLoad, current.RPM);
  if(stagingSplit > 0U) 
  { 
    uint32_t pwSecondaryStaged = calcStageSecondaryPw(totalPw, page10);
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
  uint32_t pwPrimaryStaged = calcStagePrimaryPw(calcTotalStagePw(primaryPW, injOpenTime, page10), page10);

  //If automatic mode, the primary injectors are used all the way up to their limit (Configured by the pulsewidth limit setting)
  //If they exceed their limit, the extra duty is passed to the secondaries
  if(pwPrimaryStaged > pwLimit)
  {
    uint32_t extraPW = pwPrimaryStaged - pwLimit + injOpenTime; //The open time must be added here AND below because pwPrimaryStaged does not include an open time. The addition of it here takes into account the fact that pwLlimit does not contain an allowance for an open time. 
    uint32_t secondary = fastDiv(extraPW * page10.stagedInjSizePri, page10.stagedInjSizeSec) + injOpenTime;
    return { 
      pwLimit,
      (uint16_t)min(secondary, (uint32_t)UINT16_MAX),
    };
  }

  return { (uint16_t)min(pwPrimaryStaged + injOpenTime, (uint32_t)UINT16_MAX), 0U};
}


TESTABLE_INLINE_STATIC pulseWidths calculateSecondaryPw(uint16_t primaryPw, uint16_t pwLimit, uint16_t injOpenTime, const config2 &page2, const config10 &page10, const statuses &current) {
  if(canApplyStaging(page2, page10) && (primaryPw!=0U) )
  {
    //Scale the 'full' pulsewidth by each of the injector capacities
    if(page10.stagingMode == STAGING_MODE_TABLE) {
      return applyStagingModeTable(primaryPw, injOpenTime, page10, current);
    };
    if(page10.stagingMode == STAGING_MODE_AUTO) {
      return applyStagingModeAuto(primaryPw, pwLimit, injOpenTime, page10);
    }
  }

  return { primaryPw, 0U };
}


void applyPwToInjectorChannels(const pulseWidths &pulse_widths, const config2 &page2, statuses &current) {
  current.PW1 = current.PW2 = current.PW3 = current.PW4 = current.PW5 = current.PW6 = current.PW7 = current.PW8 = 0U;

  #define ASSIGN_PULSEWIDTH_OR_ZERO(index, pw) \
    current.PW ## index = ((current.maxInjOutputs) >= (uint8_t)(index)) ? (pw) : 0U;

  // The PW calcs already applied the logic to enable staging or not. If there is a valid
  // secondary PW, staging is enabled 
  if ((pulse_widths.secondary!=0U)) {
    //Allocate the primary and secondary pulse widths based on the fuel configuration
    switch (page2.nCylinders) {
      case 1:
        ASSIGN_PULSEWIDTH_OR_ZERO(1, pulse_widths.primary);
#if INJ_CHANNELS >= 2
        ASSIGN_PULSEWIDTH_OR_ZERO(2, pulse_widths.secondary);
#endif
        break;

      case 2:
        //Primary pulsewidth on channels 1 and 2, secondary on channels 3 and 4
        ASSIGN_PULSEWIDTH_OR_ZERO(1, pulse_widths.primary);
#if INJ_CHANNELS >= 2
        ASSIGN_PULSEWIDTH_OR_ZERO(2, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 3
        ASSIGN_PULSEWIDTH_OR_ZERO(3, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 4
        ASSIGN_PULSEWIDTH_OR_ZERO(4, pulse_widths.secondary);
#endif
        break;

      case 3:
        ASSIGN_PULSEWIDTH_OR_ZERO(1, pulse_widths.primary);
#if INJ_CHANNELS >= 2
        ASSIGN_PULSEWIDTH_OR_ZERO(2, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 3
        ASSIGN_PULSEWIDTH_OR_ZERO(3, pulse_widths.primary);
#endif
        //6 channels required for 'normal' 3 cylinder staging support
#if INJ_CHANNELS >= 4
        ASSIGN_PULSEWIDTH_OR_ZERO(4, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 5
        ASSIGN_PULSEWIDTH_OR_ZERO(5, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 6
        ASSIGN_PULSEWIDTH_OR_ZERO(6, pulse_widths.secondary);
#endif
        break;

      case 4:
        ASSIGN_PULSEWIDTH_OR_ZERO(1, pulse_widths.primary);
#if INJ_CHANNELS >= 2
        ASSIGN_PULSEWIDTH_OR_ZERO(2, pulse_widths.primary);
#endif
        if( (page2.injLayout == INJ_SEQUENTIAL) || (page2.injLayout == INJ_SEMISEQUENTIAL) )
        {
#if INJ_CHANNELS >= 3
          ASSIGN_PULSEWIDTH_OR_ZERO(3, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 4
          ASSIGN_PULSEWIDTH_OR_ZERO(4, pulse_widths.primary);
#endif
        // Staging with 4 cylinders semi/sequential requires 8 total channels
#if INJ_CHANNELS >= 5
          ASSIGN_PULSEWIDTH_OR_ZERO(5, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 6
          ASSIGN_PULSEWIDTH_OR_ZERO(6, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 7
          ASSIGN_PULSEWIDTH_OR_ZERO(7, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 8
          ASSIGN_PULSEWIDTH_OR_ZERO(8, pulse_widths.secondary);
#endif
        } else {
#if INJ_CHANNELS >= 3
          ASSIGN_PULSEWIDTH_OR_ZERO(3, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 4
          ASSIGN_PULSEWIDTH_OR_ZERO(4, pulse_widths.secondary);
#endif
        }
        break;
        
      case 5:
        ASSIGN_PULSEWIDTH_OR_ZERO(1, pulse_widths.primary);
#if INJ_CHANNELS >= 2
        ASSIGN_PULSEWIDTH_OR_ZERO(2, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 3
        ASSIGN_PULSEWIDTH_OR_ZERO(3, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 4
        ASSIGN_PULSEWIDTH_OR_ZERO(4, pulse_widths.primary);
#endif
        //No easily supportable 5 cylinder staging option unless there are at least 5 channels
          if (page2.injLayout != INJ_SEQUENTIAL) {
#if INJ_CHANNELS >= 5
        ASSIGN_PULSEWIDTH_OR_ZERO(5, pulse_widths.secondary);
#endif
          } else {
#if INJ_CHANNELS >= 5
        ASSIGN_PULSEWIDTH_OR_ZERO(5, pulse_widths.primary);
#endif
          }
#if INJ_CHANNELS >= 6
        ASSIGN_PULSEWIDTH_OR_ZERO(6, pulse_widths.secondary);
#endif
        break;

      case 6:
        ASSIGN_PULSEWIDTH_OR_ZERO(1, pulse_widths.primary);
#if INJ_CHANNELS >= 2
        ASSIGN_PULSEWIDTH_OR_ZERO(2, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 3
        ASSIGN_PULSEWIDTH_OR_ZERO(3, pulse_widths.primary);
#endif
        // 6 cylinder staging only if not sequential
        if (page2.injLayout != INJ_SEQUENTIAL) {
#if INJ_CHANNELS >= 4
        ASSIGN_PULSEWIDTH_OR_ZERO(4, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 5
        ASSIGN_PULSEWIDTH_OR_ZERO(5, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 6
        ASSIGN_PULSEWIDTH_OR_ZERO(6, pulse_widths.secondary);
#endif
        } else {
#if INJ_CHANNELS >= 4
          ASSIGN_PULSEWIDTH_OR_ZERO(4, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 5
          ASSIGN_PULSEWIDTH_OR_ZERO(5, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 6
          ASSIGN_PULSEWIDTH_OR_ZERO(6, pulse_widths.primary);
#endif
          //If there are 8 channels, then the 6 cylinder sequential option is available by using channels 7 + 8 for staging
#if INJ_CHANNELS >= 7
          ASSIGN_PULSEWIDTH_OR_ZERO(7, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 8
          ASSIGN_PULSEWIDTH_OR_ZERO(8, pulse_widths.secondary);
#endif
        }
        break;

      case 8:
        ASSIGN_PULSEWIDTH_OR_ZERO(1, pulse_widths.primary);
#if INJ_CHANNELS >= 2
        ASSIGN_PULSEWIDTH_OR_ZERO(2, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 3
        ASSIGN_PULSEWIDTH_OR_ZERO(3, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 4
        ASSIGN_PULSEWIDTH_OR_ZERO(4, pulse_widths.primary);
#endif
        //8 cylinder staging only if not sequential
        if (page2.injLayout != INJ_SEQUENTIAL)
        {
#if INJ_CHANNELS >= 5
          ASSIGN_PULSEWIDTH_OR_ZERO(5, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 6
          ASSIGN_PULSEWIDTH_OR_ZERO(6, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 7
          ASSIGN_PULSEWIDTH_OR_ZERO(7, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 8
          ASSIGN_PULSEWIDTH_OR_ZERO(8, pulse_widths.secondary);
#endif
        } else {
#if INJ_CHANNELS >= 5
        ASSIGN_PULSEWIDTH_OR_ZERO(5, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 6
        ASSIGN_PULSEWIDTH_OR_ZERO(6, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 7
        ASSIGN_PULSEWIDTH_OR_ZERO(7, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 8
        ASSIGN_PULSEWIDTH_OR_ZERO(8, pulse_widths.primary);
#endif          
        }
        break;

      default:
        //Assume 4 cylinder non-seq for default
        ASSIGN_PULSEWIDTH_OR_ZERO(1, pulse_widths.primary);
#if INJ_CHANNELS >= 2
        ASSIGN_PULSEWIDTH_OR_ZERO(2, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 3
          ASSIGN_PULSEWIDTH_OR_ZERO(3, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 4
          ASSIGN_PULSEWIDTH_OR_ZERO(4, pulse_widths.secondary);
#endif
       break;
    }
  }
  else if (pulse_widths.primary!=0U)
  { 
    ASSIGN_PULSEWIDTH_OR_ZERO(1, pulse_widths.primary);
#if INJ_CHANNELS >= 2
    ASSIGN_PULSEWIDTH_OR_ZERO(2, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 3
    ASSIGN_PULSEWIDTH_OR_ZERO(3, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 4
    ASSIGN_PULSEWIDTH_OR_ZERO(4, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 5
    ASSIGN_PULSEWIDTH_OR_ZERO(5, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 6
    ASSIGN_PULSEWIDTH_OR_ZERO(6, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 7
    ASSIGN_PULSEWIDTH_OR_ZERO(7, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 8
    ASSIGN_PULSEWIDTH_OR_ZERO(8, pulse_widths.primary);
#endif
  } else {
    //No pulse widths to apply
  } 
}

TESTABLE_INLINE_STATIC uint16_t calculateOpenTime(const config2 &page2, const statuses &current) {
  // Convert injector open time from tune to microseconds & apply voltage correction if required
  return page2.injOpen * current.batCorrection; 
}

pulseWidths computePulseWidths(const config2 &page2, const config6 &page6, const config10 &page10, const statuses &current) {
  if (current.corrections!=0U) {
    uint16_t pwLimit = calculatePWLimit(page2, current);
    uint16_t injOpenTime = calculateOpenTime(page2, current);
    uint16_t primaryPw = applyPwLimits(calcPrimaryPulseWidth( injOpenTime, 
                                                              page2,
                                                              page6,
                                                              page10, 
                                                              current),
                                        pwLimit,
                                        page10,
                                        current);
    return calculateSecondaryPw(primaryPw, pwLimit, injOpenTime, page2, page10, current);  
  }
  return { 0U, 0U };
}