#include "../test_utils.h"
#include "timers.h"

extern volatile byte loop5ms;
extern volatile byte loop20ms;
extern volatile byte loop33ms;
extern volatile byte loop66ms;
extern volatile byte loop100ms;
extern volatile byte loop250ms;
extern volatile int loopSec;
extern volatile uint16_t lastRPM_100ms;

static void test_initialiseTimers_resets_counters(void)
{
  // Push counters to non-default
  loop5ms = 99U; loop20ms = 99U; loop33ms = 99U; loop66ms = 99U;
  loop100ms = 99U; loop250ms = 99U; loopSec = 999;
  lastRPM_100ms = 555U;

  initialiseTimers();
  TEST_ASSERT_EQUAL_UINT8(0U, loop5ms);
  TEST_ASSERT_EQUAL_UINT8(0U, loop20ms);
  TEST_ASSERT_EQUAL_UINT8(0U, loop33ms);
  TEST_ASSERT_EQUAL_UINT8(0U, loop66ms);
  TEST_ASSERT_EQUAL_UINT8(0U, loop100ms);
  TEST_ASSERT_EQUAL_UINT8(0U, loop250ms);
  TEST_ASSERT_EQUAL_INT(0, loopSec);
  TEST_ASSERT_EQUAL_UINT16(0U, lastRPM_100ms);
}

void testInit(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_initialiseTimers_resets_counters);
  }
}