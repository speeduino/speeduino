#pragma once
#include "config_pages.h"
#include "statuses.h"
#include "src/pins/pinNumbers_t.h"

void initialiseLaunchControl(const pinNumbers_t &pins);

void checkLaunchAndFlatShift(statuses &current, const config2 &page2, const config6 &page6, const config10 &page10, const config15 &page15);
