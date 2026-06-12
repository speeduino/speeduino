#include "../test_utils.h"
#include "src/controllers/launch/launchController.h"

static void test_checkLaunchAndFlatShift_enablesHardLaunchWhenConditionsAreMet(void)
{
    statuses current = {};
    config2 page2 = {};
    config6 page6 = {};
    config10 page10 = {};
    config15 page15 = {};

    constexpr uint8_t launchPin = 13;
    pinMode(launchPin, INPUT);
    digitalWrite(launchPin, HIGH);

    current.RPM = 11000;
    current.TPS = 90;
    page2.vssMode = 0;
    page6.launchEnabled = 1;
    page6.flatSEnable = 0;
    page6.launchHiLo = 1;
    page6.lnchHardLim = 90;
    page6.flatSArm = 200;
    page10.lnchCtrlTPS = 0;

    checkLaunchAndFlatShift(current, launchPin, page2, page6, page10, page15);

    TEST_ASSERT_TRUE(current.clutchTrigger);
    TEST_ASSERT_TRUE(current.clutchTriggerActive);
    TEST_ASSERT_EQUAL_UINT16(current.RPM, current.clutchEngagedRPM);
    TEST_ASSERT_TRUE(current.launchingHard);
    TEST_ASSERT_TRUE(current.hardLaunchActive);
    TEST_ASSERT_FALSE(current.flatShiftingHard);
}

static void test_checkLaunchAndFlatShift_enablesFlatShiftWhenLaunchIsDisabled(void)
{
    statuses current = {};
    config2 page2 = {};
    config6 page6 = {};
    config10 page10 = {};
    config15 page15 = {};

    constexpr uint8_t launchPin = 13;
    pinMode(launchPin, INPUT);
    digitalWrite(launchPin, HIGH);

    current.clutchTrigger = true;
    current.previousClutchTrigger = true;
    current.RPM = 11000;
    current.TPS = 50;
    current.clutchEngagedRPM = 10000;

    page2.vssMode = 0;
    page6.launchEnabled = 0;
    page6.flatSEnable = 1;
    page6.launchHiLo = 1;
    page6.flatSArm = 100;
    page10.lnchCtrlTPS = 0;

    checkLaunchAndFlatShift(current, launchPin, page2, page6, page10, page15);

    TEST_ASSERT_TRUE(current.clutchTrigger);
    TEST_ASSERT_TRUE(current.clutchTriggerActive);
    TEST_ASSERT_TRUE(current.flatShiftingHard);
    TEST_ASSERT_FALSE(current.launchingHard);
    TEST_ASSERT_FALSE(current.hardLaunchActive);
}

static void test_checkLaunchAndFlatShift_usesInvertedLaunchInput(void)
{
    statuses current = {};
    config2 page2 = {};
    config6 page6 = {};
    config10 page10 = {};
    config15 page15 = {};

    constexpr uint8_t launchPin = 13;
    pinMode(launchPin, OUTPUT);
    digitalWrite(launchPin, LOW);
    pinMode(launchPin, INPUT);

    current.RPM = 9500;
    current.TPS = 50;
    page2.vssMode = 0;
    page6.launchEnabled = 1;
    page6.flatSEnable = 0;
    page6.launchHiLo = 0;
    page6.lnchHardLim = 90;
    page6.flatSArm = 200;
    page10.lnchCtrlTPS = 0;

    checkLaunchAndFlatShift(current, launchPin, page2, page6, page10, page15);

    TEST_ASSERT_TRUE(current.clutchTrigger);
    TEST_ASSERT_TRUE(current.clutchTriggerActive);
    TEST_ASSERT_TRUE(current.launchingHard);
    TEST_ASSERT_TRUE(current.hardLaunchActive);
}

static void test_checkLaunchAndFlatShift_appliesRollingCutDelta(void)
{
    statuses current = {};
    config2 page2 = {};
    config6 page6 = {};
    config10 page10 = {};
    config15 page15 = {};

    constexpr uint8_t launchPin = 13;
    pinMode(launchPin, INPUT);
    digitalWrite(launchPin, HIGH);

    current.RPM = 9000;
    current.TPS = 50;
    page2.vssMode = 0;
    page2.hardCutType = HARD_CUT_ROLLING;
    page6.launchEnabled = 1;
    page6.flatSEnable = 0;
    page6.launchHiLo = 1;
    page6.lnchHardLim = 90;
    page6.flatSArm = 200;
    page10.lnchCtrlTPS = 0;
    page15.rollingProtRPMDelta[0] = -5;

    checkLaunchAndFlatShift(current, launchPin, page2, page6, page10, page15);

    TEST_ASSERT_TRUE(current.launchingHard);
    TEST_ASSERT_TRUE(current.hardLaunchActive);
}

void testLaunchControl(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_checkLaunchAndFlatShift_enablesHardLaunchWhenConditionsAreMet);
        RUN_TEST_P(test_checkLaunchAndFlatShift_enablesFlatShiftWhenLaunchIsDisabled);
        RUN_TEST_P(test_checkLaunchAndFlatShift_usesInvertedLaunchInput);
        RUN_TEST_P(test_checkLaunchAndFlatShift_appliesRollingCutDelta);
    }
}
