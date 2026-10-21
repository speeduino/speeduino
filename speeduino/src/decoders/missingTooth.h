/**
 * @file 
 * @brief A decoder for a (single) multi-tooth wheel with one of more 'missing' teeth.
 * 
 * The first tooth after the missing one is considered number 1 and is the basis for the trigger angle.
 * Optionally a cam signal can be added to provide a sequential reference. 
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace missing_tooth {

    shared::state_t initialise(const config4 &page4);

    void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4);

    void triggerSecondary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config4 &page4, const config6 &page6, const config10 &page10);

    void triggerTertiary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config4 &page4, const config6 &page6);

    uint32_t getRevolutionTime(const statuses &current, const shared::state_t &decoderState, const config4 &page4);

    void setEndTeeth(shared::state_t &decoderState, const config4 &page4);

    int16_t getCrankAngle(uint32_t currMicros, const shared::state_t &decoderState, const config4 &page4);
}

}