#pragma once
#include "config_pages.h"
#include "statuses.h"
#include "src/pins/pinNumbers_t.h"

void initialiseLaunchControl(config6 &page6, const pinNumbers_t &pins);

void updateLaunchAndFlatShift(statuses &current, const config2 &page2, const config6 &page6, const config10 &page10, const config15 &page15);
