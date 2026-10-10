#include "decoder_state.h"

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

}
}