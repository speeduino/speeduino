#pragma once

#include "statuses.h"

constexpr uint8_t RESET_CONTROL_DISABLED             = 0U;
constexpr uint8_t RESET_CONTROL_PREVENT_WHEN_RUNNING = 1U;
constexpr uint8_t RESET_CONTROL_PREVENT_ALWAYS       = 2U;
constexpr uint8_t RESET_CONTROL_SERIAL_COMMAND       = 3U;

void initialiseResetControl(statuses &current, uint8_t resetControlMode, uint8_t resetPin);

uint8_t getResetControl(void);

void matchResetControlToEngineState(statuses &current);