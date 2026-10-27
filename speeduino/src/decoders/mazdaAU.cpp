#include "mazdaau.h"
#include "scheduledIO_ign.h"

namespace decoders {

namespace mazdaau {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(void)
{
  shared::state_t state;

  state.triggerToothAngle = 108; //The number of degrees that passes from tooth to tooth (primary). This is the maximum gap
  state.toothCurrentCount = 99; //Fake tooth count represents no sync

  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.hasFixedCrankingTiming = true;

  state.toothAngles[0] = 348; //tooth #1
  state.toothAngles[1] = 96; //tooth #2
  state.toothAngles[2] = 168; //tooth #3
  state.toothAngles[3] = 276; //tooth #4

  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  state.triggerFilterTime = 1500; //10000 rpm, assuming we're triggering on both edges off the crank tooth.
  state.triggerSecFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 2U)) / 2U; //Same as above, but fixed at 2 teeth on the secondary input and divided by 2 (for cam speed)

  return state;
}

void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t & decoderState, const config4 &page4)
{
  decoderState.curGap = curTime - decoderState.toothLastToothTime;
  if ( decoderState.curGap >= decoderState.triggerFilterTime )
  {
    decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)

    decoderState.toothCurrentCount++;
    if( (decoderState.toothCurrentCount == 1) || (decoderState.toothCurrentCount == 5) ) //Trigger is on CHANGE, hence 4 pulses = 1 crank rev
    {
       decoderState.toothCurrentCount = 1; //Reset the counter
       decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
       decoderState.toothOneTime = curTime;
       decoderState.decoderStatus.syncStatus = SyncStatus::Full;
       current.startRevolutions++; //Counter
    }

    if (decoderState.decoderStatus.syncStatus==SyncStatus::Full)
    {
      // LCOV_EXCL_START
      // Not testable ATM
      // Locked cranking timing is available, fixed at 12* BTDC
      if ( (current.rotationStatus==EngineRotationStatus::Cranking) && page4.ignCranklock )
      {
        if( decoderState.toothCurrentCount == 1 ) { endCoilCharge(1U); }
        else if( decoderState.toothCurrentCount == 3 ) { endCoilCharge(2U); }
      }
      // LCOV_EXCL_STOP

      //Whilst this is an uneven tooth pattern, if the specific angle between the last 2 teeth is specified, 1st deriv prediction can be used
      if( (decoderState.toothCurrentCount == 1) || (decoderState.toothCurrentCount == 3) ) { decoderState.triggerToothAngle = 72; decoderState.triggerFilterTime = decoderState.curGap; } //Trigger filter is set to whatever time it took to do 72 degrees (Next trigger is 108 degrees away)
      else { decoderState.triggerToothAngle = 108; decoderState.triggerFilterTime = rshift<3>((uint32_t)(decoderState.curGap * 3UL)); } //Trigger filter is set to (108*3)/8=40 degrees (Next trigger is 70 degrees away).

      decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
      decoderState.toothLastToothTime = curTime;
    } //Has sync
  } //Filter time
}

void triggerSecondary(uint32_t curTime, shared::state_t & decoderState)
{
  decoderState.lastGap = decoderState.curGap2;
  decoderState.curGap2 = curTime - decoderState.toothLastSecToothTime;
  decoderState.toothLastSecToothTime = curTime;

  if(decoderState.decoderStatus.syncStatus!=SyncStatus::Full)
  {
    //we find sync by looking for the 2 teeth that are close together. The next crank tooth after that is the one we're looking for.
    //For the sake of this decoder, the lone cam tooth will be designated #1
    if(decoderState.secondaryToothCount == 2)
    {
      decoderState.toothCurrentCount = 1;
      decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    }
    else
    {
      decoderState.triggerFilterTime = 1500; //In case the engine has been running and then lost sync.
      decoderState.targetGap = (decoderState.lastGap) >> 1; //The target gap is set at half the last tooth gap
      if ( decoderState.curGap2 < decoderState.targetGap) //If the gap between this tooth and the last one is less than half of the previous gap, then we are very likely at the extra (3rd) tooth on the cam). This tooth is located at 421 crank degrees (aka 61 degrees) and therefore the last crank tooth seen was number 1 (At 350 degrees)
      {
        decoderState.secondaryToothCount = 2;
      }
    }
    decoderState.secondaryToothCount++;
  }
}

}
}