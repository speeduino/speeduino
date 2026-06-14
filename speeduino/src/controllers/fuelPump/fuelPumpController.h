#pragma once

#include "../../../config_pages.h"
#include "../../../statuses.h"

void initialiseFuelPump(statuses &current, const config2 &page2, uint8_t pumpPin);

void startPumpPriming(statuses &current, const config2 &page2);
void stopPumpPriming(statuses &current, const config2 &page2);

void fuelPumpOn(void);
void fuelPumpOff(void);
