#include <unity.h>
#include <Arduino.h>
#include "timers.h"
#include "../test_utils.h"

static void test_initialiseTimers_runs(void)
{
  // No externally-visible state to verify; we just want the call to execute
  // without crashing so the line coverage records execution.
  initialiseTimers();
}

static void test_initTacho_setsInactiveFlag(void)
{
  constexpr uint8_t pin = 30U;
  tachoOutputFlag = ACTIVE;  // start non-default
  initTacho(pin);
  TEST_ASSERT_EQUAL(TACHO_INACTIVE, tachoOutputFlag);
}

static void test_tachoPulseHighAndLow_togglePin(void)
{
  constexpr uint8_t pin = 31U;
  initTacho(pin);

  tachoPulseHigh();
  TEST_ASSERT_EQUAL(HIGH, digitalRead(pin));

  tachoPulseLow();
  TEST_ASSERT_EQUAL(LOW, digitalRead(pin));

  // Toggle again to make sure the pin functions are idempotent
  tachoPulseHigh();
  TEST_ASSERT_EQUAL(HIGH, digitalRead(pin));
}

void testTimers(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_initialiseTimers_runs);
    RUN_TEST(test_initTacho_setsInactiveFlag);
    RUN_TEST(test_tachoPulseHighAndLow_togglePin);
  }
}
