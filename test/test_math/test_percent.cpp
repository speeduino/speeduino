#include <unity.h>
#include "test_fp_support.h"
#include "maths.h"
#include "../timer.hpp"
#include "../test_utils.h"

static void test_percent(uint16_t percent, uint32_t value) {
  assert_rounded_div((uint32_t)percent*value, (uint32_t)100U, percentage(percent, value));
}

void test_maths_percent_U8_U8(void)
{
  uint8_t percentOf = 77;
  test_percent(0, percentOf);
  test_percent(33, percentOf);
  test_percent(50, percentOf);
  test_percent(66, percentOf);
  test_percent(75, percentOf);
  test_percent(100, percentOf);
  test_percent(125, percentOf);
}

void test_maths_percent_U8_U16(void)
{
  uint16_t percentOf = 33333;
  test_percent(0, percentOf);
  test_percent(33, percentOf);
  test_percent(50, percentOf);
  test_percent(66, percentOf);
  test_percent(75, percentOf);
  test_percent(100, percentOf);
  test_percent(125, percentOf);
}

void test_maths_percent_U16_U32(void)
{
  uint32_t percentOf = UINT16_MAX*37UL;
  test_percent(0, percentOf);
  test_percent(33, percentOf);
  test_percent(50, percentOf);
  test_percent(66, percentOf);
  test_percent(75, percentOf);
  test_percent(100, percentOf);
  test_percent(125, percentOf);
  test_percent(256, percentOf);
  // The fuel corrections can be very high
  test_percent(1000, percentOf);
  test_percent(1500, percentOf);
}

static void test_halfPercentage(uint8_t percent, uint16_t value) {
  assert_rounded_div((uint32_t)percent*value, (uint32_t)200U, (uint32_t)halfPercentage(percent, value));
}

void test_maths_halfpercent_U8(void)
{
  uint8_t percentOf = 111;
  test_halfPercentage(0, percentOf);
  test_halfPercentage(33, percentOf);
  test_halfPercentage(50, percentOf);
  test_halfPercentage(66, percentOf);
  test_halfPercentage(75, percentOf);
  test_halfPercentage(100, percentOf);
  test_halfPercentage(125, percentOf);  
}

void test_maths_halfpercent_U16(void)
{
  uint16_t percentOf = 57357;
  test_halfPercentage(0, percentOf);
  test_halfPercentage(33, percentOf);
  test_halfPercentage(50, percentOf);
  test_halfPercentage(66, percentOf);
  test_halfPercentage(75, percentOf);
  test_halfPercentage(100, percentOf);
  test_halfPercentage(125, percentOf); 
}

static void test_percentApprox_u16(uint16_t percent, uint32_t value, uint32_t delta) {
  uint32_t expected = ((uint32_t)percent*value)/100;
  uint32_t actual = percentageApprox(percent, value);
  char msg[64];
  sprintf(msg, "pct:%" PRIu16 ", v:%" PRIu32 " Expected:%" PRIu32, percent, value, expected);
  TEST_ASSERT_UINT32_WITHIN_MESSAGE(delta, expected, actual, msg);
}

static void test_percentApprox_u8(uint8_t percent, uint32_t value, uint32_t delta) {
  uint32_t expected = ((uint32_t)percent*value)/100;
  uint32_t actual = percentageApprox(percent, value);
  char msg[64];
  sprintf(msg, "pct:%" PRIu8 ", v:%" PRIu32 " Expected:%" PRIu32, percent, value, expected);
  TEST_ASSERT_UINT32_WITHIN_MESSAGE(delta, expected, actual, msg);
}

static void test_maths_percentApprox(void)
{
  uint16_t percentOf = 33333;
  test_percentApprox_u16(0, percentOf, 1);
  test_percentApprox_u16(33, percentOf, 5);
  test_percentApprox_u16(50, percentOf, 1);
  test_percentApprox_u16(66, percentOf, 6);
  test_percentApprox_u16(75, percentOf, 1);
  test_percentApprox_u16(100, percentOf, 1);
  test_percentApprox_u16(125, percentOf, 1);
  test_percentApprox_u16(255, percentOf, 55);
  test_percentApprox_u16(511, percentOf, 25);
  test_percentApprox_u16(1023, percentOf, 150);
  test_percentApprox_u16(2046, percentOf, 300);

  test_percentApprox_u8(0, percentOf, 1);
  test_percentApprox_u8(33, percentOf, 5);
  test_percentApprox_u8(50, percentOf, 1);
  test_percentApprox_u8(66, percentOf, 6);
  test_percentApprox_u8(75, percentOf, 1);
  test_percentApprox_u8(100, percentOf, 1);
  test_percentApprox_u8(125, percentOf, 1);
  test_percentApprox_u8(INT8_MAX, percentOf, 15);
  test_percentApprox_u8(UINT8_MAX, percentOf, 26);
}

