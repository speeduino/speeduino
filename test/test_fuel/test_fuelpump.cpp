#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"

static void test_startPumpPriming_prime(void)
{
    statuses current = {};
    config2 page2 = {};

    page2.fpPrime = true;
    current.secl = 99;
    startPumpPriming(current, page2);

    TEST_ASSERT_FALSE(current.fpPrimed);
    TEST_ASSERT_EQUAL(current.secl, fpPrimeTime);
}

static void test_startPumpPriming_noprime(void)
{
    statuses current = {};
    config2 page2 = {};

    page2.fpPrime = false;
    current.secl = 99;
    startPumpPriming(current, page2);

    TEST_ASSERT_TRUE(current.fpPrimed);
    TEST_ASSERT_EQUAL(0, fpPrimeTime);
}

void testFuelPump(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_startPumpPriming_prime);
    RUN_TEST_P(test_startPumpPriming_noprime);
  }
}