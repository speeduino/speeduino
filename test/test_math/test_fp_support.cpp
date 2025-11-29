#include <stdio.h>
#include <unity.h>
#include "test_fp_support.h"

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
  // char msg[64];
  // sprintf(msg, "a: %" PRIi32 ", b:  %" PRIi32 " fExpected: %s", a, b, fp64_to_string(fExpected, 17, 15));
  // TEST_ASSERT_EQUAL_MESSAGE(expected, actual, msg);
  TEST_ASSERT_INT32_WITHIN(delta, getExpectedSigned<int32_t>(a, b), actual);
}
void assert_rounded_div(int16_t a, int16_t b, int16_t actual, uint8_t delta) {
  // char msg[64];
  // sprintf(msg, "a: %" PRIi32 ", b:  %" PRIi32 " fExpected: %s", a, b, fp64_to_string(fExpected, 17, 15));
  // TEST_ASSERT_EQUAL_MESSAGE(expected, actual, msg);
  TEST_ASSERT_INT32_WITHIN(delta, getExpectedSigned<int16_t>(a, b), actual);
}
void assert_rounded_div(uint32_t a, uint32_t b, uint32_t actual, uint8_t delta) {
  // char msg[64];
  // sprintf(msg, "a: %" PRIi32 ", b:  %" PRIi32 " fExpected: %s", a, b, fp64_to_string(fExpected, 17, 15));
  // TEST_ASSERT_EQUAL_MESSAGE(expected, actual, msg);
  TEST_ASSERT_UINT32_WITHIN(delta, getExpectedUnsigned<uint32_t>(a, b), actual);
}
void assert_rounded_div(uint16_t a, uint16_t b, uint16_t actual, uint8_t delta) {
  // char msg[64];
  // sprintf(msg, "a: %" PRIi32 ", b:  %" PRIi32 " fExpected: %s", a, b, fp64_to_string(fExpected, 17, 15));
  // TEST_ASSERT_EQUAL_MESSAGE(expected, actual, msg);
  TEST_ASSERT_UINT32_WITHIN(delta, getExpectedUnsigned<uint16_t>(a, b), actual);
}
