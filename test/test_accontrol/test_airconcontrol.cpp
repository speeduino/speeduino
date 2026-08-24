#include "../test_utils.h"
#include "src/controllers/aircon/airconController.h"
#include "shared.h"
#include "units.h"

extern void airConOn(statuses &current, const config15 &page15);
extern void airConOff(statuses &current, const config15 &page15);

static void setup_acon_status(test_context &context)
{
    context.current.rotationStatus = EngineRotationStatus::Running;
    context.current.coolant = TEMPERATURE.toUser(context.page15.airConClTempCut-5);
    context.current.TPS = context.page15.airConTPSCut-5;
    context.current.setRpm(RPM_COARSE.toUser(context.page15.airConMaxRPMdiv100-5));

    context.current.airconTpsLockout = false;
    context.current.airconRpmLockout = false;

    acIsEnabled = true;
    acAfterEngineStartDelay = context.page15.airConAfterStartDelay+1;
    waitedAfterCranking = true;
    acTPSLockoutDelay = context.page15.airConTPSCutTime + 1;
    if (context.page15.airConReqPol)
    {
        aircon_req_pin._pin.setPinHigh();
    }
    else
    {
        aircon_req_pin._pin.setPinLow();
    }
}

static void test_reset_startdelay_when_stopped(void)
{
    auto context = setup_ac_tune();
    context.initialise();
    setup_acon_status(context);

    acAfterEngineStartDelay = 99;
    waitedAfterCranking = true;
    context.current.rotationStatus = EngineRotationStatus::Stopped;
    context.control();

    TEST_ASSERT_EQUAL(0, acAfterEngineStartDelay);
    TEST_ASSERT_FALSE(waitedAfterCranking);
}

static void test_disabled_no_effect(void)
{
    auto context = setup_ac_tune();
    context.initialise();
    setup_acon_status(context);

    acAfterEngineStartDelay = 99;
    waitedAfterCranking = true;
    acIsEnabled = false;
    context.control();

    TEST_ASSERT_EQUAL(99, acAfterEngineStartDelay);
    TEST_ASSERT_TRUE(waitedAfterCranking);    
}

static void test_startdelay_counter(void)
{
    auto context = setup_ac_tune();
    context.initialise();
    setup_acon_status(context);

    acAfterEngineStartDelay = 0;
    waitedAfterCranking = false;
    for (uint8_t index=acAfterEngineStartDelay; index<context.page15.airConAfterStartDelay; ++index)
    {
        context.control();
        TEST_ASSERT_EQUAL(index+1, acAfterEngineStartDelay);
        TEST_ASSERT_FALSE(waitedAfterCranking);    
    }

    context.control();
    TEST_ASSERT_TRUE(waitedAfterCranking);    
}

static void test_checkAirConCoolantLockout(void)
{
    auto context = setup_ac_tune();
    context.initialise();
    setup_acon_status(context);

    context.control();
    TEST_ASSERT_FALSE(context.current.airconCltLockout);

    context.current.coolant = TEMPERATURE.toUser(context.page15.airConClTempCut);
    context.control();
    TEST_ASSERT_FALSE(context.current.airconCltLockout);

    context.current.coolant = TEMPERATURE.toUser(context.page15.airConClTempCut+1);
    context.control();
    TEST_ASSERT_TRUE(context.current.airconCltLockout);
    assert_ac_off(context);
}

static void test_checkAirConTPSLockout(void)
{
    auto context = setup_ac_tune();
    context.initialise();
    setup_acon_status(context);

    acTPSLockoutDelay = 99;
    context.control();
    TEST_ASSERT_FALSE(context.current.airconTpsLockout);
    TEST_ASSERT_EQUAL(0, acTPSLockoutDelay);

    acTPSLockoutDelay = 99;
    context.current.TPS = context.page15.airConTPSCut;
    context.control();
    TEST_ASSERT_FALSE(context.current.airconTpsLockout);
    TEST_ASSERT_EQUAL(0, acTPSLockoutDelay);

    acTPSLockoutDelay = context.page15.airConTPSCutTime+1;
    context.current.TPS = context.page15.airConTPSCut+2;
    context.control();
    TEST_ASSERT_TRUE(context.current.airconTpsLockout);
    TEST_ASSERT_EQUAL(0, acTPSLockoutDelay);
    assert_ac_off(context);
}

