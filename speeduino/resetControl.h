/**
 * @file
 * @brief Reset control functionality, to prevent resets while the engine is running (if configured to do so)
 * 
 * @see https://wiki.speeduino.com/en/configuration/Reset_Control
 */
#pragma once

#include "statuses.h"

/** @brief Available reset control modes */
enum class ResetControlMode : uint8_t
{
    /** Disabled */
    Disabled = 0U,
    /** Prevent reset when engine is running */
    PreventWhenRunning = 1U,
    /** Prevent reset always */
    PreventAlways = 2U,
    /** Reset via serial command */
    SerialCommand = 3U
};

/** @brief Initialise the reset control system, with the given mode and reset pin. */
void initialiseResetControl(ResetControlMode resetControlMode, uint8_t resetPin);

/** @brief Get the reset control mode as set during initialisation. */
ResetControlMode getResetControlMode(void);

/** @brief Check if reset prevention is active. */
bool isResetPreventActive(void);

/**
 * @brief Match reset control to the current engine state.
 * 
 * When the mode is set to PreventWhenRunning, this will set the reset pin HIGH (to prevent reset) if the engine is 
 * running, and LOW otherwise. This should be called regularly in the main loop.
*/
void matchResetControlToEngineState(const statuses &current);