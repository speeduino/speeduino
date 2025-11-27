#include "fuel_calcs.h"
#include "globals.h"

uint16_t req_fuel_uS = 0; /**< The required fuel variable (As calculated by TunerStudio) in uS */
uint16_t inj_opentime_uS = 0;
uint16_t staged_req_fuel_mult_pri = 0;
uint16_t staged_req_fuel_mult_sec = 0;   

uint16_t PW(int REQ_FUEL, byte VE, long MAP, uint16_t corrections, int injOpen)
{
  //Standard float version of the calculation
  //return (REQ_FUEL * (float)(VE/100.0) * (float)(MAP/100.0) * (float)(TPS/100.0) * (float)(corrections/100.0) + injOpen);
  //Note: The MAP and TPS portions are currently disabled, we use VE and corrections only
  uint16_t iVE;
  uint16_t iMAP = 100;
  uint16_t iAFR = 147;

  //100% float free version, does sacrifice a little bit of accuracy, but not much.
 
  //iVE = ((unsigned int)VE << 7) / 100;
  iVE = div100(((uint16_t)VE << 7U));

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

  uint32_t intermediate = rshift<7U>((uint32_t)REQ_FUEL * (uint32_t)iVE); //Need to use an intermediate value to avoid overflowing the long
  if ( configPage2.multiplyMAP > 0 ) { intermediate = rshift<7U>(intermediate * (uint32_t)iMAP); }
  
  if ( (configPage2.includeAFR == true) && (configPage6.egoType == EGO_TYPE_WIDE) && (currentStatus.runSecs > configPage6.ego_sdelay) ) {
    //EGO type must be set to wideband and the AFR warmup time must've elapsed for this to be used
    intermediate = rshift<7U>(intermediate * (uint32_t)iAFR);  
  }
  if ( (configPage2.incorporateAFR == true) && (configPage2.includeAFR == false) ) {
    intermediate = rshift<7U>(intermediate * (uint32_t)iAFR);
  }

  //If corrections are huge, use less bitshift to avoid overflow. Sacrifices a bit more accuracy (basically only during very cold temp cranking)
  if (corrections < 512 ) { 
    intermediate = rshift<7U>(intermediate * div100(lshift<7U>(corrections))); 
  } else if (corrections < 1024 ) { 
    intermediate = rshift<6U>(intermediate * div100(lshift<6U>(corrections)));
  } else {
    intermediate = rshift<5U>(intermediate * div100(lshift<5U>(corrections)));
  }

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
  return (unsigned int)(intermediate);
}

uint16_t calculatePWLimit()
{
  uint32_t tempLimit = percentage(configPage2.dutyLim, revolutionTime); //The pulsewidth limit is determined to be the duty cycle limit (Eg 85%) by the total time it takes to perform 1 revolution
  //Handle multiple squirts per rev
  if (configPage2.strokes == FOUR_STROKE) { tempLimit = tempLimit * 2; }
  //Optimise for power of two divisions where possible
  switch(currentStatus.nSquirts)
  {
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
      tempLimit = tempLimit / currentStatus.nSquirts;
      break;
  }
  if(tempLimit > UINT16_MAX) { tempLimit = UINT16_MAX; }

  return tempLimit;
}

