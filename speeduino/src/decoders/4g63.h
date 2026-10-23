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

namespace fourg63 {

    shared::state_t initialise(uint32_t curMicros, const statuses &current, const config2 &page2);

}

}