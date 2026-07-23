#include "globals.h"
#include "auxiliaries.h"
#include "units.h"
#include "../test_utils.h"
#include "shared.h"
#include "src/pins/boardOutputPin.h"

extern boardOutputPin_t fan_pin;
extern long fan_pwm_value;

static void test_initialiseFan_resets_state(void)
{
  setup_nopwm_tune();
  currentStatus.fanOn = true;
  currentStatus.fanDuty = 99U;
  initialiseFan(TEST_FAN_PIN);

  TEST_ASSERT_FALSE(currentStatus.fanOn);
  TEST_ASSERT_EQUAL_UINT8(0U, currentStatus.fanDuty);
  // Normal polarity off -> pin LOW
  TEST_ASSERT_TRUE(fan_pin._pin.isPinLow());
}

static void test_initialiseFan_reverse_polarity(void)
{
  setup_nopwm_tune();
  configPage6.fanInv = true;

  initialiseFan(TEST_FAN_PIN);
  // Revere polarity off -> pin HIGH
  TEST_ASSERT_TRUE(fan_pin._pin.isPinHigh());
}


static void test_initialisePWMFan_resets_state(void)
{
#if defined(PWM_FAN_AVAILABLE)
  setup_pwm_tune();
  initialiseFan(TEST_FAN_PIN);

  TEST_ASSERT_EQUAL(0, fan_pwm_value);
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