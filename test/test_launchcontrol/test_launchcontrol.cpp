#include "../test_utils.h"
#include "launch_fixture.h"
#include "units.h"

static void test_enablesHardLaunchWhenConditionsAreMet(void)
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

static void test_enablesFlatShiftWhenLaunchIsDisabled(void)
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

static void test_usesInvertedLaunchInput(void)
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

static void test_appliesRollingCutDelta(void)
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

static void assert_hard_launch_rpm_boundary(launch_fixture &fixture, uint16_t thresholdRpm)
{
    fixture.current.setRpm(thresholdRpm-1);
    fixture.current.launchStatus.previousClutchTrigger = false;
    fixture.update();
    TEST_ASSERT_FALSE(fixture.current.launchStatus.launchingHard);

    fixture.current.setRpm(thresholdRpm);
    fixture.current.launchStatus.previousClutchTrigger = false;
    fixture.update();
    TEST_ASSERT_FALSE(fixture.current.launchStatus.launchingHard);

    fixture.current.setRpm(thresholdRpm+1);
    fixture.current.launchStatus.previousClutchTrigger = false;
    fixture.update();
    TEST_ASSERT_TRUE(fixture.current.launchStatus.launchingHard);
}

static void test_hard_launch_rpm_boundary_fullcut(void)
{
    launch_fixture fixture;
    fixture.init();

    fixture.page2.hardCutType = HARD_CUT_FULL;
    fixture.armHardLaunch();

    assert_hard_launch_rpm_boundary(fixture, RPM_COARSE.toUser(fixture.page6.lnchHardLim));
}

static void test_hard_launch_rpm_boundary_rollingcut(void)
{
    launch_fixture fixture;
    fixture.init();
    fixture.page2.hardCutType = HARD_CUT_ROLLING;
    int16_t adjust = SIGNED_RPM_MEDIUM.toUser(fixture.page15.rollingProtRPMDelta[0]);
    fixture.armHardLaunch();

    assert_hard_launch_rpm_boundary(fixture, RPM_COARSE.toUser(fixture.page6.lnchHardLim)+adjust);
}

static void assert_hard_shift_rpm_boundary(launch_fixture &fixture, uint16_t thresholdRpm, int16_t rpmOffset)
{
    fixture.current.setRpm(thresholdRpm+rpmOffset-1);
    fixture.current.launchStatus.clutchEngagedRPM = thresholdRpm;
    fixture.update();
    TEST_ASSERT_FALSE(fixture.current.launchStatus.flatShiftingHard);

    fixture.current.setRpm(thresholdRpm+rpmOffset);
    fixture.current.launchStatus.clutchEngagedRPM = thresholdRpm;
    fixture.update();
    TEST_ASSERT_FALSE(fixture.current.launchStatus.flatShiftingHard);

    fixture.current.setRpm(thresholdRpm+rpmOffset+1);
    fixture.current.launchStatus.clutchEngagedRPM = thresholdRpm;
    fixture.update();
    TEST_ASSERT_TRUE(fixture.current.launchStatus.flatShiftingHard);
}

static void test_flat_shift_rpm_boundary_fullcut(void)
{
    launch_fixture fixture;
    fixture.init();

    fixture.page2.hardCutType = HARD_CUT_FULL;
    fixture.armFlatShift();

    assert_hard_shift_rpm_boundary(fixture, RPM_COARSE.toUser(fixture.page6.flatSArm), 0);
}

static void test_flat_shift_rpm_boundary_rollingcut(void)
{
    launch_fixture fixture;
    fixture.init();
    int16_t adjust = SIGNED_RPM_MEDIUM.toUser(fixture.page15.rollingProtRPMDelta[0]);

    fixture.page2.hardCutType = HARD_CUT_ROLLING;
    fixture.armFlatShift();

    assert_hard_shift_rpm_boundary(fixture, RPM_COARSE.toUser(fixture.page6.flatSArm), adjust);
}

