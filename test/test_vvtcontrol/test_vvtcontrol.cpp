#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "units.h"

extern long vvt1_pwm_value;
extern long vvt2_pwm_value;
extern volatile bool vvt1_pwm_state;
extern volatile bool vvt2_pwm_state;
extern bool vvtTimeHold;
extern bool vvtIsHot;

static constexpr uint8_t TEST_VVT1_PIN = 19U;
static constexpr uint8_t TEST_VVT2_PIN = 20U;

static constexpr table3d_axis_t TEST_VVT_AXIS_X[8] = {
  7000U/100U,
  6000U/100U,
  5000U/100U,
  4000U/100U,
  3000U/100U,
  2000U/100U,
  1000U/100U,
  800U/100U
};

static constexpr table3d_axis_t TEST_VVT_AXIS_Y[8] = {
  100U/2U,
  80U/2U,
  60U/2U,
  50U/2U,
  40U/2U,
  30U/2U,
  20U/2U,
  10U/2U
};

// For coverage, we need to run all tests against TPS and MAP
static uint8_t loadSource = VVT_LOAD_MAP;
static bool testVvt2Enabled = false;
static bool testWmiEnabled = false;

static void assert_vvt2_duty(uint8_t expected)
{
  TEST_ASSERT_EQUAL(testVvt2Enabled ? expected : 0U, currentStatus.vvt2Duty);
}

static void setup_vvt_basic_tune(uint8_t mode)
{
  pinVVT_1 = TEST_VVT1_PIN;
  pinVVT_2 = TEST_VVT2_PIN;

  configPage6.vvtEnabled = true;
  configPage6.vvtFreq = 1U;
  configPage4.vvtMinClt = temperatureAddOffset(60);
  configPage4.vvtDelay = 0U;
  configPage6.vvtLoadSource = loadSource;
  configPage6.vvtMode = mode;
  configPage4.TrigPattern = 0U;

  configPage10.vvt2Enabled = testVvt2Enabled;
  configPage10.wmiEnabled = testWmiEnabled;;
  configPage10.vvtCLMinAng = INT8_MIN+5;
  configPage10.vvtCLMaxAng = UINT8_MAX-5;
  configPage10.vvtCLholdDuty = 100U;

  vvt_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (16U * configPage6.vvtFreq * 2U));
}

static void setup_vvt_onconditions(void)
{
  currentStatus.coolant = temperatureAddOffset(temperatureRemoveOffset(configPage4.vvtMinClt) + 1);
  currentStatus.rotationStatus = EngineRotationStatus::Running;
  currentStatus.setRpm(1000U);
  currentStatus.MAP = 50U;
  currentStatus.TPS = currentStatus.MAP / 2U;
  runSecsX10 = 0U;
  vvt2_pwm_state = false;
}

static void populate_vvt_tables(table3d_value_t vvt1Value, table3d_value_t vvt2Value)
{
  populate_table_axis_P(vvtTable.axisX.begin(), TEST_VVT_AXIS_X);
  populate_table_axis_P(vvtTable.axisY.begin(), TEST_VVT_AXIS_Y);
  fill_table_values(vvtTable, vvt1Value);

  populate_table_axis_P(vvt2Table.axisX.begin(), TEST_VVT_AXIS_X);
  populate_table_axis_P(vvt2Table.axisY.begin(), TEST_VVT_AXIS_Y);
  fill_table_values(vvt2Table, vvt2Value);
}

static void setup_vvt_openloop_tune(void)
{
    setup_vvt_basic_tune(VVT_MODE_OPEN_LOOP);
    populate_vvt_tables(150U, 150U);
}

static void setup_vvt_onoff_tune(void)
{
    setup_vvt_basic_tune(VVT_MODE_ONOFF);
}

static void setup_vvt_closedloop_tune(void)
{
  setup_vvt_basic_tune(VVT_MODE_CLOSED_LOOP);
  configPage6.vvtPWMdir = 1U;
  configPage4.vvt2PWMdir = configPage6.vvtPWMdir;
  configPage10.vvtCLholdDuty = 120U;
  configPage10.vvtCLKP = 5;
  configPage10.vvtCLKI = 4;
  configPage10.vvtCLKD = 3;
  configPage10.vvtCLminDuty = 0;
  configPage10.vvtCLmaxDuty = UINT8_MAX;
  populate_vvt_tables(150U, 150U);
}

// ============================ Shared assertions  ===============================

