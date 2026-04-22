#pragma once

#include <stdint.h>
#include "statuses.h"
#include "config_pages.h"

void initialiseProgrammableIO(statuses& current, const config13& page13);
void checkProgrammableIO(statuses& current, const config13& page13);
int16_t ProgrammableIOGetData(uint16_t index);
