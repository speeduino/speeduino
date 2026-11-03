/**
 * @file 
 * @brief Yamaha Vmax 1990+ with 6 uneven teeth, triggering on the wide lobe.
 * 
 * Within the decoder code, the sync tooth is referred to as tooth #1. Derived from Harley and made to work on the Yamah Vmax.
 * Trigger is based on 'CHANGE' so we get a signal on the up and downward edges of the lobe. This is required to identify the wide lobe.
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace vmax {

  shared::state_t initialise(uint32_t curMicros, const statuses &current);

  void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config4 &page4);

}
}