#include <unity.h>
#include "polling.hpp"
#include "static_for.hpp"
#include "bit_manip.h"
#include "../test_utils.h"

static uint16_t polling_counter[3];

static void incCounter0(void) { ++polling_counter[0]; }
static void incCounter1(void) { ++polling_counter[1]; }
static void incCounter2(void) { ++polling_counter[2]; }

static void reset_counters(void)
{
  polling_counter[0] = 0U;
  polling_counter[1] = 0U;
  polling_counter[2] = 0U;
}

static void test_executePolledAction_bit_set(void)
{
  reset_counters();
  polledAction_t action = { 2U, &incCounter0 };
  byte loopTimer = 0U;
  BIT_SET(loopTimer, 2U);
  executePolledAction(action, loopTimer);
  TEST_ASSERT_EQUAL_UINT16(1U, polling_counter[0]);
}

static void test_executePolledAction_bit_clear(void)
{
  reset_counters();
  polledAction_t action = { 3U, &incCounter0 };
  byte loopTimer = 0U;
  BIT_SET(loopTimer, 5U);  // a different bit is set
  executePolledAction(action, loopTimer);
  TEST_ASSERT_EQUAL_UINT16(0U, polling_counter[0]);
}

static void test_executePolledArrayAction(void)
{
  reset_counters();
  const polledAction_t actions[] = {
    { 0U, &incCounter0 },
    { 1U, &incCounter1 },
    { 2U, &incCounter2 },
  };
  byte loopTimer = 0U;
  BIT_SET(loopTimer, 0U);
  BIT_SET(loopTimer, 2U);

  executePolledArrayAction(0U, actions, loopTimer);
  executePolledArrayAction(1U, actions, loopTimer);
  executePolledArrayAction(2U, actions, loopTimer);

  TEST_ASSERT_EQUAL_UINT16(1U, polling_counter[0]);
  TEST_ASSERT_EQUAL_UINT16(0U, polling_counter[1]);
  TEST_ASSERT_EQUAL_UINT16(1U, polling_counter[2]);
}

static void recordCall(uint8_t idx, uint8_t *log)
{
  log[idx] += 1U;
}

static void test_static_for_repeat_n(void)
{
  uint8_t log[10] = { 0U };
  static_for<2U, 7U>::repeat_n(recordCall, log);
  TEST_ASSERT_EQUAL_UINT8(0U, log[0]);
  TEST_ASSERT_EQUAL_UINT8(0U, log[1]);
  TEST_ASSERT_EQUAL_UINT8(1U, log[2]);
  TEST_ASSERT_EQUAL_UINT8(1U, log[3]);
  TEST_ASSERT_EQUAL_UINT8(1U, log[4]);
  TEST_ASSERT_EQUAL_UINT8(1U, log[5]);
  TEST_ASSERT_EQUAL_UINT8(1U, log[6]);
  TEST_ASSERT_EQUAL_UINT8(0U, log[7]);
}

static void test_static_for_array_polling(void)
{
  reset_counters();
  const polledAction_t actions[] = {
    { 0U, &incCounter0 },
    { 1U, &incCounter1 },
    { 2U, &incCounter2 },
  };
  byte loopTimer = 0U;
  BIT_SET(loopTimer, 1U);

  static_for<0U, 3U>::repeat_n(executePolledArrayAction, actions, loopTimer);

  TEST_ASSERT_EQUAL_UINT16(0U, polling_counter[0]);
  TEST_ASSERT_EQUAL_UINT16(1U, polling_counter[1]);
  TEST_ASSERT_EQUAL_UINT16(0U, polling_counter[2]);
}

void testPolling(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_executePolledAction_bit_set);
    RUN_TEST(test_executePolledAction_bit_clear);
    RUN_TEST(test_executePolledArrayAction);
    RUN_TEST(test_static_for_repeat_n);
    RUN_TEST(test_static_for_array_polling);
  }
}
