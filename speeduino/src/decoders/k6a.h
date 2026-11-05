/**
 * @file 
 * @brief Suzuki K6A 3 cylinder engine
 * 
 * @see https://www.msextra.com/forums/viewtopic.php?t=74614
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace k6a {

  shared::state_t initialise(uint32_t curMicros, const statuses &current, config4 &page4);

}
}