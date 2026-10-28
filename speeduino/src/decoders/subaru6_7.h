/**
 * @file 
 * @brief Subaru 6/7 Trigger pattern decoder for 6 tooth (irregularly spaced) crank and 7 tooth (also fairly irregular) cam wheels (eg late 90's Impreza 2.2).
 * 
 * This seems to be present in late 90's Subaru. In 2001 Subaru moved to 36-2-2-2.
 * @see http://www.vems.hu/wiki/index.php?page=InputTrigger%2FSubaruTrigger.
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace subaru67 {

  shared::state_t initialise(void);

  void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4);

  void triggerSecondary(uint32_t curTime, statuses &current, shared::state_t &decoderState);

}
}