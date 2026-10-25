/**
 * @file 
 * @brief Honda D17 (1.7 liter 4 cyl SOHC).
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace hondad17 {

    shared::state_t initialise(void);

    void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState);

}
}