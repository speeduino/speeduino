#pragma once

#include "decoder_t.h"
#include "src/pins/boardInputPin.h"

void configureStateForPrimaryTrigger(uint8_t decoder, decoder_status_t &status);
void configurePinState(boardInputPin_t &p, uint8_t edge);