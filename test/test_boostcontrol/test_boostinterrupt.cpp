#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "shared.h"

extern volatile bool boost_pwm_state;

static void test_on_to_off(void)
{
    boost_pwm_state = true;
    pinBoost = TEST_BOOST_PIN;

    boostInterrupt();

    TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_BOOST_PIN));
    TEST_ASSERT_FALSE(boost_pwm_state);
}

static void test_off_to_on(void)
{
    boost_pwm_state = false;
    pinBoost = TEST_BOOST_PIN;

    boostInterrupt();

    TEST_ASSERT_EQUAL(HIGH, digitalRead(TEST_BOOST_PIN));
    TEST_ASSERT_TRUE(boost_pwm_state);
}

void testBoostInterrupt(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_on_to_off);
    RUN_TEST_P(test_off_to_on);
  }
}