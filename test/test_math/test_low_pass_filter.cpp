#include <unity.h>
#include "maths.h"
#include "../test_utils.h"

static void test_U16_min(void) {
    // Passing zero for the filter value should return the input value
    TEST_ASSERT_EQUAL_UINT16(0,     LOW_PASS_FILTER(0U,    0U, 0U));
    TEST_ASSERT_EQUAL_UINT16(1234U, LOW_PASS_FILTER(1234U, 0U, 0U));
    TEST_ASSERT_EQUAL_UINT16(1234U, LOW_PASS_FILTER(1234U, 0U, 9999U));
}

static void test_U16_max(void) {
    // Passing UINT8_MAX for the filter value should make the input close to the previous value
    TEST_ASSERT_EQUAL_UINT16(0,     LOW_PASS_FILTER(0U,    UINT8_MAX, 0U));
    TEST_ASSERT_EQUAL_UINT16(4U,    LOW_PASS_FILTER(1234U, UINT8_MAX, 0U));
    TEST_ASSERT_EQUAL_UINT16(9964U, LOW_PASS_FILTER(1234U, UINT8_MAX, 9999U));
}

static void test_S16_min(void) {
    // Passing zero for the filter value should return the input value
    TEST_ASSERT_EQUAL_INT16(0,    LOW_PASS_FILTER(0,   0U, 0));
    TEST_ASSERT_EQUAL_INT16(1234, LOW_PASS_FILTER(1234, 0U, 0));
    TEST_ASSERT_EQUAL_INT16(-1234, LOW_PASS_FILTER(-1234, 0U, 0));
    TEST_ASSERT_EQUAL_INT16(1234, LOW_PASS_FILTER(1234, 0U, 9999));
    TEST_ASSERT_EQUAL_INT16(-1234, LOW_PASS_FILTER(-1234, 0U, 9999));
}

static void test_S16_max(void) {
    // Passing UINT8_MAX for the filter value should make the input close to the previous value
    TEST_ASSERT_EQUAL_INT16(0,    LOW_PASS_FILTER(0,   UINT8_MAX, 0));
    TEST_ASSERT_EQUAL_INT16(4,    LOW_PASS_FILTER(1234, UINT8_MAX, 0));
    TEST_ASSERT_EQUAL_INT16(-4,    LOW_PASS_FILTER(-1234, UINT8_MAX, 0));
    TEST_ASSERT_EQUAL_INT16(9964, LOW_PASS_FILTER(1234, UINT8_MAX, 9999));
    TEST_ASSERT_EQUAL_INT16(-9964, LOW_PASS_FILTER(-1234, UINT8_MAX, -9999));
}

void test_LOW_PASS_FILTER(void) {
  SET_UNITY_FILENAME() {
    RUN_TEST(test_U16_min);
    RUN_TEST(test_U16_max);
    RUN_TEST(test_S16_min);
    RUN_TEST(test_S16_max);
  }    
}