#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "channel_test_helpers.h"

extern void SetRevolutionTime(uint32_t revTime);

struct ignition_test_context_t
{
    config2 page2 = {};
    config4 page4 = {};
    decoder_status_t decoderStatus = {};
    statuses current = {};

    ignition_test_context_t() {
        // Set basic engine config defaults
        page2.nCylinders = 4U;
        page2.strokes = FOUR_STROKE;
        page4.sparkMode = IGN_MODE_WASTED;
        decoderStatus.syncStatus = SyncStatus::Full;
        current.maxIgnOutputs = 4U;
        current.dwell = 3000U; // 3ms dwell
        current.advance = 15U;  // 15 degrees advance
        SetRevolutionTime(UDIV_ROUND_CLOSEST(60UL*1000000UL, 4000, uint32_t));
    }

    void calculateIgnitionAngles(void)
    {
        ::calculateIgnitionAngles(page2, page4, decoderStatus, current);        
    }
};

static void setup_ignition_channel_angles(void)
{
    RUNIF_IGNCHANNEL1( { ignitionSchedule1.channelDegrees = (CRANK_ANGLE_MAX_IGN/8U) * 0U; }, {});
    RUNIF_IGNCHANNEL2( { ignitionSchedule2.channelDegrees = (CRANK_ANGLE_MAX_IGN/8U) * 1U; }, {});
    RUNIF_IGNCHANNEL3( { ignitionSchedule3.channelDegrees = (CRANK_ANGLE_MAX_IGN/8U) * 2U; }, {});
    RUNIF_IGNCHANNEL4( { ignitionSchedule4.channelDegrees = (CRANK_ANGLE_MAX_IGN/8U) * 3U; }, {});
    RUNIF_IGNCHANNEL5( { ignitionSchedule5.channelDegrees = (CRANK_ANGLE_MAX_IGN/8U) * 4U; }, {});
    RUNIF_IGNCHANNEL6( { ignitionSchedule6.channelDegrees = (CRANK_ANGLE_MAX_IGN/8U) * 5U; }, {});
    RUNIF_IGNCHANNEL7( { ignitionSchedule7.channelDegrees = (CRANK_ANGLE_MAX_IGN/8U) * 6U; }, {});
    RUNIF_IGNCHANNEL8( { ignitionSchedule8.channelDegrees = (CRANK_ANGLE_MAX_IGN/8U) * 7U; }, {});
}

static void assert_ignition_angles(const ignition_test_context_t &context)
{
    // We only need to confirm the calculations were run. There
    // are separate detailed tests for the calculations.
    RUNIF_IGNCHANNEL1( { if (context.current.maxIgnOutputs>=1) { TEST_ASSERT_GREATER_THAN(0U, ignitionSchedule1.chargeAngle + ignitionSchedule1.dischargeAngle); }}, {});
    RUNIF_IGNCHANNEL2( { if (context.current.maxIgnOutputs>=2) { TEST_ASSERT_GREATER_THAN(0U, ignitionSchedule2.chargeAngle + ignitionSchedule2.dischargeAngle); }}, {});
    RUNIF_IGNCHANNEL3( { if (context.current.maxIgnOutputs>=3) { TEST_ASSERT_GREATER_THAN(0U, ignitionSchedule3.chargeAngle + ignitionSchedule3.dischargeAngle); }}, {});
    RUNIF_IGNCHANNEL4( { if (context.current.maxIgnOutputs>=4) { TEST_ASSERT_GREATER_THAN(0U, ignitionSchedule4.chargeAngle + ignitionSchedule4.dischargeAngle); }}, {});
    RUNIF_IGNCHANNEL5( { if (context.current.maxIgnOutputs>=5) { TEST_ASSERT_GREATER_THAN(0U, ignitionSchedule5.chargeAngle + ignitionSchedule5.dischargeAngle); }}, {});
    RUNIF_IGNCHANNEL6( { if (context.current.maxIgnOutputs>=6) { TEST_ASSERT_GREATER_THAN(0U, ignitionSchedule6.chargeAngle + ignitionSchedule6.dischargeAngle); }}, {});
    RUNIF_IGNCHANNEL7( { if (context.current.maxIgnOutputs>=7) { TEST_ASSERT_GREATER_THAN(0U, ignitionSchedule7.chargeAngle + ignitionSchedule7.dischargeAngle); }}, {});
    RUNIF_IGNCHANNEL8( { if (context.current.maxIgnOutputs>=8) { TEST_ASSERT_GREATER_THAN(0U, ignitionSchedule8.chargeAngle + ignitionSchedule8.dischargeAngle); }}, {});
}

