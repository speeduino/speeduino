#pragma once

#include "statuses.h"

enum class ResetControlMode : uint8_t
{
    Disabled = 0U,
    PreventWhenRunning = 1U,
    PreventAlways = 2U,
    SerialCommand = 3U
};

void initialiseResetControl(statuses &current, ResetControlMode resetControlMode, uint8_t resetPin);

ResetControlMode getResetControlMode(void);

void matchResetControlToEngineState(statuses &current);