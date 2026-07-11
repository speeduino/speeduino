#include <unity.h>
#include "elapsed_time.h"
#include "../test_utils.h"

static void test_timeElapsed_no_rollover(void) {
  TEST_ASSERT_EQUAL_UINT32(0U, timeElapsed(0U, 0U));
  TEST_ASSERT_EQUAL_UINT32(0U, timeElapsed(1000U, 1000U));
  TEST_ASSERT_EQUAL_UINT32(250U, timeElapsed(1250U, 1000U));
  TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, timeElapsed(UINT32_MAX, 0U));
}

static void test_timeElapsed_rollover(void) {
  // The counter wrapped between start and now: modular subtraction stays exact
  TEST_ASSERT_EQUAL_UINT32(1U, timeElapsed(0U, UINT32_MAX));
  TEST_ASSERT_EQUAL_UINT32(20U, timeElapsed(9U, UINT32_MAX - 10U));
  TEST_ASSERT_EQUAL_UINT32(500U, timeElapsed(250U, UINT32_MAX - 249U));
}

static void test_hasIntervalElapsed_no_rollover(void) {
  TEST_ASSERT_FALSE(hasIntervalElapsed(1000U, 1000U, 50U));
  TEST_ASSERT_FALSE(hasIntervalElapsed(1049U, 1000U, 50U));
  // Contract: the boundary is inclusive
  TEST_ASSERT_TRUE(hasIntervalElapsed(1050U, 1000U, 50U));
  TEST_ASSERT_TRUE(hasIntervalElapsed(1051U, 1000U, 50U));
  // A zero interval has always elapsed
  TEST_ASSERT_TRUE(hasIntervalElapsed(0U, 0U, 0U));
}

static void test_hasIntervalElapsed_rollover(void) {
  // start is just before the counter wrap, now is just after it.
  // The naive form `now > start + interval` would be true immediately here
  // (start + interval overflows to a small number), cutting the interval short.
  TEST_ASSERT_FALSE(hasIntervalElapsed(0U, UINT32_MAX - 24U, 50U));
  TEST_ASSERT_FALSE(hasIntervalElapsed(24U, UINT32_MAX - 24U, 50U));
  TEST_ASSERT_TRUE(hasIntervalElapsed(25U, UINT32_MAX - 24U, 50U));
  TEST_ASSERT_TRUE(hasIntervalElapsed(100U, UINT32_MAX - 24U, 50U));

  // The deadline-style failure: now has wrapped, start has not.
  // The naive form `now >= deadline` would never be true, deferring forever.
  TEST_ASSERT_TRUE(hasIntervalElapsed(10U, UINT32_MAX - 100U, 90U));
}

void testElapsedTime(void) {
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_timeElapsed_no_rollover);
    RUN_TEST_P(test_timeElapsed_rollover);
    RUN_TEST_P(test_hasIntervalElapsed_no_rollover);
    RUN_TEST_P(test_hasIntervalElapsed_rollover);
  }
}
