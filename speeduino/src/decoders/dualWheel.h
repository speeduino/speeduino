/**
 * @file 
 * @brief A decoder for 2 wheels located either both on the crank or with the primary on the crank and the secondary on the cam.
 * 
 * @note There can be no missing teeth on the primary wheel.
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace dual_wheel {

    shared::state_t initialise(const config4 &page4);

    void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4);

    void triggerSecondary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config4 &page4);
}
}