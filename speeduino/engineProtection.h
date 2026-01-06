#pragma once

#include "statuses.h"
#include "config_pages.h"

statuses::scheduler_cut_t calculateFuelIgnitionChannelCut(statuses &current, const config2 &page2, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10);
statuses::engine_protect_flags_t checkEngineProtection(const statuses &current, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10);