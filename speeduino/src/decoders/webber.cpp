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


}
}