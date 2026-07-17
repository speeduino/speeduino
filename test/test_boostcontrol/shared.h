#pragma once
#include <stdint.h>

constexpr uint8_t TEST_BOOST_PIN = 19U;

void setup_simplepid_tune(void);
void setup_fullpid_tune(void);