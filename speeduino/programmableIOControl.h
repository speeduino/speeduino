#pragma once

#include <stdint.h>
#include "statuses.h"
#include "config_pages.h"

constexpr uint8_t COMPARATOR_EQUAL = 0;
constexpr uint8_t COMPARATOR_NOT_EQUAL = 1;
constexpr uint8_t COMPARATOR_GREATER = 2;
constexpr uint8_t COMPARATOR_GREATER_EQUAL = 3;
constexpr uint8_t COMPARATOR_LESS = 4;
constexpr uint8_t COMPARATOR_LESS_EQUAL = 5;
constexpr uint8_t COMPARATOR_AND = 6;
constexpr uint8_t COMPARATOR_XOR = 7;

constexpr uint8_t BITWISE_DISABLED = 0;
constexpr uint8_t BITWISE_AND = 1;
constexpr uint8_t BITWISE_OR = 2;
constexpr uint8_t BITWISE_XOR = 3;

constexpr uint8_t REUSE_RULES = 240;

void initialiseProgrammableIO(statuses& current, const config13& page13);
void checkProgrammableIO(statuses& current, const config13& page13);
int16_t ProgrammableIOGetData(uint16_t index);
