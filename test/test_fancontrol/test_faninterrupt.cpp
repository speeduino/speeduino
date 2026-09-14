#include "src/controllers/fan/fanController.h"
#include "units.h"
#include "../test_utils.h"
#include "shared.h"
#include "src/pins/outputPin.h"
#include "src/pwm/PwmOutputChannel.h"
#include "src/pins/invertableOutputPin.h"

using fanPwmChannel_t = PwmOutputChannel<invertableOutputPinAdaper_t<outputPin_t>>;
extern fanPwmChannel_t _fanPwm;

static void test_fan_state_true(void)
{
#if defined(PWM_FAN_AVAILABLE)//PWM fan not available on Arduino MEGA
  auto context = setup_pwm_tune();
  context.initialise();

  _fanPwm.setTargetDuty(100);
  _fanPwm.pin.setPinHigh();
  fanInterrupt();

  TEST_ASSERT_FALSE(_fanPwm.pin.isPinHigh());
#else
  TEST_IGNORE_MESSAGE("PWM fan not available");
#endif
}

static void test_fan_state_false(void)
{
#if defined(PWM_FAN_AVAILABLE)//PWM fan not available on Arduino MEGA
  auto context = setup_pwm_tune();
  context.initialise();

  _fanPwm.setTargetDuty(100);
  _fanPwm.pin.setPinLow();
  fanInterrupt();

  TEST_ASSERT_TRUE(_fanPwm.pin.isPinHigh());
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