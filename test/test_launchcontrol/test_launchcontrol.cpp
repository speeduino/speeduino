#include "../test_utils.h"
#include "src/controllers/launch/launchController.h"
#include "src/pins/inputPin.h"

extern inputPin_t launchPin;

static void initLaunch(uint8_t pin)
{
    pinNumbers_t pins;
    pins.pinLaunch = pin;
    initialiseLaunchControl(pins);
}

static void test_checkLaunchAndFlatShift_enablesHardLaunchWhenConditionsAreMet(void)
{
    statuses current = {};
    config2 page2 = {};
    config6 page6 = {};
    config10 page10 = {};
    config15 page15 = {};

    constexpr uint8_t launchPinNumber = 13;
    initLaunch(launchPinNumber);
    launchPin._pin.setPinHigh();

    current.RPM = 11000;
    current.TPS = 90;
    page2.vssMode = 0;
    page6.launchEnabled = 1;
    page6.flatSEnable = 0;
    page6.launchHiLo = 1;
    page6.lnchHardLim = 90;
    page6.flatSArm = 200;
    page10.lnchCtrlTPS = 0;

    checkLaunchAndFlatShift(current, page2, page6, page10, page15);

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

    constexpr uint8_t launchPinNumber = 13;
    initLaunch(launchPinNumber);
    launchPin._pin.setPinHigh();

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

    checkLaunchAndFlatShift(current, page2, page6, page10, page15);

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

    constexpr uint8_t launchPinNumber = 13;
    initLaunch(launchPinNumber);
    launchPin._pin.setPinLow();

    current.RPM = 9500;
    current.TPS = 50;
    page2.vssMode = 0;
    page6.launchEnabled = 1;
    page6.flatSEnable = 0;
    page6.launchHiLo = 0;
    page6.lnchHardLim = 90;
    page6.flatSArm = 200;
    page10.lnchCtrlTPS = 0;

    checkLaunchAndFlatShift(current, page2, page6, page10, page15);

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

    constexpr uint8_t launchPinNumber = 13;
    initLaunch(launchPinNumber);
    launchPin._pin.setPinHigh();

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

    checkLaunchAndFlatShift(current, page2, page6, page10, page15);

    TEST_ASSERT_TRUE(current.launchingHard);
    TEST_ASSERT_TRUE(current.hardLaunchActive);
}


struct launch_fixture
{
    statuses current = {};
    config2 page2 = {};
    config6 page6 = {};
    config10 page10 = {};
    config15 page15 = {};

    launch_fixture()
    {
        constexpr uint8_t launchPinNumber = 13;
        initLaunch(launchPinNumber);

        setClutch(true);
        current.clutchTrigger = true;
        current.clutchEngagedRPM = 3000;
        current.RPM = 5000;
        current.TPS = 50;
        page6.launchEnabled = 1;
        page6.flatSEnable = 1;
        page6.launchHiLo = 1;
        page6.flatSArm = 40;
        page6.lnchHardLim = 45;
        page10.lnchCtrlTPS = 50;
        page10.lnchCtrlVss = 50;
        page15.rollingProtRPMDelta[0] = -5;
    }

    void setClutch(bool engaged)
    {
        if (engaged) {
            launchPin._pin.setPinHigh();
        } else {
            launchPin._pin.setPinLow();
        }
    }

    void update()
    {
        checkLaunchAndFlatShift(current, page2, page6, page10, page15);
    }

    void assertState(bool launch, bool flatShift)
    {
        TEST_ASSERT_EQUAL(launch, current.launchingHard);
        TEST_ASSERT_EQUAL(launch, current.hardLaunchActive);
        TEST_ASSERT_EQUAL(flatShift, current.flatShiftingHard);
    }
};

static void assert_rpm_boundary(launch_fixture &fixture, uint16_t limit, bool flatShift)
{
    fixture.current.RPM = limit - 1U;
    fixture.update();
    fixture.assertState(false, false);
    fixture.current.RPM = limit;
    fixture.update();
    fixture.assertState(false, false);
    fixture.current.RPM = limit + 1U;
    fixture.update();
    fixture.assertState(!flatShift, flatShift);
    fixture.current.RPM = limit;
    fixture.update();
    fixture.assertState(false, false); // Clear an already active cut at equality
}