static void assert_vvt1_off(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, currentStatus.vvt1Duty);
  TEST_ASSERT_EQUAL_UINT8(0U, vvt1_pwm_value);
  TEST_ASSERT_FALSE(vvt1_pwm_state);
}

static void assert_vvt1_on(void)
{
  TEST_ASSERT_NOT_EQUAL_UINT8(0U, currentStatus.vvt1Duty);
  TEST_ASSERT_NOT_EQUAL_UINT8(0U, vvt1_pwm_value);
  if (vvt1_pwm_value==200U)
  {
    TEST_ASSERT_TRUE(vvt1_pwm_state);
  }
}

static void assert_vvt2_off(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, currentStatus.vvt2Duty);
  TEST_ASSERT_EQUAL_UINT8(0U, vvt2_pwm_value);
  TEST_ASSERT_FALSE(vvt2_pwm_state);
}

static void assert_vvt2_on(void)
{
  if (testVvt2Enabled)
  {
    TEST_ASSERT_NOT_EQUAL_UINT8(0U, currentStatus.vvt2Duty);
    TEST_ASSERT_NOT_EQUAL_UINT8(0U, vvt2_pwm_value);
    if (vvt2_pwm_value==200U)
    {
        TEST_ASSERT_TRUE(vvt2_pwm_state);
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
  setup_vvt_openloop_tune();
  initialiseAuxPWM();
  vvt1Off();
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_VVT1_PIN));
  vvt1On();
  TEST_ASSERT_EQUAL(HIGH, digitalRead(TEST_VVT1_PIN));
  vvt1Off();
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_VVT1_PIN));
}

static void test_vvt2On_and_Off_toggle_pin(void)
{
  setup_vvt_openloop_tune();
  initialiseAuxPWM();
  vvt2Off();
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_VVT2_PIN));
  vvt2On();
  TEST_ASSERT_EQUAL(HIGH, digitalRead(TEST_VVT2_PIN));
  vvt2Off();
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_VVT2_PIN));
}

// ============================ VVT Control ===============================

static void test_vvtControl_disabled_noduty(void)
{
  setup_vvt_openloop_tune();
  configPage6.vvtEnabled = false;
  initialiseAuxPWM();

  setup_vvt_onconditions();
  vvtControl();

  assert_vvt1_off();
  assert_vvt2_off();
}

static void test_vvtControl_coolantlow_noduty(void)
{
  setup_vvt_openloop_tune();
  initialiseAuxPWM();

  setup_vvt_onconditions();
  currentStatus.coolant = temperatureRemoveOffset(configPage4.vvtMinClt-1);
  vvtControl();

  assert_vvt1_off();
  assert_vvt2_off();
}

static void test_vvtControl_engineoff_noduty(void)
{
  setup_vvt_openloop_tune();
  initialiseAuxPWM();

  setup_vvt_onconditions();
  currentStatus.rotationStatus = EngineRotationStatus::Stopped;
  vvtControl();

  assert_vvt1_off();
  assert_vvt2_off();
}

static void test_vvtControl_enginecranking_noduty(void)
{
  setup_vvt_openloop_tune();
  initialiseAuxPWM();

  setup_vvt_onconditions();
  currentStatus.rotationStatus = EngineRotationStatus::Cranking;
  vvtControl();

  assert_vvt1_off();
  assert_vvt2_off();
}

static void test_vvtControl_open_loop_sets_vvt_duty_from_table(void)
{
  setup_vvt_openloop_tune();
  initialiseAuxPWM();

  setup_vvt_onconditions();
  vvtControl();

  assert_vvt1_on();
  assert_vvt2_on();
}

static void test_vvtControl_onoff_mode_turns_pins_off_when_duty_below_threshold(void)
{
  setup_vvt_onoff_tune();
  initialiseAuxPWM();

  setup_vvt_onconditions();
  vvtControl();

  assert_vvt1_off();
  assert_vvt2_off();
}

static void test_vvtControl_onoff_mode_turns_pins_on_when_duty_above_threshold(void)
{
  setup_vvt_onoff_tune();
  initialiseAuxPWM();

  setup_vvt_onconditions();
  populate_vvt_tables(210U, 210U);
  vvtControl();

  assert_vvt1_on();
  assert_vvt2_on();
}

static void test_vvtControl_delay_holds_until_elapsed(void)
{
  setup_vvt_openloop_tune();
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
  setup_vvt_closedloop_tune();
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
  setup_vvt_closedloop_tune();
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
  setup_vvt_closedloop_tune();
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