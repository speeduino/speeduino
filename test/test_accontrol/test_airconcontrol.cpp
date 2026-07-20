#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "shared.h"
#include "units.h"

extern void airConOn(void);
extern void airConOff(void);

static void setup_acon_status(void)
{
    currentStatus.rotationStatus = EngineRotationStatus::Running;
    currentStatus.coolant = TEMPERATURE.toUser(configPage15.airConClTempCut-5);
    currentStatus.TPS = configPage15.airConTPSCut-5;
    currentStatus.setRpm(RPM_COARSE.toUser(configPage15.airConMaxRPMdiv100-5));

    currentStatus.airconTpsLockout = false;
    currentStatus.airconRpmLockout = false;

    acIsEnabled = true;
    acAfterEngineStartDelay = configPage15.airConAfterStartDelay+1;
    waitedAfterCranking = true;
    acTPSLockoutDelay = configPage15.airConTPSCutTime + 1;
    if (configPage15.airConReqPol)
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
    setup_ac_tune();
    initialiseAirCon();
    setup_acon_status();

    acAfterEngineStartDelay = 99;
    waitedAfterCranking = true;
    currentStatus.rotationStatus = EngineRotationStatus::Stopped;
    airConControl();

    TEST_ASSERT_EQUAL(0, acAfterEngineStartDelay);
    TEST_ASSERT_FALSE(waitedAfterCranking);
}

static void test_disabled_no_effect(void)
{
    setup_ac_tune();
    initialiseAirCon();
    setup_acon_status();

    acAfterEngineStartDelay = 99;
    waitedAfterCranking = true;
    acIsEnabled = false;
    airConControl();

    TEST_ASSERT_EQUAL(99, acAfterEngineStartDelay);
    TEST_ASSERT_TRUE(waitedAfterCranking);    
}

static void test_startdelay_counter(void)
{
    setup_ac_tune();
    initialiseAirCon();
    setup_acon_status();

    acAfterEngineStartDelay = 0;
    waitedAfterCranking = false;
    for (uint8_t index=acAfterEngineStartDelay; index<configPage15.airConAfterStartDelay; ++index)
    {
        airConControl();
        TEST_ASSERT_EQUAL(index+1, acAfterEngineStartDelay);
        TEST_ASSERT_FALSE(waitedAfterCranking);    
    }

    airConControl();
    TEST_ASSERT_TRUE(waitedAfterCranking);    
}

static void test_checkAirConCoolantLockout(void)
{
    setup_ac_tune();
    initialiseAirCon();
    setup_acon_status();

    airConControl();
    TEST_ASSERT_FALSE(currentStatus.airconCltLockout);

    currentStatus.coolant = TEMPERATURE.toUser(configPage15.airConClTempCut);
    airConControl();
    TEST_ASSERT_FALSE(currentStatus.airconCltLockout);

    currentStatus.coolant = TEMPERATURE.toUser(configPage15.airConClTempCut+1);
    airConControl();
    TEST_ASSERT_TRUE(currentStatus.airconCltLockout);
    assert_ac_off();
}

static void test_checkAirConTPSLockout(void)
{
    setup_ac_tune();
    initialiseAirCon();
    setup_acon_status();

    acTPSLockoutDelay = 99;
    airConControl();
    TEST_ASSERT_FALSE(currentStatus.airconTpsLockout);
    TEST_ASSERT_EQUAL(0, acTPSLockoutDelay);

    acTPSLockoutDelay = 99;
    currentStatus.TPS = configPage15.airConTPSCut;
    airConControl();
    TEST_ASSERT_FALSE(currentStatus.airconTpsLockout);
    TEST_ASSERT_EQUAL(0, acTPSLockoutDelay);

    acTPSLockoutDelay = configPage15.airConTPSCutTime+1;
    currentStatus.TPS = configPage15.airConTPSCut+2;
    airConControl();
    TEST_ASSERT_TRUE(currentStatus.airconTpsLockout);
    TEST_ASSERT_EQUAL(0, acTPSLockoutDelay);
    assert_ac_off();
}

static void test_checkAirConTPSLockout_delay(void)
{
    setup_ac_tune();
    initialiseAirCon();
    setup_acon_status();

    currentStatus.TPS = configPage15.airConTPSCut-2;
    currentStatus.airconTpsLockout = true;

    acTPSLockoutDelay = 0;
    for (uint8_t index=acTPSLockoutDelay; index<configPage15.airConTPSCutTime; ++index)
    {
        airConControl();
        TEST_ASSERT_EQUAL(index+1, acTPSLockoutDelay);
        TEST_ASSERT_TRUE(currentStatus.airconTpsLockout);
        assert_ac_off();
    }

    airConControl();
    TEST_ASSERT_FALSE(currentStatus.airconTpsLockout);
}

