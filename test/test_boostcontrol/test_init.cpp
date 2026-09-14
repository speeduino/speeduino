#include "../test_utils.h"
#include "globals.h"
#include "src/controllers/boost/boostController.h"
#include "shared.h"
#include "src/pwm/PwmOutputChannel.h"

extern PwmOutputChannel boostOutput;

static void test_initialise(void)
{
    auto context = setup_boost_tune(false, VSS_MODE_OFF, BOOST_MODE_SIMPLE, BOOST_BY_GEAR_OFF);
    context.current.boostDuty = 99;

    context.initialise();

    TEST_ASSERT_EQUAL_UINT16(0, context.current.boostDuty);
    TEST_ASSERT_TRUE(boostOutput.pin.isPinLow());
}

void testBoostInit(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_initialise);
  }
}