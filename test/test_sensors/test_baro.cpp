#include "../test_utils.h"
#include "globals.h"

extern bool isValidBaro(uint16_t baro);

static void test_isValidBaro_inRange(void)
{
  // Values inside the physical plausibility window [65, 108] kPa are valid.
  TEST_ASSERT_TRUE(isValidBaro(65U));
  TEST_ASSERT_TRUE(isValidBaro(100U));
  TEST_ASSERT_TRUE(isValidBaro(108U));
}

static void test_isValidBaro_outOfRange(void)
{
  // Just outside the window on either side is rejected.
  TEST_ASSERT_FALSE(isValidBaro(64U));
  TEST_ASSERT_FALSE(isValidBaro(109U));
  TEST_ASSERT_FALSE(isValidBaro(0U));
}

static void test_isValidBaro_noTruncationAbove255(void)
{
  // A MAP-derived reading can exceed 255 kPa on boosted / high-mapMax setups.
  // The parameter must be wide enough that such a reading is rejected rather
  // than truncated modulo 256 into the valid window (e.g. 356 & 0xFF == 100,
  // which would previously have passed as a valid baro reading).
  TEST_ASSERT_FALSE(isValidBaro(356U));
  TEST_ASSERT_FALSE(isValidBaro(300U));
  TEST_ASSERT_FALSE(isValidBaro(65535U));
}

void test_baro(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_isValidBaro_inRange);
    RUN_TEST(test_isValidBaro_outOfRange);
    RUN_TEST(test_isValidBaro_noTruncationAbove255);
  }
}
