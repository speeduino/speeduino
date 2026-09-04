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

static void setup_preboost_state(statuses &current)
{
  current.flexBoostCorrection = 77;
  current.boostDuty = 77;
  current.boostTarget = 77;
  boostOutput.targetDuty = 77;
}

static void assert_boost_off(statuses &current)
{
  TEST_ASSERT_EQUAL(0, current.flexBoostCorrection);
  TEST_ASSERT_EQUAL(0, current.boostDuty);
  TEST_ASSERT_EQUAL(0, current.boostTarget);
  TEST_ASSERT_EQUAL(0, boostOutput.targetDuty);
}

static void test_boost_disabled(void)
{
  auto context = setup_boost_tune(false, VSS_MODE_OFF, BOOST_MODE_SIMPLE, BOOST_BY_GEAR_OFF);
  context.initialise();

  // Initial state
  context.setup_boost_enabled();
  setup_preboost_state(context.current);
  context.boostControl();
  TEST_ASSERT_NOT_EQUAL(0, context.current.flexBoostCorrection);
  TEST_ASSERT_NOT_EQUAL_UINT16(0, context.current.boostDuty);
  TEST_ASSERT_NOT_EQUAL_UINT16(0, context.current.boostTarget);
  TEST_ASSERT_NOT_EQUAL_UINT16(0, boostOutput.targetDuty);

  context.current.rotationStatus = EngineRotationStatus::Running;
  context.page6.boostEnabled = false;
  setup_preboost_state(context.current);
  context.boostControl();
  assert_boost_off(context.current);

  context.current.rotationStatus = EngineRotationStatus::Cranking;
  context.page6.boostEnabled = true;
  setup_preboost_state(context.current);
  context.boostControl();
  assert_boost_off(context.current);

  context.current.rotationStatus = EngineRotationStatus::Stopped;
  context.page6.boostEnabled = true;
  setup_preboost_state(context.current);
  context.boostControl();
  assert_boost_off(context.current);
}

static uint8_t testBoostType;
static uint8_t testBoostByGearType;
static bool testPidType;
static uint8_t testVssMode;

static test_context_t setup_boost_tune(void)
{
  return setup_boost_tune(testPidType, testVssMode, testBoostType, testBoostByGearType);
}

static void test_boost_ol_duty_clamp(void)
{
  auto context = setup_boost_tune();
  context.page9.boostByGear[0] = 255;
  context.page9.boostByGear[1] = 255;
  context.page9.boostByGear[2] = 255;
  context.page9.boostByGear[3] = 255;
  context.page9.boostByGear[4] = 255;
  context.page9.boostByGear[5] = 255;
  fill_table_values(boostTable, 255);

  context.setup_boost_enabled();
  for (uint8_t gear=1; gear<=6; ++gear)
  {
    context.current.boostDuty = 1;
    context.current.gear = gear;

    context.boostControl();
    TEST_ASSERT_EQUAL_UINT16(10000, context.current.boostDuty);
    TEST_ASSERT_EQUAL_UINT16(0, context.current.boostTarget);
    TEST_ASSERT_NOT_EQUAL_UINT16(0, boostOutput.targetDuty);
  }

    // Invalid gear
  if (context.page9.boostByGearEnabled!=BOOST_BY_GEAR_OFF && isExternalVssMode(context.page2))
  {
    context.current.boostDuty = 33;
    context.current.gear = 0;
    context.boostControl();
    TEST_ASSERT_EQUAL_UINT16(0, context.current.boostDuty);
    TEST_ASSERT_EQUAL_UINT16(0, context.current.boostTarget);
    TEST_ASSERT_EQUAL_UINT16(0, boostOutput.targetDuty);
  }
}

