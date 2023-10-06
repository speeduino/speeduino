#include <unity.h>
#include <fp64lib.h>
#include "globals.h"
#include "maths.h"
#include "crankMaths.h"
#include "..\timer.hpp"

float64_t floatDivision(int32_t a, int32_t b) {
  return fp64_div(fp64_int32_to_float64(a), fp64_int32_to_float64(b));
}

static void assert_rounded_div(int32_t a, int32_t b, int32_t actual) {
  float64_t fExpected = floatDivision(a, b);
  int32_t expected = fp64_lround(fExpected);

  char msg[64];
  sprintf(msg, "a: %" PRIi32 ", b:  %" PRIi32 " fExpected: %s", a, b, fp64_to_string(fExpected, 17, 15));
  TEST_ASSERT_EQUAL_MESSAGE(expected, actual, msg);
}

static void test_percent(uint8_t percent, uint32_t value) {
  assert_rounded_div(percent*value, 100, percentage(percent, value));
}

void test_maths_percent_U8(void)
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

void test_maths_percent_U16(void)
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

void test_maths_percent_U32(void)
{
  uint32_t percentOf = 5357915UL;
  test_percent(0, percentOf);
  test_percent(33, percentOf);
  test_percent(50, percentOf);
  test_percent(66, percentOf);
  test_percent(75, percentOf);
  test_percent(100, percentOf);
  test_percent(125, percentOf);
}

