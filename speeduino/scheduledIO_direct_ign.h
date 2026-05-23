#pragma once
#include "src/pins/pinNumbers_t.h"
#include "config_pages.h"

void initIgnDirectIO(const config4 &page4, const coil_pins_t &coilPins);

void coilCharging_DIRECT(uint8_t channel);
void coilStopCharging_DIRECT(uint8_t channel);
