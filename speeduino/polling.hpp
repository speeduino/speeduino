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

/** @brief Conditionally execute a polled action if the timer bit is set
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

/** @brief Execute a polled action at a specific index in the array of actions
 * 
 * @note Intent is to use this function with static_for to conditionally execute all actions in the array
 * 
 * @param index The index of the action to execute
 * @param pActions The array of actions to execute from
 * @param loopTimer The current loop timer value (e.g. LOOP_TIMER)
 */
static inline void executePolledArrayAction(uint8_t index, const polledAction_t *pActions, byte loopTimer)
{
    executePolledAction(pActions[index], loopTimer);
}