#pragma once

#include "statuses.h"
#include "config_pages.h"

bool checkEngineProtect(statuses &current, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10);

uint8_t checkRevLimit(statuses &current, const config4 &page4, const config6 &page6, const config9 &page9);

struct schedulers_onoff_t
{
  byte ignitionChannels;
  byte fuelChannels;
};
schedulers_onoff_t calculateFuelIgnitionChannelCut(statuses &current, const config2 &page2, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10);