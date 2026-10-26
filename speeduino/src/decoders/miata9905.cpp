#include "miata9905.h"
#include "shared/perToothIgnition.h"
#include "scheduledIO_ign.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"

namespace decoders {

namespace miata9905 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(uint32_t curMicros, const statuses &current)
{
  shared::state_t state = {};

  state.triggerToothAngle = 90; //The number of degrees that passes from tooth to tooth (primary)
  state.toothCurrentCount = 99; //Fake tooth count represents no sync
  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.supportsPerToothIgnition = true;
  state.triggerActualTeeth = 8;

  if(current.initialisationComplete == false) { state.toothLastToothTime = curMicros; } //Set a startup value here to avoid filter errors when starting. This MUST have the initial check to prevent the fuel pump just staying on all the time

  //Note that these angles are for every rising and falling edge
  state.toothAngles[0] = 710; //
  state.toothAngles[1] = 100; //First crank pulse after the SINGLE cam pulse
  state.toothAngles[2] = 170; //
  state.toothAngles[3] = 280; //
  state.toothAngles[4] = 350; //
  state.toothAngles[5] = 460; //First crank pulse AFTER the DOUBLE cam pulse
  state.toothAngles[6] = 530; //
  state.toothAngles[7] = 640; //

  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  state.triggerFilterTime = 1500; //10000 rpm, assuming we're triggering on both edges off the crank tooth.
  state.triggerSecFilterTime = 0; //Need to figure out something better for this
  state.decoderFeatures.hasFixedCrankingTiming = true;
  state.decoderStatus.toothAngleIsCorrect = true;

  return state;
}

void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
  decoderState.curGap = curTime - decoderState.toothLastToothTime;
  if ( (decoderState.curGap >= decoderState.triggerFilterTime) || (current.startRevolutions == 0) )
  {
    decoderState.toothCurrentCount++;
    decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)
    if( (decoderState.toothCurrentCount == (decoderState.triggerActualTeeth + 1)) )
    {
       decoderState.toothCurrentCount = 1; //Reset the counter
       decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
       decoderState.toothOneTime = curTime;
       current.startRevolutions++; //Counter
    }
    else
    {
      if( (decoderState.decoderStatus.syncStatus!=SyncStatus::Full) || (page4.useResync == true) )
      {
        if(decoderState.secondaryToothCount == 2)
        {
          decoderState.toothCurrentCount = 6;
          decoderState.decoderStatus.syncStatus = SyncStatus::Full;
        }
      }
    }

    if (decoderState.decoderStatus.syncStatus==SyncStatus::Full)
    {

      //Whilst this is an uneven tooth pattern, if the specific angle between the last 2 teeth is specified, 1st deriv prediction can be used
      if( (page4.triggerFilter == 1) || (current.RPM < 1400) )
      {
        //Lite filter
        if( (decoderState.toothCurrentCount == 1) || (decoderState.toothCurrentCount == 3) || (decoderState.toothCurrentCount == 5) || (decoderState.toothCurrentCount == 7) ) { decoderState.triggerToothAngle = 70; decoderState.triggerFilterTime = decoderState.curGap; } //Trigger filter is set to whatever time it took to do 70 degrees (Next trigger is 110 degrees away)
        else { decoderState.triggerToothAngle = 110; decoderState.triggerFilterTime = rshift<3>((uint32_t)(decoderState.curGap * 3UL)); } //Trigger filter is set to (110*3)/8=41.25=41 degrees (Next trigger is 70 degrees away).
      }
      else if(page4.triggerFilter == 2)
      {
        //Medium filter level
        if( (decoderState.toothCurrentCount == 1) || (decoderState.toothCurrentCount == 3) || (decoderState.toothCurrentCount == 5) || (decoderState.toothCurrentCount == 7) ) { decoderState.triggerToothAngle = 70; decoderState.triggerFilterTime = (decoderState.curGap * 5) >> 2 ; } //87.5 degrees with a target of 110
        else { decoderState.triggerToothAngle = 110; decoderState.triggerFilterTime = (decoderState.curGap >> 1); } //55 degrees with a target of 70
      }
      else if (page4.triggerFilter == 3)
      {
        //Aggressive filter level
        if( (decoderState.toothCurrentCount == 1) || (decoderState.toothCurrentCount == 3) || (decoderState.toothCurrentCount == 5) || (decoderState.toothCurrentCount == 7) ) { decoderState.triggerToothAngle = 70; decoderState.triggerFilterTime = rshift<3>((uint32_t)(decoderState.curGap * 11UL)) ; } //96.26 degrees with a target of 110
        else { decoderState.triggerToothAngle = 110; decoderState.triggerFilterTime = rshift<5>((uint32_t)(decoderState.curGap * 9UL)); } //61.87 degrees with a target of 70
      }
      else if (page4.triggerFilter == 0)
      {
        //trigger filter is turned off.
        decoderState.triggerFilterTime = 0;
        decoderState.triggerSecFilterTime = 0;
        if( (decoderState.toothCurrentCount == 1) || (decoderState.toothCurrentCount == 3) || (decoderState.toothCurrentCount == 5) || (decoderState.toothCurrentCount == 7) ) { decoderState.triggerToothAngle = 70; } //96.26 degrees with a target of 110
        else { decoderState.triggerToothAngle = 110; }
      }

      //EXPERIMENTAL!
      //New ignition mode is ONLY available on 9905 when the trigger angle is set to the stock value of 0.
      if(    (page2.perToothIgn == true) 
          && (page4.triggerAngle == 0) 
          && (current.advance > 0) )
      {
        int16_t crankAngle = decoderState.toothAngles[(decoderState.toothCurrentCount-1)];

        //Handle non-sequential tooth counts 
        if( (page4.sparkMode != IGN_MODE_SEQUENTIAL) && (decoderState.toothCurrentCount > page2.nCylinders) ) { checkPerToothTiming(current, decoderState, page4, crankAngle, (decoderState.toothCurrentCount-page2.nCylinders) ); }
        else { checkPerToothTiming(current, decoderState, page4, crankAngle, decoderState.toothCurrentCount); }
      }
    } //Has sync

    decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
    decoderState.toothLastToothTime = curTime;

    // LCOV_EXCL_START
    // No way to test this ATM
    if ( (current.RPM < (current.crankRPM + 30)) && (page4.ignCranklock) ) //The +30 here is a safety margin. When switching from fixed timing to normal, there can be a situation where a pulse started when fixed and ending when in normal mode causes problems. This prevents that.
    {
      if( (decoderState.toothCurrentCount == 1) || (decoderState.toothCurrentCount == 5) ) { endCoilCharge(1U); endCoilCharge(3U); }
      else if( (decoderState.toothCurrentCount == 3) || (decoderState.toothCurrentCount == 7) ) { endCoilCharge(2U); endCoilCharge(4U); }
    }
    // LCOV_EXCL_STOP

    decoderState.secondaryToothCount = 0;
  } //Trigger filter
}

void triggerSecondary(uint32_t curTime, const statuses &current, shared::state_t &decoderState)
{
  decoderState.curGap2 = curTime - decoderState.toothLastSecToothTime;

  if((current.rotationStatus==EngineRotationStatus::Cranking) || (decoderState.decoderStatus.syncStatus!=SyncStatus::Full) )
  {
    decoderState.triggerFilterTime = 1500; //If this is removed, can have trouble getting sync again after the engine is turned off (but ECU not reset).
  }

  if ( decoderState.curGap2 >= decoderState.triggerSecFilterTime )
  {
    decoderState.toothLastSecToothTime = curTime;
    decoderState.lastGap = decoderState.curGap2;
    decoderState.secondaryToothCount++;

    //TODO Add some secondary filtering here

    //Record the VVT tooth time
    if( (decoderState.toothCurrentCount == 1) && (curTime > decoderState.toothLastToothTime) )
    {
      decoderState.lastVVTtime = curTime - decoderState.toothLastToothTime;
    }
  }
}

uint32_t getRevolutionTime(const statuses &current, shared::state_t &decoderState)
{
  uint32_t revolutionTime = 0U;

  /*
  During cranking, revolution time is calculated 4 times per revolution, once for each tooth on the crank signal.
  Because these signals aren't even (Alternating 110 and 70 degrees), this needs a special function
  */
  auto sample = atomicToothSpeedSample(current, decoderState);
  if ( (current.RPM < current.crankRPM) && (sample.syncStatus==SyncStatus::Full) )
  {
    if (revolutionTimeFromLastTooth(sample, revolutionTime)==true)
    {
      decoderState.MAX_STALL_TIME = 366667UL; // 50RPM
      return revolutionTime;
    }
    return shared::publishedRevolutionTime(current);
  }

  revolutionTime = stdGetRevolutionTime(current, decoderState, CAM_SPEED);
  decoderState.MAX_STALL_TIME = revolutionTime << 1; //Set the stall time to be twice the current RPM. This is a safe figure as there should be no single revolution where this changes more than this
  if (decoderState.MAX_STALL_TIME < 366667UL) { decoderState.MAX_STALL_TIME = 366667UL; } //Check for 50rpm minimum
  return revolutionTime;
}

int16_t getCrankAngle(uint32_t currMicros, const shared::state_t &decoderState, const config4 &page4)
{
  return decoders::shared::clampCrankAngle(decoders::shared::atomic_make_lookup_caa(decoderState).calculateCrankAngle(currMicros, decoderState.toothAngles, page4));
}

void setEndTeeth(const statuses &current, shared::state_t &decoderState, const config4 &page4)
{
  if(page4.sparkMode == IGN_MODE_SEQUENTIAL) 
  { 
    if(current.advance >= 10)
    {
      decoderState.ignitionEndTeeth[0] = 8;
      decoderState.ignitionEndTeeth[1] = 2;
      decoderState.ignitionEndTeeth[2] = 4;
      decoderState.ignitionEndTeeth[3] = 6;
    }
    else if (current.advance > 0)
    {
      decoderState.ignitionEndTeeth[0] = 1;
      decoderState.ignitionEndTeeth[1] = 3;
      decoderState.ignitionEndTeeth[2] = 5;
      decoderState.ignitionEndTeeth[3] = 7;
    }
  }
  else
  {
    if(current.advance >= 10)
    {
      decoderState.ignitionEndTeeth[0] = 4;
      decoderState.ignitionEndTeeth[1] = 2;
      decoderState.ignitionEndTeeth[2] = 4; //Not used
      decoderState.ignitionEndTeeth[3] = 2; //Not used
    }
    else if(current.advance > 0)
    {
      decoderState.ignitionEndTeeth[0] = 1;
      decoderState.ignitionEndTeeth[1] = 3;
      decoderState.ignitionEndTeeth[2] = 1; //Not used
      decoderState.ignitionEndTeeth[3] = 3; //Not used
    }
  }
}

}
}