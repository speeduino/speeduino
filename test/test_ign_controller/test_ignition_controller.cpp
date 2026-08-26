#include "../test_utils.h"
#include "scheduler_ignition_controller.h"
#include "units.h"
#include "../channel_test_helpers.h"

extern uint8_t validateSparkMode(uint8_t mode, const config2 &page2);
extern void validateIgnitionSetup(config2 &page2, config4 &page4, config13 &page13);

static void test_validateSparkMode(void)
{
    config2 page2 = {};
    page2.nCylinders = 1;

    TEST_ASSERT_EQUAL(IGN_MODE_WASTED, validateSparkMode(IGN_MODE_WASTED, page2));
    TEST_ASSERT_EQUAL(IGN_MODE_SINGLE, validateSparkMode(IGN_MODE_SINGLE, page2));
    TEST_ASSERT_EQUAL(IGN_MODE_WASTEDCOP, validateSparkMode(IGN_MODE_WASTEDCOP, page2));
    TEST_ASSERT_EQUAL(IGN_MODE_SEQUENTIAL, validateSparkMode(IGN_MODE_SEQUENTIAL, page2));
    TEST_ASSERT_EQUAL(IGN_MODE_ROTARY, validateSparkMode(IGN_MODE_ROTARY, page2));

    page2.nCylinders = IGN_CHANNELS+1;
    TEST_ASSERT_EQUAL(IGN_MODE_WASTED, validateSparkMode(IGN_MODE_SEQUENTIAL, page2));
}

static void test_validateIgnitionSetup_oddfire(void)
{
    config2 page2 = {};
    config4 page4 = {};
    config13 page13 = {};

    page2.engineType = ODD_FIRE;
    page2.nCylinders = 11;

    validateIgnitionSetup(page2, page4, page13);

    TEST_ASSERT_EQUAL(EVEN_FIRE, page2.engineType);
}

static void test_validateIgnitionSetup_trims(void)
{
    config2 page2 = {};
    config4 page4 = {};
    config13 page13 = {};

    page4.sparkMode = IGN_MODE_SEQUENTIAL;
    memset(page13.ignTrim, -7, sizeof(page13.ignTrim));
    validateIgnitionSetup(page2, page4, page13);
    TEST_ASSERT_EACH_EQUAL_INT8 (-7, page13.ignTrim, _countof(page13.ignTrim));

    page4.sparkMode = IGN_MODE_SINGLE;
    validateIgnitionSetup(page2, page4, page13);
    TEST_ASSERT_EACH_EQUAL_INT8 (0, page13.ignTrim, _countof(page13.ignTrim));
}

void testIgnitionController(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_validateSparkMode);
    RUN_TEST_P(test_validateIgnitionSetup_oddfire);
    RUN_TEST_P(test_validateIgnitionSetup_trims);
  }
}