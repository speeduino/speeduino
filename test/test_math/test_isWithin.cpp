#include <unity.h>
#include "maths.h"
#include "../test_utils.h"

// Test unsigned integer types
static void test_isWithin_U8_inside_range(void) {
    TEST_ASSERT_TRUE(isWithin<uint8_t>(5, 1, 10));
    TEST_ASSERT_TRUE(isWithin<uint8_t>(1, 1, 10));
    TEST_ASSERT_TRUE(isWithin<uint8_t>(10, 1, 10));
    TEST_ASSERT_TRUE(isWithin<uint8_t>(3, 3, 3));
}

static void test_isWithin_U8_outside_range(void) {
    TEST_ASSERT_FALSE(isWithin<uint8_t>(0, 1, 10));
    TEST_ASSERT_FALSE(isWithin<uint8_t>(11, 1, 10));
    TEST_ASSERT_FALSE(isWithin<uint8_t>(100, 1, 10));
}

static void test_isWithin_U16_inside_range(void) {
    TEST_ASSERT_TRUE(isWithin<uint16_t>(500, 100, 1000));
    TEST_ASSERT_TRUE(isWithin<uint16_t>(100, 100, 1000));
    TEST_ASSERT_TRUE(isWithin<uint16_t>(1000, 100, 1000));
}

static void test_isWithin_U16_outside_range(void) {
    TEST_ASSERT_FALSE(isWithin<uint16_t>(99, 100, 1000));
    TEST_ASSERT_FALSE(isWithin<uint16_t>(1001, 100, 1000));
}

// Test signed integer types
static void test_isWithin_S16_inside_range(void) {
    TEST_ASSERT_TRUE(isWithin<int16_t>(0, -10, 10));
    TEST_ASSERT_TRUE(isWithin<int16_t>(-10, -10, 10));
    TEST_ASSERT_TRUE(isWithin<int16_t>(10, -10, 10));
    TEST_ASSERT_TRUE(isWithin<int16_t>(-5, -10, 10));
    TEST_ASSERT_TRUE(isWithin<int16_t>(5, -10, 10));
}

static void test_isWithin_S16_outside_range(void) {
    TEST_ASSERT_FALSE(isWithin<int16_t>(-11, -10, 10));
    TEST_ASSERT_FALSE(isWithin<int16_t>(11, -10, 10));
    TEST_ASSERT_FALSE(isWithin<int16_t>(100, -10, 10));
    TEST_ASSERT_FALSE(isWithin<int16_t>(-100, -10, 10));
}

// Test negative ranges
static void test_isWithin_S16_negative_range(void) {
    TEST_ASSERT_TRUE(isWithin<int16_t>(-50, -100, -10));
    TEST_ASSERT_TRUE(isWithin<int16_t>(-100, -100, -10));
    TEST_ASSERT_TRUE(isWithin<int16_t>(-10, -100, -10));
    TEST_ASSERT_FALSE(isWithin<int16_t>(-101, -100, -10));
    TEST_ASSERT_FALSE(isWithin<int16_t>(-9, -100, -10));
}

// Test edge cases - reversed range (should still work)
static void test_isWithin_reversed_range(void) {
    // When lower > upper, it should return false (no valid range)
    TEST_ASSERT_FALSE(isWithin<uint8_t>(5, 10, 1));
}

// Test with hysteresis-like scenarios (similar to idle.cpp usage)
static void test_isWithin_hysteresis_scenario(void) {
    // Simulate the hysteresis check from idle.cpp
    // target should be within [cur - hyst, cur + hyst]
    uint16_t curIdleStep = 100;
    uint8_t hysteresis = 5;
    
    // Test values within hysteresis range
    TEST_ASSERT_TRUE(isWithin<uint16_t>(curIdleStep - hysteresis, curIdleStep - hysteresis, curIdleStep + hysteresis));
    TEST_ASSERT_TRUE(isWithin<uint16_t>(curIdleStep, curIdleStep - hysteresis, curIdleStep + hysteresis));
    TEST_ASSERT_TRUE(isWithin<uint16_t>(curIdleStep + hysteresis, curIdleStep - hysteresis, curIdleStep + hysteresis));
    TEST_ASSERT_TRUE(isWithin<uint16_t>(curIdleStep - 3, curIdleStep - hysteresis, curIdleStep + hysteresis));
    TEST_ASSERT_TRUE(isWithin<uint16_t>(curIdleStep + 3, curIdleStep - hysteresis, curIdleStep + hysteresis));
    
    // Test values outside hysteresis range
    TEST_ASSERT_FALSE(isWithin<uint16_t>(curIdleStep - hysteresis - 1, curIdleStep - hysteresis, curIdleStep + hysteresis));
    TEST_ASSERT_FALSE(isWithin<uint16_t>(curIdleStep + hysteresis + 1, curIdleStep - hysteresis, curIdleStep + hysteresis));
}

// Main test function
void test_isWithin(void) {
  SET_UNITY_FILENAME() {
    RUN_TEST(test_isWithin_U8_inside_range);
    RUN_TEST(test_isWithin_U8_outside_range);
    RUN_TEST(test_isWithin_U16_inside_range);
    RUN_TEST(test_isWithin_U16_outside_range);
    RUN_TEST(test_isWithin_S16_inside_range);
    RUN_TEST(test_isWithin_S16_outside_range);
    RUN_TEST(test_isWithin_S16_negative_range);
    RUN_TEST(test_isWithin_reversed_range);
    RUN_TEST(test_isWithin_hysteresis_scenario);
  }    
}

