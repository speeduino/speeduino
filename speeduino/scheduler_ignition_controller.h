#pragma once
#include "scheduler.h"
#include "config_pages.h"
#include "statuses.h"

void calculateIgnitionAngles(const config2 &page2, const config4 &page4, statuses &current);

void setIgnitionChannels(const statuses &current, uint16_t crankAngle, uint16_t dwellTime);