static void test_checkAirConTPSLockout_delay(void)
{
    auto context = setup_ac_tune();
    context.initialise();
    setup_acon_status(context);

    context.current.TPS = context.page15.airConTPSCut-2;
    context.current.airconTpsLockout = true;

    acTPSLockoutDelay = 0;
    for (uint8_t index=acTPSLockoutDelay; index<context.page15.airConTPSCutTime; ++index)
    {
        context.control();
        TEST_ASSERT_EQUAL(index+1, acTPSLockoutDelay);
        TEST_ASSERT_TRUE(context.current.airconTpsLockout);
        assert_ac_off(context);
    }

    context.control();
    TEST_ASSERT_FALSE(context.current.airconTpsLockout);
}

static void test_checkAirConRPMLockout(void)
{
    auto context = setup_ac_tune();
    context.initialise();
    setup_acon_status(context);

    acRPMLockoutDelay = 99;
    context.control();
    TEST_ASSERT_FALSE(context.current.airconRpmLockout);
    TEST_ASSERT_EQUAL(99, acRPMLockoutDelay);

    // Max
    setup_acon_status(context);
    acRPMLockoutDelay = 99;
    context.current.setRpm(RPM_COARSE.toUser(context.page15.airConMaxRPMdiv100));
    context.control();
    TEST_ASSERT_FALSE(context.current.airconRpmLockout);
    TEST_ASSERT_EQUAL(99, acRPMLockoutDelay);

    setup_acon_status(context);
    acRPMLockoutDelay = 99;
    context.current.setRpm(RPM_COARSE.toUser(context.page15.airConMaxRPMdiv100+1));
    context.control();
    TEST_ASSERT_TRUE(context.current.airconRpmLockout);
    TEST_ASSERT_EQUAL(0, acRPMLockoutDelay);
    assert_ac_off(context);

    // Min
    setup_acon_status(context);
    acRPMLockoutDelay = 99;
    context.current.setRpm(RPM_MEDIUM.toUser(context.page15.airConMinRPMdiv10));
    context.control();
    TEST_ASSERT_FALSE(context.current.airconRpmLockout);
    TEST_ASSERT_EQUAL(99, acRPMLockoutDelay);

    setup_acon_status(context);
    acRPMLockoutDelay = 99;
    context.current.setRpm(RPM_MEDIUM.toUser(context.page15.airConMinRPMdiv10-1));
    context.control();
    TEST_ASSERT_TRUE(context.current.airconRpmLockout);
    TEST_ASSERT_EQUAL(0, acRPMLockoutDelay);
    assert_ac_off(context);
}

static void test_checkAirConRMPLockout_delay(void)
{
    auto context = setup_ac_tune();
    context.initialise();
    setup_acon_status(context);

    context.current.setRpm(RPM_MEDIUM.toUser(context.page15.airConMinRPMdiv10+1));
    context.current.airconRpmLockout = true;

    acRPMLockoutDelay = 0;
    for (uint8_t index=acRPMLockoutDelay; index<context.page15.airConRPMCutTime; ++index)
    {
        context.control();
        TEST_ASSERT_EQUAL(index+1, acRPMLockoutDelay);
        TEST_ASSERT_TRUE(context.current.airconRpmLockout);
        assert_ac_off(context);
    }

    context.control();
    TEST_ASSERT_FALSE(context.current.airconRpmLockout);
}

static void test_ac_request_pin(void)
{
    auto context = setup_ac_tune();
    context.initialise();

    setup_acon_status(context);
    context.current.airconRequested = false;
    context.current.airconTurningOn = false;
    context.control();
    TEST_ASSERT_TRUE(context.current.airconRequested);
    TEST_ASSERT_TRUE(context.current.airconTurningOn);
}

static void test_ac_request_pin_inverted(void)
{
    auto context = setup_ac_tune();
    context.page15.airConReqPol = !context.page15.airConReqPol;
    context.initialise();

    setup_acon_status(context);
    context.current.airconRequested = false;
    context.current.airconTurningOn = false;
    context.control();
    TEST_ASSERT_TRUE(context.current.airconRequested);
    TEST_ASSERT_TRUE(context.current.airconTurningOn);
}

