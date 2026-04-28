#pragma once

#include <stdint.h>
#include "config_pages.h"

void initialiseProgrammableIO(const config13& page13);
void checkProgrammableIO(const config13& page13);
int16_t ProgrammableIOGetData(uint16_t index);
uint8_t getProgrammableIOOutputStatus(void);
