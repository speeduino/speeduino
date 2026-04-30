#pragma once
#include "scheduler.h"
#include "config_pages.h"
#include "statuses.h"

extern IgnitionSchedule ignitionSchedule1;
extern IgnitionSchedule ignitionSchedule2;
extern IgnitionSchedule ignitionSchedule3;
extern IgnitionSchedule ignitionSchedule4;
extern IgnitionSchedule ignitionSchedule5;
#if IGN_CHANNELS >= 6
extern IgnitionSchedule ignitionSchedule6;
#endif
#if IGN_CHANNELS >= 7
extern IgnitionSchedule ignitionSchedule7;
#endif
#if IGN_CHANNELS >= 8
extern IgnitionSchedule ignitionSchedule8;
#endif

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

/**
 * @brief Calculate the charge & discharge angles for all ignition channels
 * 
 * @param page2 The tune
 * @param page4 The tune
 * @param current Current system state
 */
void calculateIgnitionAngles(const config2 &page2, const config4 &page4, statuses &current);

/**
 * @brief Schedule all ignition channels
 * 
 * @param current Current system state
 * @param crankAngle Crank angle
 * @param dwellTime Target dwell time
 */
void setIgnitionChannels(const statuses &current, uint16_t crankAngle, uint16_t dwellTime);

/** @brief Reset all schedulers to their default state */
void resetIgnitionSchedulers(void);

/** @brief Start the timers that drive schedulers  */
void startIgnitionSchedulers(void);

/** @brief Stop the timers that drive schedulers  */
void stopIgnitionSchedulers(void);

/** @brief Initialise all ignition schedules */
void initialiseIgnitionSchedules(uint8_t sparkMode, uint8_t numCylinders, uint8_t rotaryMode);