static void test_halfPercentage(uint8_t percent, uint32_t value) {
  assert_rounded_div(percent*value, 200, halfPercentage(percent, value));
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

void test_maths_halfpercent_U32(void)
{
  uint32_t percentOf = 5357915UL;
  test_halfPercentage(0, percentOf);
  test_halfPercentage(33, percentOf);
  test_halfPercentage(50, percentOf);
  test_halfPercentage(66, percentOf);
  test_halfPercentage(75, percentOf);
  test_halfPercentage(100, percentOf);
  test_halfPercentage(125, percentOf); 
}

template <typename T>
static void test_div100(T testValue) {
  assert_rounded_div(testValue, 100, div100(testValue));
}

template <typename T>
static void test_div100_Seed(T seedValue) {
  test_div100<T>((T)seedValue-(T)100);
  test_div100<T>((T)seedValue-(T)51);
  test_div100<T>((T)seedValue-(T)50);
  test_div100<T>((T)seedValue);
  test_div100<T>((T)seedValue+(T)50);
  test_div100<T>((T)seedValue+(T)51);
  test_div100<T>((T)seedValue+(T)100);  
}

void test_maths_div100_U16(void)
{
  test_div100_Seed<uint16_t>(100U);
  test_div100_Seed<uint16_t>(10000U);

  test_div100<uint16_t>(0);
  test_div100<uint16_t>(99);
  test_div100<uint16_t>(UINT16_MAX-100);

  // We expect this to fail - the rounding doesn't do integer promotion
  TEST_ASSERT_EQUAL_UINT16(0, div100((uint16_t)UINT16_MAX));
}

void test_maths_div100_U32(void)
{
  test_div100<uint32_t>(0);
  test_div100_Seed<uint32_t>(100U);
  test_div100_Seed<uint32_t>(10000U);
  test_div100_Seed<uint32_t>(100000000UL);
}

void test_maths_div100_S16(void)
{
  //Check both the signed and unsigned results
  test_div100_Seed<uint16_t>(100U);
  test_div100_Seed<uint16_t>(10000U);

  test_div100<uint16_t>(0);
  test_div100<uint16_t>(UINT16_MAX-100);

  test_div100_Seed<int16_t>(-100);
  test_div100_Seed<int16_t>(-10000);

  test_div100<int16_t>(INT16_MIN+100);

  // We expect this to fail - the rounding doesn't do integer promotion
  TEST_ASSERT_EQUAL_UINT16(327, div100((int16_t)INT16_MIN));
}

void test_maths_div100_S32(void)
{
  //Check both the signed and unsigned results
#if defined(__arm__)
  test_div100_Seed<int>(100U);
  test_div100_Seed<int>(10000U);
  test_div100_Seed<int>(100000000UL);

  test_div100_Seed<int>(-100);
  test_div100_Seed<int>(-10000);
  test_div100_Seed<int>(-100000000);
#else
  test_div100_Seed<int32_t>(100U);
  test_div100_Seed<int32_t>(10000U);
  test_div100_Seed<int32_t>(100000000UL);

  test_div100_Seed<int32_t>(-100);
  test_div100_Seed<int32_t>(-10000);
  test_div100_Seed<int32_t>(-100000000);
#endif
}

static void test_div360(uint32_t testValue) {
  assert_rounded_div(testValue, 360, div360(testValue));
}
void test_maths_div360(void)
{
  test_div360(10000000UL-360UL);
  test_div360((10000000UL-180UL)-1UL);
  test_div360(10000000UL-180UL);
  test_div360((10000000UL-180UL)+1UL);
  test_div360(10000000UL);
  test_div360((10000000UL+180UL)-1UL);
  test_div360(10000000UL+180UL);
  test_div360((10000000UL+180UL)+1UL);
  test_div360(10000000UL+360UL);

  test_div360((360UL*MICROS_PER_DEG_1_RPM)/MAX_RPM); // Min revolution time
  test_div360((360UL*MICROS_PER_DEG_1_RPM)/MIN_RPM); // Max revolution time
}

void assert_udiv_32_16(uint32_t dividend, uint16_t divisor) {
    TEST_ASSERT_EQUAL_UINT16(dividend/(uint32_t)divisor, udiv_32_16(dividend, divisor));
}

void test_maths_udiv_32_16(void)
{
  // Divide by zero
  TEST_ASSERT_EQUAL_UINT16(65535, udiv_32_16(0, 0));

  // Result doesn't fit into 16-bits
  TEST_ASSERT_EQUAL_UINT16(32768, udiv_32_16(UINT32_MAX, UINT16_MAX));

  assert_udiv_32_16(1, 1);
  assert_udiv_32_16(UINT16_MAX+1, UINT16_MAX);
  assert_udiv_32_16(UINT16_MAX-1, UINT16_MAX);
  assert_udiv_32_16(MICROS_PER_MIN, 60000); // 1000 RPM
  assert_udiv_32_16(MICROS_PER_MIN, 54005); // 1111 RPM
  assert_udiv_32_16(MICROS_PER_MIN, 7590);  // 7905 RPM
  assert_udiv_32_16(MICROS_PER_MIN, 7715);  // 7777 RPM  
  assert_udiv_32_16(MICROS_PER_MIN, 3333);  // 18000 RPM  
}


void assert_udiv_32_16_closest(uint32_t dividend, uint16_t divisor) {
    assert_rounded_div(dividend, divisor, udiv_32_16_closest(dividend, divisor));
}

void test_maths_udiv_32_16_closest(void)
{
  // Divide by zero
  TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, udiv_32_16_closest(0, 0));

  // Result doesn't fit into 16-bits
  TEST_ASSERT_EQUAL_UINT16(32768, udiv_32_16_closest(UINT32_MAX-(UINT16_MAX/2)-1, UINT16_MAX));

  assert_udiv_32_16(1, 1);
  assert_udiv_32_16(2, 3);
  assert_udiv_32_16(UINT16_MAX+1, UINT16_MAX);
  assert_udiv_32_16(UINT16_MAX-1, UINT16_MAX);
  assert_udiv_32_16(60000000, 60000); // 1000 RPM
  assert_udiv_32_16(60000000, 54005); // 1111 RPM
  assert_udiv_32_16(60000000, 7590);  // 7905 RPM
  assert_udiv_32_16(60000000, 7715);  // 7777 RPM  
  assert_udiv_32_16(60000000, 3333);  // 18000 RPM  
}

static uint32_t indexToDividend(int16_t index) {
  return (uint32_t)index  + (UINT16_MAX*index);
}
void test_maths_udiv_32_16_perf(void)
{
    uint16_t iters = 32;
    uint16_t start_index = UINT16_MAX/3;
    uint16_t end_index = UINT16_MAX/3*2;
    uint16_t step = 111;

    timer native_timer;
    uint32_t checkSumNative = 0;
    auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += (uint32_t)indexToDividend(index) / (uint32_t)index; };
    measure_executiontime<uint16_t, uint32_t&>(iters, start_index, end_index, step, native_timer, checkSumNative, nativeTest);

    timer optimized_timer;
    uint32_t checkSumOptimized = 0;
    auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += udiv_32_16(indexToDividend(index), index); };
    measure_executiontime<uint16_t, uint32_t&>(iters, start_index, end_index, step, optimized_timer, checkSumOptimized, optimizedTest);

    TEST_ASSERT_EQUAL(checkSumNative, checkSumOptimized);
    char buffer[256];
    sprintf(buffer, "muldiv u16 timing: %lu, %lu", native_timer.duration_micros(), optimized_timer.duration_micros());
    TEST_MESSAGE(buffer);
    TEST_ASSERT_LESS_THAN(native_timer.duration_micros(), optimized_timer.duration_micros());
}

