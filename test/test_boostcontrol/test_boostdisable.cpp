#include "../test_utils.h"
#include "globals.h"
#include "src/controllers/boost/boostController.h"
#include "shared.h"
#include "src/pwm/PwmOutputChannel.h"

extern PwmOutputChannel boostOutput;

static void test_disable(void)
{
    setup_simplepid_tune();
    initialiseBoost(TEST_BOOST_PIN);
    currentStatus.boostDuty = 99;

    boostDisable();

    TEST_ASSERT_EQUAL(0, currentStatus.boostDuty);
    TEST_ASSERT_TRUE(boostOutput.pin.isPinLow());
}

void testBoostDisable(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_disable);
  }
}