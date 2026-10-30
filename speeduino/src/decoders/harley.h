/**
 * @file 
 * @brief Harley Davidson (V2) with 2 unevenly Spaced Teeth.
 * 
 * Within the decoder code, the sync tooth is referred to as tooth #1. Derived from GMX7 and adapted for Harley.
 * Only rising Edge is used for simplicity. The second input is ignored, as it does not help to resolve cam position.
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace harley {

  shared::state_t initialise(uint32_t curMicros, const statuses &current);

}
}