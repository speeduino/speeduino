#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "shared.h"

extern uint8_t boostCounter;

static void test_initialise(void)
{
    setup_simplepid_tune();
    currentStatus.boostDuty = 99;
    boostCounter = 101;

    initialiseAuxPWM();

    TEST_ASSERT_EQUAL(0, currentStatus.boostDuty);
    TEST_ASSERT_EQUAL(0, boostCounter);
    // TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_BOOST_PIN));
}

void testBoostInit(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_initialise);
  }
}