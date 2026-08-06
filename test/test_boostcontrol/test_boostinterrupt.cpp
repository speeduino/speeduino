#include "../test_utils.h"
#include "globals.h"
#include "src/controllers/boost/boostController.h"
#include "shared.h"
#include "src/pins/boardOutputPin.h"

extern volatile bool boost_pwm_state;
extern long boost_pwm_target_value;
extern uint16_t boost_pwm_max_count;
extern boardOutputPin_t boost_pin;

static void test_duty_full(void)
{
    pinNumbers.pinBoost = TEST_BOOST_PIN;
    initialiseBoost(TEST_BOOST_PIN);

    boost_pin.setPinHigh();
    boost_pwm_state = false;
    boost_pwm_target_value = boost_pwm_max_count;

    // for (uint8_t loop=0; loop<7; ++loop)
    {
      boostInterrupt();

      TEST_ASSERT_TRUE(boost_pin._pin.isPinHigh());
      TEST_ASSERT_TRUE(boost_pwm_state);
    }
}


static void test_partial_duty(void)
{
    pinNumbers.pinBoost = TEST_BOOST_PIN;
    initialiseBoost(TEST_BOOST_PIN);

    boost_pin.setPinLow();
    boost_pwm_state = false;
    boost_pwm_target_value = percentage(66, boost_pwm_max_count);

    for (uint8_t loop=0; loop<7; ++loop)
    {
      boostInterrupt();
      TEST_ASSERT_TRUE(boost_pin._pin.isPinHigh());
      TEST_ASSERT_TRUE(boost_pwm_state);

      boostInterrupt();
      TEST_ASSERT_TRUE(boost_pin._pin.isPinLow());
      TEST_ASSERT_FALSE(boost_pwm_state);
    }
}

static void test_duty_none(void)
{
    pinNumbers.pinBoost = TEST_BOOST_PIN;
    initialiseBoost(TEST_BOOST_PIN);

    boost_pin.setPinHigh();
    boost_pwm_state = true;
    boost_pwm_target_value = 0;

    // for (uint8_t loop=0; loop<7; ++loop)
    {
      boostInterrupt();

      TEST_ASSERT_TRUE(boost_pin._pin.isPinLow());
      TEST_ASSERT_FALSE(boost_pwm_state);
    }
}

void testBoostInterrupt(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_duty_none);
    RUN_TEST_P(test_partial_duty);
    RUN_TEST_P(test_duty_full);
  }
}