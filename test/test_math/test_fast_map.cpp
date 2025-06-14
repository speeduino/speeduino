#include <unity.h>
#include "maths.h"
#include "../test_utils.h"

// Test normal range mapping
static void test_fast_map_normal_range(void) {
    TEST_ASSERT_EQUAL_UINT8(0, fast_map(0, 0, 100, 0, 255));
    TEST_ASSERT_EQUAL_UINT8(127, fast_map(50, 0, 100, 0, 255));
    TEST_ASSERT_EQUAL_UINT8(255, fast_map(100, 0, 100, 0, 255));
}

// Test edge cases
static void test_fast_map_edge_cases(void) {
    // Input equals start of range
    TEST_ASSERT_EQUAL_UINT8(0, fast_map(10, 10, 20, 0, 100));
    // Input equals end of range
    TEST_ASSERT_EQUAL_UINT8(100, fast_map(20, 10, 20, 0, 100));
    // Single point range
    TEST_ASSERT_EQUAL_UINT8(50, fast_map(10, 10, 10, 50, 50));
}

// Test negative input ranges
static void test_fast_map_negative_ranges(void) {
    TEST_ASSERT_EQUAL_UINT8(127, fast_map(0, -100, 100, 0, 255));
    TEST_ASSERT_EQUAL_UINT8(0, fast_map(-100, -100, 100, 0, 255));
    TEST_ASSERT_EQUAL_UINT8(255, fast_map(100, -100, 100, 0, 255));
}

// Test reversed output range
static void test_fast_map_reversed_range(void) {
    TEST_ASSERT_EQUAL_UINT8(255, fast_map(0, 0, 100, 255, 0));
    TEST_ASSERT_EQUAL_UINT8(128, fast_map(50, 0, 100, 255, 0));
    TEST_ASSERT_EQUAL_UINT8(0, fast_map(100, 0, 100, 255, 0));
}

// Main test function

void test_fast_map(void) {
  SET_UNITY_FILENAME() {
    RUN_TEST(test_fast_map_normal_range);
    RUN_TEST(test_fast_map_edge_cases);
    RUN_TEST(test_fast_map_negative_ranges);
    RUN_TEST(test_fast_map_reversed_range);    
  }    
}