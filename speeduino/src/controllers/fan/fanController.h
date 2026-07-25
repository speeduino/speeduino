#pragma once

#include <stdint.h>

void initialiseFan(uint8_t fanPin);

void fanControl(void);

void fanOn(void);
void fanOff(void);

void fanInterrupt(void);
