#include "globals.h"
#include "auxiliaries.h"
#include "units.h"
#include "../test_utils.h"
#include "shared.h"
#include "board_definition.h"
#include "src/pins/fastInputPin.h"
#include "src/pins/fastOutputPin.h"
#include "src/pins/outputPin.h"

extern fastInputPin_t n2o_arming_pin;
extern boardOutputPin_t n2o_stage1_pin;
extern boardOutputPin_t n2o_stage2_pin;

static void assert_n2o_off(void)
{
  currentStatus.nitrousActive = true;
  currentStatus.nitrous_status = NITROUS_BOTH;
  nitrousControl();
  TEST_ASSERT_FALSE(currentStatus.nitrousActive);
  TEST_ASSERT_EQUAL(NITROUS_OFF, currentStatus.nitrous_status);
  TEST_ASSERT_FALSE(n2o_stage1_pin._pin.isPinHigh());
  TEST_ASSERT_FALSE(n2o_stage2_pin._pin.isPinHigh());
}

static void test_off(void)
{
  setup_n20_tune(NITROUS_OFF);
  assert_n2o_off();

  setup_n20_tune(NITROUS_OFF);
  configPage10.n2o_enable = false;
  assert_n2o_off();
}

static void setup_valid_conditions(void)
{
  if (configPage10.n2o_pin_polarity)
  {
    n2o_arming_pin._pin.setPinLow();
  }
  else
  {
    n2o_arming_pin._pin.setPinHigh();
  }
  currentStatus.coolant = temperatureRemoveOffset(configPage10.n2o_minCLT)+1;
  currentStatus.TPS = configPage10.n2o_minTPS+1;
  currentStatus.O2 = configPage10.n2o_maxAFR -1;
  currentStatus.MAP = (configPage10.n2o_maxMAP-1)/2U;
}

static void setup_valid_conditions_stage1(void)
{
  setup_valid_conditions();
  uint8_t maxRpm = min(configPage10.n2o_stage1_maxRPM, configPage10.n2o_stage2_minRPM); // Just in case the ranges overlap
  currentStatus.setRpm(RPM_COARSE.toUser(intermediate(configPage10.n2o_stage1_minRPM, maxRpm, (uint8_t)50)));
}

static void setup_valid_conditions_stage2(void)
{
  setup_valid_conditions();
  uint8_t minRpm = max(configPage10.n2o_stage1_maxRPM, configPage10.n2o_stage2_minRPM); // Just in case the ranges overlap
  currentStatus.setRpm(RPM_COARSE.toUser(intermediate(minRpm, configPage10.n2o_stage2_maxRPM, (uint8_t)50)));
}

static void setup_valid_conditions_stageboth(void)
{
  setup_valid_conditions();
  currentStatus.setRpm(RPM_COARSE.toUser(intermediate(configPage10.n2o_stage2_minRPM, configPage10.n2o_stage1_maxRPM, (uint8_t)50)));
}

static void test_not_armed(void)
{
  setup_rpm_overlap_tune(NITROUS_BOTH);
  initialiseAuxPWM();

  setup_valid_conditions_stageboth();
  if (configPage10.n2o_pin_polarity)
  {
    n2o_arming_pin._pin.setPinHigh();
  }
  else
  {
    n2o_arming_pin._pin.setPinLow();
  }

  assert_n2o_off();
}

static void test_coolant_threshhold(void)
{
  setup_rpm_overlap_tune(NITROUS_BOTH);
  initialiseAuxPWM();

  setup_valid_conditions_stageboth();
  currentStatus.coolant = TEMPERATURE.toUser(configPage10.n2o_minCLT-1);
  assert_n2o_off();
}

static void test_tps_threshhold(void)
{
  setup_rpm_overlap_tune(NITROUS_BOTH);
  initialiseAuxPWM();

  setup_valid_conditions_stageboth();
  currentStatus.TPS =configPage10.n2o_minTPS-1;
  assert_n2o_off();
}

static void test_o2_threshhold(void)
{
  setup_rpm_overlap_tune(NITROUS_BOTH);
  initialiseAuxPWM();

  setup_valid_conditions_stageboth();
  currentStatus.O2 = configPage10.n2o_maxAFR+1;
  assert_n2o_off();
}

static void test_map_threshhold(void)
{
  setup_rpm_overlap_tune(NITROUS_BOTH);
  initialiseAuxPWM();

  setup_valid_conditions_stageboth();
  currentStatus.MAP = (configPage10.n2o_maxMAP * 2U) + 1;
  assert_n2o_off();
}

static void test_stage1(void)
{
  setup_rpm_overlap_tune(NITROUS_BOTH);
  initialiseAuxPWM();
  setup_valid_conditions_stage1();

  nitrousControl();
  TEST_ASSERT_TRUE(currentStatus.nitrousActive);
  TEST_ASSERT_EQUAL(NITROUS_STAGE1, currentStatus.nitrous_status);
  TEST_ASSERT_TRUE(n2o_stage1_pin._pin.isPinHigh());
  TEST_ASSERT_FALSE(n2o_stage2_pin._pin.isPinHigh());
}

static void test_stage2(void)
{
  setup_rpm_overlap_tune(NITROUS_STAGE2);
  initialiseAuxPWM();
  setup_valid_conditions_stage2();

  nitrousControl();
  TEST_ASSERT_TRUE(currentStatus.nitrousActive);
  TEST_ASSERT_EQUAL(NITROUS_STAGE2, currentStatus.nitrous_status);
  // TEST_ASSERT_FALSE(n2o_stage1_pin._pin.isPinHigh());
  TEST_ASSERT_TRUE(n2o_stage2_pin._pin.isPinHigh());
}

static void test_stage_both(void)
{
  setup_rpm_overlap_tune(NITROUS_STAGE2);
  initialiseAuxPWM();
  setup_valid_conditions_stageboth();

  nitrousControl();
  TEST_ASSERT_TRUE(currentStatus.nitrousActive);
  TEST_ASSERT_EQUAL(NITROUS_BOTH, currentStatus.nitrous_status);
  TEST_ASSERT_TRUE(n2o_stage1_pin._pin.isPinHigh());
  TEST_ASSERT_TRUE(n2o_stage2_pin._pin.isPinHigh());
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
    RUN_TEST_P(test_stage_both);
  }
}