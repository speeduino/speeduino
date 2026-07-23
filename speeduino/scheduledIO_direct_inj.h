#pragma once
#include "src/pins/pinNumbers_t.h"

void initInjDirectIO(const injector_pins_t &injPins);

void openInjector_DIRECT(uint8_t channel);
void closeInjector_DIRECT(uint8_t channel);
