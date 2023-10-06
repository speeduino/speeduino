//#include <Arduino.h>
#include <string.h> // memcpy
#include <unity.h>
#include <stdio.h>
#include "tests_maths.h"
#include "maths.h"
#include "..\timer.hpp"


void testMaths()
{
  RUN_TEST(test_maths_percent_U8);
  RUN_TEST(test_maths_percent_U16);
  RUN_TEST(test_maths_percent_U32);
  RUN_TEST(test_maths_halfpercent_U8);
  RUN_TEST(test_maths_halfpercent_U16);
  RUN_TEST(test_maths_halfpercent_U32);
  RUN_TEST(test_maths_div100_U8);
  RUN_TEST(test_maths_div100_U16);
  RUN_TEST(test_maths_div100_U32);
  RUN_TEST(test_maths_div100_S8);
  RUN_TEST(test_maths_div100_S16);
  RUN_TEST(test_maths_div100_S32);
  RUN_TEST(test_maths_udiv_32_16);
  RUN_TEST(test_maths_udiv_32_16_perf);
}

void test_maths_percent_U8(void)
{
  uint8_t percentOf = 200;
  TEST_ASSERT_EQUAL(100, percentage(50,  percentOf));
  TEST_ASSERT_EQUAL(150, percentage(75,  percentOf));
  TEST_ASSERT_EQUAL(0, percentage(0,   percentOf));
  TEST_ASSERT_EQUAL(200, percentage(100, percentOf));
  TEST_ASSERT_EQUAL(250, percentage(125, percentOf));
}

void test_maths_percent_U16(void)
{
  uint16_t percentOf = 20000;
  TEST_ASSERT_EQUAL(10000, percentage(50,  percentOf));
  TEST_ASSERT_EQUAL(15000, percentage(75,  percentOf));
  TEST_ASSERT_EQUAL(0, percentage(0,   percentOf));
  TEST_ASSERT_EQUAL(20000, percentage(100, percentOf));
  TEST_ASSERT_EQUAL(25000, percentage(125, percentOf));
}

void test_maths_percent_U32(void)
{
  uint32_t percentOf = 20000000UL;
  TEST_ASSERT_EQUAL(10000000UL, percentage(50, percentOf));
  TEST_ASSERT_EQUAL(15000000UL, percentage(75, percentOf));
  TEST_ASSERT_EQUAL(0, percentage(0, percentOf));
  TEST_ASSERT_EQUAL(20000000UL, percentage(100, percentOf));
  TEST_ASSERT_EQUAL(25000000UL, percentage(125, percentOf));
}

void test_maths_halfpercent_U8(void)
{
  uint8_t percentOf = 200;
  TEST_ASSERT_EQUAL(50, halfPercentage(50, percentOf));
  TEST_ASSERT_EQUAL(75, halfPercentage(75, percentOf));
  TEST_ASSERT_EQUAL(0, halfPercentage(0, percentOf));
  TEST_ASSERT_EQUAL(100, halfPercentage(100, percentOf));
  TEST_ASSERT_EQUAL(125, halfPercentage(125, percentOf));
}

void test_maths_halfpercent_U16(void)
{
  uint16_t percentOf = 20000;
  TEST_ASSERT_EQUAL(5000, halfPercentage(50, percentOf));
  TEST_ASSERT_EQUAL(7500, halfPercentage(75, percentOf));
  TEST_ASSERT_EQUAL(0, halfPercentage(0, percentOf));
  TEST_ASSERT_EQUAL(10000, halfPercentage(100, percentOf));
  TEST_ASSERT_EQUAL(12500, halfPercentage(125, percentOf));
}

void test_maths_halfpercent_U32(void)
{
  uint32_t percentOf = 20000000UL;
  TEST_ASSERT_EQUAL(5000000UL, halfPercentage(50, percentOf));
  TEST_ASSERT_EQUAL(7500000UL, halfPercentage(75, percentOf));
  TEST_ASSERT_EQUAL(0, halfPercentage(0, percentOf));
  TEST_ASSERT_EQUAL(10000000UL, halfPercentage(100, percentOf));
  TEST_ASSERT_EQUAL(12500000UL, halfPercentage(125, percentOf));
}

void test_maths_div100_U8(void)
{
  TEST_ASSERT_EQUAL_UINT8(1, div100((uint8_t)100U));
  TEST_ASSERT_EQUAL_UINT8(2, div100((uint8_t)200U));
  TEST_ASSERT_EQUAL_UINT8(0, div100((uint8_t)0U));
  TEST_ASSERT_EQUAL_UINT8(0, div100((uint8_t)50U));
  TEST_ASSERT_EQUAL_UINT8(2, div100((uint8_t)250U));
}

void test_maths_div100_U16(void)
{
  TEST_ASSERT_EQUAL_UINT16(100, div100((uint16_t)10000U));
  TEST_ASSERT_EQUAL_UINT16(400, div100((uint16_t)40000U));
}

void test_maths_div100_U32(void)
{
  TEST_ASSERT_EQUAL_UINT32(1000000UL, div100(100000000UL));
  TEST_ASSERT_EQUAL_UINT32(2000000UL, div100(200000000UL));
}

