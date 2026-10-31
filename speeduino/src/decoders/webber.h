/**
 * @file 
 * @brief Weber-Marelli trigger setup with 2 wheels, 4 teeth 90deg apart on crank and 2 90deg apart on cam.
 * 
 * Uses DualWheel decoders, There can be no missing teeth on the primary wheel.
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace webber {

  void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config4 &page4);

}
}