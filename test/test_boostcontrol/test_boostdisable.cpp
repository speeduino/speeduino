#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "shared.h"

static void test_disable(void)
{
    setup_simplepid_tune();
    initialiseAuxPWM();
    currentStatus.boostDuty = 99;

    boostDisable();

    TEST_ASSERT_EQUAL(0, currentStatus.boostDuty);
    TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_BOOST_PIN));

}
void testBoostDisable(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_disable);
  }
}