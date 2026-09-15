#include "../test_utils.h"
#include "src/controllers/fuelPump/fuelPumpController.h"
#include "src/controllers/fuelPump/fuelPumpController_detail.h"

extern fuelPumpController::detail::pump_state_t pump_state;
extern void fuelPumpControlCore(const statuses &current, const config2 &page2);

constexpr uint8_t TEST_PUMP_PIN = 17;
struct test_context_t
{
    statuses current = {};
    config2 page2 = {};
    pinNumbers_t pins = {};

    test_context_t()
    {
        pins.pinFuelPump = TEST_PUMP_PIN;
    }

    void iniitialise(void)
    {
        initialiseFuelPump(current, page2, pins);
    }

    void fuelPumpControl(void)
    {
        fuelPumpControlCore(current, page2);
    }
};

static void test_startPumpPriming_prime(void)
{
    test_context_t context;
    context.iniitialise();

    TEST_ASSERT_FALSE(pump_state.pump_pin._pin.isPinHigh());

    context.page2.fpPrime = 5;
    context.current.secl = 99;
    startPumpPriming(context.current, context.page2);

    TEST_ASSERT_FALSE(pump_state.isPrimingComplete);
    TEST_ASSERT_TRUE(pump_state.pump_pin._pin.isPinHigh());
    TEST_ASSERT_EQUAL(context.current.secl, pump_state.fpPrimeTime);
}

static void test_startPumpPriming_noprime(void)
{
    test_context_t context;
    context.iniitialise();
    TEST_ASSERT_FALSE(pump_state.pump_pin._pin.isPinHigh());

    context.page2.fpPrime = 0;
    context.current.secl = 99;
    startPumpPriming(context.current, context.page2);

    TEST_ASSERT_TRUE(pump_state.isPrimingComplete);
    TEST_ASSERT_FALSE(pump_state.pump_pin._pin.isPinHigh());
    TEST_ASSERT_EQUAL(0, pump_state.fpPrimeTime);
}

static void test_initialiseFuelPump_no_prime_pumpoff(void)
{
    test_context_t context;
    context.page2.fpPrime = 0U;
    context.iniitialise();

    TEST_ASSERT_TRUE(pump_state.isPrimingComplete);
    TEST_ASSERT_FALSE(pump_state.pump_pin._pin.isPinHigh());
    TEST_ASSERT_EQUAL(0, pump_state.fpPrimeTime);
}

static void test_initialiseFuelPump_with_prime_pumpon(void)
{
    test_context_t context;
    context.page2.fpPrime = 5U;
    context.current.secl = 99;

    context.iniitialise();

    TEST_ASSERT_FALSE(pump_state.isPrimingComplete);
    TEST_ASSERT_TRUE(pump_state.pump_pin._pin.isPinHigh());
    TEST_ASSERT_EQUAL(context.current.secl, pump_state.fpPrimeTime);
}

static void test_fuelPumpControl_engine_onoff(void)
{
    test_context_t context;

    context.iniitialise();

    TEST_ASSERT_FALSE(pump_state.pump_pin._pin.isPinHigh());

    context.current.rotationStatus = EngineRotationStatus::Running;
    context.fuelPumpControl();
    TEST_ASSERT_TRUE(pump_state.pump_pin._pin.isPinHigh());

    context.current.rotationStatus = EngineRotationStatus::Cranking;
    context.fuelPumpControl();
    TEST_ASSERT_TRUE(pump_state.pump_pin._pin.isPinHigh());    

    context.current.rotationStatus = EngineRotationStatus::Stopped;
    context.fuelPumpControl();
    TEST_ASSERT_TRUE(pump_state.pump_pin._pin.isPinHigh());
    TEST_ASSERT_EQUAL(1, pump_state.offDelay);

    context.fuelPumpControl();
    TEST_ASSERT_TRUE(pump_state.pump_pin._pin.isPinHigh());
    TEST_ASSERT_EQUAL(0, pump_state.offDelay);

    context.fuelPumpControl();
    TEST_ASSERT_FALSE(pump_state.pump_pin._pin.isPinHigh());
    TEST_ASSERT_EQUAL(0, pump_state.offDelay);
}

static void test_fuelPumpControl_priming_not_elapsed(void)
{
    test_context_t context;

    context.page2.fpPrime = 5U;
    context.current.rotationStatus = EngineRotationStatus::Stopped;
    context.current.secl = 99;

    context.iniitialise();
    TEST_ASSERT_TRUE(pump_state.pump_pin._pin.isPinHigh());

    context.current.secl = (pump_state.fpPrimeTime + context.page2.fpPrime)-1;
    context.fuelPumpControl();
    TEST_ASSERT_TRUE(pump_state.pump_pin._pin.isPinHigh());
    TEST_ASSERT_FALSE(pump_state.isPrimingComplete);
    TEST_ASSERT_EQUAL(0, pump_state.offDelay);
}

static void test_fuelPumpControl_priming_elapsed_eq(void)
{
    test_context_t context;
    context.page2.fpPrime = 5U;
    context.current.rotationStatus = EngineRotationStatus::Stopped;
    context.current.secl = 99;

    context.iniitialise();
    TEST_ASSERT_TRUE(pump_state.pump_pin._pin.isPinHigh());

    context.current.secl = pump_state.fpPrimeTime + context.page2.fpPrime;
    context.fuelPumpControl();
    TEST_ASSERT_FALSE(pump_state.pump_pin._pin.isPinHigh());
    TEST_ASSERT_TRUE(pump_state.isPrimingComplete);
    TEST_ASSERT_EQUAL(0, pump_state.offDelay);
}

static void test_fuelPumpControl_priming_elapsed_gt(void)
{
    test_context_t context;
    context.page2.fpPrime = 5U;
    context.current.rotationStatus = EngineRotationStatus::Stopped;
    context.current.secl = 99;

    context.iniitialise();
    TEST_ASSERT_TRUE(pump_state.pump_pin._pin.isPinHigh());

    context.current.secl = pump_state.fpPrimeTime + context.page2.fpPrime + 1;
    context.fuelPumpControl();
    TEST_ASSERT_FALSE(pump_state.pump_pin._pin.isPinHigh());
    TEST_ASSERT_TRUE(pump_state.isPrimingComplete);
    TEST_ASSERT_EQUAL(0, pump_state.offDelay);
}

static void test_fuelPumpControl_priming_elapsed_rollover(void)
{
    test_context_t context;
    context.page2.fpPrime = 5U;
    context.current.rotationStatus = EngineRotationStatus::Stopped;
    context.current.secl = 99;

    context.iniitialise();
    TEST_ASSERT_TRUE(pump_state.pump_pin._pin.isPinHigh());

    context.current.secl = pump_state.fpPrimeTime - 1;
    context.fuelPumpControl();
    TEST_ASSERT_FALSE(pump_state.pump_pin._pin.isPinHigh());
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
    RUN_TEST_P(test_fuelPumpControl_priming_not_elapsed);
    RUN_TEST_P(test_fuelPumpControl_priming_elapsed_eq);
    RUN_TEST_P(test_fuelPumpControl_priming_elapsed_gt);
    RUN_TEST_P(test_fuelPumpControl_priming_elapsed_rollover);
  }
}