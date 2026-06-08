#pragma once

#include <stdint.h>
#include "config_pages.h"

/** @brief Initialise the programmable I/O system.
 * @param page13 The tune
 */
void initialiseProgrammableIO(const config13& page13);

/** @brief Control function for the programmable I/O system.
 * Should be called regularly (e.g. in main loop) to check the rules and update outputs accordingly.
 * @param page13 The tune
 */
void programmableIOControl(const config13& page13);

/** @brief Get single I/O data var (from current) for comparison.
 * @param index Field index/number (?)
 */
int16_t programmableIOGetData(uint16_t index);

/** @brief Get the output status of the programmable I/O system.
 * @return The compressed output status.
 */
uint8_t getProgrammableIOOutputStatus(void);
