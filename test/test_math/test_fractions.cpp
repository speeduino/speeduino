#include "../test_utils.h"
#include "src/utils/FractionalMath.h"
#include "../timer.hpp"

template <uint8_t denominator>
static void assert_fraction(const FastFraction<denominator> &fraction, uint32_t toConvert)
{
  TEST_ASSERT_EQUAL((toConvert*fraction.numerator_)/denominator, fraction*toConvert);
}

template <uint8_t denominator>
static void test_fraction(const FastFraction<denominator> &fraction)
{
  assert_fraction(fraction, 1);
  assert_fraction(fraction, UINT8_MAX);
  assert_fraction(fraction, UINT16_MAX);
}

static void test_fractions(void)
{
  TEST_ASSERT_EQUAL(1, ONE_HALF*2);
  test_fraction(ONE_HALF);

  TEST_ASSERT_EQUAL(1, ONE_QUARTER*4);
  test_fraction(ONE_QUARTER);

  TEST_ASSERT_EQUAL(1, ONE_EIGTH*8);
  test_fraction(ONE_EIGTH);

  TEST_ASSERT_EQUAL(1, ONE_SIXTEENTH*16);
  test_fraction(ONE_SIXTEENTH);  

  TEST_ASSERT_EQUAL(3, THREE_SIXTEENTHS*16);
  test_fraction(THREE_SIXTEENTHS);  

  TEST_ASSERT_EQUAL(5, FIVE_SIXTEENTHS*16);
  test_fraction(FIVE_SIXTEENTHS);  
  
  TEST_ASSERT_EQUAL(3, THREE_EIGTHS*8);
  test_fraction(THREE_EIGTHS);
  
  TEST_ASSERT_EQUAL(9, NINE_SIXTEENTHS*16);
  test_fraction(NINE_SIXTEENTHS);
  
  TEST_ASSERT_EQUAL(5, FIVE_EIGTHS*8);
  test_fraction(FIVE_EIGTHS);
  
  TEST_ASSERT_EQUAL(3, THREE_QUARTERS*4);
  test_fraction(THREE_QUARTERS);
  
  TEST_ASSERT_EQUAL(9, NINE_EIGTHS*8);
  test_fraction(NINE_EIGTHS);
  
  TEST_ASSERT_EQUAL(11, ELEVEN_EIGTHS*8);
  test_fraction(ELEVEN_EIGTHS);
  
  TEST_ASSERT_EQUAL(5, FIVE_QUARTERS*4);
  test_fraction(FIVE_QUARTERS);
  
  TEST_ASSERT_EQUAL(3, ONE_AND_HALF*2);
  test_fraction(ONE_AND_HALF);
}

static void test_perf_div(void)
{
  constexpr uint16_t iters = 1;
  constexpr uint16_t start_index = 0;
  constexpr uint16_t end_index = 9999;
  constexpr uint16_t step = 7;

  static constexpr auto fraction = ONE_SIXTEENTH*FastFraction<1>(7);
  auto nativeTestDiv = [] (uint16_t index, uint32_t &checkSum) { checkSum += ((index*99UL)*fraction.numerator_)/fraction.denominator_; };
  auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += (index*99UL)*fraction; };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(iters, start_index, end_index, step, nativeTestDiv, optimizedTest);
  
  // The checksums will be different due to rounding. This is only
  // here to force the compiler to run the loops above
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

#if defined(__AVR__) // Speed up only noticeable on AVR
  TEST_ASSERT_LESS_THAN(comparison.timeA.durationMicros, comparison.timeB.durationMicros);
#endif
}

static void test_perf_shift(void)
{
  constexpr uint16_t iters = 1;
  constexpr uint16_t start_index = 0;
  constexpr uint16_t end_index = 9999;
  constexpr uint16_t step = 7;

  static constexpr auto fraction = ONE_SIXTEENTH*FastFraction<1>(7);
  auto nativeTestDiv = [] (uint16_t index, uint32_t &checkSum) { checkSum += ((index*99UL)*fraction.numerator_) >> fraction.shift_; };
  auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += (index*99UL)*fraction; };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(iters, start_index, end_index, step, nativeTestDiv, optimizedTest);
  
  // The checksums will be different due to rounding. This is only
  // here to force the compiler to run the loops above
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

#if defined(__AVR__) // Speed up only noticeable on AVR
  TEST_ASSERT_LESS_THAN(comparison.timeA.durationMicros, comparison.timeB.durationMicros);
#endif
}

void testFractions(void) {
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_fractions);
  RUN_TEST_P(test_perf_div);
  RUN_TEST_P(test_perf_shift);
}