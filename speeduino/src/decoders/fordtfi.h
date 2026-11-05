/**
 * @file 
 * @brief  Distributor mounted signal tooth wheel. Same number of teeth as cylinders.
 * 
 * Evenly spaced rising edge triggers, Cylinder 1 has a narrow teeth and will have a signature falling edge
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace ford_tfi {

    shared::state_t initialise(const config2 &page2);

}
}