void test_maths_div100_s16_perf(void)
{
    constexpr int16_t iters = 1;
    constexpr int16_t start_index = -10000;
    constexpr int16_t end_index = -1;
    constexpr int16_t step = 11;

    timer native_timer;
    int32_t checkSumNative = 0;
    auto nativeTest = [] (int16_t index, int32_t &checkSum) { checkSum += (int16_t)index / (int16_t)100; };
    measure_executiontime<int16_t, int32_t&>(iters, start_index, end_index, step, native_timer, checkSumNative, nativeTest);

    timer optimized_timer;
    int32_t checkSumOptimized = 0;
    auto optimizedTest = [] (int16_t index, int32_t &checkSum) { checkSum += div100(index); };
    measure_executiontime<int16_t, int32_t&>(iters, start_index, end_index, step, optimized_timer, checkSumOptimized, optimizedTest);

    // The checksums will be different due to rounding. This is only
    // here to force the compiler to run the loops above
    TEST_ASSERT_INT32_WITHIN(10000, checkSumNative, checkSumOptimized);

    char buffer[256];
    sprintf(buffer, "div100 s16 timing: %lu, %lu", native_timer.duration_micros(), optimized_timer.duration_micros());
    TEST_MESSAGE(buffer);
    TEST_ASSERT_LESS_THAN(native_timer.duration_micros(), optimized_timer.duration_micros());
}


void test_maths_div100_s32_perf(void)
{
    constexpr int32_t iters = 1;
    constexpr int32_t start_index = -1439190;
    constexpr int32_t end_index = -1;
    constexpr int32_t step = 3715;

    timer native_timer;
    int32_t checkSumNative = 0;
    auto nativeTest = [] (int32_t index, int32_t &checkSum) { checkSum += (int32_t)index / (int32_t)100; };
    measure_executiontime<int32_t, int32_t&>(iters, start_index, end_index, step, native_timer, checkSumNative, nativeTest);

    timer optimized_timer;
    int32_t checkSumOptimized = 0;
    auto optimizedTest = [] (int32_t index, int32_t &checkSum) { checkSum += div100(index); };
    measure_executiontime<int32_t, int32_t&>(iters, start_index, end_index, step, optimized_timer, checkSumOptimized, optimizedTest);

    // The checksums will be different due to rounding. This is only
    // here to force the compiler to run the loops above
    TEST_ASSERT_INT32_WITHIN(10000, checkSumNative, checkSumOptimized);

    char buffer[256];
    sprintf(buffer, "div100 s32 timing: %lu, %lu", native_timer.duration_micros(), optimized_timer.duration_micros());
    TEST_MESSAGE(buffer);
    TEST_ASSERT_LESS_THAN(native_timer.duration_micros(), optimized_timer.duration_micros());
}

void testMaths()
{
  RUN_TEST(test_maths_percent_U8);
  RUN_TEST(test_maths_percent_U16);
  RUN_TEST(test_maths_percent_U32);
  RUN_TEST(test_maths_halfpercent_U8);
  RUN_TEST(test_maths_halfpercent_U16);
  RUN_TEST(test_maths_halfpercent_U32);
  RUN_TEST(test_maths_div100_U16);
  RUN_TEST(test_maths_div100_U32);
  RUN_TEST(test_maths_div100_S16);
  RUN_TEST(test_maths_div100_S32);
  RUN_TEST(test_maths_udiv_32_16);
  RUN_TEST(test_maths_udiv_32_16_closest);
  RUN_TEST(test_maths_udiv_32_16_perf);
  RUN_TEST(test_maths_div360);
  RUN_TEST(test_maths_div100_s16_perf);
  RUN_TEST(test_maths_div100_s32_perf);
}
