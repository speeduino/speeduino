#pragma once

#include "config_pages.h"
#include "statuses.h"

void initialiseNitrous(statuses &current, config10 &page10);

void nitrousControl(statuses &current, const config10 &page10);

static inline bool isNitrousStage1(uint8_t status)
{
  return (status==NITROUS_STAGE1) || (status==NITROUS_BOTH);
}
static inline bool isNitrousStage2(uint8_t status)
{
  return (status==NITROUS_STAGE2) || (status==NITROUS_BOTH);
}
