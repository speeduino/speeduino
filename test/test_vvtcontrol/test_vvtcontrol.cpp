#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "units.h"
#include "src/pins/boardOutputPin.h"
#include "src/controllers/vvt/VvtOutputChannel.h"
#include "shared.h"

extern VvtOutputChannel vvtChannel1;
extern VvtOutputChannel vvtChannel2;
extern bool vvtTimeHold;
extern bool vvtIsHot;

// For coverage, we need to run all tests against TPS and MAP
static uint8_t loadSource = VVT_LOAD_MAP;
static bool testVvt2Enabled = false;
static bool testWmiEnabled = false;

static void assert_vvt2_duty(uint8_t expected)
{
  TEST_ASSERT_EQUAL(testVvt2Enabled ? expected : 0U, currentStatus.vvt2Duty);
}

static void setup_vvt_onconditions(void)
{
  currentStatus.coolant = temperatureAddOffset(temperatureRemoveOffset(configPage4.vvtMinClt) + 1);
  currentStatus.rotationStatus = EngineRotationStatus::Running;
  currentStatus.setRpm(1000U);
  currentStatus.MAP = 50U;
  currentStatus.TPS = currentStatus.MAP / 2U;
  runSecsX10 = 0U;
  vvtChannel2.pin.setPinLow();
}

// ============================ Shared assertions  ===============================

static void assert_vvt1_off(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, currentStatus.vvt1Duty);
  TEST_ASSERT_EQUAL_UINT8(0U, vvtChannel1.targetDuty);
  TEST_ASSERT_FALSE(vvtChannel1.pin.isPinHigh());
}

static void assert_vvt1_on(void)
{
  TEST_ASSERT_NOT_EQUAL_UINT8(0U, currentStatus.vvt1Duty);
  TEST_ASSERT_NOT_EQUAL_UINT8(0U, vvtChannel1.targetDuty);
  if (vvtChannel1.targetDuty==200U)
  {
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
  }
}

static void assert_vvt2_off(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, currentStatus.vvt2Duty);
  TEST_ASSERT_EQUAL_UINT8(0U, vvtChannel2.targetDuty);
  TEST_ASSERT_FALSE(vvtChannel2.pin.isPinHigh());
}

static void assert_vvt2_on(void)
{
  if (testVvt2Enabled)
  {
    TEST_ASSERT_NOT_EQUAL_UINT8(0U, currentStatus.vvt2Duty);
    TEST_ASSERT_NOT_EQUAL_UINT8(0U, vvtChannel2.targetDuty);
    if (vvtChannel2.targetDuty==200U)
    {
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    }
  }
  else
  {
    assert_vvt2_off();
  }
}

// ============================ VVT pin drivers ===============================

static void test_vvt1On_and_Off_toggle_pin(void)
{
  setup_vvt_openloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  initialiseAuxPWM();
  vvt1Off();
  TEST_ASSERT_TRUE(vvtChannel1.pin.isPinLow());
  vvt1On();
  TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
  vvt1Off();
  TEST_ASSERT_TRUE(vvtChannel1.pin.isPinLow());
}

static void test_vvt2On_and_Off_toggle_pin(void)
{
  setup_vvt_openloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  initialiseAuxPWM();
  vvt2Off();
  TEST_ASSERT_TRUE(vvtChannel2.pin.isPinLow());
  vvt2On();
  TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
  vvt2Off();
  TEST_ASSERT_TRUE(vvtChannel2.pin.isPinLow());
}

// ============================ VVT Control ===============================

static void test_vvtControl_disabled_noduty(void)
{
  setup_vvt_openloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  configPage6.vvtEnabled = false;
  initialiseAuxPWM();

  setup_vvt_onconditions();
  vvtControl();

  assert_vvt1_off();
  assert_vvt2_off();
}

static void test_vvtControl_coolantlow_noduty(void)
{
  setup_vvt_openloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  initialiseAuxPWM();

  setup_vvt_onconditions();
  currentStatus.coolant = temperatureRemoveOffset(configPage4.vvtMinClt-1);
  vvtControl();

  assert_vvt1_off();
  assert_vvt2_off();
}

static void test_vvtControl_engineoff_noduty(void)
{
  setup_vvt_openloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  initialiseAuxPWM();

  setup_vvt_onconditions();
  currentStatus.rotationStatus = EngineRotationStatus::Stopped;
  vvtControl();

  assert_vvt1_off();
  assert_vvt2_off();
}

static void test_vvtControl_enginecranking_noduty(void)
{
  setup_vvt_openloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  initialiseAuxPWM();

  setup_vvt_onconditions();
  currentStatus.rotationStatus = EngineRotationStatus::Cranking;
  vvtControl();

  assert_vvt1_off();
  assert_vvt2_off();
}

static void test_vvtControl_open_loop_sets_vvt_duty_from_table(void)
{
  setup_vvt_openloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  initialiseAuxPWM();

  setup_vvt_onconditions();
  vvtControl();

  assert_vvt1_on();
  assert_vvt2_on();
}

static void test_vvtControl_onoff_mode_turns_pins_off_when_duty_below_threshold(void)
{
  setup_vvt_onoff_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  initialiseAuxPWM();

  setup_vvt_onconditions();
  vvtControl();

  assert_vvt1_off();
  assert_vvt2_off();
}

static void test_vvtControl_onoff_mode_turns_pins_on_when_duty_above_threshold(void)
{
  setup_vvt_onoff_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  initialiseAuxPWM();

  setup_vvt_onconditions();
  populate_vvt_tables(210U, 210U);
  vvtControl();

  assert_vvt1_on();
  assert_vvt2_on();
}

