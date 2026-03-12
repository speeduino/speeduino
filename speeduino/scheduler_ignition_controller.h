#pragma once
#include "scheduler.h"
#include "config_pages.h"
#include "statuses.h"

/**
 * @brief Check that no ignition channel has been charging the coil for too long
 * 
 * The over dwell protection system runs independently of the standard ignition 
 * schedules and monitors the time that each ignition output has been active. If the 
 * active time exceeds the tune defined amount, the output will be ended to prevent damage to coils.
 * 
 * @note Must be called once per millisecond by an **external** timer.
 */
void applyOverDwellProtection(const config4 &page4, const statuses &current);

void calculateIgnitionAngles(const config2 &page2, const config4 &page4, statuses &current);

void setIgnitionChannels(const statuses &current, uint16_t crankAngle, uint16_t dwellTime);

/** @brief Initialize all schedulers to the OFF state */
void initialiseIgnitionSchedulers(void);

/** @brief Start the timers that drive schedulers  */
void startIgnitionSchedulers(void);

/** @brief Stop the timers that drive schedulers  */
void stopIgnitionSchedulers(void);
