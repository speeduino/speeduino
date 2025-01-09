#pragma once

#include <stdint.h>
#include "statuses.h"
#include "config_pages.h"
#include "table3d.h"

void calculateSecondaryFuel(const config10 &page10, const table3d16RpmLoad &veLookupTable, statuses &current);
void calculateSecondarySpark(const config2 &page2, const config10 &page10, const table3d16RpmLoad &sparkLookupTable, statuses &current);
