#pragma once

#include "src/pins/pinNumbers_t.h"
#include "statuses.h"

void initialiseAirCon(statuses &current, const pinNumbers_t &pins);

void airConControl(statuses &current);
