#include "../test_utils.h"
#include "scheduler_ignition_controller.h"
#include "../channel_test_helpers.h"
#include "test_context.h"

static void setup_ignition_channel_angles(void)
{
    RUNIF_IGNCHANNEL1( { ignitionSchedules[0].channelDegrees = (CRANK_ANGLE_MAX_IGN/8U) * 0U; }, {});
    RUNIF_IGNCHANNEL2( { ignitionSchedules[1].channelDegrees = (CRANK_ANGLE_MAX_IGN/8U) * 1U; }, {});
    RUNIF_IGNCHANNEL3( { ignitionSchedules[2].channelDegrees = (CRANK_ANGLE_MAX_IGN/8U) * 2U; }, {});
    RUNIF_IGNCHANNEL4( { ignitionSchedules[3].channelDegrees = (CRANK_ANGLE_MAX_IGN/8U) * 3U; }, {});
    RUNIF_IGNCHANNEL5( { ignitionSchedules[4].channelDegrees = (CRANK_ANGLE_MAX_IGN/8U) * 4U; }, {});
    RUNIF_IGNCHANNEL6( { ignitionSchedules[5].channelDegrees = (CRANK_ANGLE_MAX_IGN/8U) * 5U; }, {});
    RUNIF_IGNCHANNEL7( { ignitionSchedules[6].channelDegrees = (CRANK_ANGLE_MAX_IGN/8U) * 6U; }, {});
    RUNIF_IGNCHANNEL8( { ignitionSchedules[7].channelDegrees = (CRANK_ANGLE_MAX_IGN/8U) * 7U; }, {});
}

static void assert_ignition_angles(const test_context_t &context)
{
    // We only need to confirm the calculations were run. There
    // are separate detailed tests for the calculations.
    RUNIF_IGNCHANNEL1( { if (context.current.maxIgnOutputs>=1) { TEST_ASSERT_GREATER_THAN(0U, ignitionSchedules[0].chargeAngle + ignitionSchedules[0].dischargeAngle); }}, {});
    RUNIF_IGNCHANNEL2( { if (context.current.maxIgnOutputs>=2) { TEST_ASSERT_GREATER_THAN(0U, ignitionSchedules[1].chargeAngle + ignitionSchedules[1].dischargeAngle); }}, {});
    RUNIF_IGNCHANNEL3( { if (context.current.maxIgnOutputs>=3) { TEST_ASSERT_GREATER_THAN(0U, ignitionSchedules[2].chargeAngle + ignitionSchedules[2].dischargeAngle); }}, {});
    RUNIF_IGNCHANNEL4( { if (context.current.maxIgnOutputs>=4) { TEST_ASSERT_GREATER_THAN(0U, ignitionSchedules[3].chargeAngle + ignitionSchedules[3].dischargeAngle); }}, {});
    RUNIF_IGNCHANNEL5( { if (context.current.maxIgnOutputs>=5) { TEST_ASSERT_GREATER_THAN(0U, ignitionSchedules[4].chargeAngle + ignitionSchedules[4].dischargeAngle); }}, {});
    RUNIF_IGNCHANNEL6( { if (context.current.maxIgnOutputs>=6) { TEST_ASSERT_GREATER_THAN(0U, ignitionSchedules[5].chargeAngle + ignitionSchedules[5].dischargeAngle); }}, {});
    RUNIF_IGNCHANNEL7( { if (context.current.maxIgnOutputs>=7) { TEST_ASSERT_GREATER_THAN(0U, ignitionSchedules[6].chargeAngle + ignitionSchedules[6].dischargeAngle); }}, {});
    RUNIF_IGNCHANNEL8( { if (context.current.maxIgnOutputs>=8) { TEST_ASSERT_GREATER_THAN(0U, ignitionSchedules[7].chargeAngle + ignitionSchedules[7].dischargeAngle); }}, {});
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

    context.calculateIgnitionAngles();

    TEST_ASSERT_EQUAL_INT16(704, ignitionSchedules[0].dischargeAngle);
    TEST_ASSERT_EQUAL_INT16(77, ignitionSchedules[1].dischargeAngle);
    TEST_ASSERT_EQUAL_INT16(162, ignitionSchedules[2].dischargeAngle);
    TEST_ASSERT_EQUAL_INT16(259, ignitionSchedules[3].dischargeAngle);
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
