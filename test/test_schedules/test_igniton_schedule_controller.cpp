#include <unity.h>
#include "../test_utils.h"
#include "scheduler_ignition_controller.h"
#include "channel_test_helpers.h"
#include "../fake_decoder_status.h"
#include "scheduledIO_ign.h"

extern void SetRevolutionTime(uint32_t revTime);
extern void changeIgnitionToFullSequential(const config2 &page2, statuses &current);
extern void changeIgnitionToHalfSync(const config2 &page2, statuses &current);
extern bool isAnyIgnScheduleRunning(void);
extern void resetIgnitionSchedulers(void);

struct ignition_test_context_t
{
    config2 page2 = {};
    config4 page4 = {};
    statuses current = {};

    ignition_test_context_t() {
        // Set basic engine config defaults
        page2.nCylinders = 4U;
        page2.strokes = FOUR_STROKE;
        page4.sparkMode = IGN_MODE_WASTED;
        fakeDecoderStatus.syncStatus = SyncStatus::Full;
        current.maxIgnOutputs = 4U;
        current.dwell = 3000U; // 3ms dwell
        current.advance = 15U;  // 15 degrees advance
        current.decoder = decoder_builder_t().setGetStatus(getFakeDecoderStatus).build();
        SetRevolutionTime(UDIV_ROUND_CLOSEST(60UL*1000000UL, 4000, uint32_t));
    }

    void calculateIgnitionAngles(void)
    {
        ::calculateIgnitionAngles(page2, page4, current);        
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
    fakeDecoderStatus.syncStatus = SyncStatus::Full;

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
    fakeDecoderStatus.syncStatus = SyncStatus::Full;
    context.calculateIgnitionAngles();
    TEST_ASSERT_EQUAL_UINT16(720U, CRANK_ANGLE_MAX_IGN);
    
    // Re-setup context for partial sync
    fakeDecoderStatus.syncStatus = SyncStatus::Partial;
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
        context.current.schedulerCutState.ignitionChannels = 0b01010101;
        setIgnitionChannels(context.current, 0U, context.current.dwell);

        // Enabled channels should be pending, disabled should remain OFF
        RUNIF_IGNCHANNEL1( { if (context.current.maxIgnOutputs>=1) { TEST_ASSERT_EQUAL_UINT8(PENDING, (uint8_t)ignitionSchedule1._status); } }, {});
        RUNIF_IGNCHANNEL2( { if (context.current.maxIgnOutputs>=2) { TEST_ASSERT_EQUAL_UINT8(OFF, (uint8_t)ignitionSchedule2._status); } }, {});
        RUNIF_IGNCHANNEL3( { if (context.current.maxIgnOutputs>=3) { TEST_ASSERT_EQUAL_UINT8(PENDING, (uint8_t)ignitionSchedule3._status); } }, {});
        RUNIF_IGNCHANNEL4( { if (context.current.maxIgnOutputs>=4) { TEST_ASSERT_EQUAL_UINT8(OFF, (uint8_t)ignitionSchedule4._status); } }, {});
        RUNIF_IGNCHANNEL5( { if (context.current.maxIgnOutputs>=5) { TEST_ASSERT_EQUAL_UINT8(PENDING, (uint8_t)ignitionSchedule5._status); } }, {});
        RUNIF_IGNCHANNEL6( { if (context.current.maxIgnOutputs>=6) { TEST_ASSERT_EQUAL_UINT8(OFF, (uint8_t)ignitionSchedule6._status); } }, {});
        RUNIF_IGNCHANNEL7( { if (context.current.maxIgnOutputs>=7) { TEST_ASSERT_EQUAL_UINT8(PENDING, (uint8_t)ignitionSchedule7._status); } }, {});
        RUNIF_IGNCHANNEL8( { if (context.current.maxIgnOutputs>=8) { TEST_ASSERT_EQUAL_UINT8(OFF, (uint8_t)ignitionSchedule8._status); } }, {});
    }
}

