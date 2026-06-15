#pragma once

#include "../../../config_pages.h"
#include "../../../statuses.h"

void initialiseFuelPump(const statuses &current, const config2 &page2, uint8_t pumpPin);

void startPumpPriming(const statuses &current, const config2 &page2);
void stopPumpPriming(const statuses &current, const config2 &page2);

void fuelPumpOn(void);
void fuelPumpOff(void);
