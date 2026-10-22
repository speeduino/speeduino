#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace basic_distributor {

    shared::state_t initialise(const config2 &page2);

}
}