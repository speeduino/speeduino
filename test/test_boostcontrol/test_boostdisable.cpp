#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "shared.h"
#include "board_definition.h"
#include "src/pins/fastOutputPin.h"
#include "src/pins/outputPin.h"

extern boardOutputPin_t boost_pin;

static void test_disable(void)
{
    setup_simplepid_tune();
    initialiseAuxPWM();
    currentStatus.boostDuty = 99;

    boostDisable();

    TEST_ASSERT_EQUAL(0, currentStatus.boostDuty);
    TEST_ASSERT_TRUE(boost_pin._pin.isPinLow());
}

void testBoostDisable(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_disable);
  }
}