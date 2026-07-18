#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "units.h"
#include "shared.h"

extern byte boostCounter;
extern long boost_pwm_target_value;
extern volatile bool boost_pwm_state;
extern volatile unsigned int boost_pwm_cur_value;
extern table2D_u8_s16_6 flexBoostTable;

static void test_boost_disabled(void)
{
    setup_simplepid_tune();
    initialiseAuxPWM();

    currentStatus.flexBoostCorrection = 99;
    configPage6.boostEnabled = false;
    uint8_t oldCounter =  boostCounter;
    boostControl();

    TEST_ASSERT_EQUAL(0, currentStatus.flexBoostCorrection);
    TEST_ASSERT_NOT_EQUAL(oldCounter, boostCounter);
}

static void setup_boost_tune(bool fullPid, uint8_t vssMode, uint8_t boostType, uint8_t gearMode)
{
  if (fullPid)
  {
    setup_fullpid_tune();
  }
  else
  {
   setup_simplepid_tune();
  }
  configPage2.flexEnabled = false;
  configPage2.vssMode = vssMode;
  configPage4.boostType = boostType;
  configPage9.boostByGearEnabled = gearMode;
  configPage9.boostByGear1 = 1;
  configPage9.boostByGear2 = 2;
  configPage9.boostByGear3 = 3;
  configPage9.boostByGear4 = 4;
  configPage9.boostByGear5 = 5;
  configPage9.boostByGear6 = 6;
  configPage15.boostControlEnable = EN_BOOST_CONTROL_FIXED;
  fill_table_values(boostTable, 33);
  populate_table_axis(boostTable.axisX.begin(), 10);
  populate_table_axis(boostTable.axisY.begin(), 10);

  fill_table_values(boostTableLookupDuty, 11);
  populate_table_axis(boostTableLookupDuty.axisX.begin(), 10);
  populate_table_axis(boostTableLookupDuty.axisY.begin(), 10);
}

static uint8_t testBoostType;
static uint8_t testBoostByGearType;
static bool testPidType;
static uint8_t testVssMode;

static void setup_boost_tune(void)
{
  setup_boost_tune(testPidType, testVssMode, testBoostType, testBoostByGearType);
}

static void test_boost_ol_duty_clamp(void)
{
  setup_boost_tune();
  configPage9.boostByGear1 = 255;
  configPage9.boostByGear2 = 255;
  configPage9.boostByGear3 = 255;
  configPage9.boostByGear4 = 255;
  configPage9.boostByGear5 = 255;
  configPage9.boostByGear6 = 255;
  fill_table_values(boostTable, 255);

  for (uint8_t gear=1; gear<=6; ++gear)
  {
    currentStatus.boostDuty = 1;
    currentStatus.gear = gear;
    boostControl();
    TEST_ASSERT_EQUAL(10000, currentStatus.boostDuty);
  }

    // Invalid gear
  if (configPage9.boostByGearEnabled!=BOOST_BY_GEAR_OFF && isExternalVssMode(configPage2))
  {
    currentStatus.boostDuty = 33;
    currentStatus.gear = 0;
    boostControl();
    TEST_ASSERT_EQUAL(33, currentStatus.boostDuty);
  }
}

static void test_ol_zero_duty(void)
{
  setup_boost_tune(false, VSS_MODE_EXTERNAL_MI, OPEN_LOOP_BOOST, BOOST_BY_GEAR_OFF);
  fill_table_values(boostTable, 0);
  initialiseAuxPWM();
  currentStatus.boostDuty = 99;

  boostControl();

  TEST_ASSERT_EQUAL(0, currentStatus.boostDuty);
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_BOOST_PIN));
}

static void run_ol_tests(void)
{
  RUN_TEST_P(test_ol_zero_duty);

  testBoostType = OPEN_LOOP_BOOST;
  for (testBoostByGearType = BOOST_BY_GEAR_OFF; testBoostByGearType<=BOOST_BY_GEAR_CONSTANT; ++testBoostByGearType)
  {
    for (testVssMode = VSS_MODE_OFF; testVssMode<=VSS_MODE_EXTERNAL_MI; ++testVssMode)
    {       
      for (bool pidType : (bool[2]){ false /* Simple */, true /* Full */ })
      {
        testPidType = pidType;
        char szPostFix[32];
        snprintf(szPostFix, _countof(szPostFix)-1, "_bt%" PRIu8 "_gt%" PRIu8 "_vss%" PRIu8 "_pid%c", testBoostType, testBoostByGearType, testVssMode, pidType ? 'F' : 'S');

        RUN_TEST_POSTFIX_P(test_boost_ol_duty_clamp, szPostFix);
      }
    }
  }
}

static void test_boost_cl_target_clamp(void)
{
  setup_boost_tune(false, VSS_MODE_EXTERNAL_MI, CLOSED_LOOP_BOOST, BOOST_BY_GEAR_PERCENT);
  configPage9.boostByGear1 = 255;
  configPage9.boostByGear2 = 255;
  configPage9.boostByGear3 = 255;
  configPage9.boostByGear4 = 255;
  configPage9.boostByGear5 = 255;
  configPage9.boostByGear6 = 255;
  fill_table_values(boostTable, 255);

  currentStatus.boostTarget = 1;
  currentStatus.MAP = 50;
  for (uint8_t gear=1; gear<=6; ++gear)
  {
    boostCounter = 1;
    currentStatus.gear = gear;
    boostControl();

    TEST_ASSERT_EQUAL(511, currentStatus.boostTarget);
    TEST_ASSERT_EQUAL(0, currentStatus.flexBoostCorrection);
  }

  // Invalid gear
  currentStatus.boostTarget = 1;
  currentStatus.MAP = 50;
  boostCounter = 1;
  currentStatus.gear = 0;
  boostControl();
  TEST_ASSERT_EQUAL(1, currentStatus.boostTarget);
}

