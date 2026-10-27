/**
 * @file 
 * @brief A decoder Non-360 Dual wheel with 2 wheels located either both on the crank or with the primary on the crank and the secondary on the cam.
 * 
 * There can be no missing teeth on the primary wheel.
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace non360 {

    shared::state_t initialise(const config4 &page4);

}
}