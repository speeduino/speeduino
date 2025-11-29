#include "fuel_calcs.h"
#include "globals.h"
#include "unit_testing.h"

static inline uint16_t calcNitrousStagePulseWidth(uint8_t minRPM, uint8_t maxRPM, uint8_t adderMin, uint8_t adderMax, const statuses &current)
{
  int16_t adderRange = (maxRPM - minRPM) * 100;
  int16_t adderPercent = ((current.RPM - (minRPM * 100)) * 100) / adderRange; //The percentage of the way through the RPM range
  adderPercent = 100 - adderPercent; //Flip the percentage as we go from a higher adder to a lower adder as the RPMs rise
  return (adderMax + percentage(adderPercent, (adderMin - adderMax))) * 100; //Calculate the above percentage of the calculated ms value.
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

uint16_t calculateRequiredFuel(const config2 &page2, const statuses &current) {
  uint16_t reqFuel = page2.reqFuel * 100U; //Convert to uS and an int. This is the only variable to be used in calculations
  if ((page2.strokes == FOUR_STROKE) && ((page2.injLayout != INJ_SEQUENTIAL) || (current.halfSync)))
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

// Force this to be inlined via LTO: it's worth 40 loop/sec on AVR
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
uint16_t __attribute__((always_inline)) PW(int REQ_FUEL, byte VE, long MAP, uint16_t corrections, int injOpen, const config10 &page10, const statuses &current)
{
  //Standard float version of the calculation
  //return (REQ_FUEL * (float)(VE/100.0) * (float)(MAP/100.0) * (float)(TPS/100.0) * (float)(corrections/100.0) + injOpen);
  //Note: The MAP and TPS portions are currently disabled, we use VE and corrections only
  uint16_t iMAP = 100;
  uint16_t iAFR = 147;

  //100% float free version, does sacrifice a little bit of accuracy, but not much.

  //Check whether either of the multiply MAP modes is turned on
  //if ( configPage2.multiplyMAP == MULTIPLY_MAP_MODE_100) { iMAP = ((unsigned int)MAP << 7) / 100; }
  if ( configPage2.multiplyMAP == MULTIPLY_MAP_MODE_100) { iMAP = div100( ((uint16_t)MAP << 7U) ); }
  else if( configPage2.multiplyMAP == MULTIPLY_MAP_MODE_BARO) { iMAP = ((unsigned int)MAP << 7U) / currentStatus.baro; }
  
  if ( (configPage2.includeAFR == true) && (configPage6.egoType == EGO_TYPE_WIDE) && (currentStatus.runSecs > configPage6.ego_sdelay) ) {
    iAFR = ((unsigned int)currentStatus.O2 << 7U) / currentStatus.afrTarget;  //Include AFR (vs target) if enabled
  }
  if ( (configPage2.incorporateAFR == true) && (configPage2.includeAFR == false) ) {
    iAFR = ((unsigned int)configPage2.stoich << 7U) / currentStatus.afrTarget;  //Incorporate stoich vs target AFR, if enabled.
  }

  uint32_t intermediate = percentageApprox(VE, REQ_FUEL); //Need to use an intermediate value to avoid overflowing the long
  if ( configPage2.multiplyMAP > 0 ) { intermediate = rshift<7U>(intermediate * (uint32_t)iMAP); }
  
  if ( (configPage2.includeAFR == true) && (configPage6.egoType == EGO_TYPE_WIDE) && (currentStatus.runSecs > configPage6.ego_sdelay) ) {
    //EGO type must be set to wideband and the AFR warmup time must've elapsed for this to be used
    intermediate = rshift<7U>(intermediate * (uint32_t)iAFR);  
  }
  if ( (configPage2.incorporateAFR == true) && (configPage2.includeAFR == false) ) {
    intermediate = rshift<7U>(intermediate * (uint32_t)iAFR);
  }

  //If corrections are huge, use less bitshift to avoid overflow. Sacrifices a bit more accuracy (basically only during very cold temp cranking)
  intermediate = percentageApprox(corrections, intermediate);

  if (intermediate != 0)
  {
    //If intermediate is not 0, we need to add the opening time (0 typically indicates that one of the full fuel cuts is active)
    intermediate += injOpen; //Add the injector opening time
    //AE calculation only when ACC is active.
    if ( currentStatus.isAcceleratingTPS )
    {
      //AE Adds % of req_fuel
      if ( configPage2.aeApplyMode == AE_MODE_ADDER )
        {
          intermediate += div100(((uint32_t)REQ_FUEL) * (currentStatus.AEamount - 100U));
        }
    }

    if ( intermediate > UINT16_MAX)
    {
      intermediate = UINT16_MAX;  //Make sure this won't overflow when we convert to uInt. This means the maximum pulsewidth possible is 65.535mS
    }
  }
  return pwApplyNitrous((unsigned int)intermediate, page10, current);
}
#pragma GCC diagnostic pop

uint16_t calculatePWLimit(const config2 &page2, const statuses &current, uint32_t revTime)
{
  uint32_t tempLimit = percentageApprox(page2.dutyLim, revTime); //The pulsewidth limit is determined to be the duty cycle limit (Eg 85%) by the total time it takes to perform 1 revolution
  //Handle multiple squirts per rev
  if (page2.strokes == FOUR_STROKE) { tempLimit = tempLimit * 2; }
  //Optimise for power of two divisions where possible
  switch(current.nSquirts)  {
    case 1:
      //No action needed
      break;
    case 2:
      tempLimit = tempLimit / 2;
      break;
    case 4:
      tempLimit = tempLimit / 4;
      break;
    case 8:
      tempLimit = tempLimit / 8;
      break;
    default:
      //Non-PoT squirts value. Perform (slow) uint32_t division
      tempLimit = tempLimit / current.nSquirts;
      break;
  }
  if(tempLimit > (uint32_t)UINT16_MAX) { tempLimit = UINT16_MAX; }

  return tempLimit;
}


static inline uint32_t calcTotalStagePw(uint16_t primaryPW, uint16_t injOpenTime, const config10 &page10) {
  // Subtract the opening time from PW1 as it needs to be multiplied out again by the pri/sec req_fuel values below. 
  // It is added on again after that calculation. 
  primaryPW = primaryPW - injOpenTime;
  uint32_t totalInjector = page10.stagedInjSizePri + page10.stagedInjSizeSec;
  return ((uint32_t)primaryPW)*totalInjector;
}

static inline uint32_t calcStagePrimaryPw(uint16_t primaryPW, uint16_t injOpenTime, const config10 &page10) {
  return fastDiv(calcTotalStagePw(primaryPW, injOpenTime, page10), page10.stagedInjSizePri);
}
static inline uint32_t calcStageSecondaryPw(uint16_t primaryPW, uint16_t injOpenTime, const config10 &page10) {
  return fastDiv(calcTotalStagePw(primaryPW, injOpenTime, page10), page10.stagedInjSizeSec);
}

pulseWidths calculateSecondaryPw(uint16_t primaryPw, uint16_t pwLimit, uint16_t injOpenTime, const config2 &page2, const config10 &page10, const statuses &current) {
  uint16_t secondaryPW = 0U;
  if(canApplyStaging(page2, page10) && (primaryPw!=0U) )
  {
    uint32_t pwPrimaryStaged = calcStagePrimaryPw(primaryPw, injOpenTime, page10);

    if(page10.stagingMode == STAGING_MODE_TABLE)
    {
      uint8_t stagingSplit = get3DTableValue(&stagingTable, current.fuelLoad, current.RPM);
      if(stagingSplit > 0U) 
      { 
        uint32_t pwSecondaryStaged = calcStageSecondaryPw(primaryPw, injOpenTime, page10);
        uint32_t primary = percentage(100U - stagingSplit, pwPrimaryStaged) + injOpenTime;
        uint32_t secondary = percentage(stagingSplit, pwSecondaryStaged) + injOpenTime;
        primaryPw = (uint16_t)min(primary, (uint32_t)UINT16_MAX);
        secondaryPW = (uint16_t)min(secondary, (uint32_t)UINT16_MAX);
      } else {
        primaryPw = pwPrimaryStaged + injOpenTime;
      }
    }
    else if(page10.stagingMode == STAGING_MODE_AUTO)
    {
      //If automatic mode, the primary injectors are used all the way up to their limit (Configured by the pulsewidth limit setting)
      //If they exceed their limit, the extra duty is passed to the secondaries
      if(pwPrimaryStaged > pwLimit)
      {
        uint32_t extraPW = pwPrimaryStaged - pwLimit + injOpenTime; //The open time must be added here AND below because pwPrimaryStaged does not include an open time. The addition of it here takes into account the fact that pwLlimit does not contain an allowance for an open time. 
        uint32_t secondary = fastDiv(extraPW * page10.stagedInjSizePri, page10.stagedInjSizeSec) + injOpenTime;
        primaryPw = pwLimit;
        secondaryPW = (uint16_t)min(secondary, (uint32_t)UINT16_MAX);
      } else {
        primaryPw = pwPrimaryStaged + injOpenTime;
      }
    }
  }

  return { primaryPw, secondaryPW };
}


void applyPwToInjectorChannels(const pulseWidths &pulse_widths, const config2 &page2, statuses &current) {
  current.PW1 = pulse_widths.primary;
  current.PW2 = current.PW3 = current.PW4 = current.PW5 = current.PW6 = current.PW7 = current.PW8 = 0U;

  if(pulse_widths.secondary != 0U)
  {
    current.PW2 = pulse_widths.secondary;
    //Allocate the primary and secondary pulse widths based on the fuel configuration
    switch (page2.nCylinders) 
    {
      case 1:
        //Nothing required for 1 cylinder, channels are correct already
        break;
      case 2:
        //Primary pulsewidth on channels 1 and 2, secondary on channels 3 and 4
        current.PW3 = current.PW2;
        current.PW4 = current.PW2;
        current.PW2 = current.PW1;
        break;
      case 3:
        //6 channels required for 'normal' 3 cylinder staging support
        #if INJ_CHANNELS >= 6
          //Primary pulsewidth on channels 1, 2 and 3, secondary on channels 4, 5 and 6
          current.PW4 = current.PW2;
          current.PW5 = current.PW2;
          current.PW6 = current.PW2;
        #else
          //If there are not enough channels, then primary pulsewidth is on channels 1, 2 and 3, secondary on channel 4
          current.PW4 = current.PW2;
        #endif
        current.PW2 = current.PW1;
        current.PW3 = current.PW1;
        break;
      case 4:
        if( (page2.injLayout == INJ_SEQUENTIAL) || (page2.injLayout == INJ_SEMISEQUENTIAL) )
        {
          current.PW2 = current.PW1;
          current.PW3 = current.PW1;
          current.PW4 = current.PW1;
        //Staging with 4 cylinders semi/sequential requires 8 total channels
        #if INJ_CHANNELS >= 8
          current.PW5 = current.PW2;
          current.PW6 = current.PW2;
          current.PW7 = current.PW2;
          current.PW8 = current.PW2;
        #elif INJ_CHANNELS >= 5
          //This is an invalid config as there are not enough outputs to support sequential + staging
          //Put the staging output to the non-existent channel 5
          current.PW5 = current.PW2;
        #endif
        }
        else
        {
          current.PW3 = current.PW2;
          current.PW4 = current.PW2;
          current.PW2 = current.PW1;
        }
        break;
        
      case 5:
        //No easily supportable 5 cylinder staging option unless there are at least 5 channels
        #if INJ_CHANNELS >= 5
          if (page2.injLayout != INJ_SEQUENTIAL)
          {
            current.PW5 = current.PW2;
          }
          #if INJ_CHANNELS >= 6
            current.PW6 = current.PW2;
          #endif
        #endif
        
        current.PW2 = current.PW1;
        current.PW3 = current.PW1;
        current.PW4 = current.PW1;
        break;

      case 6:
        current.PW2 = current.PW1;
        current.PW3 = current.PW1;
        #if INJ_CHANNELS >= 6
          //8 cylinder staging only if not sequential
          if (page2.injLayout != INJ_SEQUENTIAL)
          {
            current.PW4 = current.PW2;
            current.PW5 = current.PW2;
            current.PW6 = current.PW2;
          }
          #if INJ_CHANNELS >= 8
          else
            {
              //If there are 8 channels, then the 6 cylinder sequential option is available by using channels 7 + 8 for staging
              current.PW7 = current.PW2;
              current.PW8 = current.PW2;

              current.PW4 = current.PW1;
              current.PW5 = current.PW1;
              current.PW6 = current.PW1;
            }
          #endif
        #endif
        break;

      case 8:
        #if INJ_CHANNELS >= 8
          //8 cylinder staging only if not sequential
          if (page2.injLayout != INJ_SEQUENTIAL)
          {
            current.PW5 = current.PW2;
            current.PW6 = current.PW2;
            current.PW7 = current.PW2;
            current.PW8 = current.PW2;
          }
        #endif
        current.PW2 = current.PW1;
        current.PW3 = current.PW1;
        current.PW4 = current.PW1;
        break;

      default:
        //Assume 4 cylinder non-seq for default
        current.PW3 = current.PW2;
        current.PW4 = current.PW2;
        current.PW2 = current.PW1;
        break;
    }
  }
  else 
  { 
    if(maxInjOutputs >= 2) { current.PW2 = current.PW1; }
    else { current.PW2 = 0; }
    if(maxInjOutputs >= 3) { current.PW3 = current.PW1; }
    else { current.PW3 = 0; }
    if(maxInjOutputs >= 4) { current.PW4 = current.PW1; }
    else { current.PW4 = 0; }
    if(maxInjOutputs >= 5) { current.PW5 = current.PW1; }
    else { current.PW5 = 0; }
    if(maxInjOutputs >= 6) { current.PW6 = current.PW1; }
    else { current.PW6 = 0; }
    if(maxInjOutputs >= 7) { current.PW7 = current.PW1; }
    else { current.PW7 = 0; }
    if(maxInjOutputs >= 8) { current.PW8 = current.PW1; }
    else { current.PW8 = 0; }

    current.stagingActive = false; //Clear the staging active flag   
  } 
}