static void test_launch_rpm_boundaries(void)
{
    launch_fixture fixture;
    fixture.page2.hardCutType = HARD_CUT_FULL;
    assert_rpm_boundary(fixture, 4500, false); // Full cut ignores the configured delta
    fixture.page2.hardCutType = HARD_CUT_ROLLING;
    assert_rpm_boundary(fixture, 4450, false);
}

static void test_flat_shift_rpm_boundaries(void)
{
    launch_fixture fixture;
    fixture.current.clutchEngagedRPM = 6000;
    fixture.page2.hardCutType = HARD_CUT_FULL;
    assert_rpm_boundary(fixture, 6000, true);
    fixture.page2.hardCutType = HARD_CUT_ROLLING;
    assert_rpm_boundary(fixture, 5950, true);
}

static void test_launch_tps_boundary(void)
{
    launch_fixture fixture;
    fixture.current.TPS = 49;
    fixture.update();
    fixture.assertState(false, false);
    fixture.current.TPS = 50;
    fixture.update();
    fixture.assertState(true, false);
    fixture.current.TPS = 51;
    fixture.update();
    fixture.assertState(true, false);
}

static void test_launch_speed_boundary(void)
{
    launch_fixture fixture;
    for (uint8_t mode = 1; mode <= 3; ++mode)
    {
        fixture.page2.vssMode = mode;
        fixture.current.vss = 49;
        fixture.update();
        fixture.assertState(true, false);
        fixture.current.vss = 50;
        fixture.update();
        fixture.assertState(false, false);
        fixture.current.vss = 51;
        fixture.update();
        fixture.assertState(false, false);
    }
    fixture.page2.vssMode = 0;
    fixture.update();
    fixture.assertState(true, false); // Ignore vehicle speed when VSS is disabled
}

static void test_clutch_arming_boundary(void)
{
    launch_fixture fixture;
    fixture.current.clutchEngagedRPM = 3999;
    fixture.update();
    fixture.assertState(true, false);
    fixture.current.clutchEngagedRPM = 4000;
    fixture.update();
    fixture.assertState(false, true); // Equality belongs to flat shift
    fixture.current.clutchEngagedRPM = 4001;
    fixture.update();
    fixture.assertState(false, true);
}

static void test_clutch_rpm_is_captured_on_engagement(void)
{
    launch_fixture fixture;
    fixture.current.clutchTrigger = false;
    fixture.current.RPM = 3000;
    fixture.update();
    TEST_ASSERT_FALSE(fixture.current.previousClutchTrigger);
    TEST_ASSERT_TRUE(fixture.current.clutchTriggerActive);
    TEST_ASSERT_EQUAL_UINT16(3000, fixture.current.clutchEngagedRPM);

    fixture.current.RPM = 5000;
    fixture.update();
    TEST_ASSERT_TRUE(fixture.current.previousClutchTrigger);
    TEST_ASSERT_EQUAL_UINT16(3000, fixture.current.clutchEngagedRPM);
    fixture.assertState(true, false);

    fixture.setClutch(false);
    fixture.update();
    TEST_ASSERT_FALSE(fixture.current.clutchTriggerActive);
    TEST_ASSERT_EQUAL_UINT16(3000, fixture.current.clutchEngagedRPM);
    fixture.assertState(false, false);

    fixture.setClutch(true);
    fixture.current.RPM = 6000;
    fixture.update();
    TEST_ASSERT_EQUAL_UINT16(6000, fixture.current.clutchEngagedRPM);
    fixture.assertState(false, false); // No full cut at the newly captured RPM
    fixture.current.RPM = 6001;
    fixture.update();
    fixture.assertState(false, true);
}

void testLaunchControl(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_launch_rpm_boundaries);
        RUN_TEST_P(test_flat_shift_rpm_boundaries);
        RUN_TEST_P(test_launch_tps_boundary);
        RUN_TEST_P(test_launch_speed_boundary);
        RUN_TEST_P(test_clutch_arming_boundary);
        RUN_TEST_P(test_clutch_rpm_is_captured_on_engagement);
        RUN_TEST_P(test_checkLaunchAndFlatShift_enablesHardLaunchWhenConditionsAreMet);
        RUN_TEST_P(test_checkLaunchAndFlatShift_enablesFlatShiftWhenLaunchIsDisabled);
        RUN_TEST_P(test_checkLaunchAndFlatShift_usesInvertedLaunchInput);
        RUN_TEST_P(test_checkLaunchAndFlatShift_appliesRollingCutDelta);
    }
}
