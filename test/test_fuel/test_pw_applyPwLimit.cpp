#include <unity.h>
#include "../test_utils.h"
#include "fuel_calcs.h"

extern uint16_t applyPwLimits(uint16_t pw, uint16_t pwLimit, const config10 &page10, const statuses &current);

static void test_inactive_cranking(void) {
    config10 page10 = {};
    statuses current = {};

    current.engineIsCranking = true;
    TEST_ASSERT_EQUAL(1000, applyPwLimits(1000, 500, page10, current));
}

static void test_inactive_staging(void) {
    config10 page10 = {};
    statuses current = {};

    page10.stagingEnabled = true;
    TEST_ASSERT_EQUAL(1000, applyPwLimits(1000, 500, page10, current));
}

static void test_limit_applied(void) {
    config10 page10 = {};
    statuses current = {};

    TEST_ASSERT_EQUAL(500, applyPwLimits(1000, 500, page10, current));
}

static void test_limit_not_applied(void) {
    config10 page10 = {};
    statuses current = {};

    TEST_ASSERT_EQUAL(1000, applyPwLimits(1000, 5000, page10, current));
}

void testApplyPwLimit(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_inactive_cranking);
    RUN_TEST_P(test_inactive_staging);
    RUN_TEST_P(test_limit_applied);
    RUN_TEST_P(test_limit_not_applied);
  }
}