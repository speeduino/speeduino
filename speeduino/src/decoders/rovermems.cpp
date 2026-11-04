#include "rovermems.h"
#include "shared/perToothIgnition.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"
#include "scheduler_ignition_controller.h"

namespace decoders {

namespace rovermems {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(config4 &page4)
{
  shared::state_t state = {};

  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 36U)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  state.triggerSecFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U)); // only 1 tooth on the wheel not 36

  page4.triggerTeeth = 36;
  state.triggerToothAngle = 360 / page4.triggerTeeth; //The number of degrees that passes from tooth to tooth 360 / 36 theortical teeth
  state.triggerActualTeeth = 36; //The number of physical teeth on the wheel. Need to fix now so we can identify the wheel on the first rotation and not risk a  type 1 wheel not being spotted

  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle * 2U); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  state.decoderFeatures.supportsPerToothIgnition = true;

   return state;
}

#define ID_TOOTH_PATTERN 0 // have we identified teeth to skip for calculating RPM?
#define SKIP_TOOTH1 1
#define SKIP_TOOTH2 2
#define SKIP_TOOTH3 3
#define SKIP_TOOTH4 4

static volatile unsigned long roverMEMSTeethSeen = 0; // used for flywheel gap pattern matching

static void triggerRoverMEMSCommon(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
  // pattern 1 isn't unique & if we don't have a cam we need special code to identify if we're tooth 18 or 36 - this allows batch injection but not spark to run
  // as we have to be greater than 18 teeth when using the cam this code also works for that.
  if( decoderState.toothCurrentCount > 18) 
  {
    decoderState.toothCurrentCount = 1;
    decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
    decoderState.toothOneTime = curTime;
    decoderState.revolutionOne = !decoderState.revolutionOne; //Flip sequential revolution tracker   
  }

  if( (page4.sparkMode == IGN_MODE_SEQUENTIAL) || (page2.injLayout == INJ_SEQUENTIAL) )
  {
    //If either fuel or ignition is sequential, only declare sync if the cam tooth has been seen OR if the missing wheel is on the cam
    if( (decoderState.secondaryToothCount > 0) || (page4.TrigSpeed == CAM_SPEED) )
    {
      decoderState.decoderStatus.syncStatus = SyncStatus::Full; //the engine is fully synced so clear the Half Sync bit
      if(page4.trigPatternSec == SEC_TRIGGER_SINGLE) { decoderState.secondaryToothCount = 0; } //Reset the secondary tooth counter to prevent it overflowing
    }
    else if(decoderState.decoderStatus.syncStatus!=SyncStatus::Full) 
    { decoderState.decoderStatus.syncStatus = SyncStatus::Partial; } //If there is primary trigger but no secondary we only have half sync.
  }
  else { decoderState.decoderStatus.syncStatus = SyncStatus::Partial; } //If nothing is using sequential, we  set half sync bit

  current.startRevolutions++;  
}