void calculateStaging(uint32_t pwLimit)
{
  //Calculate staging pulsewidths if used
  //To run staged injection, the number of cylinders must be less than or equal to the injector channels (ie Assuming you're running paired injection, you need at least as many injector channels as you have cylinders, half for the primaries and half for the secondaries)
  if( (configPage10.stagingEnabled == true) && (configPage2.nCylinders <= INJ_CHANNELS || configPage2.injType == INJ_TYPE_TBODY) && (currentStatus.PW1 > inj_opentime_uS) ) //Final check is to ensure that DFCO isn't active, which would cause an overflow below (See #267)
  {
    //Scale the 'full' pulsewidth by each of the injector capacities
    currentStatus.PW1 -= inj_opentime_uS; //Subtract the opening time from PW1 as it needs to be multiplied out again by the pri/sec req_fuel values below. It is added on again after that calculation. 
    uint32_t tempPW1 = div100((uint32_t)currentStatus.PW1 * staged_req_fuel_mult_pri);

    if(configPage10.stagingMode == STAGING_MODE_TABLE)
    {
      uint32_t tempPW3 = div100((uint32_t)currentStatus.PW1 * staged_req_fuel_mult_sec); //This is ONLY needed in in table mode. Auto mode only calculates the difference.

      uint8_t stagingSplit = get3DTableValue(&stagingTable, currentStatus.fuelLoad, currentStatus.RPM);
      currentStatus.PW1 = div100((100U - stagingSplit) * tempPW1);
      currentStatus.PW1 += inj_opentime_uS; 

      //PW2 is used temporarily to hold the secondary injector pulsewidth. It will be assigned to the correct channel below
      if(stagingSplit > 0) 
      { 
        currentStatus.stagingActive = true; //Set the staging active flag
        currentStatus.PW2 = div100(stagingSplit * tempPW3); 
        currentStatus.PW2 += inj_opentime_uS;
      }
      else
      {
        currentStatus.stagingActive = false; //Clear the staging active flag
        currentStatus.PW2 = 0; 
      }
    }
    else if(configPage10.stagingMode == STAGING_MODE_AUTO)
    {
      currentStatus.PW1 = tempPW1;
      //If automatic mode, the primary injectors are used all the way up to their limit (Configured by the pulsewidth limit setting)
      //If they exceed their limit, the extra duty is passed to the secondaries
      if(tempPW1 > pwLimit)
      {
        currentStatus.stagingActive = true; //Set the staging active flag
        uint32_t extraPW = tempPW1 - pwLimit + inj_opentime_uS; //The open time must be added here AND below because tempPW1 does not include an open time. The addition of it here takes into account the fact that pwLlimit does not contain an allowance for an open time. 
        currentStatus.PW1 = pwLimit;
        currentStatus.PW2 = udiv_32_16(extraPW * staged_req_fuel_mult_sec, staged_req_fuel_mult_pri); //Convert the 'left over' fuel amount from primary injector scaling to secondary
        currentStatus.PW2 += inj_opentime_uS;
      }
      else 
      {
        //If tempPW1 < pwLImit it means that the entire fuel load can be handled by the primaries and staging is inactive. 
        currentStatus.PW1 += inj_opentime_uS; //Add the open time back in
        currentStatus.stagingActive = false; //Clear the staging active flag 
        currentStatus.PW2 = 0; //Secondary PW is simply set to 0 as it is not required
      } 
    }

    //Allocate the primary and secondary pulse widths based on the fuel configuration
    switch (configPage2.nCylinders) 
    {
      case 1:
        //Nothing required for 1 cylinder, channels are correct already
        break;
      case 2:
        //Primary pulsewidth on channels 1 and 2, secondary on channels 3 and 4
        currentStatus.PW3 = currentStatus.PW2;
        currentStatus.PW4 = currentStatus.PW2;
        currentStatus.PW2 = currentStatus.PW1;
        break;
      case 3:
        //6 channels required for 'normal' 3 cylinder staging support
        #if INJ_CHANNELS >= 6
          //Primary pulsewidth on channels 1, 2 and 3, secondary on channels 4, 5 and 6
          currentStatus.PW4 = currentStatus.PW2;
          currentStatus.PW5 = currentStatus.PW2;
          currentStatus.PW6 = currentStatus.PW2;
        #else
          //If there are not enough channels, then primary pulsewidth is on channels 1, 2 and 3, secondary on channel 4
          currentStatus.PW4 = currentStatus.PW2;
        #endif
        currentStatus.PW2 = currentStatus.PW1;
        currentStatus.PW3 = currentStatus.PW1;
        break;
      case 4:
        if( (configPage2.injLayout == INJ_SEQUENTIAL) || (configPage2.injLayout == INJ_SEMISEQUENTIAL) )
        {
          //Staging with 4 cylinders semi/sequential requires 8 total channels
          #if INJ_CHANNELS >= 8
            currentStatus.PW5 = currentStatus.PW2;
            currentStatus.PW6 = currentStatus.PW2;
            currentStatus.PW7 = currentStatus.PW2;
            currentStatus.PW8 = currentStatus.PW2;

            currentStatus.PW2 = currentStatus.PW1;
            currentStatus.PW3 = currentStatus.PW1;
            currentStatus.PW4 = currentStatus.PW1;
          #else
            //This is an invalid config as there are not enough outputs to support sequential + staging
            //Put the staging output to the non-existent channel 5
            currentStatus.PW5 = currentStatus.PW2;
          #endif
        }
        else
        {
          currentStatus.PW3 = currentStatus.PW2;
          currentStatus.PW4 = currentStatus.PW2;
          currentStatus.PW2 = currentStatus.PW1;
        }
        break;
        
      case 5:
        //No easily supportable 5 cylinder staging option unless there are at least 5 channels
        #if INJ_CHANNELS >= 5
          if (configPage2.injLayout != INJ_SEQUENTIAL)
          {
            currentStatus.PW5 = currentStatus.PW2;
          }
          #if INJ_CHANNELS >= 6
            currentStatus.PW6 = currentStatus.PW2;
          #endif
        #endif
        
          currentStatus.PW2 = currentStatus.PW1;
          currentStatus.PW3 = currentStatus.PW1;
          currentStatus.PW4 = currentStatus.PW1;
        break;

      case 6:
        #if INJ_CHANNELS >= 6
          //8 cylinder staging only if not sequential
          if (configPage2.injLayout != INJ_SEQUENTIAL)
          {
            currentStatus.PW4 = currentStatus.PW2;
            currentStatus.PW5 = currentStatus.PW2;
            currentStatus.PW6 = currentStatus.PW2;
          }
          #if INJ_CHANNELS >= 8
          else
            {
              //If there are 8 channels, then the 6 cylinder sequential option is available by using channels 7 + 8 for staging
              currentStatus.PW7 = currentStatus.PW2;
              currentStatus.PW8 = currentStatus.PW2;

              currentStatus.PW4 = currentStatus.PW1;
              currentStatus.PW5 = currentStatus.PW1;
              currentStatus.PW6 = currentStatus.PW1;
            }
          #endif
        #endif
        currentStatus.PW2 = currentStatus.PW1;
        currentStatus.PW3 = currentStatus.PW1;
        break;

      case 8:
        #if INJ_CHANNELS >= 8
          //8 cylinder staging only if not sequential
          if (configPage2.injLayout != INJ_SEQUENTIAL)
          {
            currentStatus.PW5 = currentStatus.PW2;
            currentStatus.PW6 = currentStatus.PW2;
            currentStatus.PW7 = currentStatus.PW2;
            currentStatus.PW8 = currentStatus.PW2;
          }
        #endif
        currentStatus.PW2 = currentStatus.PW1;
        currentStatus.PW3 = currentStatus.PW1;
        currentStatus.PW4 = currentStatus.PW1;
        break;

      default:
        //Assume 4 cylinder non-seq for default
        currentStatus.PW3 = currentStatus.PW2;
        currentStatus.PW4 = currentStatus.PW2;
        currentStatus.PW2 = currentStatus.PW1;
        break;
    }
  }
  else 
  { 
    if(maxInjOutputs >= 2) { currentStatus.PW2 = currentStatus.PW1; }
    else { currentStatus.PW2 = 0; }
    if(maxInjOutputs >= 3) { currentStatus.PW3 = currentStatus.PW1; }
    else { currentStatus.PW3 = 0; }
    if(maxInjOutputs >= 4) { currentStatus.PW4 = currentStatus.PW1; }
    else { currentStatus.PW4 = 0; }
    if(maxInjOutputs >= 5) { currentStatus.PW5 = currentStatus.PW1; }
    else { currentStatus.PW5 = 0; }
    if(maxInjOutputs >= 6) { currentStatus.PW6 = currentStatus.PW1; }
    else { currentStatus.PW6 = 0; }
    if(maxInjOutputs >= 7) { currentStatus.PW7 = currentStatus.PW1; }
    else { currentStatus.PW7 = 0; }
    if(maxInjOutputs >= 8) { currentStatus.PW8 = currentStatus.PW1; }
    else { currentStatus.PW8 = 0; }

    currentStatus.stagingActive = false; //Clear the staging active flag
    
  } 

}
