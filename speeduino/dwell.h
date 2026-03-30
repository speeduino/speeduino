#pragma once

#include <stdint.h>
#include "statuses.h"
#include "config_pages.h"
#include "table3d.h"

/**
 * @brief Compute the ignition dwell
 * 
 * @param current Current system state
 * @param page2 Tune
 * @param page4 Tune
 * @param dwellTable Tune 
 * @return uint16_t Dwell in µS
 */
uint16_t computeDwell(const statuses &current, const config2 &page2, const config4 &page4, const table3d4RpmLoad &dwellTable);