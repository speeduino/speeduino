/**
 * @file 
 * @brief Daihatsu +1 trigger for 3 and 4 cylinder engines.
 * 
 * Tooth equal to the number of cylinders are evenly spaced on the cam. No position sensing (Distributor is retained),
 * so crank angle is a made up figure based purely on the first teeth to be seen.
 * 
 * @see https://en.wikipedia.org/wiki/Suzuki_DR-Z400
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace drz400 {

  shared::state_t initialise(const config4 &page4);

  void triggerSecondary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config4 &page4);

}
}