static void test_calculateIgnitionAngles_nonrotary(void)
{
    ignition_test_context_t context;
    CRANK_ANGLE_MAX_IGN = 720U;
    context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
    context.decoderStatus.syncStatus = SyncStatus::Full;
    setup_ignition_channel_angles();
    
    for (uint8_t index=0; index<=IGN_CHANNELS; ++index)
    {
        context.current.maxIgnOutputs = index;
        context.calculateIgnitionAngles();
        assert_ignition_angles(context);
    }
}

static void test_calculateIgnitionAngles_rotary(void)
{
    ignition_test_context_t context;
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
    ignition_test_context_t context;
    CRANK_ANGLE_MAX_IGN = 720;
    context.current.maxIgnOutputs = 5U;  // Not 4
    context.page4.sparkMode = IGN_MODE_ROTARY;

    setup_ignition_channel_angles();
    context.calculateIgnitionAngles();

    // Even though sparkMode is ROTARY, if maxIgnOutputs != 4, non-rotary path is used
    TEST_ASSERT_NOT_EQUAL(0U, ignitionSchedule5.chargeAngle + ignitionSchedule5.dischargeAngle);
}

static void test_calculateIgnitionAngles_sync_state_transitions(void)
{
    ignition_test_context_t context;
    context.page2.nCylinders = 4U;
    context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
    context.current.maxIgnOutputs = 4U;
    
    // Test transition from no sync to full sync
    context.decoderStatus.syncStatus = SyncStatus::Full;
    context.calculateIgnitionAngles();
    TEST_ASSERT_EQUAL_UINT16(720U, CRANK_ANGLE_MAX_IGN);
    
    // Re-setup context for partial sync
    context.decoderStatus.syncStatus = SyncStatus::Partial;
    context.current.maxIgnOutputs = 4U; // Reset for next call
    context.calculateIgnitionAngles();
    TEST_ASSERT_EQUAL_UINT16(360U, CRANK_ANGLE_MAX_IGN);
}

static void test_setIgnitionChannels_mask_enables_and_disables_channels(void)
{
    ignition_test_context_t context;
    context.current.maxIgnOutputs = IGN_CHANNELS;
    context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
    CRANK_ANGLE_MAX_IGN = 720U;
    setup_ignition_channel_angles();
    context.calculateIgnitionAngles();

    for (uint8_t index=0; index<=IGN_CHANNELS; ++index)
    {
        context.current.maxIgnOutputs = index;
        // Enable channels 1, 3, 5 & 7
        setIgnitionChannels(context.current, 0U, context.current.dwell, 0b01010101);

        // Enabled channels should be pending, disabled should remain OFF
        RUNIF_IGNCHANNEL1( { if (context.current.maxIgnOutputs>=1) { TEST_ASSERT_EQUAL_UINT8(PENDING, (uint8_t)ignitionSchedule1.Status); } }, {});
        RUNIF_IGNCHANNEL2( { if (context.current.maxIgnOutputs>=2) { TEST_ASSERT_EQUAL_UINT8(OFF, (uint8_t)ignitionSchedule2.Status); } }, {});
        RUNIF_IGNCHANNEL3( { if (context.current.maxIgnOutputs>=3) { TEST_ASSERT_EQUAL_UINT8(PENDING, (uint8_t)ignitionSchedule3.Status); } }, {});
        RUNIF_IGNCHANNEL4( { if (context.current.maxIgnOutputs>=4) { TEST_ASSERT_EQUAL_UINT8(OFF, (uint8_t)ignitionSchedule4.Status); } }, {});
        RUNIF_IGNCHANNEL5( { if (context.current.maxIgnOutputs>=5) { TEST_ASSERT_EQUAL_UINT8(PENDING, (uint8_t)ignitionSchedule5.Status); } }, {});
        RUNIF_IGNCHANNEL6( { if (context.current.maxIgnOutputs>=6) { TEST_ASSERT_EQUAL_UINT8(OFF, (uint8_t)ignitionSchedule6.Status); } }, {});
        RUNIF_IGNCHANNEL7( { if (context.current.maxIgnOutputs>=7) { TEST_ASSERT_EQUAL_UINT8(PENDING, (uint8_t)ignitionSchedule7.Status); } }, {});
        RUNIF_IGNCHANNEL8( { if (context.current.maxIgnOutputs>=8) { TEST_ASSERT_EQUAL_UINT8(OFF, (uint8_t)ignitionSchedule8.Status); } }, {});
    }
}

void test_ignition_schedule_controller(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST(test_calculateIgnitionAngles_nonrotary);
    RUN_TEST(test_calculateIgnitionAngles_rotary);
    RUN_TEST(test_calculateIgnitionAngles_rotary_non_4_output_uses_non_rotary);
    RUN_TEST(test_calculateIgnitionAngles_sync_state_transitions);
    RUN_TEST(test_setIgnitionChannels_mask_enables_and_disables_channels);
  }
}