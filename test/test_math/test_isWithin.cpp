#include <unity.h>
#include "maths.h"
#include "../test_utils.h"

// ============================ isWithinInclusive ============================

// Test unsigned integer types
static void test_isWithinInclusive_U8_inside_range(void) {
    TEST_ASSERT_TRUE(isWithinInclusive<uint8_t>(5, 1, 10));
    TEST_ASSERT_TRUE(isWithinInclusive<uint8_t>(1, 1, 10));
    TEST_ASSERT_TRUE(isWithinInclusive<uint8_t>(10, 1, 10));
    TEST_ASSERT_TRUE(isWithinInclusive<uint8_t>(3, 3, 3));
}

static void test_isWithinInclusive_U8_outside_range(void) {
    TEST_ASSERT_FALSE(isWithinInclusive<uint8_t>(0, 1, 10));
    TEST_ASSERT_FALSE(isWithinInclusive<uint8_t>(11, 1, 10));
    TEST_ASSERT_FALSE(isWithinInclusive<uint8_t>(100, 1, 10));
}

static void test_isWithinInclusive_U16_inside_range(void) {
    TEST_ASSERT_TRUE(isWithinInclusive<uint16_t>(500, 100, 1000));
    TEST_ASSERT_TRUE(isWithinInclusive<uint16_t>(100, 100, 1000));
    TEST_ASSERT_TRUE(isWithinInclusive<uint16_t>(1000, 100, 1000));
}

static void test_isWithinInclusive_U16_outside_range(void) {
    TEST_ASSERT_FALSE(isWithinInclusive<uint16_t>(99, 100, 1000));
    TEST_ASSERT_FALSE(isWithinInclusive<uint16_t>(1001, 100, 1000));
}

// Test signed integer types
static void test_isWithinInclusive_S16_inside_range(void) {
    TEST_ASSERT_TRUE(isWithinInclusive<int16_t>(0, -10, 10));
    TEST_ASSERT_TRUE(isWithinInclusive<int16_t>(-10, -10, 10));
    TEST_ASSERT_TRUE(isWithinInclusive<int16_t>(10, -10, 10));
    TEST_ASSERT_TRUE(isWithinInclusive<int16_t>(-5, -10, 10));
    TEST_ASSERT_TRUE(isWithinInclusive<int16_t>(5, -10, 10));
}

static void test_isWithinInclusive_S16_outside_range(void) {
    TEST_ASSERT_FALSE(isWithinInclusive<int16_t>(-11, -10, 10));
    TEST_ASSERT_FALSE(isWithinInclusive<int16_t>(11, -10, 10));
    TEST_ASSERT_FALSE(isWithinInclusive<int16_t>(100, -10, 10));
    TEST_ASSERT_FALSE(isWithinInclusive<int16_t>(-100, -10, 10));
}

// Test negative ranges
static void test_isWithinInclusive_S16_negative_range(void) {
    TEST_ASSERT_TRUE(isWithinInclusive<int16_t>(-50, -100, -10));
    TEST_ASSERT_TRUE(isWithinInclusive<int16_t>(-100, -100, -10));
    TEST_ASSERT_TRUE(isWithinInclusive<int16_t>(-10, -100, -10));
    TEST_ASSERT_FALSE(isWithinInclusive<int16_t>(-101, -100, -10));
    TEST_ASSERT_FALSE(isWithinInclusive<int16_t>(-9, -100, -10));
}

// Test edge cases - reversed range (should still work)
static void test_isWithinInclusive_reversed_range(void) {
    // When lower > upper, it should return false (no valid range)
    TEST_ASSERT_FALSE(isWithinInclusive<uint8_t>(5, 10, 1));
}

