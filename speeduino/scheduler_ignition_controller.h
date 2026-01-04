#pragma once
#include "scheduler.h"
#include "decoders.h"
#include "config_pages.h"
#include "statuses.h"

void matchIgnitionSchedulersToSyncState(const config2 &page2, const config4 &page4, statuses &current);
