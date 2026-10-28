#include "nissan360.h"
#include "shared/perToothIgnition.h"

namespace decoders {

namespace nissan360 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(void)
{
  shared::state_t state = {};

  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 360UL)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  state.triggerSecFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 2U)) / 2U; //Same as above, but fixed at 2 teeth on the secondary input and divided by 2 (for cam speed)
  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.supportsPerToothIgnition = true;
  state.toothCurrentCount = 1;
  state.triggerToothAngle = 2;
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

  return state;
}

void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
  decoderState.curGap = curTime - decoderState.toothLastToothTime;
  if ( decoderState.curGap < decoderState.triggerFilterTime ) { return; }

  decoderState.toothCurrentCount++; //Increment the tooth counter
  decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)

  decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
  decoderState.toothLastToothTime = curTime;

  if ( decoderState.decoderStatus.syncStatus==SyncStatus::Full )
  {
    if ( decoderState.toothCurrentCount == 361 ) //2 complete crank revolutions
    {
      decoderState.toothCurrentCount = 1;
      decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
      decoderState.toothOneTime = curTime;
      current.startRevolutions++; //Counter
    }
    //Recalc the new filter value
    decoderState.setFilter(decoderState.curGap, page4);

    //EXPERIMENTAL!
    if(page2.perToothIgn == true)
    {
      int16_t crankAngle = ( (decoderState.toothCurrentCount-1) * 2 ) + page4.triggerAngle;
      if(crankAngle > CRANK_ANGLE_MAX_IGN) 
      { 
        crankAngle -= CRANK_ANGLE_MAX_IGN;
        checkPerToothTiming(current, decoderState, page4, crankAngle, (decoderState.toothCurrentCount/2) );
      }
      else
      {
        checkPerToothTiming(current, decoderState, page4, crankAngle, decoderState.toothCurrentCount);
      }
    }
  }
}


}
}