void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, config4 &page4)
{
  decoderState.curGap = curTime - decoderState.toothLastToothTime;      

  if ( decoderState.curGap >= decoderState.triggerFilterTime ) //Pulses should never be less than decoderState.triggerFilterTime, so if they are it means a false trigger. (A 36-1 wheel at 8000pm will have triggers approx. every 200uS)
  {
    if( (decoderState.toothLastToothTime > 0) && (decoderState.toothLastMinusOneToothTime > 0) ) // have we seen more than 1 tooth so we start processing
    {   
      //Begin the missing tooth detection
      decoderState.targetGap = (3 * (decoderState.toothLastToothTime - decoderState.toothLastMinusOneToothTime)) >> 1;  //Multiply by 1.5 (Checks for a gap 1.5x greater than the last one) (Uses bitshift to multiply by 3 then divide by 2. Much faster than multiplying by 1.5)

      if ( decoderState.curGap > decoderState.targetGap) // we've found a gap
      {
        roverMEMSTeethSeen = roverMEMSTeethSeen << 2; // add the space for the gap and the tooth we've just seen so shift 2 bits
        roverMEMSTeethSeen++; // add the tooth seen to the variable
        decoderState.toothCurrentCount++; // Increment the tooth counter on the wheel (used to spot a revolution and trigger igition timing)

        // the missing tooth gap messing up timing as it appears in different parts of the cycle. Don't update setFilter as it would be wrong with the gap
        decoderState.toothCurrentCount++;
      }
      else //Regular (non-missing) tooth so update things
      {    
        roverMEMSTeethSeen = roverMEMSTeethSeen << 1; // make a space, shift the bits 1 place to the left
        roverMEMSTeethSeen++; // add the tooth seen
        decoderState.toothCurrentCount++; //Increment the tooth counter on the wheel (used to spot a revolution)
        decoderState.setFilter(decoderState.curGap, page4);
      }

      // reduce checks to minimise cpu load when looking for key point to identify where we are on the wheel
      if( decoderState.toothCurrentCount >= decoderState.triggerActualTeeth )
      {                           //12345678901234567890123456789012 
        if( roverMEMSTeethSeen == 0b11111101111111011111111110111111) // Binary pattern for trigger pattern 9-7-10-6- (#5)
        {
          if(decoderState.toothAngles[ID_TOOTH_PATTERN] != 5)
          {
            //teeth to skip when calculating RPM as they've just had a gap
            decoderState.toothAngles[SKIP_TOOTH1] = 1;
            decoderState.toothAngles[SKIP_TOOTH2] = 11;
            decoderState.toothAngles[SKIP_TOOTH3] = 19;
            decoderState.toothAngles[SKIP_TOOTH4] = 30;
            decoderState.toothAngles[ID_TOOTH_PATTERN] = 5;
            page4.triggerMissingTeeth = 4; // this could be read in from the config file, but people could adjust it.
            decoderState.triggerActualTeeth = 36; // should be 32 if not hacking toothcounter 
          }  
          triggerRoverMEMSCommon(curTime, current, decoderState, page2, page4);
        }                             //123456789012345678901234567890123456
        else if( roverMEMSTeethSeen == 0b11011101111111111111101101111111) // Binary pattern for trigger pattern 3-14-2-13- (#4)
        {
          if(decoderState.toothAngles[ID_TOOTH_PATTERN] != 4)
          {
            //teeth to skip when calculating RPM as they've just had a gap
            decoderState.toothAngles[SKIP_TOOTH1] = 8;
            decoderState.toothAngles[SKIP_TOOTH2] = 11;
            decoderState.toothAngles[SKIP_TOOTH3] = 25;
            decoderState.toothAngles[SKIP_TOOTH4] = 27;
            decoderState.toothAngles[ID_TOOTH_PATTERN] = 4;
            page4.triggerMissingTeeth = 4; // this could be read in from the config file, but people could adjust it.
            decoderState.triggerActualTeeth = 36; // should be 32 if not hacking toothcounter 
          }  
          triggerRoverMEMSCommon(curTime, current, decoderState, page2, page4);
        }                             //123456789012345678901234567890123456
        else if(roverMEMSTeethSeen == 0b11011011111111111111011101111111) // Binary pattern for trigger pattern 2-14-3-13- (#3)
        {
          if(decoderState.toothAngles[ID_TOOTH_PATTERN] != 3)
          {
            //teeth to skip when calculating RPM as they've just had a gap
            decoderState.toothAngles[SKIP_TOOTH1] = 8;
            decoderState.toothAngles[SKIP_TOOTH2] = 10;
            decoderState.toothAngles[SKIP_TOOTH3] = 24;
            decoderState.toothAngles[SKIP_TOOTH4] = 27;
            decoderState.toothAngles[ID_TOOTH_PATTERN] = 3;
            page4.triggerMissingTeeth = 4; // this could be read in from the config file, but people could adjust it.
            decoderState.triggerActualTeeth = 36; // should be 32 if not hacking toothcounter 
          } 
          triggerRoverMEMSCommon(curTime, current, decoderState, page2, page4);
        }                             //12345678901234567890123456789012
        else if(roverMEMSTeethSeen == 0b11111101111101111111111110111101) // Binary pattern for trigger pattern 11-5-12-4- (#2)
        {
          if(decoderState.toothAngles[ID_TOOTH_PATTERN] != 2)
          {
            //teeth to skip when calculating RPM as they've just had a gap
            decoderState.toothAngles[SKIP_TOOTH1] = 1;
            decoderState.toothAngles[SKIP_TOOTH2] = 12;
            decoderState.toothAngles[SKIP_TOOTH3] = 17;
            decoderState.toothAngles[SKIP_TOOTH4] = 29;
            decoderState.toothAngles[ID_TOOTH_PATTERN] = 2;
            page4.triggerMissingTeeth = 4; // this could be read in from the config file, but people could adjust it.
            decoderState.triggerActualTeeth = 36; // should be 32 if not hacking toothcounter 
          }  
          triggerRoverMEMSCommon(curTime, current, decoderState, page2, page4);
        }                             //12345678901234567890123456789012
        else if(roverMEMSTeethSeen == 0b11111111111101111111111111111101) // Binary pattern for trigger pattern 17-17- (#1)
        {
          if(decoderState.toothAngles[ID_TOOTH_PATTERN] != 1)
          {
            //teeth to skip when calculating RPM as they've just had a gap
            decoderState.toothAngles[SKIP_TOOTH1] = 1;
            decoderState.toothAngles[SKIP_TOOTH2] = 18;
            decoderState.toothAngles[ID_TOOTH_PATTERN] = 1;
            page4.triggerMissingTeeth = 2; // this should be read in from the config file, but people could adjust it.            
            decoderState.triggerActualTeeth = 36; // should be 34 if not hacking toothcounter 
          }
          triggerRoverMEMSCommon(curTime, current, decoderState, page2, page4);
        }
        else if(decoderState.toothCurrentCount > decoderState.triggerActualTeeth+1) // no patterns match after a rotation when we only need 32 teeth to match, we've lost sync
        {
          decoderState.decoderStatus.syncStatus = SyncStatus::None;
          current.syncLossCounter++;              
        }
      }
    }
    
    decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
    decoderState.toothLastToothTime = curTime;

    //NEW IGNITION MODE
    if( (page2.perToothIgn == true) && (current.rotationStatus!=EngineRotationStatus::Cranking) ) 
    {  
      int16_t crankAngle = ( (decoderState.toothCurrentCount-1) * decoderState.triggerToothAngle ) + page4.triggerAngle;
      if( (page4.sparkMode == IGN_MODE_SEQUENTIAL) && (decoderState.revolutionOne == true))
      { crankAngle += 360; checkPerToothTiming(current, decoderState, page4, crankAngle, (page4.triggerTeeth + decoderState.toothCurrentCount)); }
      else
      { checkPerToothTiming(current, decoderState, page4, crankAngle, decoderState.toothCurrentCount); }
    }     
  }
}

