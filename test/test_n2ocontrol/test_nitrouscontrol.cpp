#include "src/controllers/nitrous/nitrousController_state.h"
#include "units.h"
#include "../test_utils.h"
#include "shared.h"

extern nitrous::detail::state_t _n2oState;

static void assert_n2o_off(test_context_t &context)
{
  context.current.nitrousStatus = NITROUS_BOTH;
  context.control();
  TEST_ASSERT_EQUAL(NITROUS_OFF, context.current.nitrousStatus);
  TEST_ASSERT_FALSE(_n2oState.stage1Pin._pin.isPinHigh());
  TEST_ASSERT_FALSE(_n2oState.stage2Pin._pin.isPinHigh());
}

static void test_off(void)
{
  auto context = setup_n20_tune(NITROUS_OFF);
  assert_n2o_off(context);
}

static void setup_valid_conditions(test_context_t &context)
{
  if (context.page10.n2o_pin_polarity)
  {
    _n2oState.armingPin._pin.setPinLow();
  }
  else
  {
    _n2oState.armingPin._pin.setPinHigh();
  }
  context.current.coolant = temperatureRemoveOffset(context.page10.n2o_minCLT)+1;
  context.current.TPS = context.page10.n2o_minTPS+1;
  context.current.O2 = context.page10.n2o_maxAFR -1;
  context.current.MAP = (context.page10.n2o_maxMAP-1)/2U;
}

static void setup_valid_conditions_stage1(test_context_t &context)
{
  setup_valid_conditions(context);
  uint8_t maxRpm = (std::min)(context.page10.n2o_stage1_maxRPM, context.page10.n2o_stage2_minRPM); // Just in case the ranges overlap
  context.current.setRpm(RPM_COARSE.toUser(intermediate(context.page10.n2o_stage1_minRPM, maxRpm, (uint8_t)50)));
}

static void setup_valid_conditions_stage2(test_context_t &context)
{
  setup_valid_conditions(context);
  uint8_t minRpm = (std::max)(context.page10.n2o_stage1_maxRPM, context.page10.n2o_stage2_minRPM); // Just in case the ranges overlap
  context.current.setRpm(RPM_COARSE.toUser(intermediate(minRpm, context.page10.n2o_stage2_maxRPM, (uint8_t)50)));
}

static void setup_valid_conditions_stageboth(test_context_t &context)
{
  setup_valid_conditions(context);
  context.current.setRpm(RPM_COARSE.toUser(intermediate(context.page10.n2o_stage2_minRPM, context.page10.n2o_stage1_maxRPM, (uint8_t)50)));
}

static void test_not_armed(void)
{
  auto context = setup_rpm_overlap_tune(NITROUS_BOTH);
  context.init();

  setup_valid_conditions_stageboth(context);
  if (context.page10.n2o_pin_polarity)
  {
    _n2oState.armingPin._pin.setPinHigh();
  }
  else
  {
    _n2oState.armingPin._pin.setPinLow();
  }

  assert_n2o_off(context);
}

static void test_coolant_threshhold(void)
{
  auto context = setup_rpm_overlap_tune(NITROUS_BOTH);
  context.init();

  setup_valid_conditions_stageboth(context);
  context.current.coolant = TEMPERATURE.toUser(context.page10.n2o_minCLT-1);
  assert_n2o_off(context);
}

static void test_tps_threshhold(void)
{
  auto context = setup_rpm_overlap_tune(NITROUS_BOTH);
  context.init();

  setup_valid_conditions_stageboth(context);
  context.current.TPS =context.page10.n2o_minTPS-1;
  assert_n2o_off(context);
}

static void test_o2_threshhold(void)
{
  auto context = setup_rpm_overlap_tune(NITROUS_BOTH);
  context.init();

  setup_valid_conditions_stageboth(context);
  context.current.O2 = context.page10.n2o_maxAFR+1;
  assert_n2o_off(context);
}

static void test_map_threshhold(void)
{
  auto context = setup_rpm_overlap_tune(NITROUS_BOTH);
  context.init();

  setup_valid_conditions_stageboth(context);
  context.current.MAP = (context.page10.n2o_maxMAP * 2U) + 1;
  assert_n2o_off(context);
}

static void test_stage1(void)
{
  auto context = setup_rpm_overlap_tune(NITROUS_STAGE1);
  context.init();

  setup_valid_conditions_stage1(context);
  context.control();
  TEST_ASSERT_EQUAL(NITROUS_STAGE1, context.current.nitrousStatus);
  TEST_ASSERT_TRUE(_n2oState.stage1Pin._pin.isPinHigh());

  context.current.setRpm(RPM_COARSE.toUser(context.page10.n2o_stage1_minRPM-1));
  context.control();
  TEST_ASSERT_FALSE(_n2oState.stage1Pin._pin.isPinHigh());

  context.current.setRpm(RPM_COARSE.toUser(context.page10.n2o_stage1_maxRPM+1));
  context.control();
  TEST_ASSERT_FALSE(_n2oState.stage1Pin._pin.isPinHigh());
}

static void test_stage2(void)
{
  auto context = setup_rpm_overlap_tune(NITROUS_BOTH);
  context.init();

  setup_valid_conditions_stage2(context);
  context.control();
  TEST_ASSERT_EQUAL(NITROUS_STAGE2, context.current.nitrousStatus);
  TEST_ASSERT_TRUE(_n2oState.stage2Pin._pin.isPinHigh());

  context.current.setRpm(RPM_COARSE.toUser(context.page10.n2o_stage2_minRPM-1));
  context.control();
  TEST_ASSERT_FALSE(_n2oState.stage2Pin._pin.isPinHigh());

  context.current.setRpm(RPM_COARSE.toUser(context.page10.n2o_stage2_maxRPM+1));
  context.control();
  TEST_ASSERT_FALSE(_n2oState.stage2Pin._pin.isPinHigh());
}

static void test_controlFrequency(void)
{
  auto context = setup_rpm_overlap_tune(NITROUS_STAGE1);
  context.init();

  setup_valid_conditions_stage1(context);
  nitrousControl(context.current, context.page10);
  TEST_ASSERT_EQUAL(NITROUS_OFF, context.current.nitrousStatus);
 
  BIT_SET(context.current.LOOP_TIMER, BIT_TIMER_4HZ);
  nitrousControl(context.current, context.page10);
  TEST_ASSERT_EQUAL(NITROUS_STAGE1, context.current.nitrousStatus);
}

void testN2oControl(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_off);
    RUN_TEST_P(test_not_armed);
    RUN_TEST_P(test_coolant_threshhold);
    RUN_TEST_P(test_tps_threshhold);
    RUN_TEST_P(test_o2_threshhold);
    RUN_TEST_P(test_map_threshhold);
    RUN_TEST_P(test_stage1);
    RUN_TEST_P(test_stage2);
    RUN_TEST_P(test_controlFrequency);
  }
}