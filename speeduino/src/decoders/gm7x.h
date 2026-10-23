/**
 * @file 
 * @brief GM 7X trigger wheel decoder with six equally spaced teeth and a seventh tooth for cylinder identification.
 * 
 * @note Within the decoder code, the sync tooth is referred to as tooth #3 rather than tooth #7. This makes for simpler angle calculations
 * (See: http://www.speeduino.com/forum/download/file.php?id=4743 ).
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace gm7x {

    shared::state_t initialise(void);

    void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4);

}
}