void triggerSecondary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config4 &page4, const config6 &page6, const config10 &page10)
{
  decoderState.curGap2 = curTime - decoderState.toothLastSecToothTime;

  //Safety check for initial startup
  if( (decoderState.toothLastSecToothTime == 0) )
  { 
    decoderState.targetGap2 = decoderState.curGap * 2;
    decoderState.curGap2 = 0; 
    decoderState.toothLastSecToothTime = curTime;
  }
  
  if ( decoderState.curGap2 >= decoderState.triggerSecFilterTime )
  {
    decoderState.secondaryToothCount++;
    decoderState.toothLastSecToothTime = curTime;

    //Record the VVT Angle
    if( page6.vvtEnabled > 0 &&
        ( (page4.trigPatternSec == SEC_TRIGGER_SINGLE) || 
          (page4.trigPatternSec == SEC_TRIGGER_5_3_2 && decoderState.secondaryToothCount == 6 ) ) )
    {
      int16_t curAngle = normalize((int16_t)0, (int16_t)360, current.decoder.getCrankAngle());

      curAngle -= page4.triggerAngle; //Value at TDC
      if( page6.vvtMode == VVT_MODE_CLOSED_LOOP ) { curAngle -= page10.vvtCLMinAng; }

      current.vvt1.angle = curAngle;
    }

    if(page4.trigPatternSec == SEC_TRIGGER_SINGLE)
    {
      //Standard single tooth cam trigger
      decoderState.revolutionOne = true;
      decoderState.triggerSecFilterTime = decoderState.curGap2 >> 1; //Next secondary filter is half the current gap        
    }
    else if (page4.trigPatternSec == SEC_TRIGGER_5_3_2) // multi tooth cam 
    {
      if (decoderState.curGap2 < decoderState.targetGap2) // ie normal tooth sized gap, not a single or double gap
      {
        decoderState.triggerSecFilterTime = decoderState.curGap2 >> 1; //Next secondary filter is half the current gap        
        decoderState.targetGap2 = (3 * (decoderState.curGap2)) >> 1;  //Multiply by 1.5 (Checks for a gap 1.5x greater than the last one) (Uses bitshift to multiply by 3 then divide by 2. Much faster than multiplying by 1.5)        
      }
      else
      {  
        // gap either single or double - nb remember we've got the tooth after the gap, so on the 5 tooth pattern we'll see here tooth 6
        if(decoderState.secondaryToothCount == 6)
        {
          // if we've got the tooth after the gap from reading 5 teeth we're on cycle 360-720 & tooth 18-36
          decoderState.revolutionOne = false;
          if(decoderState.toothCurrentCount < 19)
          {
            decoderState.toothCurrentCount += 18;
          }
        }
        else if (decoderState.secondaryToothCount == 4)
        {
          // we've got the tooth after the gap from reading 3 teeth we're on cycle 0-360 & tooth 1-18
          decoderState.revolutionOne = true;
          if(decoderState.toothCurrentCount > 17)
          {
            decoderState.toothCurrentCount -= 18;
          }
        }
        else if (decoderState.secondaryToothCount == 3)
        {
          // if we've got the tooth after the gap from reading 2 teeth we're on cycle 0-360 & tooth 18-36
          decoderState.revolutionOne = true;
          if(decoderState.toothCurrentCount < 19)
          {
            decoderState.toothCurrentCount += 18;
          }
        }
        decoderState.secondaryToothCount = 1; // as we've had a gap we need to reset to this being the first tooth after the gap
      }
    }
  } //Trigger filter
}

