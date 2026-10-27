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

}
}