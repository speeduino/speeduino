#include "globals.h"
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
  setup_nopwm_tune();
  currentStatus.fanDuty = 99U;
  initialiseFan(TEST_FAN_PIN);

  TEST_ASSERT_EQUAL_UINT8(0U, currentStatus.fanDuty);
  // Normal polarity off -> pin LOW
  TEST_ASSERT_TRUE(_fanPwm.pin.isPinLow());
}

static void test_initialiseFan_reverse_polarity(void)
{
  setup_nopwm_tune();
  configPage6.fanInv = true;

  initialiseFan(TEST_FAN_PIN);
  // Revere polarity off -> pin HIGH
  TEST_ASSERT_TRUE(_fanPwm.pin._pin.isPinHigh());
}


static void test_initialisePWMFan_resets_state(void)
{
#if defined(PWM_FAN_AVAILABLE)
  setup_pwm_tune();
  initialiseFan(TEST_FAN_PIN);

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