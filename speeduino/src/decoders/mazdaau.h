/**
 * @file 
 * @brief Mazda AU version.
 * 
 * Tooth #2 is defined as the next crank tooth after the single cam tooth. Tooth number one is at 348* ATDC.
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace mazdaau  {

    shared::state_t initialise(void);

    void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t & decoderState, const config4 &page4);

    void triggerSecondary(uint32_t curTime, shared::state_t & decoderState);

    uint32_t getRevolutionTime(const statuses &current, const shared::state_t &decoderState);
}
}