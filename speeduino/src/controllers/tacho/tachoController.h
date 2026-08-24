#pragma once

#include <stdint.h>
#include "../../../statuses.h"
#include "../../../config_pages.h"

/** @brief Initialises the tacho control module */
void initialiseTachoControl(uint8_t tachoPin, const config2 &page2, const config6 &page6, const statuses &current);

/** @brief The tacho control function
 * 
 * Should be called regularly to update the tacho output status.
 * 
 * This function will check the current engine status and decide whether to 
 * turn the tacho output on or off based on the configured settings and timing.
 */
void tachoControl(const statuses &current);

/** @brief Turns the tacho output on */
void tachoOutputOn(void);

/** @brief Turns the tacho output off */
void tachoOutputOff(void);