// These are shared by all percentage perf tests for consistency
static constexpr int16_t iters = 4;
static constexpr uint16_t start_percent = 3;
static constexpr uint16_t end_percent = 2047;
static constexpr uint16_t percent_step = 3;
static constexpr uint16_t percentOf = 57357;

void test_maths_halfPercentage_perf(void)
{
    auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += ((uint32_t)percentOf * index) / 200U; };
    auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += halfPercentage(index, percentOf); };
    TEST_MESSAGE("halfPercentage ");
    auto comparison = compare_executiontime<uint16_t, uint32_t>(iters, start_percent, end_percent, percent_step, nativeTest, optimizedTest);
    
    // The checksums will be different due to rounding. This is only
    // here to force the compiler to run the loops above
    TEST_ASSERT_INT32_WITHIN(UINT32_MAX/2, comparison.timeA.result, comparison.timeB.result);

#if defined(__AVR__) // Speed up only noticeable on AVR
    TEST_ASSERT_LESS_THAN(comparison.timeA.durationMicros, comparison.timeB.durationMicros);
#endif
}


void test_maths_percentage_perf(void)
{
    auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += ((uint32_t)percentOf * index) / 100U; };
    auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += percentage(index, percentOf); };
    TEST_MESSAGE("Percentage ");
    auto comparison = compare_executiontime<uint16_t, uint32_t>(iters, start_percent, end_percent, percent_step, nativeTest, optimizedTest);
    
    // The checksums will be different due to rounding. This is only
    // here to force the compiler to run the loops above
    TEST_ASSERT_INT32_WITHIN(UINT32_MAX/2, comparison.timeA.result, comparison.timeB.result);

#if defined(__AVR__) // Speed up only noticeable on AVR
    TEST_ASSERT_LESS_THAN(comparison.timeA.durationMicros, comparison.timeB.durationMicros);
#endif
}


void test_maths_percentageApprox_perf(void)
{
    auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += ((uint32_t)percentOf * index) / 100U; };
    auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += percentageApprox(index, percentOf); };
    auto comparison = compare_executiontime<uint16_t, uint32_t>(iters, start_percent, end_percent, percent_step, nativeTest, optimizedTest);
    
    // The checksums will be different due to rounding. This is only
    // here to force the compiler to run the loops above
    TEST_ASSERT_INT32_WITHIN(UINT32_MAX/2, comparison.timeA.result, comparison.timeB.result);
#if defined(__AVR__) // Speed up only noticeable on AVR
    TEST_ASSERT_LESS_THAN(comparison.timeA.durationMicros, comparison.timeB.durationMicros);
#endif

    auto nativeTest2 = [] (uint16_t index, uint32_t &checkSum) { checkSum += percentage(index, percentOf); };
    auto comparison2 = compare_executiontime<uint16_t, uint32_t>(iters, start_percent, end_percent, percent_step, nativeTest2, optimizedTest);
    TEST_ASSERT_INT32_WITHIN(UINT32_MAX/2, comparison2.timeA.result, comparison2.timeB.result);

#if defined(__AVR__) // Speed up only noticeable on AVR
    TEST_ASSERT_LESS_THAN(comparison2.timeA.durationMicros, comparison2.timeB.durationMicros);
#endif
}

void testPercent()
{
  SET_UNITY_FILENAME() {

  RUN_TEST(test_maths_percent_U8_U8);
  RUN_TEST(test_maths_percent_U8_U16);
  RUN_TEST(test_maths_percent_U16_U32);
  RUN_TEST(test_maths_halfpercent_U8);
  RUN_TEST(test_maths_halfpercent_U16);
  RUN_TEST(test_maths_halfPercentage_perf);
  RUN_TEST(test_maths_percentage_perf);
  RUN_TEST(test_maths_percentageApprox_perf);
  RUN_TEST(test_maths_percentApprox);
  }
}
