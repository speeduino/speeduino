/**
 * @file 
 * @brief DSM 420a, For the DSM Eclipse with 16 teeth total on the crank.
 * 
 * Tracks the falling side of the signal. Sync is determined by watching for a falling edge on the secondary signal
 * and checking if the primary signal is high then.
 * 
 * @see https://github.com/noisymime/speeduino/issues/133
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace dsm420a {

  shared::state_t initialise(void);

  void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4);

  void triggerSecondary(statuses &current, shared::state_t &decoderState);

  uint32_t getRevolutionTime(const statuses &current, const shared::state_t &decoderState);

}
}