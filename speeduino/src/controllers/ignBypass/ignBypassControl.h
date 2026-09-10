#pragma once

#include "statuses.h"
#include "config_pages.h"

void initialiseIgnBypass(const statuses &current, const config4 &page4, const pinNumbers_t &pins);
void ignBypassControl(const statuses &current);