static void test_changeIgnitionToFullSequential_isapplied(uint8_t numCylinders)
{
    statuses current = {};
    current.maxIgnOutputs = 1;
    config2 page2 = {};
    page2.nCylinders = numCylinders;
    CRANK_ANGLE_MAX_IGN = 360;

    changeIgnitionToFullSequential(page2, current);
    TEST_ASSERT_EQUAL(720, CRANK_ANGLE_MAX_IGN);
    TEST_ASSERT_EQUAL(min((uint8_t)IGN_CHANNELS, page2.nCylinders), current.maxIgnOutputs);
}

static void test_changeIgnitionToFullSequential_notapplied(uint8_t numCylinders)
{
    statuses current = {};
    current.maxIgnOutputs = 0;
    config2 page2 = {};
    page2.nCylinders = numCylinders;
    CRANK_ANGLE_MAX_IGN = 360;

    changeIgnitionToFullSequential(page2, current);
    TEST_ASSERT_EQUAL(360, CRANK_ANGLE_MAX_IGN);
    TEST_ASSERT_EQUAL(0U, current.maxIgnOutputs);
}

static void test_changeIgnitionToFullSequential(void)
{
    resetIgnitionSchedulers();
    test_changeIgnitionToFullSequential_notapplied(1U);
    test_changeIgnitionToFullSequential_notapplied(2U);
    test_changeIgnitionToFullSequential_notapplied(3U);
    test_changeIgnitionToFullSequential_isapplied(4U);
    test_changeIgnitionToFullSequential_notapplied(5U);
    test_changeIgnitionToFullSequential_isapplied(6U);
    test_changeIgnitionToFullSequential_isapplied(8U);
}

static void test_changeIgnitionToFullSequential_running_schedule(void)
{
    resetIgnitionSchedulers();
    ignitionSchedule1._status = ScheduleStatus::RUNNING;
    test_changeIgnitionToFullSequential_notapplied(1U);
    test_changeIgnitionToFullSequential_notapplied(2U);
    test_changeIgnitionToFullSequential_notapplied(3U);
    test_changeIgnitionToFullSequential_notapplied(4U);
    test_changeIgnitionToFullSequential_notapplied(5U);
    test_changeIgnitionToFullSequential_notapplied(6U);
    test_changeIgnitionToFullSequential_notapplied(8U);
}

static void test_changeIgnitionToHalfSync_isapplied(uint8_t numCylinders)
{
    statuses current = {};
    current.maxIgnOutputs = 0;
    config2 page2 = {};
    page2.nCylinders = numCylinders;
    CRANK_ANGLE_MAX_IGN = 720;
    changeIgnitionToHalfSync(page2, current);

    TEST_ASSERT_EQUAL(360, CRANK_ANGLE_MAX_IGN);
    TEST_ASSERT_EQUAL(page2.nCylinders/2U, current.maxIgnOutputs);
}

static void test_changeIgnitionToHalfSync_notapplied(uint8_t numCylinders)
{
    statuses current = {};
    current.maxIgnOutputs = 0;
    config2 page2 = {};
    page2.nCylinders = numCylinders;
    CRANK_ANGLE_MAX_IGN = 720;
    changeIgnitionToHalfSync(page2, current);
    // Expect no change
    TEST_ASSERT_EQUAL(720, CRANK_ANGLE_MAX_IGN);
    TEST_ASSERT_EQUAL(0U, current.maxIgnOutputs);
}

static void test_changeIgnitionToHalfSync(void)
{
    resetIgnitionSchedulers();
    test_changeIgnitionToHalfSync_notapplied(1U);
    test_changeIgnitionToHalfSync_notapplied(2U);
    test_changeIgnitionToHalfSync_notapplied(3U);
    test_changeIgnitionToHalfSync_isapplied(4U);
    test_changeIgnitionToHalfSync_notapplied(5U);
    test_changeIgnitionToHalfSync_isapplied(6U);
    test_changeIgnitionToHalfSync_notapplied(7U);
    test_changeIgnitionToHalfSync_isapplied(8U);
}

static void test_changeIgnitionToHalfSync_runningschedule(void)
{
    resetIgnitionSchedulers();
    ignitionSchedule1._status = ScheduleStatus::RUNNING;
    test_changeIgnitionToHalfSync_notapplied(1U);
    test_changeIgnitionToHalfSync_notapplied(2U);
    test_changeIgnitionToHalfSync_notapplied(3U);
    test_changeIgnitionToHalfSync_notapplied(4U);
    test_changeIgnitionToHalfSync_notapplied(5U);
    test_changeIgnitionToHalfSync_notapplied(6U);
    test_changeIgnitionToHalfSync_notapplied(7U);
    test_changeIgnitionToHalfSync_notapplied(8U);
}