// Test with hysteresis-like scenarios (similar to idle.cpp usage)
static void test_isWithinInclusive_hysteresis_scenario(void) {
    // Simulate the hysteresis check from idle.cpp
    // target should be within [cur - hyst, cur + hyst]
    uint16_t curIdleStep = 100;
    uint8_t hysteresis = 5;

    // Test values within hysteresis range
    TEST_ASSERT_TRUE(isWithinInclusive<uint16_t>(curIdleStep - hysteresis, curIdleStep - hysteresis, curIdleStep + hysteresis));
    TEST_ASSERT_TRUE(isWithinInclusive<uint16_t>(curIdleStep, curIdleStep - hysteresis, curIdleStep + hysteresis));
    TEST_ASSERT_TRUE(isWithinInclusive<uint16_t>(curIdleStep + hysteresis, curIdleStep - hysteresis, curIdleStep + hysteresis));
    TEST_ASSERT_TRUE(isWithinInclusive<uint16_t>(curIdleStep - 3, curIdleStep - hysteresis, curIdleStep + hysteresis));
    TEST_ASSERT_TRUE(isWithinInclusive<uint16_t>(curIdleStep + 3, curIdleStep - hysteresis, curIdleStep + hysteresis));

    // Test values outside hysteresis range
    TEST_ASSERT_FALSE(isWithinInclusive<uint16_t>(curIdleStep - hysteresis - 1, curIdleStep - hysteresis, curIdleStep + hysteresis));
    TEST_ASSERT_FALSE(isWithinInclusive<uint16_t>(curIdleStep + hysteresis + 1, curIdleStep - hysteresis, curIdleStep + hysteresis));
}

// ============================ isWithinExclusive ============================

static void test_isWithinExclusive_U8_inside_range(void) {
    TEST_ASSERT_TRUE(isWithinExclusive<uint8_t>(5, 1, 10));
    TEST_ASSERT_TRUE(isWithinExclusive<uint8_t>(2, 1, 10));
    TEST_ASSERT_TRUE(isWithinExclusive<uint8_t>(9, 1, 10));
}

static void test_isWithinExclusive_U8_bounds_are_outside(void) {
    // The bounds themselves are NOT within the range (exclusive semantics)
    TEST_ASSERT_FALSE(isWithinExclusive<uint8_t>(1, 1, 10));
    TEST_ASSERT_FALSE(isWithinExclusive<uint8_t>(10, 1, 10));
    // Degenerate range: equal bounds contain nothing
    TEST_ASSERT_FALSE(isWithinExclusive<uint8_t>(3, 3, 3));
}

static void test_isWithinExclusive_U8_outside_range(void) {
    TEST_ASSERT_FALSE(isWithinExclusive<uint8_t>(0, 1, 10));
    TEST_ASSERT_FALSE(isWithinExclusive<uint8_t>(11, 1, 10));
    TEST_ASSERT_FALSE(isWithinExclusive<uint8_t>(100, 1, 10));
}

static void test_isWithinExclusive_S16_inside_range(void) {
    TEST_ASSERT_TRUE(isWithinExclusive<int16_t>(0, -10, 10));
    TEST_ASSERT_TRUE(isWithinExclusive<int16_t>(-9, -10, 10));
    TEST_ASSERT_TRUE(isWithinExclusive<int16_t>(9, -10, 10));
}

static void test_isWithinExclusive_S16_outside_range(void) {
    TEST_ASSERT_FALSE(isWithinExclusive<int16_t>(-10, -10, 10));
    TEST_ASSERT_FALSE(isWithinExclusive<int16_t>(10, -10, 10));
    TEST_ASSERT_FALSE(isWithinExclusive<int16_t>(-11, -10, 10));
    TEST_ASSERT_FALSE(isWithinExclusive<int16_t>(11, -10, 10));
    TEST_ASSERT_FALSE(isWithinExclusive<int16_t>(100, -10, 10));
    TEST_ASSERT_FALSE(isWithinExclusive<int16_t>(-100, -10, 10));
}

