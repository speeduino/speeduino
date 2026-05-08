#include <unity.h>
#include "maths.h"
#include "../test_utils.h"

static void test_random1to100_range(void)
{
  for (uint16_t i = 0U; i < 5000U; ++i)
  {
    uint8_t v = random1to100();
    TEST_ASSERT_TRUE(v >= 1U);
    TEST_ASSERT_TRUE(v <= 100U);
  }
}

static void test_random1to100_distribution(void)
{
  // Over many calls every bucket [1..100] should be hit. This also exercises
  // the do/while rejection branch (a >= 100U) inside random1to100().
  bool seen[101] = { false };
  uint16_t distinct = 0U;
  for (uint32_t i = 0U; i < 200000UL && distinct < 100U; ++i)
  {
    uint8_t v = random1to100();
    if ((v >= 1U) && (v <= 100U) && !seen[v])
    {
      seen[v] = true;
      ++distinct;
    }
  }
  TEST_ASSERT_GREATER_OR_EQUAL_UINT16(95U, distinct);
}

void testRandom(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_random1to100_range);
    RUN_TEST(test_random1to100_distribution);
  }
}