static void assert_isAnyIgnScheduleRunning(IgnitionSchedule & schedule)
{
    TEST_ASSERT_FALSE(isAnyIgnScheduleRunning());
    schedule._status = RUNNING;
    TEST_ASSERT_TRUE(isAnyIgnScheduleRunning());
    schedule._status = OFF;
}

static void test_isAnyIgnScheduleRunning(void)
{
    stopIgnitionSchedulers();
    resetIgnitionSchedulers();
    RUNIF_IGNCHANNEL1( { assert_isAnyIgnScheduleRunning(ignitionSchedule1); }, {});
    RUNIF_IGNCHANNEL2( { assert_isAnyIgnScheduleRunning(ignitionSchedule2); }, {});
    RUNIF_IGNCHANNEL3( { assert_isAnyIgnScheduleRunning(ignitionSchedule3); }, {});
    RUNIF_IGNCHANNEL4( { assert_isAnyIgnScheduleRunning(ignitionSchedule4); }, {});
    RUNIF_IGNCHANNEL5( { assert_isAnyIgnScheduleRunning(ignitionSchedule5); }, {});
    RUNIF_IGNCHANNEL6( { assert_isAnyIgnScheduleRunning(ignitionSchedule6); }, {});
    RUNIF_IGNCHANNEL7( { assert_isAnyIgnScheduleRunning(ignitionSchedule7); }, {});
    RUNIF_IGNCHANNEL8( { assert_isAnyIgnScheduleRunning(ignitionSchedule8); }, {});
}

static void assert_callbacks(IgnitionSchedule &schedule, IgnitionSchedule::callback start, IgnitionSchedule::callback end)
{
    TEST_ASSERT_EQUAL_PTR (start, schedule._pStartCallback);
    TEST_ASSERT_EQUAL_PTR (end, schedule._pEndCallback);
}

static void assert_singlechannel_callbacks(void)
{
    RUNIF_IGNCHANNEL1( { assert_callbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge); }, {});
    RUNIF_IGNCHANNEL2( { assert_callbacks(ignitionSchedule2, beginCoil1Charge, endCoil1Charge); }, {});
    RUNIF_IGNCHANNEL3( { assert_callbacks(ignitionSchedule3, beginCoil1Charge, endCoil1Charge); }, {});
    RUNIF_IGNCHANNEL4( { assert_callbacks(ignitionSchedule4, beginCoil1Charge, endCoil1Charge); }, {});
    RUNIF_IGNCHANNEL5( { assert_callbacks(ignitionSchedule5, beginCoil1Charge, endCoil1Charge); }, {});
    RUNIF_IGNCHANNEL6( { assert_callbacks(ignitionSchedule6, beginCoil1Charge, endCoil1Charge); }, {});
    RUNIF_IGNCHANNEL7( { assert_callbacks(ignitionSchedule7, beginCoil1Charge, endCoil1Charge); }, {});
    RUNIF_IGNCHANNEL8( { assert_callbacks(ignitionSchedule8, beginCoil1Charge, endCoil1Charge); }, {});
}

static void test_initialize_singlechannel_callbacks(void)
{
    initialiseIgnitionSchedules(IGN_MODE_SINGLE, 1, 0U);
    assert_singlechannel_callbacks();

    initialiseIgnitionSchedules(IGN_MODE_SINGLE, 2, 0U);
    assert_singlechannel_callbacks();

    initialiseIgnitionSchedules(IGN_MODE_SINGLE, 3, 0U);
    assert_singlechannel_callbacks();

    initialiseIgnitionSchedules(IGN_MODE_SINGLE, 4, 0U);
    assert_singlechannel_callbacks();

    initialiseIgnitionSchedules(IGN_MODE_SINGLE, 5, 0U);
    assert_singlechannel_callbacks();

    initialiseIgnitionSchedules(IGN_MODE_SINGLE, 6, 0U);
    assert_singlechannel_callbacks();

    initialiseIgnitionSchedules(IGN_MODE_SINGLE, 7, 0U);
    assert_singlechannel_callbacks();

    initialiseIgnitionSchedules(IGN_MODE_SINGLE, 8, 0U);
    assert_singlechannel_callbacks();
}

