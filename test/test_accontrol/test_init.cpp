#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "shared.h"

static void test_initialise(void)
{
    setup_ac_tune();

    acAfterEngineStartDelay = 99;
    waitedAfterCranking = true;
    acIsEnabled = false;
    acStartDelay = 99;
    acTPSLockoutDelay = 99;
    acRPMLockoutDelay = 99;
    acStandAloneFanIsEnabled = true;

    currentStatus.airconRequested = true;
    currentStatus.airconCompressorOn = true;
    currentStatus.airconRpmLockout = true;
    currentStatus.airconTpsLockout = true;
    currentStatus.airconTurningOn = true;
    currentStatus.airconCltLockout = true;
    currentStatus.airconFanOn = true;

    initialiseAirCon();
    assert_ac_off();
    
    TEST_ASSERT_EQUAL(0, acAfterEngineStartDelay);

    TEST_ASSERT_FALSE(waitedAfterCranking);

    TEST_ASSERT_EQUAL(0, acStartDelay);
    TEST_ASSERT_EQUAL(0, acTPSLockoutDelay);
    TEST_ASSERT_EQUAL(0, acRPMLockoutDelay);

    TEST_ASSERT_FALSE(currentStatus.airconRequested);
    TEST_ASSERT_FALSE(currentStatus.airconCompressorOn);
    TEST_ASSERT_FALSE(currentStatus.airconRpmLockout);
    TEST_ASSERT_FALSE(currentStatus.airconTpsLockout);
    TEST_ASSERT_FALSE(currentStatus.airconTurningOn);
    TEST_ASSERT_FALSE(currentStatus.airconCltLockout);
    TEST_ASSERT_FALSE(currentStatus.airconFanOn);

    TEST_ASSERT_TRUE(acIsEnabled);
    TEST_ASSERT_FALSE(acStandAloneFanIsEnabled);
}

static void test_initialise_inversepolarity_comp(void)
{
    setup_ac_tune();
    configPage15.airConCompPol = !configPage15.airConCompPol;

    acIsEnabled = false;
    initialiseAirCon();
    TEST_ASSERT_TRUE(acIsEnabled);
    assert_ac_off();
}

static void assert_init_acdisabled(void)
{
    acIsEnabled = true;
    initialiseAirCon();
    TEST_ASSERT_FALSE(acIsEnabled);    
}

static void test_initialise_disabled(void)
{
    setup_ac_tune();
    configPage15.airConEnable = false;

    assert_init_acdisabled();
}

static void test_initialise_badcomppin(void)
{
    setup_ac_tune();
    pinAirConComp = NOT_A_PIN;

    assert_init_acdisabled();
}

static void test_initialise_badreqin(void)
{
    setup_ac_tune();
    pinAirConRequest = NOT_A_PIN;

    assert_init_acdisabled();
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