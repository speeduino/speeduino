#pragma once

/**
 * @file
 * @brief Engine "protection" via RPM limits 
 */

#include "statuses.h"
#include "config_pages.h"
#include "decoders.h"

/**
 * @brief Determine what the engine protection state is.
 * 
 * @param current Current system state
 * @param page4 Tune
 * @param page6 Tune
 * @param page9 Tune
 * @param page10 Tune
 * @return statuses::engine_protect_flags_t Which engine protections should be applied
 */
statuses::engine_protect_flags_t checkEngineProtection(const statuses &current, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10);

/**
 * @brief Determine which fuel and ignition channels to cut in order to limit RPM.
 * 
 * @note This can only enforce RPM based limits; limits based on other factors (E.g. advance) are
 * dealt with elsewhere.
 * 
 * @param current Current system state
 * @param page2 Tune
 * @param page4 Tune
 * @param page6 Tune
 * @param page9 Tune
 * @return statuses::scheduler_cut_t Cut status & fuel/ignition channels that should be cut.
 */
statuses::scheduler_cut_t calculateFuelIgnitionChannelCut(const statuses &current, const decoder_status_t &decoderStatus, const config2 &page2, const config4 &page4, const config6 &page6, const config9 &page9);