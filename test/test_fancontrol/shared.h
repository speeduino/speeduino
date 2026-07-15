#pragma once

#include <stdint.h>

constexpr uint8_t TEST_FAN_PIN  = 19U;

void setup_nopwm_tune(void);
void setup_pwm_tune(void);