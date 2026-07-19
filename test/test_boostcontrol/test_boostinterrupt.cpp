#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "shared.h"
#include "board_definition.h"
#include "src/pins/fastOutputPin.h"
#include "src/pins/outputPin.h"

extern volatile bool boost_pwm_state;
extern boardOutputPin_t boost_pin;

static void test_on_to_off(void)
{
    pinBoost = TEST_BOOST_PIN;
    initialiseAuxPWM();

    boost_pwm_state = true;
    boostInterrupt();

    TEST_ASSERT_TRUE(boost_pin._pin.isPinLow());
    TEST_ASSERT_FALSE(boost_pwm_state);
}

static void test_off_to_on(void)
{
    pinBoost = TEST_BOOST_PIN;
    initialiseAuxPWM();

    boost_pwm_state = false;
    boostInterrupt();

    TEST_ASSERT_TRUE(boost_pin._pin.isPinHigh());
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