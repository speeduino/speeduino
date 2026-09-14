#pragma once

#include "../../../statuses.h"
#include "../../../config_pages.h"
#include "../../pins/pinNumbers_t.h"

/**
 * @brief Initialise the boost controller
 * 
 * @param current System state
 * @param page2 Tune
 * @param page6 Tune
 * @param page10 Tune
 * @param pins Board pin numbers
 */
void initialiseBoost(statuses &current, const config2 &page2, const config6 &page6, const config10 &page10, const pinNumbers_t &pins);

/**
 * @brief Run the boost control algorithm. Should be called periodically from the main loop
 * 
 * @param current System state
 * @param page2 Tune
 * @param page4 Tune
 * @param page6 Tune
 * @param page9 Tune
 * @param page10 Tune
 * @param page15 Tune
 */
void boostControl(statuses &current, const config2 &page2, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10, const config15 &page15);

/** @brief Boost timer ISR. The board needs to hook this into a timer */
void boostInterrupt(void);

