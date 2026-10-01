#pragma once

#include "decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {
namespace detail {

void checkPerToothTiming(const statuses &current, const state_t &decoderState, const config4 &page4, int16_t crankAngle, uint16_t currentTooth);

}
}