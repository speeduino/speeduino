#pragma once

#include "src/pins/pinNumbers_t.h"
#include "statuses.h"
#include "config_pages.h"

void initialiseAirCon(statuses &current, const config15 &page15, const pinNumbers_t &pins);

void airConControl(statuses &current, const config15 &page15);
