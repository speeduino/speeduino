#include "../test_utils.h"
#include "launch_fixture.h"
#include "units.h"

static void test_checkLaunchAndFlatShift_enablesHardLaunchWhenConditionsAreMet(void)
{
    launch_fixture fixture;
    fixture.init();
    fixture.setClutch(true);

    fixture.current.launchStatus.previousClutchTrigger = false;
    fixture.current.launchStatus.clutchTrigger = false;
    fixture.current.RPM = 11000;
    fixture.current.TPS = 90;
    fixture.page2.vssMode = 0;
    fixture.page6.launchEnabled = 1;
    fixture.page6.flatSEnable = 0;
    fixture.page6.launchHiLo = 1;
    fixture.page6.lnchHardLim = 90;
    fixture.page6.flatSArm = 200;
    fixture.page10.lnchCtrlTPS = 0;

    fixture.update();

    TEST_ASSERT_TRUE(fixture.current.launchStatus.clutchTrigger);
    TEST_ASSERT_EQUAL_UINT16(fixture.current.RPM, fixture.current.launchStatus.clutchEngagedRPM);
    TEST_ASSERT_TRUE(fixture.current.launchStatus.launchingHard);
    TEST_ASSERT_FALSE(fixture.current.launchStatus.flatShiftingHard);
}

static void test_checkLaunchAndFlatShift_enablesFlatShiftWhenLaunchIsDisabled(void)
{
    launch_fixture fixture;
    fixture.init();
    fixture.setClutch(true);

    fixture.current.launchStatus.clutchTrigger = true;
    fixture.current.launchStatus.previousClutchTrigger = true;
    fixture.current.RPM = 11000;
    fixture.current.TPS = 50;
    fixture.current.launchStatus.clutchEngagedRPM = 10000;

    fixture.page2.vssMode = 0;
    fixture.page6.launchEnabled = 0;
    fixture.page6.flatSEnable = 1;
    fixture.page6.launchHiLo = 1;
    fixture.page6.flatSArm = 100;
    fixture.page10.lnchCtrlTPS = 0;

    fixture.update();

    TEST_ASSERT_TRUE(fixture.current.launchStatus.clutchTrigger);
    TEST_ASSERT_TRUE(fixture.current.launchStatus.flatShiftingHard);
    TEST_ASSERT_FALSE(fixture.current.launchStatus.launchingHard);
}

static void test_checkLaunchAndFlatShift_usesInvertedLaunchInput(void)
{
    launch_fixture fixture;
    fixture.init();
    fixture.setClutch(false);

    fixture.current.RPM = 9500;
    fixture.current.TPS = 50;
    fixture.page2.vssMode = 0;
    fixture.page6.launchEnabled = 1;
    fixture.page6.flatSEnable = 0;
    fixture.page6.launchHiLo = 0;
    fixture.page6.lnchHardLim = 90;
    fixture.page6.flatSArm = 200;
    fixture.page10.lnchCtrlTPS = 0;

    fixture.update();

    TEST_ASSERT_TRUE(fixture.current.launchStatus.clutchTrigger);
    TEST_ASSERT_TRUE(fixture.current.launchStatus.launchingHard);
}

static void test_checkLaunchAndFlatShift_appliesRollingCutDelta(void)
{
    launch_fixture fixture;
    fixture.init();
    fixture.setClutch(true);

    fixture.current.RPM = 9000;
    fixture.current.TPS = 50;
    fixture.page2.vssMode = 0;
    fixture.page2.hardCutType = HARD_CUT_ROLLING;
    fixture.page6.launchEnabled = 1;
    fixture.page6.flatSEnable = 0;
    fixture.page6.launchHiLo = 1;
    fixture.page6.lnchHardLim = 90;
    fixture.page6.flatSArm = 200;
    fixture.page10.lnchCtrlTPS = 0;
    fixture.page15.rollingProtRPMDelta[0] = -5;

    fixture.update();

    TEST_ASSERT_TRUE(fixture.current.launchStatus.launchingHard);
}

