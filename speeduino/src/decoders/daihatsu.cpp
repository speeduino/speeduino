#include "daihatsu.h"
#include "scheduledIO_ign.h"


namespace decoders {

namespace daihatsu {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(const config2 &page2)
{
  shared::state_t state = {};

  state.triggerActualTeeth = page2.nCylinders + 1;
  state.triggerToothAngle = 720 / state.triggerActualTeeth; //The number of degrees that passes from tooth to tooth
  state.triggerFilterTime = MICROS_PER_MIN / MAX_RPM / page2.nCylinders; // Minimum time required between teeth
  state.triggerFilterTime = state.triggerFilterTime / 2; //Safety margin
  state.decoderFeatures.supportsSequential = true;

  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/90U) * state.triggerToothAngle)*4U;//Minimum 90rpm. (1851uS is the time per degree at 90rpm). This uses 90rpm rather than 50rpm due to the potentially very high stall time on a 4 cylinder if we wait that long.

  if(page2.nCylinders == 3)
  {
    state.toothAngles[0] = 0; //tooth #1
    state.toothAngles[1] = 30; //tooth #2 (Extra tooth)
    state.toothAngles[2] = 240; //tooth #3
    state.toothAngles[3] = 480; //tooth #4
  }
  else
  {
    //Should be 4 cylinders here
    state.toothAngles[0] = 0; //tooth #1
    state.toothAngles[1] = 30; //tooth #2 (Extra tooth)
    state.toothAngles[2] = 180; //tooth #3
    state.toothAngles[3] = 360; //tooth #4
    state.toothAngles[4] = 540; //tooth #5
  }

  return state;
}

void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
  decoderState.curGap = curTime - decoderState.toothLastToothTime;

  decoderState.toothSystemCount++;
  decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)

  if (decoderState.decoderStatus.syncStatus==SyncStatus::Full)
  {
    if( (decoderState.toothCurrentCount == decoderState.triggerActualTeeth) ) //Check if we're back to the beginning of a revolution
    {
        decoderState.toothCurrentCount = 1; //Reset the counter
        decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
        decoderState.toothOneTime = curTime;
        decoderState.decoderStatus.syncStatus = SyncStatus::Full;
        current.startRevolutions++; //Counter

        //Need to set a special filter time for the next tooth
        decoderState.triggerFilterTime = 20; //Fix this later
    }
    else
    {
      decoderState.toothCurrentCount++; //Increment the tooth counter
      decoderState.setFilter(decoderState.curGap, page4); //Recalc the new filter value
    }

    // LCOV_EXCL_START
    // Not testable ATM
    if ( page4.ignCranklock && (current.rotationStatus==EngineRotationStatus::Cranking) )
    {
      //This locks the cranking timing to 0 degrees BTDC (All the triggers allow for)
      if(decoderState.toothCurrentCount == 1) { endCoilCharge(1U); }
      else if(decoderState.toothCurrentCount == 2) { endCoilCharge(2U); }
      else if(decoderState.toothCurrentCount == 3) { endCoilCharge(3U); }
      else if(decoderState.toothCurrentCount == 4) { endCoilCharge(4U); }
    }
    // LCOV_EXCL_STOP
  }
  else //NO SYNC
  {
    //
    if(decoderState.toothSystemCount >= 3) //Need to have seen at least 3 teeth to determine SYNC
    {
      unsigned long targetTime;
      //We need to try and find the extra tooth (#2) which is located 30 degrees after tooth #1
      //Aim for tooth times less than about 60 degrees
      if(page2.nCylinders == 3)
      {
        targetTime = (decoderState.toothLastToothTime -  decoderState.toothLastMinusOneToothTime) / 4; //Teeth are 240 degrees apart for 3 cylinder. 240/4 = 60
      }
      else
      {
        targetTime = ((decoderState.toothLastToothTime -  decoderState.toothLastMinusOneToothTime) * 3) / 8; //Teeth are 180 degrees apart for 4 cylinder. (180*3)/8 = 67
      }
      if(decoderState.curGap < targetTime)
      {
        //Means we're on the extra tooth here
        decoderState.toothCurrentCount = 2; //Reset the counter
        decoderState.decoderStatus.syncStatus = SyncStatus::Full;
        decoderState.triggerFilterTime = targetTime; //Lazy, but it works
      }
    }
  }

  decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
  decoderState.toothLastToothTime = curTime;
}


}
}