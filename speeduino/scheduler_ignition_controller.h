#pragma once
#include "scheduler.h"
#include "config_pages.h"
#include "statuses.h"

void matchIgnitionSchedulersToSyncState(const config2 &page2, const config4 &page4, statuses &current);

/**
 * @brief Calculate the charge & discharge angles for all ignition channels
 * 
 * @param page2 The tune
 * @param page4 The tune
 * @param current Current system state
 */
void calculateIgnitionAngles(const config2 &page2, const config4 &page4, statuses &current);