static void assert_rpm_boundary(launch_fixture &fixture, uint16_t limit, bool flatShift)
{
    fixture.current.RPM = limit - 1U;
    fixture.update();
    fixture.assertState(false, false, false);

    fixture.current.RPM = limit;
    fixture.update();
    fixture.assertState(false, false, false);
    
    fixture.current.RPM = limit + 1U;
    fixture.update();
    fixture.assertState(!flatShift, !flatShift && fixture.current.RPM > RPM_COARSE.toUser(fixture.page6.lnchSoftLim), flatShift);
    
    fixture.current.RPM = limit;
    fixture.update();
    fixture.assertState(false, false, false); // Clear an already active cut at equality
}

static void test_launch_rpm_boundaries(void)
{
    launch_fixture fixture;
    fixture.init();
    fixture.setClutch(true);

    fixture.page2.hardCutType = HARD_CUT_FULL;
    assert_rpm_boundary(fixture, 4500, false); // Full cut ignores the configured delta
    fixture.page2.hardCutType = HARD_CUT_ROLLING;
    assert_rpm_boundary(fixture, 4450, false);
}

static void test_flat_shift_rpm_boundaries(void)
{
    launch_fixture fixture;
    fixture.init();
    fixture.setClutch(true);

    fixture.current.launchStatus.clutchEngagedRPM = 6000;
    fixture.page2.hardCutType = HARD_CUT_FULL;
    assert_rpm_boundary(fixture, 6000, true);
    fixture.page2.hardCutType = HARD_CUT_ROLLING;
    assert_rpm_boundary(fixture, 5950, true);
}

static void test_launch_tps_boundary(void)
{
    launch_fixture fixture;
    fixture.init();
    fixture.setClutch(true);

    fixture.current.TPS = 49;
    fixture.update();
    fixture.assertState(false, false, false);

    fixture.current.TPS = 50;
    fixture.update();
    fixture.assertState(true, true, false);

    fixture.current.TPS = 51;
    fixture.update();
    fixture.assertState(true, true, false);
}

static void test_launch_speed_boundary(void)
{
    launch_fixture fixture;
    fixture.init();
    fixture.setClutch(true);

    for (uint8_t mode = 1; mode <= 3; ++mode)
    {
        fixture.page2.vssMode = mode;
        fixture.current.vss = 49;
        fixture.update();
        fixture.assertState(true, true, false);

        fixture.current.vss = 50;
        fixture.update();
        fixture.assertState(false, false, false);

        fixture.current.vss = 51;
        fixture.update();
        fixture.assertState(false, false, false);
    }
    fixture.page2.vssMode = 0;
    fixture.update();
    fixture.assertState(true, true, false); // Ignore vehicle speed when VSS is disabled
}

static void test_clutch_arming_boundary(void)
{
    launch_fixture fixture;
    fixture.init();
    fixture.setClutch(true);

    fixture.current.launchStatus.clutchEngagedRPM = 3999;
    fixture.update();
    fixture.assertState(true, true, false);

    fixture.current.launchStatus.clutchEngagedRPM = 4000;
    fixture.update();
    fixture.assertState(false, false, true); // Equality belongs to flat shift

    fixture.current.launchStatus.clutchEngagedRPM = 4001;
    fixture.update();
    fixture.assertState(false, false, true);
}

