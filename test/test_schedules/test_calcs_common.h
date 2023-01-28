#pragma once

#include <stdint.h>

constexpr uint16_t DWELL_TIME_MS = 4;

extern uint16_t dwellAngle;
void setEngineSpeed(uint16_t rpm, int16_t max_ign);