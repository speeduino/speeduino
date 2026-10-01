#pragma once

#include "decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {
namespace detail {
    
inline uint16_t clampToToothCount(const config4 &page4, int16_t toothNum, uint8_t toothAdder) {
  int16_t toothRange = (int16_t)page4.triggerTeeth + (int16_t)toothAdder;
  return (uint16_t)nudge((int16_t)1, (int16_t)(toothRange+1), toothNum);
}

void checkPerToothTiming(const statuses &current, const state_t &decoderState, const config4 &page4, int16_t crankAngle, uint16_t currentTooth);

}
}