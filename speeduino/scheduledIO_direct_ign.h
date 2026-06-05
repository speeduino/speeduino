#pragma once

#include "config_pages.h"

void initIgnDirectIO(const config4 &page4, const uint8_t (&pins)[IGN_CHANNELS]);

void coilCharging_DIRECT(uint8_t channel);
void coilStopCharging_DIRECT(uint8_t channel);
