#include "../test_utils.h"
#include "scheduler_ignition_controller.h"
#include "../channel_test_helpers.h"
#include "test_context.h"

static void setup_ignition_channel_angles(void)
{
    for (uint8_t i = 0; i < _countof(ignitionSchedules); i++)
    {
        ignitionSchedules[i].channelDegrees = (CRANK_ANGLE_MAX_IGN/8U) * i;
    }
}

static void assert_ignition_angles(const test_context_t &context)
{
    // We only need to confirm the calculations were run. There
    // are separate detailed tests for the calculations.
    for (uint8_t i = 0; i < context.current.maxIgnOutputs; i++)
    {
        TEST_ASSERT_GREATER_THAN(0U, ignitionSchedules[i].chargeAngle + ignitionSchedules[i].dischargeAngle);
    }
}

static void test_calculateIgnitionAngles_nonrotary(void)
{
    test_context_t context;
    CRANK_ANGLE_MAX_IGN = 720U;
    context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
    fakeDecoderStatus.syncStatus = SyncStatus::Full;

    setup_ignition_channel_angles();
    
    for (uint8_t index=0; index<=IGN_CHANNELS; ++index)
    {
        context.current.maxIgnOutputs = index;
        context.calculateIgnitionAngles();
        assert_ignition_angles(context);
    }
}

static void test_calculateIgnitionAngles_sequential_applies_individual_trim(void)
{
    test_context_t context;
    CRANK_ANGLE_MAX_IGN = 720U;
    context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
    context.page2.nCylinders = 4U;
    context.current.maxIgnOutputs = 4U;
    context.current.advance = 15U;
    fakeDecoderStatus.syncStatus = SyncStatus::Full;

    setup_ignition_channel_angles();

    context.page13.ignTrim[0] = 1;
    context.page13.ignTrim[1] = -2;
    context.page13.ignTrim[2] = 3;
    context.page13.ignTrim[3] = -4;
    context.page13.ignTrim[4] = 4;
    context.page13.ignTrim[5] = -7;
    context.page13.ignTrim[6] = 7;
    context.page13.ignTrim[7] = -94;

    context.calculateIgnitionAngles();

    uint16_t expected[] = { 704, 77, 162, 259, 345, 435, 525, 615 };
    for (uint8_t i = 0; i < _countof(ignitionSchedules); i++)
    {
        TEST_ASSERT_EQUAL_INT16(expected[i], ignitionSchedules[i].dischargeAngle);
    }
}

static void test_calculateIgnitionAngles_wasted_ignores_individual_trim(void)
{
    test_context_t context;
    CRANK_ANGLE_MAX_IGN = 360U;
    context.page4.sparkMode = IGN_MODE_WASTED;
    context.page2.nCylinders = 4U;
    context.current.maxIgnOutputs = 2U;
    context.current.advance = 15U;

    setup_ignition_channel_angles();

    context.page13.ignTrim[0] = 5;
    context.page13.ignTrim[1] = -5;

    context.calculateIgnitionAngles();

    TEST_ASSERT_EQUAL_INT16(345, ignitionSchedules[0].dischargeAngle);
    TEST_ASSERT_EQUAL_INT16(30, ignitionSchedules[1].dischargeAngle);
}

static void test_calculateIgnitionAngles_rotary(void)
{
    test_context_t context;
    CRANK_ANGLE_MAX_IGN = 360;
    context.current.maxIgnOutputs = 4U;
    context.page4.sparkMode = IGN_MODE_ROTARY;
    context.page2.nCylinders = 4U;

    setup_ignition_channel_angles();
    context.calculateIgnitionAngles();
    assert_ignition_angles(context);
}

static void test_calculateIgnitionAngles_rotary_non_4_output_uses_non_rotary(void)
{
#if IGN_CHANNELS >= 5
    test_context_t context;
    CRANK_ANGLE_MAX_IGN = 720;
    context.current.maxIgnOutputs = 5U;  // Not 4
    context.page4.sparkMode = IGN_MODE_ROTARY;

    setup_ignition_channel_angles();
    context.calculateIgnitionAngles();

    // Even though sparkMode is ROTARY, if maxIgnOutputs != 4, non-rotary path is used
    TEST_ASSERT_NOT_EQUAL(0U, ignitionSchedules[4].chargeAngle + ignitionSchedules[4].dischargeAngle);
#else
    TEST_IGNORE_MESSAGE("Skipping - not enough ignition channels");
#endif
}

static void test_calculateIgnitionAngles_sync_state_transitions(void)
{
    test_context_t context;
    context.page2.nCylinders = 4U;
    context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
    context.current.maxIgnOutputs = 4U;
    
    // Test transition from no sync to full sync
    fakeDecoderStatus.syncStatus = SyncStatus::Full;
    context.calculateIgnitionAngles();
    TEST_ASSERT_EQUAL_UINT16(720U, CRANK_ANGLE_MAX_IGN);
    
    // Re-setup context for partial sync
    fakeDecoderStatus.syncStatus = SyncStatus::Partial;
    context.current.maxIgnOutputs = 4U; // Reset for next call
    context.calculateIgnitionAngles();
    TEST_ASSERT_EQUAL_UINT16(360U, CRANK_ANGLE_MAX_IGN);
}

void testControllerCalcs(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_calculateIgnitionAngles_nonrotary);
    RUN_TEST_P(test_calculateIgnitionAngles_sequential_applies_individual_trim);
    RUN_TEST_P(test_calculateIgnitionAngles_wasted_ignores_individual_trim);
    RUN_TEST_P(test_calculateIgnitionAngles_rotary);
    RUN_TEST_P(test_calculateIgnitionAngles_rotary_non_4_output_uses_non_rotary);
    RUN_TEST_P(test_calculateIgnitionAngles_sync_state_transitions);
  }
}
