#include "../test_utils.h"
#include "globals.h"
#include "src/controllers/boost/boostController.h"
#include "units.h"
#include "shared.h"
#include "timers.h"
#include "src/PID/integerPID_ideal.h"
#include "src/pwm/PwmOutputChannel.h"

extern PwmOutputChannel boostOutput;
extern table2D_u8_s16_6 flexBoostTable;
extern integerPID_ideal boostPID;

static void test_boost_disabled(void)
{
    setup_simplepid_tune();
    initialiseBoost(TEST_BOOST_PIN);

    currentStatus.flexBoostCorrection = 99;
    configPage6.boostEnabled = false;
    boostControl();

    TEST_ASSERT_EQUAL(0, currentStatus.flexBoostCorrection);
    TEST_ASSERT_EQUAL(0, currentStatus.boostDuty);
    TEST_ASSERT_EQUAL(0, currentStatus.boostTarget);
    TEST_ASSERT_EQUAL(0, boostOutput.targetDuty);
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
  configPage9.boostByGear[0] = 1;
  configPage9.boostByGear[1] = 2;
  configPage9.boostByGear[2] = 3;
  configPage9.boostByGear[3] = 4;
  configPage9.boostByGear[4] = 5;
  configPage9.boostByGear[5] = 6;
  configPage15.boostControlEnable = EN_BOOST_CONTROL_FIXED;
  fill_table_values(boostTable, 33);
  populate_table_axis(boostTable.axisX, (table3d_axis_t)10);
  populate_table_axis(boostTable.axisY, (table3d_axis_t)10);

  fill_table_values(boostTableLookupDuty, 11);
  populate_table_axis(boostTableLookupDuty.axisX, (table3d_axis_t)10);
  populate_table_axis(boostTableLookupDuty.axisY, (table3d_axis_t)10);
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
  configPage9.boostByGear[0] = 255;
  configPage9.boostByGear[1] = 255;
  configPage9.boostByGear[2] = 255;
  configPage9.boostByGear[3] = 255;
  configPage9.boostByGear[4] = 255;
  configPage9.boostByGear[5] = 255;
  fill_table_values(boostTable, 255);

  for (uint8_t gear=1; gear<=6; ++gear)
  {
    currentStatus.boostDuty = 1;
    currentStatus.gear = gear;
    boostControl();
    TEST_ASSERT_EQUAL(10000, currentStatus.boostDuty);
    TEST_ASSERT_EQUAL(0, currentStatus.boostTarget);
    TEST_ASSERT_NOT_EQUAL(0, boostOutput.targetDuty);
  }

    // Invalid gear
  if (configPage9.boostByGearEnabled!=BOOST_BY_GEAR_OFF && isExternalVssMode(configPage2))
  {
    currentStatus.boostDuty = 33;
    currentStatus.gear = 0;
    boostControl();
    TEST_ASSERT_EQUAL(0, currentStatus.boostDuty);
    TEST_ASSERT_EQUAL(0, currentStatus.boostTarget);
    TEST_ASSERT_EQUAL(0, boostOutput.targetDuty);
  }
}

static void test_ol_zero_duty(void)
{
  setup_boost_tune(false, VSS_MODE_EXTERNAL_MI, OPEN_LOOP_BOOST, BOOST_BY_GEAR_OFF);
  fill_table_values(boostTable, 0);
    initialiseBoost(TEST_BOOST_PIN);
  currentStatus.boostDuty = 99;

  boostControl();

  TEST_ASSERT_EQUAL(0, currentStatus.boostDuty);
  TEST_ASSERT_TRUE(boostOutput.pin.isPinLow());
  TEST_ASSERT_EQUAL(0, currentStatus.boostTarget);
  TEST_ASSERT_EQUAL(0, boostOutput.targetDuty);
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
  configPage9.boostByGear[0] = 255;
  configPage9.boostByGear[1] = 255;
  configPage9.boostByGear[2] = 255;
  configPage9.boostByGear[3] = 255;
  configPage9.boostByGear[4] = 255;
  configPage9.boostByGear[5] = 255;
  fill_table_values(boostTable, 255);

  currentStatus.MAP = 50;
  currentStatus.LOOP_TIMER = 0xFF;
  for (uint8_t gear=1; gear<=6; ++gear)
  {
    currentStatus.gear = gear;
    currentStatus.boostTarget = 1;
    currentStatus.flexBoostCorrection = 99;
    boostPID.initialize(currentStatus.MAP);
    boostControl();

    TEST_ASSERT_EQUAL(511, currentStatus.boostTarget);
    TEST_ASSERT_EQUAL(642, currentStatus.boostDuty);
    TEST_ASSERT_EQUAL(0, currentStatus.flexBoostCorrection);
    TEST_ASSERT_NOT_EQUAL(0, boostOutput.targetDuty);
}

  // Invalid gear
  currentStatus.boostTarget = 1;
  currentStatus.MAP = 50;
  currentStatus.LOOP_TIMER = 0xFF;
  currentStatus.gear = 0;
  boostControl();
  TEST_ASSERT_EQUAL(0, currentStatus.boostTarget);
  TEST_ASSERT_EQUAL(0, currentStatus.boostDuty);
  TEST_ASSERT_EQUAL(0, currentStatus.flexBoostCorrection);
  TEST_ASSERT_EQUAL(0, boostOutput.targetDuty);
}

