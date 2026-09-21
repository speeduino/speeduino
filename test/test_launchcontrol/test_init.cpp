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

void testInit(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_validPin);
        RUN_TEST_P(test_invalidPin);
    }
}