static void test_vvtControl_delay_holds_until_elapsed(void)
{
  setup_vvt_openloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  configPage4.vvtDelay = TIME_TWENTY_MILLIS.toRaw(33);
  initialiseAuxPWM();

  setup_vvt_onconditions();
  runSecsX10 = TIME_TWENTY_MILLIS.toUser(configPage4.vvtDelay-1);
  vvtIsHot = false;
  vvtTimeHold = false;
  vvtControl();

  TEST_ASSERT_TRUE(vvtTimeHold);
  TEST_ASSERT_FALSE(vvtIsHot);
  TEST_ASSERT_EQUAL_UINT8(0U, currentStatus.vvt1Duty);
  TEST_ASSERT_EQUAL_UINT8(0U, currentStatus.vvt2Duty);

  vvtControl();

  TEST_ASSERT_TRUE(vvtTimeHold);
  TEST_ASSERT_FALSE(vvtIsHot);
}

static void mirror_vvt2_conditions(void)
{
  currentStatus.vvt2Angle = currentStatus.vvt1Angle;
  currentStatus.vvt2TargetAngle = currentStatus.vvt1TargetAngle;
  currentStatus.vvt2AngleError = currentStatus.vvt1AngleError;
  currentStatus.vvt2Duty = currentStatus.vvt1Duty;
}

static void assert_angle_error(uint8_t expectedDuty, bool expectedError)
{
  TEST_ASSERT_EQUAL_UINT8(expectedDuty, currentStatus.vvt1Duty);
  TEST_ASSERT_TRUE(expectedError==currentStatus.vvt1AngleError);
  assert_vvt2_duty(currentStatus.vvt1Duty);
  TEST_ASSERT_TRUE(!testVvt2Enabled || (currentStatus.vvt2AngleError==currentStatus.vvt2AngleError));
}

static void test_vvtControl_closed_loop_hold_sets_hold_duty(void)
{
  setup_vvt_closedloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  configPage6.vvtCLUseHold = 1U;
  initialiseAuxPWM();

  setup_vvt_onconditions();
  currentStatus.vvt1Angle = 150;
  currentStatus.vvt1TargetAngle = 150;
  currentStatus.vvt1AngleError = false;
  currentStatus.vvt1Duty = 0;
  mirror_vvt2_conditions();

  vvtControl();
  assert_angle_error(120, false);
}

static void test_vvtControl_closed_loop_angle_error_sets_error(void)
{
  setup_vvt_closedloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  initialiseAuxPWM();

  setup_vvt_onconditions();
  currentStatus.vvt1Angle = configPage10.vvtCLMinAng - 1;
  currentStatus.vvt1Duty = 0;
  mirror_vvt2_conditions();
  
  vvtControl();
  assert_angle_error(0, true);

  setup_vvt_onconditions();
  currentStatus.vvt1Angle = configPage10.vvtCLMaxAng + 1;
  currentStatus.vvt1Duty = 0;
  mirror_vvt2_conditions();
  
  vvtControl();
  assert_angle_error(0, true);
}

static void test_vvtControl_closed_loop_nohold_noangle_error(void)
{
  setup_vvt_closedloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  configPage6.vvtCLUseHold = false;
  initialiseAuxPWM();

  setup_vvt_onconditions();
  currentStatus.vvt1Angle = configPage10.vvtCLMinAng + 1;
  currentStatus.vvt1TargetAngle = currentStatus.vvt1Angle + 5U;
  currentStatus.vvt1Duty = 0;
  mirror_vvt2_conditions();
  
  vvtControl();
 
  TEST_ASSERT_EQUAL_UINT8(234, currentStatus.vvt1Duty);
  TEST_ASSERT_FALSE(currentStatus.vvt1AngleError);
  assert_vvt2_duty(currentStatus.vvt1Duty);
  TEST_ASSERT_TRUE(!testVvt2Enabled || (currentStatus.vvt2AngleError==currentStatus.vvt2AngleError));
}

void testVvtControl(void)
{
  SET_UNITY_FILENAME()
  {
    // For completeness, we need to run all tests with and without WMI enabled
    for (bool wmi : (bool[2]){ false, true }) 
    {
        testWmiEnabled = wmi;
        // For completeness, we need to run all tests with and without VVT2 enabled
        for (bool vvt2 : (bool[2]){ false, true }) 
        {
            testVvt2Enabled = vvt2;
            // For completeness, we need to run all tests against TPS and MAP
            for (loadSource = VVT_LOAD_MAP; loadSource<=VVT_LOAD_TPS; ++loadSource)
            {
                RUN_TEST_P(test_vvtControl_disabled_noduty);
                RUN_TEST_P(test_vvtControl_coolantlow_noduty);
                RUN_TEST_P(test_vvtControl_engineoff_noduty);
                RUN_TEST_P(test_vvtControl_enginecranking_noduty);
                RUN_TEST_P(test_vvt1On_and_Off_toggle_pin);
                RUN_TEST_P(test_vvt2On_and_Off_toggle_pin);
                RUN_TEST_P(test_vvtControl_open_loop_sets_vvt_duty_from_table);
                RUN_TEST_P(test_vvtControl_onoff_mode_turns_pins_off_when_duty_below_threshold);
                RUN_TEST_P(test_vvtControl_onoff_mode_turns_pins_on_when_duty_above_threshold);
                RUN_TEST_P(test_vvtControl_delay_holds_until_elapsed);
                RUN_TEST_P(test_vvtControl_closed_loop_hold_sets_hold_duty);
                RUN_TEST_P(test_vvtControl_closed_loop_angle_error_sets_error);
                RUN_TEST_P(test_vvtControl_closed_loop_nohold_noangle_error);
            }
        }
    }
  }
}