static void test_launch_tps_boundary(void)
{
    launch_fixture fixture;
    fixture.init();
    fixture.armHardLaunch();

    fixture.update();
    TEST_ASSERT_TRUE(fixture.current.launchStatus.launchingHard);
    TEST_ASSERT_TRUE(fixture.current.launchStatus.launchingSoft);

    fixture.current.TPS = fixture.page10.lnchCtrlTPS - 1;
    fixture.update();
    TEST_ASSERT_FALSE(fixture.current.launchStatus.launchingHard);
    TEST_ASSERT_FALSE(fixture.current.launchStatus.launchingSoft);

    fixture.current.TPS = fixture.page10.lnchCtrlTPS;
    fixture.update();
    TEST_ASSERT_TRUE(fixture.current.launchStatus.launchingHard);
    TEST_ASSERT_TRUE(fixture.current.launchStatus.launchingSoft);

    fixture.current.TPS = fixture.page10.lnchCtrlTPS + 1;
    fixture.update();
    TEST_ASSERT_TRUE(fixture.current.launchStatus.launchingHard);
    TEST_ASSERT_TRUE(fixture.current.launchStatus.launchingSoft);
}

static void test_launch_speed_boundary(void)
{
    launch_fixture fixture;
    fixture.init();

    for (auto mode: { VSS_MODE_INTERNAL_PIN, VSS_MODE_EXTERNAL_KM, VSS_MODE_EXTERNAL_MI})
    {
        fixture.page2.vssMode = mode;

        fixture.armHardLaunch();
        fixture.current.vss = fixture.page10.lnchCtrlVss - 1;
        fixture.update();
        TEST_ASSERT_TRUE(fixture.current.launchStatus.launchingHard);
        TEST_ASSERT_TRUE(fixture.current.launchStatus.launchingSoft);

        fixture.armHardLaunch();
        fixture.current.vss = fixture.page10.lnchCtrlVss;
        fixture.update();
        TEST_ASSERT_FALSE(fixture.current.launchStatus.launchingHard);
        TEST_ASSERT_FALSE(fixture.current.launchStatus.launchingSoft);

        fixture.armHardLaunch();
        fixture.current.vss = fixture.page10.lnchCtrlVss+1;
        fixture.update();
        TEST_ASSERT_FALSE(fixture.current.launchStatus.launchingHard);
        TEST_ASSERT_FALSE(fixture.current.launchStatus.launchingSoft);
    }

    fixture.page2.vssMode = VSS_MODE_OFF;
    fixture.armHardLaunch();
    fixture.current.vss = fixture.page10.lnchCtrlVss+1;
    fixture.update();
    TEST_ASSERT_TRUE(fixture.current.launchStatus.launchingHard);
    TEST_ASSERT_TRUE(fixture.current.launchStatus.launchingSoft);
}

static void test_clutch_arming_boundary(void)
{
    launch_fixture fixture;
    fixture.init();
    fixture.armHardLaunch();

    fixture.current.launchStatus.clutchEngagedRPM = RPM_COARSE.toUser(fixture.page6.flatSArm)-1;
    fixture.current.setRpm(fixture.current.launchStatus.clutchEngagedRPM);
    fixture.update();
    TEST_ASSERT_TRUE(fixture.current.launchStatus.launchingHard);
    TEST_ASSERT_TRUE(fixture.current.launchStatus.launchingSoft);

    fixture.current.launchStatus.clutchEngagedRPM = RPM_COARSE.toUser(fixture.page6.flatSArm);
    fixture.current.setRpm(fixture.current.launchStatus.clutchEngagedRPM);
    fixture.update();
    TEST_ASSERT_FALSE(fixture.current.launchStatus.launchingHard);
    TEST_ASSERT_FALSE(fixture.current.launchStatus.launchingSoft);

    fixture.current.launchStatus.clutchEngagedRPM = RPM_COARSE.toUser(fixture.page6.flatSArm)+1;
    fixture.current.setRpm(fixture.current.launchStatus.clutchEngagedRPM);
    fixture.update();
    TEST_ASSERT_FALSE(fixture.current.launchStatus.launchingHard);
    TEST_ASSERT_FALSE(fixture.current.launchStatus.launchingSoft);
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
  
    fixture.setClutch(false);
    fixture.update();
    TEST_ASSERT_FALSE(fixture.current.launchStatus.clutchTrigger);
    TEST_ASSERT_EQUAL_UINT16(3000, fixture.current.launchStatus.clutchEngagedRPM);
  
    fixture.setClutch(true);
    fixture.current.RPM = 6000;
    fixture.update();
    TEST_ASSERT_EQUAL_UINT16(6000, fixture.current.launchStatus.clutchEngagedRPM);
}