uint32_t getRevolutionTime(const statuses &current, const shared::state_t &decoderState, const config4 &page4)
{
  uint32_t revolutionTime = 0;

  if( current.RPM < current.crankRPM)
  {
    if( (decoderState.toothCurrentCount != (unsigned int) decoderState.toothAngles[SKIP_TOOTH1]) && 
        (decoderState.toothCurrentCount != (unsigned int) decoderState.toothAngles[SKIP_TOOTH2]) && 
        (decoderState.toothCurrentCount != (unsigned int) decoderState.toothAngles[SKIP_TOOTH3]) && 
        (decoderState.toothCurrentCount != (unsigned int) decoderState.toothAngles[SKIP_TOOTH4]) )
    { revolutionTime = crankingGetRevolutionTime(current, decoderState, page4, 36, CRANK_SPEED); }
    else
    { revolutionTime = shared::publishedRevolutionTime(current); } //Can't do per tooth calculation as the missing tooth messes the calculation
  }
  else
  { revolutionTime = stdGetRevolutionTime(current, decoderState, CRANK_SPEED); }
  return revolutionTime;
}

static int16_t __attribute__((noinline)) calcEndTooth_RoverMEMS(const IgnitionSchedule &schedule, uint8_t toothAdder, const config4 &page4) {
  int16_t channelToothAdder = 36 + (int16_t)toothAdder;
  int16_t tempIgnitionEndTooth = ( (schedule.dischargeAngle - page4.triggerAngle) / (int16_t)(10) ) - 1;
  if(tempIgnitionEndTooth > channelToothAdder) { tempIgnitionEndTooth -= channelToothAdder; }
  if(tempIgnitionEndTooth <= 0) { tempIgnitionEndTooth += channelToothAdder; }
  if(tempIgnitionEndTooth > channelToothAdder) { tempIgnitionEndTooth = channelToothAdder; }
 return tempIgnitionEndTooth;
}

