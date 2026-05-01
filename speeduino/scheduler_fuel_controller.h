#pragma once
#include "scheduler.h"
#include "config_pages.h"
#include "statuses.h"

bool isSemiSequentialInjection(const config2 &page2, const decoder_status_t &decoderStatus);
void matchFuelSchedulersToSyncState(const config2 &page2, const config4 &page4, statuses &current);
uint16_t setInjectionAngles(const statuses &current);
void setFuelChannelSchedules(uint16_t crankAngle, byte injChannelMask);
