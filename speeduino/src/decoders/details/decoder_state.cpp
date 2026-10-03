#include "decoder_state.h"
#include "atomic.h"
#include "elapsed_time.h"

namespace decoders {

namespace detail {

void state_t::setFilter(unsigned long curGap, const config4 &page4)
{
    switch(page4.triggerFilter)
    {
        case TRIGGER_FILTER_LITE: 
        triggerFilterTime = curGap >> 2;
        break;
        case TRIGGER_FILTER_MEDIUM: 
        triggerFilterTime = curGap >> 1;
        break;
        case TRIGGER_FILTER_AGGRESSIVE: 
        triggerFilterTime = (curGap * 3) >> 2;
        break;
        case TRIGGER_FILTER_OFF: 
        default:
        triggerFilterTime = 0;
        break;
    }
}

void __attribute__((optimize("Os"))) state_t::setTriggerToothAngle(int16_t angle)
{
    triggerToothAngle = angle;
#ifdef USE_LIBDIVIDE
    divTriggerToothAngle = libdivide::libdivide_s16_gen(angle);
#endif
}

void __attribute__((optimize("Os"))) state_t::reset(void) 
{
  toothLastSecToothTime = 0;
  toothLastToothTime = 0;
  toothSystemCount = 0;
  secondaryToothCount = 0;
  decoderStatus.syncStatus = SyncStatus::None;
  triggerFilterTime = 0;
  decoderStatus.validTrigger = false;
}

bool state_t::toothWithinMaxStallTime(uint32_t curTime)
{
  uint32_t lastToothTime = std::get<0>(atomic_copy(toothLastToothTime));

  // lastToothTime can be slightly ahead of curTime if a pulse occurred after
  // curTime was sampled. Accept that race only within the stall interval; an
  // unconditional ordering check would mistake a real counter rollover for it.
  return (timeElapsed(curTime, lastToothTime) < MAX_STALL_TIME)
      || (timeElapsed(lastToothTime, curTime) < MAX_STALL_TIME);
}

}
}