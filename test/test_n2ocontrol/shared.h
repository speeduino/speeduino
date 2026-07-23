#pragma once

#include <stdint.h>

constexpr uint8_t TEST_N2O1_PIN = 18U;
constexpr uint8_t TEST_N2O2_PIN = 19U;
constexpr uint8_t TEST_N2OARM_PIN = 20U;

void setup_n20_tune(uint8_t enableMode);
void setup_rpm_overlap_tune(uint8_t enableMode);