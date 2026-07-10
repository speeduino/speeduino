#include "../test_utils.h"
#include "idle.h"
#include "prepare_idle.h"
// idle1/idle2_pin are fastOutputPin_t — port-register writes are not visible
// to ArduinoFake's digitalRead(). The disableIdle tests below verify the
// full set of branches dispatch (line coverage); pin state is not checked.

static void test_disableIdle_pwm_normal(void)
{
  prepare_idle(IAC_ALGORITHM_PWM_OL);
  initialiseIdle(false);
  configPage6.iacPWMdir = 0U;
  configPage6.iacChannels = 0U;
  disableIdle();
  TEST_PASS();
}

static void test_disableIdle_pwm_reversed(void)
{
  prepare_idle(IAC_ALGORITHM_PWM_OL);
  initialiseIdle(false);
  configPage6.iacPWMdir = 1U;
  configPage6.iacChannels = 0U;
  disableIdle();
  TEST_PASS();
}

static void test_disableIdle_pwm_dual_channel_normal(void)
{
  prepare_idle(IAC_ALGORITHM_PWM_OL);
  initialiseIdle(false);
  configPage6.iacPWMdir = 0U;
  configPage6.iacChannels = 1U;
  disableIdle();
  TEST_PASS();
}

static void test_disableIdle_pwm_dual_channel_reversed(void)
{
  prepare_idle(IAC_ALGORITHM_PWM_OL);
  initialiseIdle(false);
  configPage6.iacPWMdir = 1U;
  configPage6.iacChannels = 1U;
  disableIdle();
  TEST_PASS();
}

static void test_disableIdle_none_is_noop(void)
{
  prepare_idle(IAC_ALGORITHM_NONE);
  initialiseIdle(false);
  disableIdle();   // No assertion: just verify it doesn't crash.
  TEST_PASS();
}

void testDisableIdle(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_disableIdle_pwm_normal);
    RUN_TEST(test_disableIdle_pwm_reversed);
    RUN_TEST(test_disableIdle_pwm_dual_channel_normal);
    RUN_TEST(test_disableIdle_pwm_dual_channel_reversed);
    RUN_TEST(test_disableIdle_none_is_noop);
  }
}
