#include "../test_utils.h"
#include "src/controllers/aircon/airconController.h"
#include "shared.h"

static void test_initialise(void)
{
    auto context = setup_ac_tune();

    airConState.afterEngineStartDelay = 99;
    airConState.waitedAfterCranking = true;
    airConState.isEnabled = false;
    airConState.startDelay = 99;
    airConState.tpsLockoutDelay = 99;
    airConState.rpmLockoutDelay = 99;
    airConState.standAloneFanIsEnabled = true;

    context.current.airconRequested = true;
    context.current.airconCompressorOn = true;
    context.current.airconRpmLockout = true;
    context.current.airconTpsLockout = true;
    context.current.airconTurningOn = true;
    context.current.airconCltLockout = true;
    context.current.airconFanOn = true;

    context.initialise();
    assert_ac_off(context);
    
    TEST_ASSERT_EQUAL(0, airConState.afterEngineStartDelay);

    TEST_ASSERT_FALSE(airConState.waitedAfterCranking);

    TEST_ASSERT_EQUAL(0, airConState.startDelay);
    TEST_ASSERT_EQUAL(0, airConState.tpsLockoutDelay);
    TEST_ASSERT_EQUAL(0, airConState.rpmLockoutDelay);

    TEST_ASSERT_FALSE(context.current.airconRequested);
    TEST_ASSERT_FALSE(context.current.airconCompressorOn);
    TEST_ASSERT_FALSE(context.current.airconRpmLockout);
    TEST_ASSERT_FALSE(context.current.airconTpsLockout);
    TEST_ASSERT_FALSE(context.current.airconTurningOn);
    TEST_ASSERT_FALSE(context.current.airconCltLockout);
    TEST_ASSERT_FALSE(context.current.airconFanOn);

    TEST_ASSERT_TRUE(airConState.isEnabled);
    TEST_ASSERT_FALSE(airConState.standAloneFanIsEnabled);
}

static void test_initialise_inversepolarity_comp(void)
{
    auto context = setup_ac_tune();
    context.page15.airConCompPol = !context.page15.airConCompPol;

    airConState.isEnabled = false;
    context.initialise();
    TEST_ASSERT_TRUE(airConState.isEnabled);
    assert_ac_off(context);
}

static void assert_init_acdisabled(test_context &context)
{
    airConState.isEnabled = true;
    context.initialise();
    TEST_ASSERT_FALSE(airConState.isEnabled);    
}

static void test_initialise_disabled(void)
{
    auto context = setup_ac_tune();
    context.page15.airConEnable = false;

    assert_init_acdisabled(context);
}

static void test_initialise_badcomppin(void)
{
    auto context = setup_ac_tune();
    context.pins.pinAirConComp = NOT_A_PIN;

    assert_init_acdisabled(context);
}

static void test_initialise_badreqin(void)
{
    auto context = setup_ac_tune();
    context.pins.pinAirConRequest = NOT_A_PIN;

    assert_init_acdisabled(context);
}

static void test_initialize_fan(void)
{
    TEST_IGNORE_MESSAGE("Fill in when bug is fixed");
}

void testAcInit(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_initialise);
    RUN_TEST_P(test_initialise_inversepolarity_comp);
    RUN_TEST_P(test_initialise_disabled);
    RUN_TEST_P(test_initialise_badcomppin);
    RUN_TEST_P(test_initialise_badreqin);
    RUN_TEST_P(test_initialize_fan);
  }
}