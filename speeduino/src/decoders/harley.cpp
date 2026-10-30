#include "harley.h"

namespace decoders {

namespace harley {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(uint32_t curMicros, const statuses &current)
{
  shared::state_t state = {};

  state.triggerToothAngle = 0; // The number of degrees that passes from tooth to tooth, ev. 0. It alternates uneven
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * 60U); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  if(current.initialisationComplete == false) { state.toothLastToothTime = curMicros; } //Set a startup value here to avoid filter errors when starting. This MUST have the initial check to prevent the fuel pump just staying on all the time
  state.triggerFilterTime = 1500;

  return state;
}

void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config4 &page4)
{
  decoderState.lastGap = decoderState.curGap;
  decoderState.curGap = curTime - decoderState.toothLastToothTime;
  decoderState.setFilter(decoderState.curGap, page4); // Filtering adjusted according to setting
  if (decoderState.curGap > decoderState.triggerFilterTime)
  {
    if (current.decoder.primary.isPinHigh()) // Has to be the same as in main() trigger-attach, for readability we do it this way.
    {
        decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)
        decoderState.targetGap = decoderState.lastGap ; //Gap is the Time to next toothtrigger, so we know where we are
        decoderState.toothCurrentCount++;
        if (decoderState.curGap > decoderState.targetGap)
        {
          decoderState.toothCurrentCount = 1;
          decoderState.triggerToothAngle = 0;// Has to be equal to Angle Routine
          decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
          decoderState.toothOneTime = curTime;
          decoderState.decoderStatus.syncStatus = SyncStatus::Full;
        }
        else
        {
          decoderState.toothCurrentCount = 2;
          decoderState.triggerToothAngle = 157;
          //     decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
          //     decoderState.toothOneTime = curTime;
        }
        decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
        decoderState.toothLastToothTime = curTime;
        current.startRevolutions++; //Counter
    }
    else
    {
      if (decoderState.decoderStatus.syncStatus==SyncStatus::Full) { current.syncLossCounter++; }
      decoderState.decoderStatus.syncStatus = SyncStatus::None;
      decoderState.toothCurrentCount = 0;
    } //Primary trigger high
  } //Trigger filter
}

}
}