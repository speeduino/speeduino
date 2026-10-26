#include "miata9905.h"
#include "shared/perToothIgnition.h"
#include "scheduledIO_ign.h"

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

}
}