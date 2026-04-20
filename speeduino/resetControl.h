#pragma once

#include "statuses.h"

enum class ResetControlMode : uint8_t
{
    Disabled = 0U,
    PreventWhenRunning = 1U,
    PreventAlways = 2U,
    SerialCommand = 3U
};

ResetControlMode getResetControlMode(void);

bool isResetPreventActive(void);

void initialiseResetControl(ResetControlMode resetControlMode, uint8_t resetPin);

void matchResetControlToEngineState(const statuses &current);