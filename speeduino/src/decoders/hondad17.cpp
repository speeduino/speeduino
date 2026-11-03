#include "hondad17.h"
#include "shared/crank_angle_calculator.h"

namespace decoders {

namespace hondad17 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(void)
{
  shared::state_t state;
  state.triggerToothAngle = 360 / 12; //The number of degrees that passes from tooth to tooth
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

  return state;
}

void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState)
{
  decoderState.lastGap = decoderState.curGap;
  decoderState.curGap = curTime - decoderState.toothLastToothTime;
  decoderState.toothCurrentCount++; //Increment the tooth counter

  decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)

  //
  if( (decoderState.toothCurrentCount == 13) && (decoderState.decoderStatus.syncStatus==SyncStatus::Full) )
  {
    decoderState.toothCurrentCount = 0;
  }
  else if( (decoderState.toothCurrentCount == 1) && (decoderState.decoderStatus.syncStatus==SyncStatus::Full) )
  {
    decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
    decoderState.toothOneTime = curTime;
    current.startRevolutions++; //Counter

    decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
    decoderState.toothLastToothTime = curTime;
  }
  else
  {
    //13th tooth
    decoderState.targetGap = (decoderState.lastGap) >> 1; //The target gap is set at half the last tooth gap
    if ( decoderState.curGap < decoderState.targetGap) //If the gap between this tooth and the last one is less than half of the previous gap, then we are very likely at the magical 13th tooth
    {
      decoderState.toothCurrentCount = 0;
      decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    }
    else
    {
      //The tooth times below don't get set on tooth 13(The magical 13th tooth should not be considered for any calculations that use those times)
      decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
      decoderState.toothLastToothTime = curTime;
    }
  }
}

int16_t getCrankAngle(uint32_t currMicros, const shared::state_t &decoderState, const config4 &page4)
{
  auto calculator = decoders::shared::atomic_make_angle_caa(decoderState);

  // if temptoothCurrentCount is 0, the last tooth seen was the 13th one. Based on this, ignore the 13th tooth and use the 12th one as the last reference.
  if(calculator._toothCurrentCount == 0 )
  {
    calculator._toothCurrentCount = 12;
  }
  
  return decoders::shared::clampCrankAngle(calculator.calculateCrankAngle(currMicros, page4));
}

}
}