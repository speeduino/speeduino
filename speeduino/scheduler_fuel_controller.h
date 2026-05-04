#pragma once
#include "scheduler.h"
#include "config_pages.h"
#include "statuses.h"
#include "fuel_calcs.h"

bool isSemiSequentialInjection(const config2 &page2, const decoder_status_t &decoderStatus);
uint16_t setFuelChannelSchedules(const statuses &current);

/**
 * @brief Apply the calculated pulse widths to the current system state
 * 
 * @param pulseWidths Result of computePulseWidths()
 * @param page2 Tune settings
 * @param page4 Tune settings
 * @param page6 Tune settings
 * @param current Current system state
 */
void applyPwToInjectorChannels(const pulseWidths &pulse_widths, const config2 &page2, const config4 &page4, const config6 &page6, statuses &current);
