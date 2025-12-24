#include <unity.h>
#include "../test_utils.h"
#include "config_pages.h"
#include "statuses.h"
#include "test_pw_applyNitrous.h"

extern uint16_t pwApplyNitrous(uint16_t pw, const config10 &page10, const statuses &current);

static void test_pwApplyNitrous_No(void) {
  statuses current = {};
  config10 page10 = {};
  setup_nitrous_stage1(page10, current);
  current.RPM = (page10.n2o_stage1_minRPM+4)*100;

  // No fuel, no nitrous adder
  TEST_ASSERT_EQUAL(0, pwApplyNitrous(0, page10, current));
  current.nitrous_status = NITROUS_OFF;
  TEST_ASSERT_EQUAL(200, pwApplyNitrous(200, page10, current));
}

static void test_pwApplyNitrous_Stage1(void) {
  statuses current = {};
  config10 page10 = {};
  setup_nitrous_stage1(page10, current);

  // Confirm nitrous is added
  TEST_ASSERT_EQUAL(200+NITROUS_STAGE1_ADDPW, pwApplyNitrous(200, page10, current));
}

static void test_pwApplyNitrous_Stage2(void) {
  statuses current = {};
  config10 page10 = {};
  setup_nitrous_stage2(page10, current);

  // Confirm nitrous is added
  TEST_ASSERT_EQUAL(200+NITROUS_STAGE2_ADDPW, pwApplyNitrous(200, page10, current));
}

static void test_pwApplyNitrous_StageBoth(void) {
  statuses current = {};
  config10 page10 = {};

  setup_nitrous_stage1(page10, current);
  setup_nitrous_stage2(page10, current);
  current.nitrous_status = NITROUS_BOTH;

  // Confirm nitrous is added
  TEST_ASSERT_UINT16_WITHIN(1U, 200+NITROUS_STAGE1_BOTH+NITROUS_STAGE2_ADDPW, pwApplyNitrous(200, page10, current));
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