void setEndTeeth(shared::state_t &decoderState, const config4 &page4)
{
  int16_t tempIgnitionEndTooth[5]; // cheating with the array - location 1 is spark 1, location 0 not used.   
  int16_t toothAdder = 0;

  if( (page4.sparkMode == IGN_MODE_SEQUENTIAL) && (page4.TrigSpeed == CRANK_SPEED) ) { toothAdder = 36; }

  tempIgnitionEndTooth[1] = calcEndTooth_RoverMEMS(ignitionSchedule1, toothAdder, page4);
#if (IGN_CHANNELS >= 2)
  tempIgnitionEndTooth[2] = calcEndTooth_RoverMEMS(ignitionSchedule2, toothAdder, page4);
#endif
#if (IGN_CHANNELS >= 3)
  tempIgnitionEndTooth[3] = calcEndTooth_RoverMEMS(ignitionSchedule3, toothAdder, page4);
#endif
#if (IGN_CHANNELS >= 4)
  tempIgnitionEndTooth[4] = calcEndTooth_RoverMEMS(ignitionSchedule4, toothAdder, page4);
#endif

  // take into account the missing teeth on the Rover flywheels
  int tempCount=0;
 
  if(page4.sparkMode == IGN_MODE_SEQUENTIAL)
  {
    // check the calculated trigger tooth exists, if it doesn't use the previous tooth
    // nb the decoderState.toothAngles[x] holds the tooth after the gap, hence the '-1' to see if it matches a gap
 
    for(tempCount=1;tempCount <5;tempCount++)
    {
      if(tempIgnitionEndTooth[tempCount] == (decoderState.toothAngles[1]) || tempIgnitionEndTooth[tempCount] == (decoderState.toothAngles[2]) ||
         tempIgnitionEndTooth[tempCount] == (decoderState.toothAngles[3]) || tempIgnitionEndTooth[tempCount] == (decoderState.toothAngles[4]) ||
         tempIgnitionEndTooth[tempCount] == (36 + decoderState.toothAngles[1]) || tempIgnitionEndTooth[tempCount] == (36 + decoderState.toothAngles[2]) ||
         tempIgnitionEndTooth[tempCount] == (36 + decoderState.toothAngles[3]) || tempIgnitionEndTooth[tempCount] == (36 + decoderState.toothAngles[4])  )
      { tempIgnitionEndTooth[tempCount]--; }
    }
  }
  else
  {
    for(tempCount=1;tempCount<5;tempCount++)
    {
      if(tempIgnitionEndTooth[tempCount] == (decoderState.toothAngles[1]) || tempIgnitionEndTooth[tempCount] == (decoderState.toothAngles[2]) )
      { tempIgnitionEndTooth[tempCount]--; }  
    }
  }
  
  
  decoderState.ignitionEndTeeth[0] = tempIgnitionEndTooth[1];  
  decoderState.ignitionEndTeeth[1] = tempIgnitionEndTooth[2];
  decoderState.ignitionEndTeeth[2] = tempIgnitionEndTooth[3];
  decoderState.ignitionEndTeeth[3] = tempIgnitionEndTooth[4];
}

}
}