static void test_isWithinExclusive_S16_negative_range(void) {
    TEST_ASSERT_TRUE(isWithinExclusive<int16_t>(-50, -100, -10));
    TEST_ASSERT_FALSE(isWithinExclusive<int16_t>(-100, -100, -10));
    TEST_ASSERT_FALSE(isWithinExclusive<int16_t>(-10, -100, -10));
    TEST_ASSERT_FALSE(isWithinExclusive<int16_t>(-101, -100, -10));
    TEST_ASSERT_FALSE(isWithinExclusive<int16_t>(-9, -100, -10));
}

static void test_isWithinExclusive_reversed_range(void) {
    // When lower > upper, it should return false (no valid range)
    TEST_ASSERT_FALSE(isWithinExclusive<uint8_t>(5, 10, 1));
}

// Test with the hysteresis scenario from idle.cpp: the stepper must stay
// energised (check returns false) when the target sits exactly on the
// hysteresis edge - this is the behavioural fix of PR #1289.
static void test_isWithinExclusive_hysteresis_scenario(void) {
    int curIdleStep = 100;
    uint8_t hysteresis = 5;

    // Strictly inside the hysteresis band -> stepper can be disabled
    TEST_ASSERT_TRUE(isWithinExclusive<int>(curIdleStep, curIdleStep - hysteresis, curIdleStep + hysteresis));
    TEST_ASSERT_TRUE(isWithinExclusive<int>(curIdleStep - 4, curIdleStep - hysteresis, curIdleStep + hysteresis));
    TEST_ASSERT_TRUE(isWithinExclusive<int>(curIdleStep + 4, curIdleStep - hysteresis, curIdleStep + hysteresis));

    // Exactly on the hysteresis edge -> NOT within (motor stays energised),
    // unlike the inclusive variant which returns true here
    TEST_ASSERT_FALSE(isWithinExclusive<int>(curIdleStep - hysteresis, curIdleStep - hysteresis, curIdleStep + hysteresis));
    TEST_ASSERT_FALSE(isWithinExclusive<int>(curIdleStep + hysteresis, curIdleStep - hysteresis, curIdleStep + hysteresis));
    TEST_ASSERT_TRUE(isWithinInclusive<int>(curIdleStep - hysteresis, curIdleStep - hysteresis, curIdleStep + hysteresis));
    TEST_ASSERT_TRUE(isWithinInclusive<int>(curIdleStep + hysteresis, curIdleStep - hysteresis, curIdleStep + hysteresis));

    // Outside the hysteresis band -> not within
    TEST_ASSERT_FALSE(isWithinExclusive<int>(curIdleStep - hysteresis - 1, curIdleStep - hysteresis, curIdleStep + hysteresis));
    TEST_ASSERT_FALSE(isWithinExclusive<int>(curIdleStep + hysteresis + 1, curIdleStep - hysteresis, curIdleStep + hysteresis));
}

// Main test function
void test_isWithin(void) {
  SET_UNITY_FILENAME() {
    RUN_TEST(test_isWithinInclusive_U8_inside_range);
    RUN_TEST(test_isWithinInclusive_U8_outside_range);
    RUN_TEST(test_isWithinInclusive_U16_inside_range);
    RUN_TEST(test_isWithinInclusive_U16_outside_range);
    RUN_TEST(test_isWithinInclusive_S16_inside_range);
    RUN_TEST(test_isWithinInclusive_S16_outside_range);
    RUN_TEST(test_isWithinInclusive_S16_negative_range);
    RUN_TEST(test_isWithinInclusive_reversed_range);
    RUN_TEST(test_isWithinInclusive_hysteresis_scenario);
    RUN_TEST(test_isWithinExclusive_U8_inside_range);
    RUN_TEST(test_isWithinExclusive_U8_bounds_are_outside);
    RUN_TEST(test_isWithinExclusive_U8_outside_range);
    RUN_TEST(test_isWithinExclusive_S16_inside_range);
    RUN_TEST(test_isWithinExclusive_S16_outside_range);
    RUN_TEST(test_isWithinExclusive_S16_negative_range);
    RUN_TEST(test_isWithinExclusive_reversed_range);
    RUN_TEST(test_isWithinExclusive_hysteresis_scenario);
  }
}