static void test_SoftFlatShift_on(void) {
    launch_fixture fixture;
    fixture.init();

    fixture.armSoftFlatShift();
    fixture.update();
    TEST_ASSERT_TRUE(fixture.current.launchStatus.flatShiftingSoft);
}

static void test_SoftFlatShift_off_disabled(void) {
    launch_fixture fixture;
    fixture.init();

    fixture.armSoftFlatShift();
    fixture.page6.flatSEnable = false;
    fixture.update();

    TEST_ASSERT_FALSE(fixture.current.launchStatus.flatShiftingSoft);
}

static void test_SoftFlatShift_off_noclutchtrigger(void) {
    launch_fixture fixture;
    fixture.init();

    fixture.armSoftFlatShift();
    fixture.setClutch(false);
    fixture.update();

    TEST_ASSERT_FALSE(fixture.current.launchStatus.flatShiftingSoft);
}

static void test_SoftFlatShift_off_clutchrpmtoolow(void) {
    launch_fixture fixture;
    fixture.init();

    fixture.armSoftFlatShift();
    fixture.current.launchStatus.clutchEngagedRPM = ((fixture.page6.flatSArm) * 100) - 500;
    fixture.current.setRpm(fixture.current.launchStatus.clutchEngagedRPM);
    fixture.update();

    TEST_ASSERT_FALSE(fixture.current.launchStatus.flatShiftingSoft);
}

static void test_SoftFlatShift_off_rpmnotinwindow(void) {
    launch_fixture fixture;
    fixture.init();

    fixture.armSoftFlatShift();
    fixture.current.setRpm( (fixture.current.launchStatus.clutchEngagedRPM - (fixture.page6.flatSSoftWin * 100) ) - 100);
    fixture.update();

    TEST_ASSERT_FALSE(fixture.current.launchStatus.flatShiftingSoft);
}

void testLaunchControl(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_hard_launch_rpm_boundary_fullcut);
        RUN_TEST_P(test_hard_launch_rpm_boundary_rollingcut);
        RUN_TEST_P(test_flat_shift_rpm_boundary_fullcut);
        RUN_TEST_P(test_flat_shift_rpm_boundary_rollingcut);
        RUN_TEST_P(test_launch_tps_boundary);
        RUN_TEST_P(test_launch_speed_boundary);
        RUN_TEST_P(test_clutch_arming_boundary);
        RUN_TEST_P(test_clutch_rpm_is_captured_on_engagement);
        RUN_TEST_P(test_enablesHardLaunchWhenConditionsAreMet);
        RUN_TEST_P(test_enablesFlatShiftWhenLaunchIsDisabled);
        RUN_TEST_P(test_usesInvertedLaunchInput);
        RUN_TEST_P(test_appliesRollingCutDelta);
        RUN_TEST_P(test_SoftFlatShift_on);
        RUN_TEST_P(test_SoftFlatShift_off_disabled);
        RUN_TEST_P(test_SoftFlatShift_off_noclutchtrigger);
        RUN_TEST_P(test_SoftFlatShift_off_clutchrpmtoolow);
        RUN_TEST_P(test_SoftFlatShift_off_rpmnotinwindow);
    }
}