static void test_cl_flexcorrection(void)
{
  setup_boost_tune(false, VSS_MODE_EXTERNAL_MI, CLOSED_LOOP_BOOST, BOOST_BY_GEAR_OFF);
  configPage2.flexEnabled = true;
  populate_2dtable(&flexBoostTable, (int16_t)77, (uint8_t)50);
  fill_table_values(boostTable, 55);

  initialiseBoost(TEST_BOOST_PIN);

  currentStatus.LOOP_TIMER = 0xFF;
  currentStatus.flexBoostCorrection = 99;
  boostPID.initialize(currentStatus.MAP);
  boostControl();

  TEST_ASSERT_EQUAL_INT16(77, currentStatus.flexBoostCorrection);
  TEST_ASSERT_EQUAL_INT16((boostTable.values[0]*2)+77, currentStatus.boostTarget);
  TEST_ASSERT_EQUAL(577, currentStatus.boostDuty);
  TEST_ASSERT_NOT_EQUAL(0, boostOutput.targetDuty);
}

static void test_cl_flexcorrection_negative(void)
{
  setup_boost_tune(false, VSS_MODE_EXTERNAL_MI, CLOSED_LOOP_BOOST, BOOST_BY_GEAR_OFF);
  configPage2.flexEnabled = true;
  populate_2dtable(&flexBoostTable, (int16_t)-77, (uint8_t)50);
  fill_table_values(boostTable, 5);

  initialiseBoost(TEST_BOOST_PIN);

  currentStatus.LOOP_TIMER = 0xFF;
  currentStatus.flexBoostCorrection = 99;
  boostPID.initialize(currentStatus.MAP);
  boostControl();

  TEST_ASSERT_EQUAL_INT16(-77, currentStatus.flexBoostCorrection);
  TEST_ASSERT_EQUAL_INT16(0, currentStatus.boostTarget);
  TEST_ASSERT_EQUAL(0, currentStatus.boostDuty);
  TEST_ASSERT_EQUAL(0, boostOutput.targetDuty);
}

static void test_cl_boost_constant_gear(uint8_t gearNum, uint8_t &boostGear)
{
  currentStatus.LOOP_TIMER = 0xFF;
  currentStatus.boostTarget = 1;
  currentStatus.gear = gearNum;
  boostGear = gearNum*7;
  boostPID.initialize(currentStatus.MAP);
  boostControl();
  TEST_ASSERT_UINT16_WITHIN(1, boostGear << 1U, currentStatus.boostTarget);
}

static void test_cl_boost_constant_gear(void)
{
  setup_boost_tune(false, VSS_MODE_EXTERNAL_MI, CLOSED_LOOP_BOOST, BOOST_BY_GEAR_CONSTANT);

  test_cl_boost_constant_gear(1, configPage9.boostByGear[0]);
  test_cl_boost_constant_gear(2, configPage9.boostByGear[1]);
  test_cl_boost_constant_gear(3, configPage9.boostByGear[2]);
  test_cl_boost_constant_gear(4, configPage9.boostByGear[3]);
  test_cl_boost_constant_gear(5, configPage9.boostByGear[4]);
  test_cl_boost_constant_gear(6, configPage9.boostByGear[5]);

  // Invalid gear
  currentStatus.LOOP_TIMER = 0xFF;
  currentStatus.boostTarget = 1;
  currentStatus.gear = 0;
  boostControl();
  TEST_ASSERT_EQUAL(0U, currentStatus.boostTarget);
  TEST_ASSERT_EQUAL(0U, currentStatus.boostDuty);
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
  RUN_TEST_P(test_cl_flexcorrection_negative);
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