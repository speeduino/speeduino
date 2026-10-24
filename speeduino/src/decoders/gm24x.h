/**
 * @file 
 * @brief GM 24X Decoder (eg early LS1 1996-2005).
 * 
 * Provided that the cam signal is used, this decoder simply counts the teeth and then looks their angles up against 
 * a lookup table. The cam signal is used to determine tooth #1

 * @note Useful references: www.vems.hu/wiki/index.php?page=MembersPage%2FJorgenKarlsson%2FTwentyFourX
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace gm24x {

    shared::state_t initialise(uint32_t curMicros, const statuses &current);

    void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState);

    void triggerSecondary(shared::state_t &decoderState);
}
}