static void test_cl_flexcorrection(void)
{
  setup_boost_tune(false, VSS_MODE_EXTERNAL_MI, CLOSED_LOOP_BOOST, BOOST_BY_GEAR_OFF);
  configPage2.flexEnabled = true;
  populate_2dtable(&flexBoostTable, (int16_t)77, (uint8_t)50);
  fill_table_values(boostTable, 55);

  initialiseAuxPWM();

  boostCounter = 1;
  currentStatus.flexBoostCorrection = 99;
  boostControl();

  TEST_ASSERT_EQUAL_INT16(77, currentStatus.flexBoostCorrection);
  TEST_ASSERT_EQUAL_INT16((boostTable.values.values[0]*2)+77, currentStatus.boostTarget);
}

static void test_cl_boost_constant_gear(uint8_t gearNum, uint8_t &boostGear)
{
  boostCounter = 1;
  currentStatus.boostTarget = 1;
  currentStatus.gear = gearNum;
  boostGear = 3;
  boostControl();
  TEST_ASSERT_EQUAL(boostGear << 1U, currentStatus.boostTarget);
}

static void test_cl_boost_constant_gear(void)
{
  setup_boost_tune(false, VSS_MODE_EXTERNAL_MI, CLOSED_LOOP_BOOST, BOOST_BY_GEAR_CONSTANT);

  test_cl_boost_constant_gear(1, configPage9.boostByGear1);
  test_cl_boost_constant_gear(2, configPage9.boostByGear2);
  test_cl_boost_constant_gear(3, configPage9.boostByGear3);
  test_cl_boost_constant_gear(4, configPage9.boostByGear4);
  test_cl_boost_constant_gear(5, configPage9.boostByGear5);
  test_cl_boost_constant_gear(6, configPage9.boostByGear6);

  // Invalid gear
  boostCounter = 1;
  currentStatus.boostTarget = 1;
  currentStatus.gear = 0;
  boostControl();
  TEST_ASSERT_EQUAL(1U, currentStatus.boostTarget);
}

static void test_cl_boost_control_baro(void)
{
  setup_boost_tune(false, VSS_MODE_EXTERNAL_MI, CLOSED_LOOP_BOOST, BOOST_BY_GEAR_OFF);
  configPage15.boostControlEnable = EN_BOOST_CONTROL_BARO;
  configPage15.boostDCWhenDisabled = 77;
  currentStatus.MAP = 50;
  
  currentStatus.baro = currentStatus.MAP;
  boostControl();
  TEST_ASSERT_NOT_EQUAL(0, currentStatus.boostTarget);
  TEST_ASSERT_NOT_EQUAL(0, currentStatus.boostDuty);
  
  currentStatus.baro = currentStatus.MAP + 10;
  boostControl();
  TEST_ASSERT_NOT_EQUAL(0, currentStatus.boostTarget);
  TEST_ASSERT_EQUAL(configPage15.boostDCWhenDisabled*100, currentStatus.boostDuty);
  
  currentStatus.baro = currentStatus.MAP - 10;
  boostControl();
  TEST_ASSERT_NOT_EQUAL(0, currentStatus.boostTarget);
  TEST_ASSERT_NOT_EQUAL(0, currentStatus.boostDuty);
}

static void test_cl_boost_control_fixed(void)
{
  setup_boost_tune(false, VSS_MODE_EXTERNAL_MI, CLOSED_LOOP_BOOST, BOOST_BY_GEAR_OFF);
  configPage15.boostControlEnable = EN_BOOST_CONTROL_FIXED;
  configPage15.boostDCWhenDisabled = 77;
  currentStatus.MAP = 50;
  
  configPage15.boostControlEnableThreshold = currentStatus.MAP;
  boostControl();
  TEST_ASSERT_NOT_EQUAL(0, currentStatus.boostTarget);
  TEST_ASSERT_NOT_EQUAL(0, currentStatus.boostDuty);
  
  configPage15.boostControlEnableThreshold = currentStatus.MAP + 10;
  boostControl();
  TEST_ASSERT_NOT_EQUAL(0, currentStatus.boostTarget);
  TEST_ASSERT_EQUAL(configPage15.boostDCWhenDisabled*100, currentStatus.boostDuty);
  
  configPage15.boostControlEnableThreshold = currentStatus.MAP - 10;
  boostControl();
  TEST_ASSERT_NOT_EQUAL(0, currentStatus.boostTarget);
  TEST_ASSERT_NOT_EQUAL(0, currentStatus.boostDuty);
}

static void run_cl_tests(void)
{
  RUN_TEST_P(test_boost_cl_target_clamp);
  RUN_TEST_P(test_cl_flexcorrection);
  RUN_TEST_P(test_cl_boost_constant_gear);
  RUN_TEST_P(test_cl_boost_control_baro);
  RUN_TEST_P(test_cl_boost_control_fixed);
}

void testBoostControl(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_boost_disabled);
    run_ol_tests();
    run_cl_tests();
  }
}