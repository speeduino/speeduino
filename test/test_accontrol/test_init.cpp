#include "../test_utils.h"
#include "globals.h"
#include "src/controllers/aircon/airconController.h"
#include "shared.h"

static void test_initialise(void)
{
    auto context = setup_ac_tune();

    acAfterEngineStartDelay = 99;
    waitedAfterCranking = true;
    acIsEnabled = false;
    acStartDelay = 99;
    acTPSLockoutDelay = 99;
    acRPMLockoutDelay = 99;
    acStandAloneFanIsEnabled = true;

    context.current.airconRequested = true;
    context.current.airconCompressorOn = true;
    context.current.airconRpmLockout = true;
    context.current.airconTpsLockout = true;
    context.current.airconTurningOn = true;
    context.current.airconCltLockout = true;
    context.current.airconFanOn = true;

    context.initialise();
    assert_ac_off();
    
    TEST_ASSERT_EQUAL(0, acAfterEngineStartDelay);

    TEST_ASSERT_FALSE(waitedAfterCranking);

    TEST_ASSERT_EQUAL(0, acStartDelay);
    TEST_ASSERT_EQUAL(0, acTPSLockoutDelay);
    TEST_ASSERT_EQUAL(0, acRPMLockoutDelay);

    TEST_ASSERT_FALSE(context.current.airconRequested);
    TEST_ASSERT_FALSE(context.current.airconCompressorOn);
    TEST_ASSERT_FALSE(context.current.airconRpmLockout);
    TEST_ASSERT_FALSE(context.current.airconTpsLockout);
    TEST_ASSERT_FALSE(context.current.airconTurningOn);
    TEST_ASSERT_FALSE(context.current.airconCltLockout);
    TEST_ASSERT_FALSE(context.current.airconFanOn);

    TEST_ASSERT_TRUE(acIsEnabled);
    TEST_ASSERT_FALSE(acStandAloneFanIsEnabled);
}

static void test_initialise_inversepolarity_comp(void)
{
    auto context = setup_ac_tune();
    configPage15.airConCompPol = !configPage15.airConCompPol;

    acIsEnabled = false;
    context.initialise();
    TEST_ASSERT_TRUE(acIsEnabled);
    assert_ac_off();
}

static void assert_init_acdisabled(test_context &context)
{
    acIsEnabled = true;
    context.initialise();
    TEST_ASSERT_FALSE(acIsEnabled);    
}

static void test_initialise_disabled(void)
{
    auto context = setup_ac_tune();
    configPage15.airConEnable = false;

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