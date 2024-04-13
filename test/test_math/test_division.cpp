#include <unity.h>
#include "test_fp_support.h"
#include "maths.h"
#include "crankMaths.h"
#include "../timer.hpp"
#include "../test_utils.h"


template <typename T>
static void test_div100(T testValue) {
  assert_rounded_div(testValue, (T)100, (T)div100(testValue));
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

  test_div100<int16_t>((int16_t)(INT16_MIN+100));

  // We expect this to fail - the rounding doesn't do integer promotion
  TEST_ASSERT_EQUAL_INT16(327, div100((int16_t)INT16_MIN));
}

void test_maths_div100_S32(void)
{
  //Check both the signed and unsigned results
  test_div100_Seed<int32_t>(100U);
  test_div100_Seed<int32_t>(10000U);
  test_div100_Seed<int32_t>(100000000UL);

  test_div100_Seed<int32_t>(-100);
  test_div100_Seed<int32_t>(-10000);
  test_div100_Seed<int32_t>(-100000000);
}

static void test_div360(uint32_t testValue) {
  assert_rounded_div(testValue, (uint32_t)360U, div360(testValue));
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
#if defined(__AVR__)
  // Divide by zero
  TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, udiv_32_16(0, 0));

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
#endif
}


void assert_udiv_32_16_closest(uint32_t dividend, uint16_t divisor) {
    assert_rounded_div(dividend, (uint32_t)divisor, (uint32_t)udiv_32_16_closest(dividend, divisor));
}

void test_maths_udiv_32_16_closest(void)
{
#if defined(__AVR__)
  // Divide by zero
  TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, udiv_32_16_closest(0, 0));

  // Result doesn't fit into 16-bits
  TEST_ASSERT_EQUAL_UINT16(32768, udiv_32_16_closest(UINT32_MAX-(UINT16_MAX/2)-1, UINT16_MAX));

  assert_udiv_32_16(1, 1);
  assert_udiv_32_16(2, 3);
  assert_udiv_32_16(UINT16_MAX+1, UINT16_MAX);
  assert_udiv_32_16(UINT16_MAX-1, UINT16_MAX);
  assert_udiv_32_16(MICROS_PER_MIN, 60000); // 1000 RPM
  assert_udiv_32_16(MICROS_PER_MIN, 54005); // 1111 RPM
  assert_udiv_32_16(MICROS_PER_MIN, 7590);  // 7905 RPM
  assert_udiv_32_16(MICROS_PER_MIN, 7715);  // 7777 RPM  
  assert_udiv_32_16(MICROS_PER_MIN, 3333);  // 18000 RPM  
#endif
}

#if defined(__AVR__)
static uint32_t indexToDividend(int16_t index) {
  return (UINT16_MAX*index)-(uint32_t)index;
}
#endif

void test_maths_udiv_32_16_perf(void)
{
#if defined(__AVR__)
    uint16_t iters = 32;
    uint16_t start_index = UINT16_MAX/3;
    uint16_t end_index = UINT16_MAX/3*2;
    uint16_t step = 111;

    auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += (uint32_t)indexToDividend(index) / (uint32_t)index; };
    auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += udiv_32_16(indexToDividend(index), index); };
    auto comparison = compare_executiontime<uint16_t, uint32_t>(iters, start_index, end_index, step, nativeTest, optimizedTest);
    
    // The checksums will be different due to rounding. This is only
    // here to force the compiler to run the loops above
    TEST_ASSERT_INT32_WITHIN(UINT32_MAX/2, comparison.timeA.result, comparison.timeB.result);

    TEST_ASSERT_LESS_THAN(comparison.timeA.durationMicros, comparison.timeB.durationMicros);
#endif
}

void test_maths_div100_s16_perf(void)
{
#if defined(__AVR__)
    constexpr int16_t iters = 1;
    constexpr int16_t start_index = -10000;
    constexpr int16_t end_index = -1;
    constexpr int16_t step = 11;

    auto nativeTest = [] (int16_t index, int32_t &checkSum) { checkSum += (int16_t)index / (int16_t)100; };
    auto optimizedTest = [] (int16_t index, int32_t &checkSum) { checkSum += div100(index); };
    auto comparison = compare_executiontime<int16_t, int32_t>(iters, start_index, end_index, step, nativeTest, optimizedTest);
    
    // The checksums will be different due to rounding. This is only
    // here to force the compiler to run the loops above
    TEST_ASSERT_INT32_WITHIN(UINT32_MAX/2, comparison.timeA.result, comparison.timeB.result);

    TEST_ASSERT_LESS_THAN(comparison.timeA.durationMicros, comparison.timeB.durationMicros);
#endif
}

