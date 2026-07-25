#include "../test_utils.h"
#include "globals.h"
#include "src/controllers/boost/boostController.h"
#include "shared.h"
#include "src/pins/boardOutputPin.h"

extern uint8_t boostCounter;
extern boardOutputPin_t boost_pin;

static void test_initialise(void)
{
    setup_simplepid_tune();
    currentStatus.boostDuty = 99;
    boostCounter = 101;

    initialiseBoost(TEST_BOOST_PIN);

    TEST_ASSERT_EQUAL(0, currentStatus.boostDuty);
    TEST_ASSERT_EQUAL(0, boostCounter);
    TEST_ASSERT_TRUE(boost_pin._pin.isPinLow());
}

void testBoostInit(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_initialise);
  }
}