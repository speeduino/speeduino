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

    TEST_ASSERT_FALSE(pump_state.isPrimingComplete);
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

    TEST_ASSERT_TRUE(pump_state.isPrimingComplete);
    TEST_ASSERT_FALSE(pump_state.isPumpOn);
    TEST_ASSERT_EQUAL(0, pump_state.fpPrimeTime);
}

static void test_stopPumpPriming_delay_not_expired(void)
{
    statuses current = {};
    config2 page2 = {};

    current.secl = 99;
    pump_state.isPumpOn = true;
    pump_state.isPrimingComplete = false;
    pump_state.fpPrimeTime = 33;
    page2.fpPrime = (current.secl - pump_state.fpPrimeTime) + 1;
    stopPumpPriming(current, page2);

    TEST_ASSERT_FALSE(pump_state.isPrimingComplete);
    TEST_ASSERT_TRUE(pump_state.isPumpOn);
    TEST_ASSERT_EQUAL(33, pump_state.fpPrimeTime);

    current.secl = pump_state.fpPrimeTime - 1;
    stopPumpPriming(current, page2);

    TEST_ASSERT_FALSE(pump_state.isPrimingComplete);
    TEST_ASSERT_TRUE(pump_state.isPumpOn);
    TEST_ASSERT_EQUAL(33, pump_state.fpPrimeTime);
}

static void test_stopPumpPriming_valid(uint16_t rpm, int8_t fpPrimeDelta, bool expectedPumpOnOff)
{
    statuses current = {};
    config2 page2 = {};

    current.secl = 99;
    current.setRpm(rpm);
    pump_state.isPumpOn = true;
    pump_state.fpPrimeTime = 33;
    pump_state.isPrimingComplete = false;
    page2.fpPrime = (current.secl - pump_state.fpPrimeTime) +  fpPrimeDelta;
    stopPumpPriming(current, page2);

    TEST_ASSERT_TRUE(pump_state.isPrimingComplete);
    TEST_ASSERT_EQUAL(expectedPumpOnOff, pump_state.isPumpOn);
    TEST_ASSERT_EQUAL(33, pump_state.fpPrimeTime);
}    

static void test_stopPumpPriming_delay_expired(void)
{
    test_stopPumpPriming_valid(1000, 0, true);
    test_stopPumpPriming_valid(1000, -1, true);
    test_stopPumpPriming_valid(0, 0, false);
    test_stopPumpPriming_valid(0, -1, false);
}

static void test_stopPumpPriming_prime_true(void)
{
    statuses current = {};
    config2 page2 = {};

    pump_state.isPrimingComplete = true;
    pump_state.isPumpOn = true;
    pump_state.fpPrimeTime = 0;
    stopPumpPriming(current, page2);
    // No effect
    TEST_ASSERT_TRUE(pump_state.isPrimingComplete);
    TEST_ASSERT_TRUE(pump_state.isPumpOn);
    TEST_ASSERT_EQUAL(0, pump_state.fpPrimeTime);

    pump_state.isPumpOn = false;
    stopPumpPriming(current, page2);
    // No effect
    TEST_ASSERT_TRUE(pump_state.isPrimingComplete);
    TEST_ASSERT_FALSE(pump_state.isPumpOn);
}

static void test_pumpOn(void)
{
    pump_state.isPumpOn = true;
    fuelPumpOn();
    TEST_ASSERT_TRUE(pump_state.isPumpOn);

    pump_state.isPumpOn = false;
    fuelPumpOn();
    TEST_ASSERT_TRUE(pump_state.isPumpOn);
}

static void test_pumpOff(void)
{
    pump_state.isPumpOn = true;
    fuelPumpOff();
    TEST_ASSERT_FALSE(pump_state.isPumpOn);

    pump_state.isPumpOn = false;
    fuelPumpOff();
    TEST_ASSERT_FALSE(pump_state.isPumpOn);
}

constexpr uint8_t TEST_PUMP_PIN = 17;

static void test_initialiseFuelPump_no_prime_pumpoff(void)
{
    statuses current = {};
    config2 page2 = {};
    page2.fpPrime = 0U;

    initialiseFuelPump(current, page2, TEST_PUMP_PIN);
    TEST_ASSERT_FALSE(pump_state.isPumpOn);
    TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_PUMP_PIN));
}

static void test_initialiseFuelPump_with_prime_pumpon(void)
{
    statuses current = {};
    config2 page2 = {};
    page2.fpPrime = 5U;

    initialiseFuelPump(current, page2, TEST_PUMP_PIN);
    TEST_ASSERT_TRUE(pump_state.isPumpOn);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(TEST_PUMP_PIN));
}


void testFuelPump(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_startPumpPriming_prime);
    RUN_TEST_P(test_startPumpPriming_noprime);
    RUN_TEST_P(test_stopPumpPriming_delay_expired);
    RUN_TEST_P(test_stopPumpPriming_delay_not_expired);
    RUN_TEST_P(test_stopPumpPriming_prime_true);
    RUN_TEST_P(test_pumpOn);
    RUN_TEST_P(test_pumpOff);
    RUN_TEST_P(test_initialiseFuelPump_no_prime_pumpoff);
    RUN_TEST_P(test_initialiseFuelPump_with_prime_pumpon);
  }
}