void test_maths_div10_s16_perf(void)
{
  // Unit test to confirm using div100 to divide by 10 is quicker than straight division by 10.
#if defined(__AVR__)
  constexpr int16_t iters = 1;
  constexpr int16_t start_index = -3213;
  constexpr int16_t end_index = 3213;
  constexpr int16_t step = 17;
  
  auto nativeTest = [] (int16_t index, int32_t &checkSum) { checkSum += (int16_t)index / (int16_t)10; };
  auto optimizedTest = [] (int16_t index, int32_t &checkSum) { checkSum += div100((int16_t)(index * 10)); };
  auto comparison = compare_executiontime<int16_t, int32_t>(iters, start_index, end_index, step, nativeTest, optimizedTest);
  
  // The checksums will be different due to rounding. This is only
  // here to force the compiler to run the loops above
  TEST_ASSERT_INT32_WITHIN(UINT32_MAX/2, comparison.timeA.result, comparison.timeB.result);

  TEST_ASSERT_LESS_THAN(comparison.timeA.durationMicros, comparison.timeB.durationMicros);
#endif
}

void test_maths_div100_s32_perf(void)
{
#if defined(__AVR__)
    constexpr int32_t iters = 1;
    constexpr int32_t start_index = -1439190;
    constexpr int32_t end_index = -1;
    constexpr int32_t step = 3715;

    auto nativeTest = [] (int32_t index, int32_t &checkSum) { checkSum += (int32_t)index / (int32_t)100; };
    auto optimizedTest = [] (int32_t index, int32_t &checkSum) { checkSum += div100(index); };
    auto comparison = compare_executiontime<int32_t, int32_t>(iters, start_index, end_index, step, nativeTest, optimizedTest);
    
    // The checksums will be different due to rounding. This is only
    // here to force the compiler to run the loops above
    TEST_ASSERT_INT32_WITHIN(UINT32_MAX/2, comparison.timeA.result, comparison.timeB.result);

    TEST_ASSERT_LESS_THAN(comparison.timeA.durationMicros, comparison.timeB.durationMicros);
#endif
}


static void assert_udiv_16_8(uint16_t dividend, uint8_t divisor) {
    TEST_ASSERT_EQUAL_UINT16(dividend/(uint16_t)divisor, udiv_16_8(dividend, divisor));
}

static void test_maths_udiv_16_8(void)
{
#if defined(__AVR__)
  // Divide by zero
  TEST_ASSERT_EQUAL_UINT16(UINT8_MAX, udiv_16_8(0, 0));

  // Result doesn't fit into 8-bits
  TEST_ASSERT_EQUAL_UINT16(UINT8_MAX, udiv_16_8(UINT16_MAX, UINT8_MAX));

  assert_udiv_16_8(1, 1);
  assert_udiv_16_8(UINT8_MAX+1, UINT8_MAX);
  assert_udiv_16_8(UINT8_MAX-1, UINT8_MAX);
  // Below are from an idle target table in a real tune
  assert_udiv_16_8(150, 30); 
  assert_udiv_16_8(70, 14);
  assert_udiv_16_8(60, 25);
  assert_udiv_16_8(40, 9);
  // Artificial
  assert_udiv_16_8(UINT8_MAX*7-1, 7); 
#endif
}

static uint32_t indexToDividend(uint8_t index) {
  return (UINT8_MAX*(uint32_t)index) - (uint32_t)index;
}
void test_maths_udiv_16_8_perf(void)
{
#if defined(__AVR__)
  uint16_t iters = 32;
  uint8_t start_index = 3;
  uint8_t end_index = 255;
  uint8_t step = 1;

  auto nativeTest = [] (uint8_t index, uint32_t &checkSum) { checkSum += (uint16_t)indexToDividend(index) / (uint16_t)index; };
  auto optimizedTest = [] (uint8_t index, uint32_t &checkSum) { checkSum += udiv_16_8(indexToDividend(index), index); };
  auto comparison = compare_executiontime<uint8_t, uint32_t>(iters, start_index, end_index, step, nativeTest, optimizedTest);
  
  // The checksums will be different due to rounding. This is only
  // here to force the compiler to run the loops above
  TEST_ASSERT_INT32_WITHIN(UINT32_MAX/2, comparison.timeA.result, comparison.timeB.result);

  TEST_ASSERT_LESS_THAN(comparison.timeA.durationMicros, comparison.timeB.durationMicros);
#endif
}

void testDivision(void) {
  SET_UNITY_FILENAME() {

  RUN_TEST(test_maths_div100_U16);
  RUN_TEST(test_maths_div100_U32);
  RUN_TEST(test_maths_div100_S16);
  RUN_TEST(test_maths_div100_S32);
  RUN_TEST(test_maths_udiv_32_16);
  RUN_TEST(test_maths_udiv_32_16_closest);
  RUN_TEST(test_maths_udiv_32_16_perf);
  RUN_TEST(test_maths_div360);
  RUN_TEST(test_maths_div100_s16_perf);
  RUN_TEST(test_maths_div10_s16_perf);
  RUN_TEST(test_maths_div100_s32_perf);
  RUN_TEST(test_maths_udiv_16_8);
  RUN_TEST(test_maths_udiv_16_8_perf);  
  }
}