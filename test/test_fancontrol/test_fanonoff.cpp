#include "globals.h"
#include "auxiliaries.h"
#include "units.h"
#include "../test_utils.h"
#include "shared.h"

static void test_fanOnOff_normal_polarity(void)
{
  setup_nopwm_tune();
  configPage6.fanInv = 0U;
  initialiseFan(TEST_FAN_PIN);

  fanOff();
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_FAN_PIN));
  fanOn();
  TEST_ASSERT_EQUAL(HIGH, digitalRead(TEST_FAN_PIN));
}


static void test_fanOnOff_inverted_polarity(void)
{
  setup_nopwm_tune();
  configPage6.fanInv = 1U;
  initialiseFan(TEST_FAN_PIN);

  fanOff();   // inverted off -> HIGH
  TEST_ASSERT_EQUAL(HIGH, digitalRead(TEST_FAN_PIN));
  fanOn();    // inverted on  -> LOW
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_FAN_PIN));
}

void testOnOff(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_fanOnOff_normal_polarity);
    RUN_TEST_P(test_fanOnOff_inverted_polarity);
  }
}