static void test_clutch_rpm_is_captured_on_engagement(void)
{
    launch_fixture fixture;
    fixture.init();
    fixture.setClutch(true);

    fixture.current.launchStatus.clutchTrigger = false;
    fixture.current.RPM = 3000;
    fixture.update();
    TEST_ASSERT_FALSE(fixture.current.launchStatus.previousClutchTrigger);
    TEST_ASSERT_TRUE(fixture.current.launchStatus.clutchTrigger);
    TEST_ASSERT_EQUAL_UINT16(3000, fixture.current.launchStatus.clutchEngagedRPM);

    fixture.current.RPM = 5000;
    fixture.update();
    TEST_ASSERT_TRUE(fixture.current.launchStatus.previousClutchTrigger);
    TEST_ASSERT_EQUAL_UINT16(3000, fixture.current.launchStatus.clutchEngagedRPM);
    fixture.assertState(true, true, false);

    fixture.setClutch(false);
    fixture.update();
    TEST_ASSERT_FALSE(fixture.current.launchStatus.clutchTrigger);
    TEST_ASSERT_EQUAL_UINT16(3000, fixture.current.launchStatus.clutchEngagedRPM);
    fixture.assertState(false, false, false);

    fixture.setClutch(true);
    fixture.current.RPM = 6000;
    fixture.update();
    TEST_ASSERT_EQUAL_UINT16(6000, fixture.current.launchStatus.clutchEngagedRPM);
    fixture.assertState(false, false, false); // No full cut at the newly captured RPM
    fixture.current.RPM = 6001;
    fixture.update();
    fixture.assertState(false, false, true);
}

static launch_fixture setup_SoftFlatShift(void) {
    launch_fixture fixture;
    fixture.page6.flatSEnable = 1;
    fixture.page6.flatSArm = 10;
    fixture.page6.flatSSoftWin = 10;
    
    fixture.current.launchStatus.clutchTrigger = 1;
    fixture.current.launchStatus.clutchEngagedRPM = ((fixture.page6.flatSArm) * 100) + 500;
    fixture.current.setRpm(fixture.current.launchStatus.clutchEngagedRPM + 600);

    return fixture;
}


static void test_correctionSoftFlatShift_on(void) {
    auto fixture = setup_SoftFlatShift();
    fixture.init();
    fixture.setClutch(true);
    fixture.update();
    TEST_ASSERT_TRUE(fixture.current.launchStatus.flatShiftingSoft);
}

static void test_correctionSoftFlatShift_off_disabled(void) {
    auto fixture = setup_SoftFlatShift();
    fixture.page6.flatSEnable = false;
    fixture.init();
    fixture.update();

    TEST_ASSERT_FALSE(fixture.current.launchStatus.flatShiftingSoft);
}

static void test_correctionSoftFlatShift_off_noclutchtrigger(void) {
    auto fixture = setup_SoftFlatShift();
    fixture.init();
    fixture.setClutch(false);
    fixture.update();

    TEST_ASSERT_FALSE(fixture.current.launchStatus.flatShiftingSoft);
}

static void test_correctionSoftFlatShift_off_clutchrpmtoolow(void) {
    auto fixture = setup_SoftFlatShift();
    fixture.current.launchStatus.clutchEngagedRPM = ((fixture.page6.flatSArm) * 100) - 500;
    fixture.init();
    fixture.setClutch(true);
    fixture.update();

    TEST_ASSERT_FALSE(fixture.current.launchStatus.flatShiftingSoft);
}

static void test_correctionSoftFlatShift_off_rpmnotinwindow(void) {
    auto fixture = setup_SoftFlatShift();
    fixture.init();
    fixture.current.setRpm( (fixture.current.launchStatus.clutchEngagedRPM - (fixture.page6.flatSSoftWin * 100) ) - 100);
    fixture.setClutch(true);
    fixture.update();

    TEST_ASSERT_FALSE(fixture.current.launchStatus.flatShiftingSoft);
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
        RUN_TEST_P(test_correctionSoftFlatShift_on);
        RUN_TEST_P(test_correctionSoftFlatShift_off_disabled);
        RUN_TEST_P(test_correctionSoftFlatShift_off_noclutchtrigger);
        RUN_TEST_P(test_correctionSoftFlatShift_off_clutchrpmtoolow);
        RUN_TEST_P(test_correctionSoftFlatShift_off_rpmnotinwindow);
    }
}
