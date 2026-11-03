/**
 * @file 
 * @brief Decoder for Audi with 135 teeth on the crank and 1 tooth on the cam.
 * 
 * This is very similar to the dual wheel decoder, however due to the 135 teeth not dividing evenly into 360,
 * only every 3rd crank tooth is used in calculating the crank angle. This effectively makes it a 45 tooth dual wheel setup.
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace audi135 {

    shared::state_t initialise(void);

    void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config4 &page4);

    void triggerSecondary(const statuses &current, shared::state_t &decoderState, const config4 &page4);

    int16_t getCrankAngle(uint32_t currMicros, const shared::state_t &decoderState, const config4 &page4);
}
}