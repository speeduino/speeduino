#include "../test_utils.h"
#include "globals.h"
#include "src/controllers/boost/boostController.h"
#include "shared.h"
#include "src/pwm/PwmOutputChannel.h"

extern PwmOutputChannel boostOutput;

static void test_duty_full(void)
{
    pinNumbers.pinBoost = TEST_BOOST_PIN;
    initialiseBoost(TEST_BOOST_PIN);

    boostOutput.setTargetDuty(200);

    for (uint8_t loop=0; loop<7; ++loop)
    {
      boostInterrupt();

      TEST_ASSERT_TRUE(boostOutput.pin.isPinHigh());
    }
}


static void test_partial_duty(void)
{
    pinNumbers.pinBoost = TEST_BOOST_PIN;
    initialiseBoost(TEST_BOOST_PIN);

    boostOutput.setTargetDuty(66*2);

    for (uint8_t loop=0; loop<7; ++loop)
    {
      boostInterrupt();
      TEST_ASSERT_TRUE(boostOutput.pin.isPinHigh());

      boostInterrupt();
      TEST_ASSERT_TRUE(boostOutput.pin.isPinLow());
    }
}

static void test_duty_none(void)
{
    pinNumbers.pinBoost = TEST_BOOST_PIN;
    initialiseBoost(TEST_BOOST_PIN);

    boostOutput.setTargetDuty(0);

    // for (uint8_t loop=0; loop<7; ++loop)
    {
      boostInterrupt();

      TEST_ASSERT_TRUE(boostOutput.pin.isPinLow());
    }
}

void testBoostInterrupt(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_duty_none);
    RUN_TEST_P(test_partial_duty);
    RUN_TEST_P(test_duty_full);
  }
}