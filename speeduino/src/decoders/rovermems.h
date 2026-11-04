/**
 * @file 
 * @brief * Rover MEMS decoder
 * 
 * Covers multiple trigger wheels used interchanbably over the range of MEMS units. 
 * 
 * Specifically covers teeth patterns on the primary trigger (crank):
 * 3 gap 14 gap 2 gap 13 gap
 * 11 gap 5 gap 12 gap 4 gap 
 * 2 gap 14 gap 3 gap 13 gap 
 * 17 gap 17 gap 
 *
 * On the secondary trigger (cam) supports:
 *   * No cam
 *   * Single tooth cam (or half moon cam)
 *   * Multi tooth (5-3-2 teeth) 
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace rovermems {

  shared::state_t initialise(config4 &page4);

  void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, config4 &page4);
}
}