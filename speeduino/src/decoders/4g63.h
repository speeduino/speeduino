/**
 * @file 
 * @brief Mitsubishi 4G63 / NA/NB Miata + MX-5 / 4/2 decoder
 * 
 * @note raw.githubusercontent.com/noisymime/speeduino/master/reference/wiki/decoders/4g63_trace.png
 * 
 * Tooth #1 is defined as the next crank tooth after the crank signal is HIGH when the cam signal is falling.
 * Tooth number one is at 355* ATDC.
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace fourg63 {

    shared::state_t initialise(uint32_t curMicros, const statuses &current, const config2 &page2);

    void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4);

}

}