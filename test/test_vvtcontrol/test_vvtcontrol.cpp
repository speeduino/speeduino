#include "../test_utils.h"
#include "auxiliaries.h"
#include "units.h"
#include "src/pins/boardOutputPin.h"
#include "src/controllers/vvt/VvtOutputChannel.h"
#include "shared.h"

extern VvtOutputChannel vvtChannel1;
extern VvtOutputChannel vvtChannel2;
extern uint32_t vvtWarmStartTime;
extern volatile uint32_t runSecsX10;

// For coverage, we need to run all tests against TPS and MAP
static uint8_t loadSource = VVT_LOAD_MAP;
static bool testVvt2Enabled = false;
static bool testWmiEnabled = false;

static void assert_vvt2_duty(const test_context_t &context, uint8_t expected)
{
  TEST_ASSERT_EQUAL((testVvt2Enabled && !testWmiEnabled) ? expected : 0U, context.current.vvt2.duty);
}

static void setup_vvt_onconditions(test_context_t &context)
{
  context.current.coolant = temperatureAddOffset(temperatureRemoveOffset(context.page4.vvtMinClt) + 1);
  context.current.rotationStatus = EngineRotationStatus::Running;
  context.current.setRpm(1000U);
  context.current.MAP = 50U;
  context.current.TPS = context.current.MAP / 2U;
  runSecsX10 = 0U;
  vvtChannel2.pin.setPinLow();
}

// ============================ Shared assertions  ===============================

static void assert_vvt1_off(const test_context_t &context)
{
  TEST_ASSERT_EQUAL_UINT8(0U, context.current.vvt1.duty);
  TEST_ASSERT_EQUAL_UINT8(0U, vvtChannel1.targetDuty);
  TEST_ASSERT_FALSE(vvtChannel1.pin.isPinHigh());
}

static void assert_vvt1_on(const test_context_t &context)
{
  TEST_ASSERT_NOT_EQUAL_UINT8(0U, context.current.vvt1.duty);
  TEST_ASSERT_NOT_EQUAL_UINT8(0U, vvtChannel1.targetDuty);
  if (vvtChannel1.targetDuty==200U)
  {
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
  }
}

static void assert_vvt2_off(const test_context_t &context)
{
  TEST_ASSERT_EQUAL_UINT8(0U, context.current.vvt2.duty);
  TEST_ASSERT_EQUAL_UINT8(0U, vvtChannel2.targetDuty);
  TEST_ASSERT_FALSE(vvtChannel2.pin.isPinHigh());
}

static void assert_vvt2_on(const test_context_t &context)
{
  if (testVvt2Enabled && !testWmiEnabled)
  {
    TEST_ASSERT_NOT_EQUAL_UINT8(0U, context.current.vvt2.duty);
    TEST_ASSERT_NOT_EQUAL_UINT8(0U, vvtChannel2.targetDuty);
    if (vvtChannel2.targetDuty==200U)
    {
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    }
  }
  else
  {
    assert_vvt2_off(context);
}
}


// ============================ VVT pin drivers ===============================

static void test_vvt1On_and_Off_toggle_pin(void)
{
  auto context = setup_vvt_openloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  context.initialise();
  vvt1Off();
  TEST_ASSERT_TRUE(vvtChannel1.pin.isPinLow());
  vvt1On();
  TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
  vvt1Off();
  TEST_ASSERT_TRUE(vvtChannel1.pin.isPinLow());
}

static void test_vvt2On_and_Off_toggle_pin(void)
{
  auto context = setup_vvt_openloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  context.initialise();
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
  auto context = setup_vvt_openloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  context.page6.vvtEnabled = false;
  context.initialise();

  setup_vvt_onconditions(context);
  context.vvtControl();

  assert_vvt1_off(context);
  assert_vvt2_off(context);
}

static void test_vvtControl_coolantlow_noduty(void)
{
  auto context = setup_vvt_openloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  context.initialise();

  setup_vvt_onconditions(context);
  context.current.coolant = temperatureRemoveOffset(context.page4.vvtMinClt-1);
  context.vvtControl();

  assert_vvt1_off(context);
  assert_vvt2_off(context);
}

static void test_vvtControl_engineoff_noduty(void)
{
  auto context = setup_vvt_openloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  context.initialise();

  setup_vvt_onconditions(context);
  context.current.rotationStatus = EngineRotationStatus::Stopped;
  context.vvtControl();

  assert_vvt1_off(context);
  assert_vvt2_off(context);
}

static void test_vvtControl_enginecranking_noduty(void)
{
  auto context = setup_vvt_openloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  context.initialise();

  setup_vvt_onconditions(context);
  context.current.rotationStatus = EngineRotationStatus::Cranking;
  context.vvtControl();

  assert_vvt1_off(context);
  assert_vvt2_off(context);
}

static void test_vvtControl_open_loop_sets_vvt_duty_from_table(void)
{
  auto context = setup_vvt_openloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  context.initialise();

  setup_vvt_onconditions(context);
  context.vvtControl();

  assert_vvt1_on(context);
  assert_vvt2_on(context);
}

static void test_vvtControl_onoff_mode_turns_pins_off_when_duty_below_threshold(void)
{
  auto context = setup_vvt_onoff_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  context.initialise();

  setup_vvt_onconditions(context);
  context.vvtControl();

  assert_vvt1_off(context);
  assert_vvt2_off(context);
}

