/**
 * @file
 * @brief Basic Distributor where tooth count is equal to the number of cylinders and teeth are evenly spaced on the cam.
 * 
 * No position sensing (Distributor is retained) so crank angle is a made up figure based purely on the first teeth to be seen.
 * @note This is a very simple decoder. See http://www.megamanual.com/ms2/GM_7pinHEI.htm
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace basic_distributor {

    shared::state_t initialise(const config2 &page2);

    void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4);

    uint32_t getRevolutionTime(const statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4);

    int16_t getCrankAngle(uint32_t currMicros, const shared::state_t &decoderState, const config4 &page4);

}
}