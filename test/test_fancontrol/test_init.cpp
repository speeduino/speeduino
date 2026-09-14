#include "src/controllers/fan/fanController.h"
#include "units.h"
#include "../test_utils.h"
#include "shared.h"
#include "src/pins/outputPin.h"
#include "src/pwm/PwmOutputChannel.h"
#include "src/pins/invertableOutputPin.h"

using fanPwmChannel_t = PwmOutputChannel<invertableOutputPinAdaper_t<outputPin_t>>;
extern fanPwmChannel_t _fanPwm;

static void test_initialiseFan_resets_state(void)
{
  auto context = setup_nopwm_tune();
  context.current.fanDuty = 99U;
  context.initialise();

  TEST_ASSERT_EQUAL_UINT8(0U, context.current.fanDuty);
  // Normal polarity off -> pin LOW
  TEST_ASSERT_TRUE(_fanPwm.pin.isPinLow());
}

static void test_initialiseFan_reverse_polarity(void)
{
  auto context = setup_nopwm_tune();
  context.page6.fanInv = true;

  context.initialise();
  // Revere polarity off -> pin HIGH
  TEST_ASSERT_TRUE(_fanPwm.pin._pin.isPinHigh());
}


static void test_initialisePWMFan_resets_state(void)
{
#if defined(PWM_FAN_AVAILABLE)
  auto context = setup_pwm_tune();
  context.initialise();

  TEST_ASSERT_EQUAL(0, _fanPwm.targetDuty);
#endif
}

void testInit(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_initialiseFan_resets_state);
    RUN_TEST_P(test_initialisePWMFan_resets_state);
    RUN_TEST_P(test_initialiseFan_reverse_polarity);
  }
}