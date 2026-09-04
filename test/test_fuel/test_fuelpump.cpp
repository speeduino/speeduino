#include "../test_utils.h"
#include "src/controllers/fuelPump/fuelPumpController.h"
#include "src/controllers/fuelPump/fuelPumpController_detail.h"

extern fuelPumpController::detsil::pump_state_t pump_state;
extern void fuelPumpControlCore(const statuses &current, const config2 &page2);

static void test_startPumpPriming_prime(void)
{
    statuses current = {};
    config2 page2 = {};

    page2.fpPrime = true;
    current.secl = 99;
    startPumpPriming(current, page2);

    TEST_ASSERT_FALSE(pump_state.isPrimingComplete);
    TEST_ASSERT_TRUE(pump_state.pump_pin.isPinHigh());
    TEST_ASSERT_EQUAL(current.secl, pump_state.fpPrimeTime);
}

static void test_startPumpPriming_noprime(void)
{
    statuses current = {};
    config2 page2 = {};

    page2.fpPrime = false;
    pump_state.pump_pin.setPinLow();;
    current.secl = 99;
    startPumpPriming(current, page2);

    TEST_ASSERT_TRUE(pump_state.isPrimingComplete);
    TEST_ASSERT_FALSE(pump_state.pump_pin.isPinHigh());
    TEST_ASSERT_EQUAL(0, pump_state.fpPrimeTime);
}

constexpr uint8_t TEST_PUMP_PIN = 17;

static void test_initialiseFuelPump_no_prime_pumpoff(void)
{
    statuses current = {};
    config2 page2 = {};
    page2.fpPrime = 0U;

    initialiseFuelPump(current, page2, TEST_PUMP_PIN);
    TEST_ASSERT_FALSE(pump_state.pump_pin.isPinHigh());
}

static void test_initialiseFuelPump_with_prime_pumpon(void)
{
    statuses current = {};
    config2 page2 = {};
    page2.fpPrime = 5U;

    initialiseFuelPump(current, page2, TEST_PUMP_PIN);
    TEST_ASSERT_TRUE(pump_state.pump_pin.isPinHigh());
}

static void test_fuelPumpControl_engine_onoff(void)
{
    statuses current = {};
    config2 page2 = {};

    initialiseFuelPump(current, page2, TEST_PUMP_PIN);
    TEST_ASSERT_FALSE(pump_state.pump_pin.isPinHigh());

    current.rotationStatus = EngineRotationStatus::Running;
    fuelPumpControlCore(current, page2);
    TEST_ASSERT_TRUE(pump_state.pump_pin.isPinHigh());

    current.rotationStatus = EngineRotationStatus::Cranking;
    fuelPumpControlCore(current, page2);
    TEST_ASSERT_TRUE(pump_state.pump_pin.isPinHigh());    

    current.rotationStatus = EngineRotationStatus::Stopped;
    fuelPumpControlCore(current, page2);
    TEST_ASSERT_TRUE(pump_state.pump_pin.isPinHigh());
    TEST_ASSERT_EQUAL(1, pump_state.offDelay);

    fuelPumpControlCore(current, page2);
    TEST_ASSERT_TRUE(pump_state.pump_pin.isPinHigh());
    TEST_ASSERT_EQUAL(0, pump_state.offDelay);

    fuelPumpControlCore(current, page2);
    TEST_ASSERT_FALSE(pump_state.pump_pin.isPinHigh());
    TEST_ASSERT_EQUAL(0, pump_state.offDelay);
}

static void test_fuelPumpControl_priming_lt(void)
{
    statuses current = {};
    config2 page2 = {};
    page2.fpPrime = 5U;
    current.rotationStatus = EngineRotationStatus::Stopped;
    current.secl = 99;

    initialiseFuelPump(current, page2, TEST_PUMP_PIN);
    TEST_ASSERT_TRUE(pump_state.pump_pin.isPinHigh());

    current.secl = (pump_state.fpPrimeTime + page2.fpPrime)-1;
    fuelPumpControlCore(current, page2);
    TEST_ASSERT_TRUE(pump_state.pump_pin.isPinHigh());
    TEST_ASSERT_FALSE(pump_state.isPrimingComplete);
    TEST_ASSERT_EQUAL(0, pump_state.offDelay);
}

static void test_fuelPumpControl_priming_eq(void)
{
    statuses current = {};
    config2 page2 = {};
    page2.fpPrime = 5U;
    current.rotationStatus = EngineRotationStatus::Stopped;
    current.secl = 99;

    initialiseFuelPump(current, page2, TEST_PUMP_PIN);
    TEST_ASSERT_TRUE(pump_state.pump_pin.isPinHigh());

    current.secl = pump_state.fpPrimeTime + page2.fpPrime;
    fuelPumpControlCore(current, page2);
    TEST_ASSERT_FALSE(pump_state.pump_pin.isPinHigh());
    TEST_ASSERT_TRUE(pump_state.isPrimingComplete);
    TEST_ASSERT_EQUAL(0, pump_state.offDelay);
}

static void test_fuelPumpControl_priming_gt(void)
{
    statuses current = {};
    config2 page2 = {};
    page2.fpPrime = 5U;
    current.rotationStatus = EngineRotationStatus::Stopped;
    current.secl = 99;

    initialiseFuelPump(current, page2, TEST_PUMP_PIN);
    TEST_ASSERT_TRUE(pump_state.pump_pin.isPinHigh());

    current.secl = pump_state.fpPrimeTime + page2.fpPrime + 1;
    fuelPumpControlCore(current, page2);
    TEST_ASSERT_FALSE(pump_state.pump_pin.isPinHigh());
    TEST_ASSERT_TRUE(pump_state.isPrimingComplete);
    TEST_ASSERT_EQUAL(0, pump_state.offDelay);
}

static void test_fuelPumpControl_priming_rollover(void)
{
    statuses current = {};
    config2 page2 = {};
    page2.fpPrime = 5U;
    current.rotationStatus = EngineRotationStatus::Stopped;

    initialiseFuelPump(current, page2, TEST_PUMP_PIN);
    TEST_ASSERT_TRUE(pump_state.pump_pin.isPinHigh());

    current.secl = pump_state.fpPrimeTime - 1;
    fuelPumpControlCore(current, page2);
    TEST_ASSERT_FALSE(pump_state.pump_pin.isPinHigh());
    TEST_ASSERT_TRUE(pump_state.isPrimingComplete);
    TEST_ASSERT_EQUAL(0, pump_state.offDelay);
}

void testFuelPump(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_startPumpPriming_prime);
    RUN_TEST_P(test_startPumpPriming_noprime);
    RUN_TEST_P(test_initialiseFuelPump_no_prime_pumpoff);
    RUN_TEST_P(test_initialiseFuelPump_with_prime_pumpon);
    RUN_TEST_P(test_fuelPumpControl_engine_onoff);
    RUN_TEST_P(test_fuelPumpControl_priming_lt);
    RUN_TEST_P(test_fuelPumpControl_priming_eq);
    RUN_TEST_P(test_fuelPumpControl_priming_gt);
    RUN_TEST_P(test_fuelPumpControl_priming_rollover);
  }
}