void test_maths_div100_S8(void)
{
  //Check both the signed and unsigned results
  TEST_ASSERT_EQUAL_INT8(1, div100((int8_t)100));
  TEST_ASSERT_EQUAL_INT8(0, div100((int8_t)0));
  TEST_ASSERT_EQUAL_INT8(0, div100((int8_t)50));

  TEST_ASSERT_EQUAL_INT8(-1, div100((int8_t)-100));
  TEST_ASSERT_EQUAL_INT8(0, div100((int8_t)-50));
  TEST_ASSERT_EQUAL_INT8(-1, div100((int8_t)-120));
}

void test_maths_div100_S16(void)
{
  //Check both the signed and unsigned results
  TEST_ASSERT_EQUAL_INT16(100, div100((int16_t)10000));
  TEST_ASSERT_EQUAL_INT16(0, div100((int16_t)0));
  TEST_ASSERT_EQUAL_INT16(0, div100((int16_t)50));

  TEST_ASSERT_EQUAL_INT16(-100, div100((int16_t)-10000));
  TEST_ASSERT_EQUAL_INT16(0, div100((int16_t)-50));
  TEST_ASSERT_EQUAL_INT16(-1, div100((int16_t)-120));
}

void test_maths_div100_S32(void)
{
  //Check both the signed and unsigned results
#if defined(__arm__)
  TEST_ASSERT_EQUAL_INT32(1000000L, div100((int)100000000L));
  TEST_ASSERT_EQUAL_INT32(0, div100((int)0));
  TEST_ASSERT_EQUAL_INT32(0, div100((int)50));

  TEST_ASSERT_EQUAL_INT32(-1000000L, div100((int)-100000000L));
  TEST_ASSERT_EQUAL_INT32(0, div100((int)-50));
  TEST_ASSERT_EQUAL_INT32(-1, div100((int)-120));
#else
  TEST_ASSERT_EQUAL_INT32(1000000L, div100((int32_t)100000000L));
  TEST_ASSERT_EQUAL_INT32(0, div100((int32_t)0));
  TEST_ASSERT_EQUAL_INT32(0, div100((int32_t)50));

  TEST_ASSERT_EQUAL_INT32(-1000000L, div100((int32_t)-100000000L));
  TEST_ASSERT_EQUAL_INT32(0, div100((int32_t)-50));
  TEST_ASSERT_EQUAL_INT32(-1, div100((int32_t)-120));
#endif
}

void assert_udiv_32_16(uint32_t dividend, uint16_t divisor) {
    TEST_ASSERT_EQUAL_UINT16(dividend/(uint32_t)divisor, udiv_32_16(dividend, divisor));
}

void test_maths_udiv_32_16(void)
{
  // Divide by zero
  TEST_ASSERT_EQUAL_UINT16(0, udiv_32_16(0, 0));

  // Result doesn't fit into 16-bits
  TEST_ASSERT_EQUAL_UINT16(32768, udiv_32_16(UINT32_MAX, UINT16_MAX));

  assert_udiv_32_16(1, 1);
  assert_udiv_32_16(UINT16_MAX+1, UINT16_MAX);
  assert_udiv_32_16(UINT16_MAX-1, UINT16_MAX);
  assert_udiv_32_16(60000000, 60000); // 1000 RPM
  assert_udiv_32_16(60000000, 54005); // 1111 RPM
  assert_udiv_32_16(60000000, 7590);  // 7905 RPM
  assert_udiv_32_16(60000000, 7715);  // 7777 RPM  
  assert_udiv_32_16(60000000, 3333);  // 18000 RPM  
}

void test_maths_udiv_32_16_perf(void)
{
    constexpr uint16_t step = 111;
    constexpr uint16_t max_divisor = UINT16_MAX/3*2;
    constexpr uint16_t min_divisor = UINT16_MAX/3;
    constexpr uint16_t iters = 64;

    timer native_timer;
    uint32_t checkSumNative = 0;
    native_timer.start();
    for (uint16_t loop=0; loop<iters; ++loop)
    {
      for (uint16_t a = min_divisor; a < max_divisor; a+=step)
      {
        uint32_t dividend = (uint32_t)a  + (UINT16_MAX*a);
        checkSumNative += (uint32_t)dividend / (uint32_t)a;
      }
    }
    native_timer.stop();

    timer udiv_32_16_timer;
    uint32_t checkSumudiv_32_16 = 0;
    udiv_32_16_timer.start();
    for (uint16_t loop=0; loop<iters; ++loop)
    {
      for (uint16_t a = min_divisor; a < max_divisor; a+=step)
      {
        uint32_t dividend = (uint32_t)a  + (UINT16_MAX*a);
        checkSumudiv_32_16 += udiv_32_16(dividend, a);
      }
    }
    udiv_32_16_timer.stop();

    TEST_ASSERT_EQUAL(checkSumNative, checkSumudiv_32_16);
    char buffer[256];
    sprintf(buffer, "muldiv u16 timing: %lu, %lu", native_timer.duration_micros(), udiv_32_16_timer.duration_micros());
    TEST_MESSAGE(buffer);
    TEST_ASSERT_LESS_THAN(native_timer.duration_micros(), udiv_32_16_timer.duration_micros());

}