static void test_ol_zero_duty(void)
{
  auto context = setup_boost_tune(false, VSS_MODE_EXTERNAL_MI, OPEN_LOOP_BOOST, BOOST_BY_GEAR_OFF);
  fill_table_values(boostTable, 0);
  context.initialise();
  context.current.boostDuty = 99;
  context.setup_boost_enabled();

  context.boostControl();

  TEST_ASSERT_EQUAL_UINT16(0, context.current.boostDuty);
  TEST_ASSERT_TRUE(boostOutput.pin.isPinLow());
  TEST_ASSERT_EQUAL_UINT16(0, context.current.boostTarget);
  TEST_ASSERT_EQUAL_UINT16(0, boostOutput.targetDuty);
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
  auto context = setup_boost_tune(false, VSS_MODE_EXTERNAL_MI, CLOSED_LOOP_BOOST, BOOST_BY_GEAR_PERCENT);
  context.page9.boostByGear[0] = 255;
  context.page9.boostByGear[1] = 255;
  context.page9.boostByGear[2] = 255;
  context.page9.boostByGear[3] = 255;
  context.page9.boostByGear[4] = 255;
  context.page9.boostByGear[5] = 255;
  fill_table_values(boostTable, 255);

  context.setup_boost_enabled();
  for (uint8_t gear=1; gear<=6; ++gear)
  {
    context.current.gear = gear;
    context.current.boostTarget = 1;
    context.current.flexBoostCorrection = 99;
    boostPID.initialize(context.current.MAP);
    context.boostControl();

    TEST_ASSERT_EQUAL_UINT16(511, context.current.boostTarget);
    TEST_ASSERT_EQUAL_UINT16(642, context.current.boostDuty);
    TEST_ASSERT_EQUAL(0, context.current.flexBoostCorrection);
    TEST_ASSERT_NOT_EQUAL_UINT16(0, boostOutput.targetDuty);
}

  // Invalid gear
  context.current.boostTarget = 1;
  context.current.gear = 0;
  context.boostControl();
  TEST_ASSERT_EQUAL_UINT16(0, context.current.boostTarget);
  TEST_ASSERT_EQUAL_UINT16(0, context.current.boostDuty);
  TEST_ASSERT_EQUAL(0, context.current.flexBoostCorrection);
  TEST_ASSERT_EQUAL_UINT16(0, boostOutput.targetDuty);
}

static void test_cl_flexcorrection(void)
{
  auto context = setup_boost_tune(false, VSS_MODE_EXTERNAL_MI, CLOSED_LOOP_BOOST, BOOST_BY_GEAR_OFF);
  context.page2.flexEnabled = true;
  populate_2dtable(&flexBoostTable, (int16_t)77, (uint8_t)50);
  fill_table_values(boostTable, 55);

  context.initialise();

  context.setup_boost_enabled();
  context.current.flexBoostCorrection = 99;
  boostPID.initialize(context.current.MAP);
  context.boostControl();

  TEST_ASSERT_EQUAL_INT16(77, context.current.flexBoostCorrection);
  TEST_ASSERT_EQUAL_UINT16((boostTable.values[0]*2)+77, context.current.boostTarget);
  TEST_ASSERT_EQUAL_UINT16(577, context.current.boostDuty);
  TEST_ASSERT_NOT_EQUAL_UINT16(0, boostOutput.targetDuty);
}

static void test_cl_flexcorrection_negative(void)
{
  auto context = setup_boost_tune(false, VSS_MODE_EXTERNAL_MI, CLOSED_LOOP_BOOST, BOOST_BY_GEAR_OFF);
  context.page2.flexEnabled = true;
  populate_2dtable(&flexBoostTable, (int16_t)-77, (uint8_t)50);
  fill_table_values(boostTable, 5);

  context.initialise();

  context.setup_boost_enabled();
  context.current.flexBoostCorrection = 99;
  boostPID.initialize(context.current.MAP);
  context.boostControl();

  TEST_ASSERT_EQUAL_INT16(-77, context.current.flexBoostCorrection);
  TEST_ASSERT_EQUAL_UINT16(0, context.current.boostTarget);
  TEST_ASSERT_EQUAL_UINT16(0, context.current.boostDuty);
  TEST_ASSERT_EQUAL_UINT16(0, boostOutput.targetDuty);
}