static void test_vvtControl_onoff_mode_turns_pins_on_when_duty_above_threshold(void)
{
  auto context = setup_vvt_onoff_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  context.initialise();

  setup_vvt_onconditions(context);
  populate_vvt_tables(210U, 210U);
  context.vvtControl();

  assert_vvt1_on(context);
  assert_vvt2_on(context);
}

static void test_vvtControl_delay_holds_until_elapsed(void)
{
  auto context = setup_vvt_openloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  context.page4.vvtDelay = TIME_TWO_MILLIS.toRaw(500);
  context.initialise();

  setup_vvt_onconditions(context);
  context.current.vvt1.duty = 0U;
  vvtWarmStartTime = 0U;

  constexpr uint16_t INTIAL_RUNSECSX10 = 500;
  runSecsX10 = INTIAL_RUNSECSX10;
  context.vvtControl();
  TEST_ASSERT_EQUAL(runSecsX10, vvtWarmStartTime);
  TEST_ASSERT_EQUAL(0, context.current.vvt1.duty);

  runSecsX10 = INTIAL_RUNSECSX10 + TIME_TWO_MILLIS.toUser(context.page4.vvtDelay) - 1U;
  context.vvtControl();
  TEST_ASSERT_EQUAL(INTIAL_RUNSECSX10, vvtWarmStartTime);
  TEST_ASSERT_EQUAL(0, context.current.vvt1.duty);
 
  runSecsX10 = INTIAL_RUNSECSX10 + TIME_TWO_MILLIS.toUser(context.page4.vvtDelay);
  context.vvtControl();
  TEST_ASSERT_EQUAL(INTIAL_RUNSECSX10, vvtWarmStartTime);
  TEST_ASSERT_NOT_EQUAL(0, context.current.vvt1.duty);

  // Test reset
  context.current.rotationStatus = EngineRotationStatus::Stopped;
  context.vvtControl();
  TEST_ASSERT_EQUAL(0, vvtWarmStartTime);
  TEST_ASSERT_EQUAL(0, context.current.vvt1.duty);

  context.current.rotationStatus = EngineRotationStatus::Running;
  runSecsX10 = INTIAL_RUNSECSX10;
  context.vvtControl();
  TEST_ASSERT_EQUAL(runSecsX10, vvtWarmStartTime);
  TEST_ASSERT_EQUAL(0, context.current.vvt1.duty);
}

static void mirror_vvt2_conditions(test_context_t &context)
{
  context.current.vvt2.angle = context.current.vvt1.angle;
  context.current.vvt2.targetAngle = context.current.vvt1.targetAngle;
  context.current.vvt2.angleError = context.current.vvt1.angleError;
  context.current.vvt2.duty = context.current.vvt1.duty;
}

static void assert_angle_error(const test_context_t&context, uint8_t expectedDuty, bool expectedError)
{
  TEST_ASSERT_EQUAL_UINT8(expectedDuty, context.current.vvt1.duty);
  TEST_ASSERT_TRUE(expectedError==context.current.vvt1.angleError);
  assert_vvt2_duty(context, context.current.vvt1.duty);
  TEST_ASSERT_TRUE(!testVvt2Enabled || (context.current.vvt2.angleError==context.current.vvt2.angleError));
}

static void test_vvtControl_closed_loop_hold_sets_hold_duty(void)
{
  auto context = setup_vvt_closedloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  context.page6.vvtCLUseHold = 1U;
  context.initialise();

  setup_vvt_onconditions(context);
  context.current.vvt1.angle = 150;
  context.current.vvt1.targetAngle = 150;
  context.current.vvt1.angleError = false;
  context.current.vvt1.duty = 0;
  mirror_vvt2_conditions(context);

  context.vvtControl();
  assert_angle_error(context, 120, false);
}

static void test_vvtControl_closed_loop_angle_error_sets_error(void)
{
  auto context = setup_vvt_closedloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  context.initialise();

  setup_vvt_onconditions(context);
  context.current.vvt1.angle = context.page10.vvtCLMinAng - 1;
  context.current.vvt1.duty = 0;
  mirror_vvt2_conditions(context);
  
  context.vvtControl();
  assert_angle_error(context, 0, true);

  setup_vvt_onconditions(context);
  context.current.vvt1.angle = context.page10.vvtCLMaxAng + 1;
  context.current.vvt1.duty = 0;
  mirror_vvt2_conditions(context);
  
  context.vvtControl();
  assert_angle_error(context, 0, true);
}

static void test_vvtControl_closed_loop_nohold_noangle_error(void)
{
  auto context = setup_vvt_closedloop_tune(loadSource, testVvt2Enabled, testWmiEnabled);
  context.page6.vvtCLUseHold = false;
  context.initialise();

  setup_vvt_onconditions(context);
  context.current.vvt1.angle = context.page10.vvtCLMinAng + 1;
  context.current.vvt1.targetAngle = context.current.vvt1.angle + 5U;
  context.current.vvt1.duty = 0;
  mirror_vvt2_conditions(context);
  
  context.vvtControl();
 
  TEST_ASSERT_EQUAL_UINT8(234, context.current.vvt1.duty);
  TEST_ASSERT_FALSE(context.current.vvt1.angleError);
  assert_vvt2_duty(context, context.current.vvt1.duty);
  TEST_ASSERT_TRUE(!testVvt2Enabled || (context.current.vvt2.angleError==context.current.vvt2.angleError));
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