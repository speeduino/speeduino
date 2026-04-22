/**
 * @file
 * @brief Not used outside of programmableIOControl.cpp, but needs to be shared with unit tests
 */

#include <stdint.h>

namespace programmableIOControl_details
{

// Defined here for unit testing, but only used in programmableIOControl.cpp
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

}