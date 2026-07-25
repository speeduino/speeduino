#pragma once

#include <stdint.h>

void initialiseBoost(uint8_t boostPin);
void boostControl(void);
void boostDisable(void);

void boostInterrupt(void);

