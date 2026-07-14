#include "globals.h"
#include "src/controllers/fan/fanController.h"
#include "units.h"
#include "../test_utils.h"
#include "shared.h"

extern bool fan_pwm_state;

static void test_fan_state_true(void)
{
#if defined(PWM_FAN_AVAILABLE)//PWM fan not available on Arduino MEGA
  setup_pwm_tune();
  initialiseFan(TEST_FAN_PIN);

  fanOn();
  TEST_ASSERT_EQUAL(HIGH, digitalRead(TEST_FAN_PIN));

  fan_pwm_state = true;
  fanInterrupt();

  TEST_ASSERT_FALSE(fan_pwm_state);
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_FAN_PIN));
#else
  TEST_IGNORE_MESSAGE("PWM fan not available");
#endif
}

static void test_fan_state_false(void)
{
#if defined(PWM_FAN_AVAILABLE)//PWM fan not available on Arduino MEGA
  setup_pwm_tune();
  initialiseFan(TEST_FAN_PIN);

  fanOff();
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_FAN_PIN));

  fan_pwm_state = false;
  fanInterrupt();

  TEST_ASSERT_TRUE(fan_pwm_state);
  TEST_ASSERT_EQUAL(HIGH, digitalRead(TEST_FAN_PIN));
#else
  TEST_IGNORE_MESSAGE("PWM fan not available");
#endif
}

void testFanInterrupt(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_fan_state_true);
    RUN_TEST_P(test_fan_state_false);
  }
}