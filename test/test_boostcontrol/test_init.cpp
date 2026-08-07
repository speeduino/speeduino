#include "../test_utils.h"
#include "globals.h"
#include "src/controllers/boost/boostController.h"
#include "shared.h"
#include "src/pwm/PwmOutputChannel.h"

extern PwmOutputChannel boostOutput;

static void test_initialise(void)
{
    setup_boost_tune(false, VSS_MODE_OFF, BOOST_MODE_SIMPLE, BOOST_BY_GEAR_OFF);
    currentStatus.boostDuty = 99;

    initialiseBoost(pinNumbers.pinBoost );

    TEST_ASSERT_EQUAL_UINT16(0, currentStatus.boostDuty);
    TEST_ASSERT_TRUE(boostOutput.pin.isPinLow());
}

void testBoostInit(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_initialise);
  }
}