static void assert_wastedCOP_1_to_3_callbacks(void)
{
    RUNIF_IGNCHANNEL1( { assert_callbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge); }, {});
    RUNIF_IGNCHANNEL2( { assert_callbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge); }, {});
    RUNIF_IGNCHANNEL3( { assert_callbacks(ignitionSchedule3, beginCoil3Charge, endCoil3Charge); }, {});
    RUNIF_IGNCHANNEL4( { assert_callbacks(ignitionSchedule4, beginCoil4Charge, endCoil4Charge); }, {});
    RUNIF_IGNCHANNEL5( { assert_callbacks(ignitionSchedule5, beginCoil5Charge, endCoil5Charge); }, {});
    RUNIF_IGNCHANNEL6( { assert_callbacks(ignitionSchedule6, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL7( { assert_callbacks(ignitionSchedule7, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL8( { assert_callbacks(ignitionSchedule8, nullCallback, nullCallback); }, {});
}

static void test_initialize_wastedCOP1_callbacks(void)
{
    initialiseIgnitionSchedules(IGN_MODE_WASTEDCOP, 1, 0U);
    assert_wastedCOP_1_to_3_callbacks();
}
    
static void test_initialize_wastedCOP2_callbacks(void)
{
    initialiseIgnitionSchedules(IGN_MODE_WASTEDCOP, 2, 0U);
    assert_wastedCOP_1_to_3_callbacks();
}

static void test_initialize_wastedCOP3_callbacks(void)
{
    initialiseIgnitionSchedules(IGN_MODE_WASTEDCOP, 3, 0U);
    assert_wastedCOP_1_to_3_callbacks();
}

static void test_initialize_wastedCOP4_callbacks(void)
{
    initialiseIgnitionSchedules(IGN_MODE_WASTEDCOP, 4, 0U);

    RUNIF_IGNCHANNEL1( { assert_callbacks(ignitionSchedule1, beginCoil1and3Charge, endCoil1and3Charge); }, {});
    RUNIF_IGNCHANNEL2( { assert_callbacks(ignitionSchedule2, beginCoil2and4Charge, endCoil2and4Charge); }, {});
    RUNIF_IGNCHANNEL3( { assert_callbacks(ignitionSchedule3, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL4( { assert_callbacks(ignitionSchedule4, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL5( { assert_callbacks(ignitionSchedule5, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL6( { assert_callbacks(ignitionSchedule6, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL7( { assert_callbacks(ignitionSchedule7, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL8( { assert_callbacks(ignitionSchedule8, nullCallback, nullCallback); }, {});
}

static void test_initialize_wastedCOP5_callbacks(void)
{
    initialiseIgnitionSchedules(IGN_MODE_WASTEDCOP, 5, 0U);
    assert_wastedCOP_1_to_3_callbacks();
}

static void test_initialize_wastedCOP6_callbacks(void)
{
    initialiseIgnitionSchedules(IGN_MODE_WASTEDCOP, 6, 0U);

    RUNIF_IGNCHANNEL1( { assert_callbacks(ignitionSchedule1, beginCoil1and4Charge, endCoil1and4Charge); }, {});
    RUNIF_IGNCHANNEL2( { assert_callbacks(ignitionSchedule2, beginCoil2and5Charge, endCoil2and5Charge); }, {});
    RUNIF_IGNCHANNEL3( { assert_callbacks(ignitionSchedule3, beginCoil3and6Charge, endCoil3and6Charge); }, {});
    RUNIF_IGNCHANNEL3( { assert_callbacks(ignitionSchedule4, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL5( { assert_callbacks(ignitionSchedule5, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL6( { assert_callbacks(ignitionSchedule6, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL7( { assert_callbacks(ignitionSchedule7, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL8( { assert_callbacks(ignitionSchedule8, nullCallback, nullCallback); }, {});
}

static void test_initialize_wastedCOP8_callbacks(void)
{
    initialiseIgnitionSchedules(IGN_MODE_WASTEDCOP, 8, 0U);

    RUNIF_IGNCHANNEL1( { assert_callbacks(ignitionSchedule1, beginCoil1and5Charge, endCoil1and5Charge); }, {});
    RUNIF_IGNCHANNEL2( { assert_callbacks(ignitionSchedule2, beginCoil2and6Charge, endCoil2and6Charge); }, {});
    RUNIF_IGNCHANNEL3( { assert_callbacks(ignitionSchedule3, beginCoil3and7Charge, endCoil3and7Charge); }, {});
    RUNIF_IGNCHANNEL4( { assert_callbacks(ignitionSchedule4, beginCoil4and8Charge, endCoil4and8Charge); }, {});
    RUNIF_IGNCHANNEL5( { assert_callbacks(ignitionSchedule5, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL6( { assert_callbacks(ignitionSchedule6, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL7( { assert_callbacks(ignitionSchedule7, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL8( { assert_callbacks(ignitionSchedule8, nullCallback, nullCallback); }, {});
}

static void assert_sequential_callbacks(void)
{
    RUNIF_IGNCHANNEL1( { assert_callbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge); }, {});
    RUNIF_IGNCHANNEL2( { assert_callbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge); }, {});
    RUNIF_IGNCHANNEL3( { assert_callbacks(ignitionSchedule3, beginCoil3Charge, endCoil3Charge); }, {});
    RUNIF_IGNCHANNEL4( { assert_callbacks(ignitionSchedule4, beginCoil4Charge, endCoil4Charge); }, {});
    RUNIF_IGNCHANNEL5( { assert_callbacks(ignitionSchedule5, beginCoil5Charge, endCoil5Charge); }, {});
    RUNIF_IGNCHANNEL6( { assert_callbacks(ignitionSchedule6, beginCoil6Charge, endCoil6Charge); }, {});
    RUNIF_IGNCHANNEL7( { assert_callbacks(ignitionSchedule7, beginCoil7Charge, endCoil7Charge); }, {});
    RUNIF_IGNCHANNEL8( { assert_callbacks(ignitionSchedule8, beginCoil8Charge, endCoil8Charge); }, {});
}

static void test_initialize_sequential_callbacks(void)
{
    initialiseIgnitionSchedules(IGN_MODE_SEQUENTIAL, 1, 0U);
    assert_sequential_callbacks();

    initialiseIgnitionSchedules(IGN_MODE_SEQUENTIAL, 2, 0U);
    assert_sequential_callbacks();

    initialiseIgnitionSchedules(IGN_MODE_SEQUENTIAL, 3, 0U);
    assert_sequential_callbacks();

    initialiseIgnitionSchedules(IGN_MODE_SEQUENTIAL, 4, 0U);
    assert_sequential_callbacks();

    initialiseIgnitionSchedules(IGN_MODE_SEQUENTIAL, 5, 0U);
    assert_sequential_callbacks();

    initialiseIgnitionSchedules(IGN_MODE_SEQUENTIAL, 6, 0U);
    assert_sequential_callbacks();

    initialiseIgnitionSchedules(IGN_MODE_SEQUENTIAL, 7, 0U);
    assert_sequential_callbacks();

    initialiseIgnitionSchedules(IGN_MODE_SEQUENTIAL, 8, 0U);
    assert_sequential_callbacks();
}

static void test_initialize_rotary_fc_callbacks(void)
{
    initialiseIgnitionSchedules(IGN_MODE_ROTARY, 0U, ROTARY_IGN_FC);

    RUNIF_IGNCHANNEL1( { assert_callbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge); }, {});
    RUNIF_IGNCHANNEL2( { assert_callbacks(ignitionSchedule2, beginCoil1Charge, endCoil1Charge); }, {});
    RUNIF_IGNCHANNEL3( { assert_callbacks(ignitionSchedule3, beginTrailingCoilCharge, endTrailingCoilCharge1); }, {});
    RUNIF_IGNCHANNEL4( { assert_callbacks(ignitionSchedule4, beginTrailingCoilCharge, endTrailingCoilCharge2); }, {});
    RUNIF_IGNCHANNEL5( { assert_callbacks(ignitionSchedule5, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL6( { assert_callbacks(ignitionSchedule6, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL7( { assert_callbacks(ignitionSchedule7, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL8( { assert_callbacks(ignitionSchedule8, nullCallback, nullCallback); }, {});
}

static void test_initialize_rotary_fd_callbacks(void)
{
    initialiseIgnitionSchedules(IGN_MODE_ROTARY, 0U, ROTARY_IGN_FD);

    RUNIF_IGNCHANNEL1( { assert_callbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge); }, {});
    RUNIF_IGNCHANNEL2( { assert_callbacks(ignitionSchedule2, beginCoil1Charge, endCoil1Charge); }, {});
    RUNIF_IGNCHANNEL3( { assert_callbacks(ignitionSchedule3, beginCoil2Charge, endCoil2Charge); }, {});
    RUNIF_IGNCHANNEL4( { assert_callbacks(ignitionSchedule4, beginCoil3Charge, endCoil3Charge); }, {});
    RUNIF_IGNCHANNEL5( { assert_callbacks(ignitionSchedule5, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL6( { assert_callbacks(ignitionSchedule6, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL7( { assert_callbacks(ignitionSchedule7, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL8( { assert_callbacks(ignitionSchedule8, nullCallback, nullCallback); }, {});
}

static void test_initialize_rotary_rx8_callbacks(void)
{
    initialiseIgnitionSchedules(IGN_MODE_ROTARY, 0U, ROTARY_IGN_RX8);

    RUNIF_IGNCHANNEL1( { assert_callbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge); }, {});
    RUNIF_IGNCHANNEL2( { assert_callbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge); }, {});
    RUNIF_IGNCHANNEL3( { assert_callbacks(ignitionSchedule3, beginCoil3Charge, endCoil3Charge); }, {});
    RUNIF_IGNCHANNEL4( { assert_callbacks(ignitionSchedule4, beginCoil4Charge, endCoil4Charge); }, {});
    RUNIF_IGNCHANNEL5( { assert_callbacks(ignitionSchedule5, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL6( { assert_callbacks(ignitionSchedule6, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL7( { assert_callbacks(ignitionSchedule7, nullCallback, nullCallback); }, {});
    RUNIF_IGNCHANNEL8( { assert_callbacks(ignitionSchedule8, nullCallback, nullCallback); }, {});
}

void test_ignition_schedule_controller(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_calculateIgnitionAngles_nonrotary);
    RUN_TEST_P(test_calculateIgnitionAngles_rotary);
    RUN_TEST_P(test_calculateIgnitionAngles_rotary_non_4_output_uses_non_rotary);
    RUN_TEST_P(test_calculateIgnitionAngles_sync_state_transitions);
    RUN_TEST_P(test_setIgnitionChannels_mask_enables_and_disables_channels);
    RUN_TEST_P(test_changeIgnitionToFullSequential);
    RUN_TEST_P(test_changeIgnitionToFullSequential_running_schedule);
    RUN_TEST_P(test_changeIgnitionToHalfSync);
    RUN_TEST_P(test_changeIgnitionToHalfSync_runningschedule);
    RUN_TEST_P(test_isAnyIgnScheduleRunning);
    RUN_TEST_P(test_initialize_singlechannel_callbacks);
    RUN_TEST_P(test_initialize_wastedCOP1_callbacks);
    RUN_TEST_P(test_initialize_wastedCOP2_callbacks);
    RUN_TEST_P(test_initialize_wastedCOP3_callbacks);
    RUN_TEST_P(test_initialize_wastedCOP4_callbacks);
    RUN_TEST_P(test_initialize_wastedCOP5_callbacks);
    RUN_TEST_P(test_initialize_wastedCOP6_callbacks);
    RUN_TEST_P(test_initialize_wastedCOP8_callbacks);
    RUN_TEST_P(test_initialize_sequential_callbacks);
    RUN_TEST_P(test_initialize_rotary_fc_callbacks);
    RUN_TEST_P(test_initialize_rotary_fd_callbacks);
    RUN_TEST_P(test_initialize_rotary_rx8_callbacks);
  }
}