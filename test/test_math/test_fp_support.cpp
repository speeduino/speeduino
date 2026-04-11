#include <stdio.h>
#include <unity.h>
#include "test_fp_support.h"
#include <inttypes.h>

#if defined(ARDUINO_ARCH_AVR)
#include <fp64lib.h>
using test_float_t = float64_t;
#else
using test_float_t = double;
#endif

test_float_t floatDivision(int32_t a, int32_t b) {
#if defined(ARDUINO_ARCH_AVR)
  return fp64_div(fp64_int32_to_float64(a), fp64_int32_to_float64(b));
#else
  return (double)a/(double)b;
#endif
}

test_float_t floatDivision(uint32_t a, uint32_t b) {
#if defined(ARDUINO_ARCH_AVR)
  return fp64_div(fp64_uint32_to_float64(a), fp64_uint32_to_float64(b));
#else
  return (double)a/(double)b;
#endif
}

int32_t round_float(test_float_t f) {
#if defined(ARDUINO_ARCH_AVR)
  return fp64_lround(f);
#else
  return round(f);
#endif
}

template <typename T>
static inline T getExpectedSigned(T a, T b) {
  test_float_t fExpected = floatDivision((int32_t)a, (int32_t)b);
  return (T)round_float(fExpected);
}
template <typename T>
static inline T getExpectedUnsigned(T a, T b) {
  test_float_t fExpected = floatDivision((uint32_t)a, (uint32_t)b);
  return (T)round_float(fExpected);
}

void assert_rounded_div(int32_t a, int32_t b, int32_t actual, uint8_t delta) {
  int32_t expected = getExpectedSigned<int32_t>(a, b);
  char msg[64];
  sprintf(msg, "a:%" PRIi32 ", b:%" PRIi32 " Expected:%" PRIi32, a, b, expected);
  TEST_ASSERT_INT32_WITHIN_MESSAGE(delta, expected, actual, msg);
}
void assert_rounded_div(int16_t a, int16_t b, int16_t actual, uint8_t delta) {
  int16_t expected = getExpectedSigned<int16_t>(a, b);
  char msg[64];
  sprintf(msg, "a:%" PRIi16 ", b:%" PRIi16 " Expected:%" PRIi16, a, b, expected);
  TEST_ASSERT_INT16_WITHIN_MESSAGE(delta, expected, actual, msg);
}
void assert_rounded_div(uint32_t a, uint32_t b, uint32_t actual, uint8_t delta) {
  uint32_t expected = getExpectedUnsigned<uint32_t>(a, b);
  char msg[64];
  sprintf(msg, "a:%" PRIu32 ", b:%" PRIu32 ", Expected:%" PRIu32, a, b, expected);
  TEST_ASSERT_UINT32_WITHIN_MESSAGE(delta, expected, actual, msg);
}
void assert_rounded_div(uint16_t a, uint16_t b, uint16_t actual, uint8_t delta) {
  uint16_t expected = getExpectedUnsigned<uint16_t>(a, b);
  char msg[64];
  sprintf(msg, "a:%" PRIu16 ", b:%" PRIu16 ", Expected:%" PRIu16, a, b, expected);
  TEST_ASSERT_UINT32_WITHIN_MESSAGE(delta, expected, actual, msg);
}