static void test_cl_boost_constant_gear(test_context_t &context, uint8_t gearNum, uint8_t &boostGear)
{
  context.setup_boost_enabled();
  context.current.boostTarget = 1;
  context.current.gear = gearNum;
  boostGear = gearNum*7;
  boostPID.initialize(context.current.MAP);
  context.boostControl();
  TEST_ASSERT_UINT16_WITHIN(1, boostGear << 1U, context.current.boostTarget);
}

static void test_cl_boost_constant_gear(void)
{
  auto context = setup_boost_tune(false, VSS_MODE_EXTERNAL_MI, CLOSED_LOOP_BOOST, BOOST_BY_GEAR_CONSTANT);

  test_cl_boost_constant_gear(context, 1, context.page9.boostByGear[0]);
  test_cl_boost_constant_gear(context, 2, context.page9.boostByGear[1]);
  test_cl_boost_constant_gear(context, 3, context.page9.boostByGear[2]);
  test_cl_boost_constant_gear(context, 4, context.page9.boostByGear[3]);
  test_cl_boost_constant_gear(context, 5, context.page9.boostByGear[4]);
  test_cl_boost_constant_gear(context, 6, context.page9.boostByGear[5]);

  // Invalid gear
  context.setup_boost_enabled();
  context.current.boostTarget = 1;
  context.current.gear = 0;
  context.boostControl();
  TEST_ASSERT_EQUAL_UINT16(0U, context.current.boostTarget);
  TEST_ASSERT_EQUAL_UINT16(0U, context.current.boostDuty);
}

static void test_cl_boost_control_baro(void)
{
  auto context = setup_boost_tune(false, VSS_MODE_EXTERNAL_MI, CLOSED_LOOP_BOOST, BOOST_BY_GEAR_OFF);
  context.page15.boostControlEnable = EN_BOOST_CONTROL_BARO;
  context.page15.boostDCWhenDisabled = 77;
  
  context.setup_boost_enabled();
  context.current.baro = context.current.MAP;
  context.boostControl();
  TEST_ASSERT_NOT_EQUAL_UINT16(0, context.current.boostTarget);
  TEST_ASSERT_NOT_EQUAL_UINT16(0, context.current.boostDuty);
  
  context.current.baro = context.current.MAP + 10;
  context.boostControl();
  TEST_ASSERT_NOT_EQUAL_UINT16(0, context.current.boostTarget);
  TEST_ASSERT_EQUAL_UINT16(context.page15.boostDCWhenDisabled*100, context.current.boostDuty);
  
  context.current.baro = context.current.MAP - 10;
  context.boostControl();
  TEST_ASSERT_NOT_EQUAL_UINT16(0, context.current.boostTarget);
  TEST_ASSERT_NOT_EQUAL_UINT16(0, context.current.boostDuty);
}

static void test_cl_boost_control_fixed(void)
{
  auto context = setup_boost_tune(false, VSS_MODE_EXTERNAL_MI, CLOSED_LOOP_BOOST, BOOST_BY_GEAR_OFF);
  context.page15.boostControlEnable = EN_BOOST_CONTROL_FIXED;
  context.page15.boostDCWhenDisabled = 77;
  context.setup_boost_enabled();
  
  context.page15.boostControlEnableThreshold = context.current.MAP;
  context.boostControl();
  TEST_ASSERT_NOT_EQUAL_UINT16(0, context.current.boostTarget);
  TEST_ASSERT_NOT_EQUAL_UINT16(0, context.current.boostDuty);
  
  context.page15.boostControlEnableThreshold = context.current.MAP + 10;
  context.boostControl();
  TEST_ASSERT_NOT_EQUAL_UINT16(0, context.current.boostTarget);
  TEST_ASSERT_EQUAL_UINT16(context.page15.boostDCWhenDisabled*100, context.current.boostDuty);
  
  context.page15.boostControlEnableThreshold = context.current.MAP - 10;
  context.boostControl();
  TEST_ASSERT_NOT_EQUAL_UINT16(0, context.current.boostTarget);
  TEST_ASSERT_NOT_EQUAL_UINT16(0, context.current.boostDuty);
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