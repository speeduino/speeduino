#pragma once

#include <stdint.h>
#include "statuses.h"
#include "config_pages.h"

/**
 * @file
 * @brief Pulse width calculations
 */

/** @brief Result of pulse width calculation */
struct pulseWidths {
  /** @brief Primary pulse width in µS */
  uint16_t primary;

  /** @brief Secondary pulse width in µS. 
   * 
   * Will be zero if no secondary pulse width is required. 
   * E.g. staged injection is not turned on or the required
   * fuel can be applied using the primary injectors.
   */
  uint16_t secondary;
};

/**
 * @brief This function calculates the required pulsewidth time (in µS) given the current tune & system state
 * 
 * @param page2 Tune settings 
 * @param page6 Tune settings
 * @param page10 Tune settings
 * @param current Current system state
 * @return pulseWidths The primary and secondary injector pulse width in uS
 */
pulseWidths computePulseWidths(const config2 &page2, const config6 &page6, const config10 &page10, const statuses &current);

/**
 * @brief Apply the calculated pulse widths to the current system state
 * 
 * @param pulseWidths Result of computePulseWidths()
 * @param page2 Tune settings
 * @param current Current system state
 */
void applyPwToInjectorChannels(const pulseWidths &pulse_widths, const config2 &page2, statuses &current);
