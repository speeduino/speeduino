/**
 * @file
 * @brief The fuel schedule controller: manages the timing of opening and closing up to 8 injectors 
 * 
 * Anything related to controlling the fuel schedules should be done within this controller.
 */

#pragma once
#include "scheduler.h"
#include "config_pages.h"
#include "statuses.h"
#include "fuel_calcs.h"

extern FuelSchedule fuelSchedule1;
extern FuelSchedule fuelSchedule2;
extern FuelSchedule fuelSchedule3;
extern FuelSchedule fuelSchedule4;
#if INJ_CHANNELS >= 5
extern FuelSchedule fuelSchedule5;
#endif
#if INJ_CHANNELS >= 6
extern FuelSchedule fuelSchedule6;
#endif
#if INJ_CHANNELS >= 7
extern FuelSchedule fuelSchedule7;
#endif
#if INJ_CHANNELS >= 8
extern FuelSchedule fuelSchedule8;
#endif

/**
 * @brief Schedule all fuel channels
 * 
 * @param current Current system state
 * @return The injector angle used.
 */
uint16_t setFuelChannelSchedules(const statuses &current);

/**
 * @brief Apply the calculated pulse widths to the fuel schedules
 * 
 * @param pulseWidths Result of computePulseWidths()
 * @param page2 Tune settings
 * @param page4 Tune settings
 * @param page6 Tune settings
 * @param current Current system state
 */
void applyPwToInjectorChannels(const pulseWidths &pulse_widths, const config2 &page2, const config4 &page4, const config6 &page6, statuses &current);

/** @brief Start the timers that drive schedulers */
void startFuelSchedulers(void);

/** @brief Stop the timers that drive schedulers */
void stopFuelSchedulers(void);

/** @brief Start priming the fuel system */
void beginInjectorPriming(const statuses &current, const config4 &page4);

/** @brief Initialise this module */
void initialiseFuelSchedules(statuses &current, const config2 &page2, const config4 &page4);