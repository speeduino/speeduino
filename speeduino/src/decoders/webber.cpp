#include "webber.h"

namespace decoders {

namespace webber {

void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config4 &page4)
{
  decoderState.curGap = curTime - decoderState.toothLastToothTime;
  if ( decoderState.curGap >= decoderState.triggerFilterTime )
  {
    decoderState.toothCurrentCount++; //Increment the tooth counter
    if (decoderState.checkSyncToothCount > 0) { decoderState.checkSyncToothCount++; }
    if ( decoderState.triggerSecFilterTime <= decoderState.curGap ) { decoderState.triggerSecFilterTime = decoderState.curGap + (decoderState.curGap>>1); } //150% crank tooth
    decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)

    decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
    decoderState.toothLastToothTime = curTime;

    if ( decoderState.decoderStatus.syncStatus==SyncStatus::Full )
    {
      if ( (decoderState.toothCurrentCount == 1) || (decoderState.toothCurrentCount > page4.triggerTeeth) )
      {
        decoderState.toothCurrentCount = 1;
        decoderState.revolutionOne = !decoderState.revolutionOne; //Flip sequential revolution tracker
        decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
        decoderState.toothOneTime = curTime;
        current.startRevolutions++; //Counter
      }

      decoderState.setFilter(decoderState.curGap, page4); //Recalc the new filter value
    }
    else
    {
      if ( (decoderState.secondaryToothCount == 1) && (decoderState.checkSyncToothCount == 4) )
      {
        decoderState.toothCurrentCount = 2;
        decoderState.decoderStatus.syncStatus = SyncStatus::Full;
        decoderState.revolutionOne = false; //Sequential revolution reset
      }
    }
  } //Trigger filter
}

void triggerSecondary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config4 &page4)
{
  decoderState.curGap2 = curTime - decoderState.toothLastSecToothTime;

  if ( decoderState.curGap2 >= decoderState.triggerSecFilterTime )
  {
    decoderState.toothLastSecToothTime = curTime;

    if ( (decoderState.secondaryToothCount == 2) && (decoderState.checkSyncToothCount == 3) )
    {
      if(decoderState.decoderStatus.syncStatus!=SyncStatus::Full)
      {
        decoderState.toothLastToothTime = curTime;
        decoderState.toothLastMinusOneToothTime = curTime - 1500000; //Fixes RPM at 10rpm until a full revolution has taken place
        decoderState.toothCurrentCount = page4.triggerTeeth-1;

        decoderState.decoderStatus.syncStatus = SyncStatus::Full;
      }
      else
      {
        if ( (decoderState.toothCurrentCount != (page4.triggerTeeth-1U)) && (current.startRevolutions > 2U)) { current.syncLossCounter++; } //Indicates likely sync loss.
        if (page4.useResync == 1) { decoderState.toothCurrentCount = page4.triggerTeeth-1; }
      }
      decoderState.revolutionOne = true; //Sequential revolution reset
      decoderState.triggerSecFilterTime = decoderState.curGap << 2; //4 crank teeth
      decoderState.secondaryToothCount = 1; //Next tooth should be first
    } //Running, on first CAM pulse restart crank teeth count, on second the counter should be 3
    else if ( (decoderState.decoderStatus.syncStatus!=SyncStatus::Full) && (decoderState.toothCurrentCount >= 3) && (decoderState.secondaryToothCount == 0) )
    {
      decoderState.toothLastToothTime = curTime;
      decoderState.toothLastMinusOneToothTime = curTime - 1500000; //Fixes RPM at 10rpm until a full revolution has taken place
      decoderState.toothCurrentCount = 1;
      decoderState.revolutionOne = true; //Sequential revolution reset

      decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    } //First start, between gaps on CAM pulses have 2 teeth, sync on first CAM pulse if seen 3 teeth or more
    else
    {
      decoderState.triggerSecFilterTime = decoderState.curGap + (decoderState.curGap>>1); //150% crank tooth
      decoderState.secondaryToothCount++;
      decoderState.checkSyncToothCount = 1; //Tooth 1 considered as already been seen
    } //First time might fall here, second CAM tooth will
  }
  else
  {
    decoderState.triggerSecFilterTime = decoderState.curGap + (decoderState.curGap>>1); //Noise region, using 150% of crank tooth
    decoderState.checkSyncToothCount = 1; //Reset tooth counter
  } //Trigger filter
}


}
}