#include "subaru6_7.h"
#include "shared/perToothIgnition.h"
#include "shared/rev_time_calcs.h"
#include "scheduledIO_ign.h"
#include "shared/crank_angle_calculator.h"

namespace decoders {

namespace subaru67 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(void)
{
  shared::state_t state = {};

  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 360UL)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  state.triggerSecFilterTime = 0;
  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.supportsPerToothIgnition = true;
  state.toothCurrentCount = 1;
  state.triggerToothAngle = 2;
  state.decoderStatus.toothAngleIsCorrect = false;
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * 93U); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

  state.toothAngles[0] = 710; //tooth #1
  state.toothAngles[1] = 83; //tooth #2
  state.toothAngles[2] = 115; //tooth #3
  state.toothAngles[3] = 170; //tooth #4
  state.toothAngles[4] = state.toothAngles[1] + 180;
  state.toothAngles[5] = state.toothAngles[2] + 180;
  state.toothAngles[6] = state.toothAngles[3] + 180;
  state.toothAngles[7] = state.toothAngles[1] + 360;
  state.toothAngles[8] = state.toothAngles[2] + 360;
  state.toothAngles[9] = state.toothAngles[3] + 360;
  state.toothAngles[10] = state.toothAngles[1] + 540;
  state.toothAngles[11] = state.toothAngles[2] + 540;

  return state;
}

void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
  decoderState.curGap = curTime - decoderState.toothLastToothTime;
  if ( decoderState.curGap < decoderState.triggerFilterTime ) 
  { return; }

  decoderState.toothCurrentCount++; //Increment the tooth counter
  decoderState.toothSystemCount++; //Used to count the number of primary pulses that have occurred since the last secondary. Is part of the noise filtering system.
  decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)

  decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
  decoderState.toothLastToothTime = curTime;

 
  if(decoderState.toothCurrentCount > 13) //can't have more than 12 teeth so have lost sync 
  {
    decoderState.toothCurrentCount = 0; 
    decoderState.decoderStatus.syncStatus = SyncStatus::None; 
    current.syncLossCounter++;
  } 

  //Sync is determined by counting the number of cam teeth that have passed between the crank teeth
  switch(decoderState.secondaryToothCount)
  {
    case 0:
      //If no teeth have passed, we can't do anything
      break;

    case 1:
      //Can't do anything with a single pulse from the cam either (We need either 2 or 3 pulses)
      if(decoderState.toothCurrentCount == 5 || decoderState.toothCurrentCount == 11)
      { decoderState.decoderStatus.syncStatus = SyncStatus::Full; }
      else
      { 
        decoderState.decoderStatus.syncStatus = SyncStatus::None; 
        current.syncLossCounter++;     
        decoderState.toothCurrentCount = 5; // we don't know if its 5 or 11, but we'll be right 50% of the time and speed up getting sync 50%
      }
      decoderState.secondaryToothCount = 0;
      break;

    case 2:
      if (decoderState.toothCurrentCount == 8)  
      {  decoderState.decoderStatus.syncStatus = SyncStatus::Full; }
      else
      { 
        decoderState.decoderStatus.syncStatus = SyncStatus::None;
        current.syncLossCounter++;
        decoderState.toothCurrentCount = 8;
      }          
      decoderState.secondaryToothCount = 0;
      break;

    case 3:      
      if( decoderState.toothCurrentCount == 2)
      {  decoderState.decoderStatus.syncStatus = SyncStatus::Full; }
      else
      {  
        decoderState.decoderStatus.syncStatus = SyncStatus::None; 
        current.syncLossCounter++;
        decoderState.toothCurrentCount = 2;
      }
      decoderState.secondaryToothCount = 0;
      break;

    default:
      //Almost certainly due to noise or cranking stop/start
      decoderState.decoderStatus.syncStatus = SyncStatus::None;
      decoderState.decoderStatus.toothAngleIsCorrect = false;
      current.syncLossCounter++;
      decoderState.secondaryToothCount = 0;
      break;
  }

  //Check sync again
  if ( decoderState.decoderStatus.syncStatus==SyncStatus::Full )
  {
    //Locked timing during cranking. This is fixed at 10* BTDC.
    // LCOV_EXCL_START
    // Not testable ATM
    if ( (current.rotationStatus==EngineRotationStatus::Cranking) && page4.ignCranklock)
    {
      if( (decoderState.toothCurrentCount == 1) || (decoderState.toothCurrentCount == 7) ) { endCoilCharge(1U); endCoilCharge(3U); }
      else if( (decoderState.toothCurrentCount == 4) || (decoderState.toothCurrentCount == 10) ) { endCoilCharge(2U); endCoilCharge(4U); }
    }
    // LCOV_EXCL_STOP

    if ( decoderState.toothCurrentCount > 12 ) // done 720 degrees so increment rotation
    {
      decoderState.toothCurrentCount = 1;
      decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
      decoderState.toothOneTime = curTime;
      current.startRevolutions++; //Counter
    }

    //Set the last angle between teeth for better calc accuracy
    if(decoderState.toothCurrentCount == 1) { decoderState.triggerToothAngle = 55; } //Special case for tooth 1
    else if(decoderState.toothCurrentCount == 2) { decoderState.triggerToothAngle = 93; } //Special case for tooth 2
    else { decoderState.triggerToothAngle = decoderState.toothAngles[(decoderState.toothCurrentCount-1)] - decoderState.toothAngles[(decoderState.toothCurrentCount-2)]; }
    decoderState.decoderStatus.toothAngleIsCorrect = true;


    //NEW IGNITION MODE
    if( (page2.perToothIgn == true) && (current.rotationStatus!=EngineRotationStatus::Cranking) ) 
    {
      int16_t crankAngle = decoderState.toothAngles[(decoderState.toothCurrentCount - 1)] + page4.triggerAngle;
      if( (page4.sparkMode != IGN_MODE_SEQUENTIAL) )
      {
        crankAngle = decoderState.toothAngles[(decoderState.toothCurrentCount-1)];

        //Handle non-sequential tooth counts 
        if( (page4.sparkMode != IGN_MODE_SEQUENTIAL) && (decoderState.toothCurrentCount > 6) ) { checkPerToothTiming(current, decoderState, page4, crankAngle, (decoderState.toothCurrentCount-6) ); }
        else { checkPerToothTiming(current, decoderState, page4, crankAngle, decoderState.toothCurrentCount); }
      }
      else{ checkPerToothTiming(current, decoderState, page4, crankAngle, decoderState.toothCurrentCount); }
    }
  }
}

