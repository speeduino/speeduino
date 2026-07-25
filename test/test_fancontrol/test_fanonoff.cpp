#include "globals.h"
#include "src/controllers/fan/fanController.h"
#include "units.h"
#include "../test_utils.h"
#include "shared.h"
#include "src/pins/boardOutputPin.h"

 extern boardOutputPin_t fan_pin;

static void test_fanOnOff_normal_polarity(void)
{
  setup_nopwm_tune();
  configPage6.fanInv = 0U;
  initialiseFan(TEST_FAN_PIN);

  fanOff();
  TEST_ASSERT_TRUE(fan_pin._pin.isPinLow());
  fanOn();
  TEST_ASSERT_TRUE(fan_pin._pin.isPinHigh());
}

static void test_fanOnOff_inverted_polarity(void)
{
  setup_nopwm_tune();
  configPage6.fanInv = 1U;
  initialiseFan(TEST_FAN_PIN);

  fanOff();   // inverted off -> HIGH
  TEST_ASSERT_TRUE(fan_pin._pin.isPinHigh());
  fanOn();    // inverted on  -> LOW
  TEST_ASSERT_TRUE(fan_pin._pin.isPinLow());
}

void testOnOff(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_fanOnOff_normal_polarity);
    RUN_TEST_P(test_fanOnOff_inverted_polarity);
  }
}