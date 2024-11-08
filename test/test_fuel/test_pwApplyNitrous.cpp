#include <unity.h>
#include "../test_utils.h"

extern uint16_t pwApplyNitrous(uint16_t pw, const config10 &page10, const statuses &current);

static inline void setup_nitrous_stage1(config10 &page10, statuses &current) {
  page10.n2o_stage1_minRPM = 20; // RPM/100
  page10.n2o_stage1_maxRPM = 30; // RPM/100
  page10.n2o_stage1_adderMin = 11; // milliseconds
  page10.n2o_stage1_adderMax = 3; // milliseconds

  current.nitrous_status = NITROUS_STAGE1;
}

static inline void setup_nitrous_stage2(config10 &page10, statuses &current) {
  page10.n2o_stage2_minRPM = 25; // RPM/100
  page10.n2o_stage2_maxRPM = 30; // RPM/100
  page10.n2o_stage2_adderMin = 7; // milliseconds
  page10.n2o_stage2_adderMax = 1; // milliseconds

  current.nitrous_status = NITROUS_STAGE2;
}

static void test_pwApplyNitrous_No(void) {
  config10 page10 = {};
  statuses current = {};

  setup_nitrous_stage1(page10, current);
  current.RPMdiv100 = page10.n2o_stage1_minRPM+4;

  // No fuel, no nitrous adder
  TEST_ASSERT_EQUAL(0, pwApplyNitrous(0, page10, current));
  current.nitrous_status = NITROUS_OFF;
  TEST_ASSERT_EQUAL(200, pwApplyNitrous(200, page10, current));
}

static void test_pwApplyNitrous_Stage1(void) {
  config10 page10 = {};
  statuses current = {};

  setup_nitrous_stage1(page10, current);
  current.RPMdiv100 = page10.n2o_stage1_minRPM+4;

  // Confirm nitrous is added
  TEST_ASSERT_EQUAL(1000, pwApplyNitrous(200, page10, current));
}

static void test_pwApplyNitrous_Stage2(void) {
  config10 page10 = {};
  statuses current = {};

  setup_nitrous_stage2(page10, current);
  current.RPMdiv100 = page10.n2o_stage2_maxRPM-3U;  

  // Confirm nitrous is added
  TEST_ASSERT_EQUAL(700, pwApplyNitrous(200, page10, current));
}

static void test_pwApplyNitrous_StageBoth(void) {
  config10 page10 = {};
  statuses current = {};

  setup_nitrous_stage1(page10, current);
  setup_nitrous_stage2(page10, current);
  current.RPMdiv100 = page10.n2o_stage2_maxRPM-3U;
  current.nitrous_status = NITROUS_BOTH;

  // Confirm nitrous is added
  TEST_ASSERT_EQUAL(1300, pwApplyNitrous(200, page10, current));
}

void testPwApplyNitrous(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_pwApplyNitrous_No);
    RUN_TEST_P(test_pwApplyNitrous_Stage1);
    RUN_TEST_P(test_pwApplyNitrous_Stage2);
    RUN_TEST_P(test_pwApplyNitrous_StageBoth);    
  }
}