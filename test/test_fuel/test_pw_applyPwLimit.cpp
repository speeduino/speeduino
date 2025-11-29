#include <unity.h>
#include "../test_utils.h"
#include "fuel_calcs.h"

extern uint16_t applyPwLimits(uint16_t pw, uint16_t pwLimit, uint16_t injOpenTime, const config10 &page10, const statuses &current);

static void test_inactive_cranking(void) {
    config10 page10 = {};
    statuses current = {};

    current.engineIsCranking = true;
    TEST_ASSERT_EQUAL(1000, applyPwLimits(1000, 500, 100, page10, current));
}

static void test_inactive_staging(void) {
    config10 page10 = {};
    statuses current = {};

    page10.stagingEnabled = true;
    TEST_ASSERT_EQUAL(1000, applyPwLimits(1000, 500, 100, page10, current));
}

static void test_limit_applied(void) {
    config10 page10 = {};
    statuses current = {};

    TEST_ASSERT_EQUAL(500, applyPwLimits(1000, 500, 100, page10, current));
}

static void test_limit_not_applied(void) {
    config10 page10 = {};
    statuses current = {};

    TEST_ASSERT_EQUAL(1000, applyPwLimits(1000, 5000, 100, page10, current));
}

static void test_inj_open_applied(void) {
    config10 page10 = {};
    statuses current = {};

    TEST_ASSERT_EQUAL(0, applyPwLimits(100, 5000, 100, page10, current));
    TEST_ASSERT_EQUAL(101, applyPwLimits(101, 5000, 100, page10, current));
    TEST_ASSERT_EQUAL(0, applyPwLimits(99, 5000, 100, page10, current));
}

void testApplyPwLimit(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_inactive_cranking);
    RUN_TEST_P(test_inactive_staging);
    RUN_TEST_P(test_limit_applied);
    RUN_TEST_P(test_limit_not_applied);
    RUN_TEST_P(test_inj_open_applied);
  }
}