static void test_checkAirConRPMLockout(void)
{
    setup_ac_tune();
    initialiseAirCon();
    setup_acon_status();

    acRPMLockoutDelay = 99;
    airConControl();
    TEST_ASSERT_FALSE(currentStatus.airconRpmLockout);
    TEST_ASSERT_EQUAL(99, acRPMLockoutDelay);

    // Max
    setup_acon_status();
    acRPMLockoutDelay = 99;
    currentStatus.setRpm(RPM_COARSE.toUser(configPage15.airConMaxRPMdiv100));
    airConControl();
    TEST_ASSERT_FALSE(currentStatus.airconRpmLockout);
    TEST_ASSERT_EQUAL(99, acRPMLockoutDelay);

    setup_acon_status();
    acRPMLockoutDelay = 99;
    currentStatus.setRpm(RPM_COARSE.toUser(configPage15.airConMaxRPMdiv100+1));
    airConControl();
    TEST_ASSERT_TRUE(currentStatus.airconRpmLockout);
    TEST_ASSERT_EQUAL(0, acRPMLockoutDelay);
    assert_ac_off();

    // Min
    setup_acon_status();
    acRPMLockoutDelay = 99;
    currentStatus.setRpm(RPM_MEDIUM.toUser(configPage15.airConMinRPMdiv10));
    airConControl();
    TEST_ASSERT_FALSE(currentStatus.airconRpmLockout);
    TEST_ASSERT_EQUAL(99, acRPMLockoutDelay);

    setup_acon_status();
    acRPMLockoutDelay = 99;
    currentStatus.setRpm(RPM_MEDIUM.toUser(configPage15.airConMinRPMdiv10-1));
    airConControl();
    TEST_ASSERT_TRUE(currentStatus.airconRpmLockout);
    TEST_ASSERT_EQUAL(0, acRPMLockoutDelay);
    assert_ac_off();
}

static void test_checkAirConRMPLockout_delay(void)
{
    setup_ac_tune();
    initialiseAirCon();
    setup_acon_status();

    currentStatus.setRpm(RPM_MEDIUM.toUser(configPage15.airConMinRPMdiv10+1));
    currentStatus.airconRpmLockout = true;

    acRPMLockoutDelay = 0;
    for (uint8_t index=acRPMLockoutDelay; index<configPage15.airConRPMCutTime; ++index)
    {
        airConControl();
        TEST_ASSERT_EQUAL(index+1, acRPMLockoutDelay);
        TEST_ASSERT_TRUE(currentStatus.airconRpmLockout);
        assert_ac_off();
    }

    airConControl();
    TEST_ASSERT_FALSE(currentStatus.airconRpmLockout);
}

static void test_ac_request_pin(void)
{
    setup_ac_tune();
    initialiseAirCon();

    setup_acon_status();
    currentStatus.airconRequested = false;
    currentStatus.airconTurningOn = false;
    airConControl();
    TEST_ASSERT_TRUE(currentStatus.airconRequested);
    TEST_ASSERT_TRUE(currentStatus.airconTurningOn);
}

static void test_ac_request_pin_inverted(void)
{
    setup_ac_tune();
    configPage15.airConReqPol = !configPage15.airConReqPol;
    initialiseAirCon();

    setup_acon_status();
    currentStatus.airconRequested = false;
    currentStatus.airconTurningOn = false;
    airConControl();
    TEST_ASSERT_TRUE(currentStatus.airconRequested);
    TEST_ASSERT_TRUE(currentStatus.airconTurningOn);
}

static void test_fanon_when_acon(void)
{
    TEST_IGNORE_MESSAGE("Fill in when bug is fixed");
}

void assert_ac_on(void)
{
    TEST_ASSERT_TRUE(configPage15.airConCompPol!=aircon_comp_pin._pin.isPinHigh());
    TEST_ASSERT_TRUE(!acStandAloneFanIsEnabled || configPage15.airConFanPol==aircon_fan_pin._pin.isPinHigh());
    TEST_ASSERT_TRUE(currentStatus.airconCompressorOn); 
    // TEST_ASSERT_FALSE(currentStatus.airconTurningOn); 
    TEST_ASSERT_TRUE(!acStandAloneFanIsEnabled || currentStatus.airconFanOn);
}

static void test_start_delay(void)
{
    setup_ac_tune();
    initialiseAirCon();
    setup_acon_status();

    acStartDelay = 0;
    for (uint8_t index=acStartDelay; index<configPage15.airConCompOnDelay; ++index)
    {
        airConControl();
        TEST_ASSERT_EQUAL(index+1, acStartDelay);
        assert_ac_off();
        TEST_ASSERT_TRUE(currentStatus.airconTurningOn); 
    }

    airConControl();
    TEST_ASSERT_FALSE(currentStatus.airconRpmLockout);
    assert_ac_on();
}

static void test_airConOn(void)
{
    setup_ac_tune();
    initialiseAirCon();

    currentStatus.airconCompressorOn = false;
    airConOn();
    TEST_ASSERT_TRUE(currentStatus.airconCompressorOn);
    TEST_ASSERT_TRUE(configPage15.airConCompPol!=aircon_comp_pin._pin.isPinHigh());
}

static void test_airConOn_inversepolarity(void)
{
    setup_ac_tune();
    configPage15.airConCompPol = !configPage15.airConCompPol;
    initialiseAirCon();

    currentStatus.airconCompressorOn = false;
    airConOn();
    TEST_ASSERT_TRUE(currentStatus.airconCompressorOn);
    TEST_ASSERT_TRUE(configPage15.airConCompPol!=aircon_comp_pin._pin.isPinHigh());
}

static void test_airConOff(void)
{
    setup_ac_tune();
    initialiseAirCon();

    currentStatus.airconCompressorOn = true;
    airConOff();
    TEST_ASSERT_FALSE(currentStatus.airconCompressorOn);
    TEST_ASSERT_TRUE(configPage15.airConCompPol==aircon_comp_pin._pin.isPinHigh());
}

static void test_airConOff_inversepolarity(void)
{
    setup_ac_tune();
    configPage15.airConCompPol = !configPage15.airConCompPol;
    initialiseAirCon();

    currentStatus.airconCompressorOn = true;
    airConOff();
    TEST_ASSERT_FALSE(currentStatus.airconCompressorOn);
    TEST_ASSERT_TRUE(configPage15.airConCompPol==aircon_comp_pin._pin.isPinHigh());
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