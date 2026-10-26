/**
 * @file 
 * @brief A decoder for Miata '99 to '05 with 4x 70 degree duration teeth running at cam speed.
 * 
 * Teeth believed to be at the same angles as the 4g63 decoder. Tooth #1 is defined as the next crank
 * tooth after the crank signal is HIGH when the cam signal is falling. Tooth number one is at 355* ATDC.
 * 
 * @see www.forum.diyefi.org/viewtopic.php?f=56&t=1077
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace miata9905 {

    shared::state_t initialise(uint32_t curMicros, const statuses &current);

    void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4);

    void triggerSecondary(uint32_t curTime, const statuses &current, shared::state_t &decoderState);

    uint32_t getRevolutionTime(const statuses &current, shared::state_t &decoderState);

    int16_t getCrankAngle(uint32_t currMicros, const shared::state_t &decoderState, const config4 &page4);
}
}