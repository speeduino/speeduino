#include "../test_utils.h"
#include "launch_fixture.h"

static void test_validPin(void)
{
    launch_fixture fixture;

    fixture.page6.flatSEnable = true;
    fixture.page6.launchEnabled = true;
    fixture.init();

    TEST_ASSERT_TRUE(fixture.page6.flatSEnable);
    TEST_ASSERT_TRUE(fixture.page6.launchEnabled);

    fixture.page6.flatSEnable = false;
    fixture.page6.launchEnabled = false;
    fixture.init();

    TEST_ASSERT_FALSE(fixture.page6.flatSEnable);
    TEST_ASSERT_FALSE(fixture.page6.launchEnabled);
}

static void test_invalidPin(void)
{
    launch_fixture fixture;

    fixture.page6.flatSEnable = true;
    fixture.page6.launchEnabled = true;
    fixture.pins.pinLaunch = NOT_A_PIN;
    fixture.init();

    TEST_ASSERT_FALSE(fixture.page6.flatSEnable);
    TEST_ASSERT_FALSE(fixture.page6.launchEnabled);

    fixture.page6.flatSEnable = false;
    fixture.page6.launchEnabled = false;
    fixture.pins.pinLaunch = NOT_A_PIN;
    fixture.init();

    TEST_ASSERT_FALSE(fixture.page6.flatSEnable);
    TEST_ASSERT_FALSE(fixture.page6.launchEnabled);
}

static void test_initKnownStatus(void)
{
    launch_fixture fixture;

    fixture.current.launchStatus.clutchEngagedRPM = 999;
    fixture.current.launchStatus.clutchTrigger = true;
    fixture.current.launchStatus.flatShiftingHard = true;
    fixture.current.launchStatus.flatShiftingSoft = true;
    fixture.current.launchStatus.launchingHard = true;
    fixture.current.launchStatus.launchingSoft = true;
    fixture.current.launchStatus.previousClutchTrigger = true;
    fixture.init();

    TEST_ASSERT_EQUAL(0, fixture.current.launchStatus.clutchEngagedRPM);
    TEST_ASSERT_FALSE(fixture.current.launchStatus.clutchTrigger);
    TEST_ASSERT_FALSE(fixture.current.launchStatus.flatShiftingHard);
    TEST_ASSERT_FALSE(fixture.current.launchStatus.flatShiftingSoft);
    TEST_ASSERT_FALSE(fixture.current.launchStatus.launchingHard);
    TEST_ASSERT_FALSE(fixture.current.launchStatus.launchingSoft);
    TEST_ASSERT_FALSE(fixture.current.launchStatus.previousClutchTrigger);
}

void testInit(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_validPin);
        RUN_TEST_P(test_invalidPin);
        RUN_TEST_P(test_initKnownStatus);
    }
}
