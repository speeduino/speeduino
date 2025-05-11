#pragma once
#include <stdint.h>
#include "bit_manip.h"

/**
 * @file
 * 
 * @brief Structures and functions for polling actions at specific frequencies. 
 * 
 */

using byte = uint8_t;

/** @brief An action to be polled at a specific frequency */
struct polledAction_t {
    uint8_t timerBit; ///< The timer bit to poll this action. E.g. BIT_TIMER_1HZ
    void (*pCallback)(void); ///< The function to call when the timer bit is set
};

/** @brief Execute a polled action if the timer bit is set
 * 
 * @param action The action to execute
 * @param loopTimer The current loop timer value (e.g. LOOP_TIMER)
 */
static inline void executePolledAction(const polledAction_t &action, byte loopTimer)
{
  if (BIT_CHECK(loopTimer, action.timerBit)) {
    action.pCallback();
  }
}
