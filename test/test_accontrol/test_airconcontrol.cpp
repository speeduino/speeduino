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

    context.current.acStatus.tpsLockoutActive = false;
    context.current.acStatus.rpmLockoutActive = false;

    airConState.isEnabled = true;
    airConState.afterEngineStartDelay = context.page15.airConAfterStartDelay+1;
    airConState.tpsLockoutDelay = context.page15.airConTPSCutTime + 1;
    if (context.page15.airConReqPol)
    {
        airConState.reqPin._pin.setPinHigh();
    }
    else
    {
        airConState.reqPin._pin.setPinLow();
    }
}

static void test_reset_startdelay_when_stopped(void)
{
    auto context = setup_ac_tune();
    context.initialise();
    setup_acon_status(context);

    airConState.afterEngineStartDelay = 99;
    context.current.rotationStatus = EngineRotationStatus::Stopped;
    context.control();

    TEST_ASSERT_EQUAL(0, airConState.afterEngineStartDelay);
}

static void test_disabled_no_effect(void)
{
    auto context = setup_ac_tune();
    context.initialise();
    setup_acon_status(context);

    airConState.afterEngineStartDelay = 99;
    airConState.isEnabled = false;
    context.control();

    TEST_ASSERT_EQUAL(99, airConState.afterEngineStartDelay);  
}

static void test_startdelay_counter(void)
{
    auto context = setup_ac_tune();
    context.page15.airConCompOnDelay = 0;
    context.initialise();
    setup_acon_status(context);

    airConState.afterEngineStartDelay = 0;
    for (uint8_t index=airConState.afterEngineStartDelay; index<context.page15.airConAfterStartDelay-1; ++index)
    {
        context.control();
        TEST_ASSERT_EQUAL(index+1, airConState.afterEngineStartDelay);  
        TEST_ASSERT_TRUE(airConState.compPin._pin.isPinLow());    
    }

    context.control();
    TEST_ASSERT_TRUE(airConState.compPin._pin.isPinHigh());    
}

static void test_checkAirConCoolantLockout(void)
{
    auto context = setup_ac_tune();
    context.initialise();
    setup_acon_status(context);

    context.control();
    TEST_ASSERT_FALSE(context.current.acStatus.cltLockoutActive);

    context.current.coolant = TEMPERATURE.toUser(context.page15.airConClTempCut);
    context.control();
    TEST_ASSERT_FALSE(context.current.acStatus.cltLockoutActive);

    context.current.coolant = TEMPERATURE.toUser(context.page15.airConClTempCut+1);
    context.control();
    TEST_ASSERT_TRUE(context.current.acStatus.cltLockoutActive);
    assert_ac_off(context);
}

static void test_checkAirConTPSLockout(void)
{
    auto context = setup_ac_tune();
    context.initialise();
    setup_acon_status(context);

    airConState.tpsLockoutDelay = 99;
    context.control();
    TEST_ASSERT_FALSE(context.current.acStatus.tpsLockoutActive);
    TEST_ASSERT_EQUAL(0, airConState.tpsLockoutDelay);

    airConState.tpsLockoutDelay = 99;
    context.current.TPS = context.page15.airConTPSCut;
    context.control();
    TEST_ASSERT_FALSE(context.current.acStatus.tpsLockoutActive);
    TEST_ASSERT_EQUAL(0, airConState.tpsLockoutDelay);

    airConState.tpsLockoutDelay = context.page15.airConTPSCutTime+1;
    context.current.TPS = context.page15.airConTPSCut+2;
    context.control();
    TEST_ASSERT_TRUE(context.current.acStatus.tpsLockoutActive);
    TEST_ASSERT_EQUAL(0, airConState.tpsLockoutDelay);
    assert_ac_off(context);
}

static void test_checkAirConTPSLockout_delay(void)
{
    auto context = setup_ac_tune();
    context.initialise();
    setup_acon_status(context);

    context.current.TPS = context.page15.airConTPSCut-2;
    context.current.acStatus.tpsLockoutActive = true;

    airConState.tpsLockoutDelay = 0;
    for (uint8_t index=airConState.tpsLockoutDelay; index<context.page15.airConTPSCutTime; ++index)
    {
        context.control();
        TEST_ASSERT_EQUAL(index+1, airConState.tpsLockoutDelay);
        TEST_ASSERT_TRUE(context.current.acStatus.tpsLockoutActive);
        assert_ac_off(context);
    }

    context.control();
    TEST_ASSERT_FALSE(context.current.acStatus.tpsLockoutActive);
}

static void test_checkAirConRPMLockout(void)
{
    auto context = setup_ac_tune();
    context.initialise();
    setup_acon_status(context);

    airConState.rpmLockoutDelay = 99;
    context.control();
    TEST_ASSERT_FALSE(context.current.acStatus.rpmLockoutActive);
    TEST_ASSERT_EQUAL(99, airConState.rpmLockoutDelay);

    // Max
    setup_acon_status(context);
    airConState.rpmLockoutDelay = 99;
    context.current.setRpm(RPM_COARSE.toUser(context.page15.airConMaxRPMdiv100));
    context.control();
    TEST_ASSERT_FALSE(context.current.acStatus.rpmLockoutActive);
    TEST_ASSERT_EQUAL(99, airConState.rpmLockoutDelay);

    setup_acon_status(context);
    airConState.rpmLockoutDelay = 99;
    context.current.setRpm(RPM_COARSE.toUser(context.page15.airConMaxRPMdiv100+1));
    context.control();
    TEST_ASSERT_TRUE(context.current.acStatus.rpmLockoutActive);
    TEST_ASSERT_EQUAL(0, airConState.rpmLockoutDelay);
    assert_ac_off(context);

    // Min
    setup_acon_status(context);
    airConState.rpmLockoutDelay = 99;
    context.current.setRpm(RPM_MEDIUM.toUser(context.page15.airConMinRPMdiv10));
    context.control();
    TEST_ASSERT_FALSE(context.current.acStatus.rpmLockoutActive);
    TEST_ASSERT_EQUAL(99, airConState.rpmLockoutDelay);

    setup_acon_status(context);
    airConState.rpmLockoutDelay = 99;
    context.current.setRpm(RPM_MEDIUM.toUser(context.page15.airConMinRPMdiv10-1));
    context.control();
    TEST_ASSERT_TRUE(context.current.acStatus.rpmLockoutActive);
    TEST_ASSERT_EQUAL(0, airConState.rpmLockoutDelay);
    assert_ac_off(context);
}

