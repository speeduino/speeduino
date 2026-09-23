#pragma once

#include "config_pages.h"
#include "statuses.h"
#include "src/pins/pinNumbers_t.h"

void initialiseFuelPump(const statuses &current, const config2 &page2, const pinNumbers_t &pins);

void fuelPumpControl(const statuses &current, const config2 &page2);

void startPumpPriming(const statuses &current, const config2 &page2);