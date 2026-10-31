/**
 * @file 
 * @brief 36-2-1 / Mistsubishi 4B11 decoder: a crank based trigger with a nominal 36 teeth, but with 1 single and 1 double missing tooth.
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace thirtysixminus21 {

  shared::state_t initialise(const config4 &page4);

  void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4);

  uint32_t getRevolutionTime(const statuses &current, const shared::state_t &decoderState, const config4 &page4);

  void setEndTeeth(shared::state_t &decoderState);
}
}