#include "../test_utils.h"
#include "src/controllers/fuelPump/fuelPumpController.h"
#include "src/controllers/fuelPump/fuelPumpController_detail.h"

extern fuelPumpController::detsil::pump_state_t pump_state;

static void test_startPumpPriming_prime(void)
{
    statuses current = {};
    config2 page2 = {};

    page2.fpPrime = true;
    current.secl = 99;
    startPumpPriming(current, page2);

    TEST_ASSERT_FALSE(current.fpPrimed);
    TEST_ASSERT_TRUE(pump_state.isPumpOn);
    TEST_ASSERT_EQUAL(current.secl, pump_state.fpPrimeTime);
}

static void test_startPumpPriming_noprime(void)
{
    statuses current = {};
    config2 page2 = {};

    page2.fpPrime = false;
    pump_state.isPumpOn = false;
    current.secl = 99;
    startPumpPriming(current, page2);

    TEST_ASSERT_TRUE(current.fpPrimed);
    TEST_ASSERT_FALSE(pump_state.isPumpOn);
    TEST_ASSERT_EQUAL(0, pump_state.fpPrimeTime);
}

static void test_stopPumpPriming_delay_not_expired(void)
{
    statuses current = {};
    config2 page2 = {};

    current.secl = 99;
    pump_state.isPumpOn = true;
    pump_state.fpPrimeTime = 33;
    page2.fpPrime = (current.secl - pump_state.fpPrimeTime) + 1;
    stopPumpPriming(current, page2);

    TEST_ASSERT_FALSE(current.fpPrimed);
    TEST_ASSERT_TRUE(pump_state.isPumpOn);
    TEST_ASSERT_EQUAL(33, pump_state.fpPrimeTime);
}

static void test_stopPumpPriming_delay_expired(void)
{
    statuses current = {};
    config2 page2 = {};

    current.secl = 99;
    current.setRpm(1000);
    pump_state.isPumpOn = true;
    pump_state.fpPrimeTime = 33;
    page2.fpPrime = (current.secl - pump_state.fpPrimeTime) - 1;
    stopPumpPriming(current, page2);

    TEST_ASSERT_TRUE(current.fpPrimed);
    TEST_ASSERT_TRUE(pump_state.isPumpOn);
    TEST_ASSERT_EQUAL(33, pump_state.fpPrimeTime);
}

static void test_stopPumpPriming_delay_equaled(void)
{
    statuses current = {};
    config2 page2 = {};

    current.secl = 99;
    current.setRpm(0); // Coverage: this excercises an additional code path
    pump_state.isPumpOn = true;
    pump_state.fpPrimeTime = 33;
    page2.fpPrime = (current.secl - pump_state.fpPrimeTime);
    stopPumpPriming(current, page2);

    TEST_ASSERT_TRUE(current.fpPrimed);
    TEST_ASSERT_FALSE(pump_state.isPumpOn);
    TEST_ASSERT_EQUAL(33, pump_state.fpPrimeTime);
}

static void test_stopPumpPriming_prime_true(void)
{
    statuses current = {};
    config2 page2 = {};

    current.fpPrimed = true;
    pump_state.isPumpOn = true;
    pump_state.fpPrimeTime = 0;
    stopPumpPriming(current, page2);
    // No effect
    TEST_ASSERT_TRUE(current.fpPrimed);
    TEST_ASSERT_TRUE(pump_state.isPumpOn);
    TEST_ASSERT_EQUAL(0, pump_state.fpPrimeTime);

    pump_state.isPumpOn = false;
    stopPumpPriming(current, page2);
    // No effect
    TEST_ASSERT_TRUE(current.fpPrimed);
    TEST_ASSERT_FALSE(pump_state.isPumpOn);
}

void testFuelPump(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_startPumpPriming_prime);
    RUN_TEST_P(test_startPumpPriming_noprime);
    RUN_TEST_P(test_stopPumpPriming_delay_expired);
    RUN_TEST_P(test_stopPumpPriming_delay_equaled);
    RUN_TEST_P(test_stopPumpPriming_delay_not_expired);
    RUN_TEST_P(test_stopPumpPriming_prime_true);
  }
}