void triggerSecondary(uint32_t curTime, statuses &current, shared::state_t &decoderState)
{
  if( ((decoderState.toothSystemCount == 0) || (decoderState.toothSystemCount == 3)) )
  {
    decoderState.curGap2 = curTime - decoderState.toothLastSecToothTime;
    
    if ( decoderState.curGap2 > decoderState.triggerSecFilterTime ) 
    {
      decoderState.toothLastSecToothTime = curTime;
      decoderState.secondaryToothCount++;
      decoderState.toothSystemCount = 0;
      
      if(decoderState.secondaryToothCount > 1)
      {
        //Set filter at 25% of the current speed
        //Note that this can only be set on the 2nd or 3rd cam tooth in each set. 
        decoderState.triggerSecFilterTime = decoderState.curGap2 >> 2;
      }
      else { decoderState.triggerSecFilterTime = 0; } //Filter disabled  

    }
  }
  else
  {
    //Sanity check
    if(decoderState.toothSystemCount > 3)
    { 
      decoderState.toothSystemCount = 0; 
      decoderState.secondaryToothCount = 1;
      decoderState.decoderStatus.syncStatus = SyncStatus::None; // impossible to have more than 3 crank teeth between cam teeth - must have noise but can't have sync
      current.syncLossCounter++;
    }
    decoderState.secondaryToothCount = 0;
  }
}

uint32_t getRevolutionTime(const statuses &current, const shared::state_t &decoderState)
{
  auto sample = atomicToothSpeedSample(current, decoderState);
  if (sample.startRevolutions==0U) { return 0U; } //No speed until the first revolution has completed
  //As the tooth count is over 720 degrees
  return stdGetRevolutionTime(current, decoderState, CAM_SPEED);
}

int16_t getCrankAngle(uint32_t currMicros, const shared::state_t &decoderState, const config4 &page4)
{
  int16_t crankAngle = 0;
  if( decoderState.decoderStatus.syncStatus==SyncStatus::Full )
  {
    auto data = atomic_copy(decoderState.toothLastToothTime, decoderState.toothLastMinusOneToothTime, decoderState.revolutionOne, decoderState.toothCurrentCount, decoderState.triggerToothAngle, decoderState.decoderStatus.toothAngleIsCorrect);
    crankAngle = decoders::shared::lookup_crank_angle_calculator_tooth_interval_t(data).calculateCrankAngle(currMicros, decoderState.toothAngles, page4);
  }
  return decoders::shared::clampCrankAngle(crankAngle);
}

void setEndTeeth(const statuses &current, shared::state_t &decoderState, const config4 &page4)
{
  if(page4.sparkMode == IGN_MODE_SEQUENTIAL)
  {
    if(current.advance >= 10 ) 
    { 
      decoderState.ignitionEndTeeth[0] = 12;
      decoderState.ignitionEndTeeth[1] = 3;
      decoderState.ignitionEndTeeth[2] = 6;
      decoderState.ignitionEndTeeth[3] = 9;
    }
    else 
    { 
      decoderState.ignitionEndTeeth[0] = 1;
      decoderState.ignitionEndTeeth[1] = 4;
      decoderState.ignitionEndTeeth[2] = 7;
      decoderState.ignitionEndTeeth[3] = 10;
    }
  }
  else    
  {
    if(current.advance >= 10 ) 
    { 
      decoderState.ignitionEndTeeth[0] = 6;
      decoderState.ignitionEndTeeth[1] = 3;
    }
    else 
    { 
      decoderState.ignitionEndTeeth[0] = 1;
      decoderState.ignitionEndTeeth[1] = 4;
    }
  }
}

}

}