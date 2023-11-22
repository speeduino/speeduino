#include "test_fp_support.h"

float64_t floatDivision(int32_t a, int32_t b) {
  return fp64_div(fp64_int32_to_float64(a), fp64_int32_to_float64(b));
}

void assert_rounded_div(int32_t a, int32_t b, int32_t actual) {
  float64_t fExpected = floatDivision(a, b);
  int32_t expected = fp64_lround(fExpected);

  char msg[64];
  sprintf(msg, "a: %" PRIi32 ", b:  %" PRIi32 " fExpected: %s", a, b, fp64_to_string(fExpected, 17, 15));
  TEST_ASSERT_EQUAL_MESSAGE(expected, actual, msg);
}