static void test_fanon_when_acon(void)
{
    TEST_IGNORE_MESSAGE("Fill in when bug is fixed");
}

void assert_ac_on(const test_context &context)
{
    TEST_ASSERT_TRUE(context.page15.airConCompPol!=aircon_comp_pin._pin.isPinHigh());
    TEST_ASSERT_TRUE(!acStandAloneFanIsEnabled || context.page15.airConFanPol==aircon_fan_pin._pin.isPinHigh());
    TEST_ASSERT_TRUE(context.current.airconCompressorOn); 
    TEST_ASSERT_TRUE(!acStandAloneFanIsEnabled || context.current.airconFanOn);
}

static void test_start_delay(void)
{
    auto context = setup_ac_tune();
    context.initialise();
    setup_acon_status(context);

    acStartDelay = 0;
    for (uint8_t index=acStartDelay; index<context.page15.airConCompOnDelay; ++index)
    {
        context.control();
        TEST_ASSERT_EQUAL(index+1, acStartDelay);
        assert_ac_off(context);
        TEST_ASSERT_TRUE(context.current.airconTurningOn); 
    }

    context.control();
    TEST_ASSERT_FALSE(context.current.airconRpmLockout);
    assert_ac_on(context);
}

static void test_airConOn(void)
{
    auto context = setup_ac_tune();
    context.initialise();

    context.current.airconCompressorOn = false;
    airConOn(context.current, context.page15);
    TEST_ASSERT_TRUE(context.current.airconCompressorOn);
    TEST_ASSERT_TRUE(context.page15.airConCompPol!=aircon_comp_pin._pin.isPinHigh());
}

static void test_airConOn_inversepolarity(void)
{
    auto context = setup_ac_tune();
    context.page15.airConCompPol = !context.page15.airConCompPol;
    context.initialise();

    context.current.airconCompressorOn = false;
    airConOn(context.current, context.page15);
    TEST_ASSERT_TRUE(context.current.airconCompressorOn);
    TEST_ASSERT_TRUE(context.page15.airConCompPol!=aircon_comp_pin._pin.isPinHigh());
}

static void test_airConOff(void)
{
    auto context = setup_ac_tune();
    context.initialise();

    context.current.airconCompressorOn = true;
    airConOff(context.current, context.page15);
    TEST_ASSERT_FALSE(context.current.airconCompressorOn);
    TEST_ASSERT_TRUE(context.page15.airConCompPol==aircon_comp_pin._pin.isPinHigh());
}

static void test_airConOff_inversepolarity(void)
{
    auto context = setup_ac_tune();
    context.page15.airConCompPol = !context.page15.airConCompPol;
    context.initialise();

    context.current.airconCompressorOn = true;
    airConOff(context.current, context.page15);
    TEST_ASSERT_FALSE(context.current.airconCompressorOn);
    TEST_ASSERT_TRUE(context.page15.airConCompPol==aircon_comp_pin._pin.isPinHigh());
}

void testAcControl(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_reset_startdelay_when_stopped);
    RUN_TEST_P(test_disabled_no_effect);
    RUN_TEST_P(test_startdelay_counter);
    RUN_TEST_P(test_checkAirConCoolantLockout);
    RUN_TEST_P(test_checkAirConTPSLockout);
    RUN_TEST_P(test_checkAirConTPSLockout_delay);
    RUN_TEST_P(test_checkAirConRPMLockout);
    RUN_TEST_P(test_checkAirConRMPLockout_delay);
    RUN_TEST_P(test_ac_request_pin);
    RUN_TEST_P(test_ac_request_pin_inverted);
    RUN_TEST_P(test_fanon_when_acon);
    RUN_TEST_P(test_start_delay);
    RUN_TEST_P(test_airConOn);
    RUN_TEST_P(test_airConOn_inversepolarity);
    RUN_TEST_P(test_airConOff);
    RUN_TEST_P(test_airConOff_inversepolarity);
  }
}