static void test_checkAirConRMPLockout_delay(void)
{
    auto context = setup_ac_tune();
    context.initialise();
    setup_acon_status(context);

    context.current.setRpm(RPM_MEDIUM.toUser(context.page15.airConMinRPMdiv10+1));
    context.current.acStatus.rpmLockoutActive = true;

    airConState.rpmLockoutDelay = 0;
    for (uint8_t index=airConState.rpmLockoutDelay; index<context.page15.airConRPMCutTime; ++index)
    {
        context.control();
        TEST_ASSERT_EQUAL(index+1, airConState.rpmLockoutDelay);
        TEST_ASSERT_TRUE(context.current.acStatus.rpmLockoutActive);
        assert_ac_off(context);
    }

    context.control();
    TEST_ASSERT_FALSE(context.current.acStatus.rpmLockoutActive);
}

static void test_ac_request_pin(void)
{
    auto context = setup_ac_tune();
    context.initialise();

    setup_acon_status(context);
    context.current.acStatus.acRequested = false;
    context.current.acStatus.turningOn = false;
    context.control();
    TEST_ASSERT_TRUE(context.current.acStatus.acRequested);
    TEST_ASSERT_TRUE(context.current.acStatus.turningOn);
}

static void test_ac_request_pin_inverted(void)
{
    auto context = setup_ac_tune();
    context.page15.airConReqPol = !context.page15.airConReqPol;
    context.initialise();

    setup_acon_status(context);
    context.current.acStatus.acRequested = false;
    context.current.acStatus.turningOn = false;
    context.control();
    TEST_ASSERT_TRUE(context.current.acStatus.acRequested);
    TEST_ASSERT_TRUE(context.current.acStatus.turningOn);
}

static void test_fanon_when_acon(void)
{
    TEST_IGNORE_MESSAGE("Fill in when bug is fixed");
}

void assert_ac_on(const test_context &context)
{
    TEST_ASSERT_TRUE(context.page15.airConCompPol!=airConState.compPin._pin.isPinHigh());
    TEST_ASSERT_TRUE(!airConState.standAloneFanIsEnabled || context.page15.airConFanPol==airConState.fanPin._pin.isPinHigh());
    TEST_ASSERT_TRUE(context.current.acStatus.compressorOn); 
    TEST_ASSERT_TRUE(!airConState.standAloneFanIsEnabled || context.current.acStatus.fanOn);
}

static void test_start_delay(void)
{
    auto context = setup_ac_tune();
    context.initialise();
    setup_acon_status(context);

    airConState.startDelay = 0;
    for (uint8_t index=airConState.startDelay; index<context.page15.airConCompOnDelay; ++index)
    {
        context.control();
        TEST_ASSERT_EQUAL(index+1, airConState.startDelay);
        assert_ac_off(context);
        TEST_ASSERT_TRUE(context.current.acStatus.turningOn); 
    }

    context.control();
    TEST_ASSERT_FALSE(context.current.acStatus.rpmLockoutActive);
    assert_ac_on(context);
}

static void test_airConOn(void)
{
    auto context = setup_ac_tune();
    context.initialise();

    context.current.acStatus.compressorOn = false;
    airConOn(context.current, context.page15);
    TEST_ASSERT_TRUE(context.current.acStatus.compressorOn);
    TEST_ASSERT_TRUE(context.page15.airConCompPol!=airConState.compPin._pin.isPinHigh());
}

static void test_airConOn_inversepolarity(void)
{
    auto context = setup_ac_tune();
    context.page15.airConCompPol = !context.page15.airConCompPol;
    context.initialise();

    context.current.acStatus.compressorOn = false;
    airConOn(context.current, context.page15);
    TEST_ASSERT_TRUE(context.current.acStatus.compressorOn);
    TEST_ASSERT_TRUE(context.page15.airConCompPol!=airConState.compPin._pin.isPinHigh());
}

static void test_airConOff(void)
{
    auto context = setup_ac_tune();
    context.initialise();

    context.current.acStatus.compressorOn = true;
    airConOff(context.current, context.page15);
    TEST_ASSERT_FALSE(context.current.acStatus.compressorOn);
    TEST_ASSERT_TRUE(context.page15.airConCompPol==airConState.compPin._pin.isPinHigh());
}

static void test_airConOff_inversepolarity(void)
{
    auto context = setup_ac_tune();
    context.page15.airConCompPol = !context.page15.airConCompPol;
    context.initialise();

    context.current.acStatus.compressorOn = true;
    airConOff(context.current, context.page15);
    TEST_ASSERT_FALSE(context.current.acStatus.compressorOn);
    TEST_ASSERT_TRUE(context.page15.airConCompPol==airConState.compPin._pin.isPinHigh());
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