/**
 * @file
 * @brief Minimal polyfill for the standard library type traits, since it isn't
 * available with avr-gcc.
 */
#pragma once

#include <stdint.h>

// std::numeric_limits. See https://en.cppreference.com/w/cpp/types/numeric_limits
template <typename T>
struct numeric_limits {
  static constexpr T (max)(void);
};

template <>
struct numeric_limits<uint8_t> {
  static constexpr uint8_t (max)(void) { return UINT8_MAX; }
};

template <>
struct numeric_limits<int8_t> {
  static constexpr int8_t (max)(void) { return INT8_MAX; }
};

template <>
struct numeric_limits<uint16_t> {
  static constexpr uint16_t (max)(void) { return UINT16_MAX; }
};

template <>
struct numeric_limits<int16_t> {
  static constexpr int16_t (max)(void) { return INT16_MAX; }
};

template <>
struct numeric_limits<uint32_t> {
  static constexpr uint32_t (max)(void) { return UINT32_MAX; }
};

template <>
struct numeric_limits<int32_t> {
  static constexpr int32_t (max)(void) { return INT32_MAX; }
};