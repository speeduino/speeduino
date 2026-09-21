#include "../test_utils.h"
#include "scheduler_ignition_controller.h"
#include "units.h"
#include "../channel_test_helpers.h"
#include "test_context.h"

extern uint8_t validateSparkMode(uint8_t mode, const config2 &page2);
extern void validateIgnitionSetup(config2 &page2, config4 &page4, config13 &page13);
extern void changeIgnitionToFullSequential(const config2 &page2, statuses &current);
extern void changeIgnitionToHalfSync(const config2 &page2, statuses &current);
extern bool isAnyIgnScheduleRunning(void);
extern void resetIgnitionSchedulers(void);
extern void setCallbacks(uint8_t sparkMode, uint8_t numCylinders, uint8_t rotaryMode);

static decoder_features_t fakeDecoderFeatures = {};
static decoder_features_t fakeGetFeatures(void) noexcept
{
    return fakeDecoderFeatures;
}
static void test_validateSparkMode(void)
{
    config2 page2 = {};
    page2.nCylinders = 1;
    page2.strokes = FOUR_STROKE;

    TEST_ASSERT_EQUAL(IGN_MODE_WASTED, validateSparkMode(IGN_MODE_WASTED, page2));
    TEST_ASSERT_EQUAL(IGN_MODE_SINGLE, validateSparkMode(IGN_MODE_SINGLE, page2));
    TEST_ASSERT_EQUAL(IGN_MODE_SEQUENTIAL, validateSparkMode(IGN_MODE_SEQUENTIAL, page2));
    page2.nCylinders = 4U;
    TEST_ASSERT_EQUAL(IGN_MODE_ROTARY, validateSparkMode(IGN_MODE_ROTARY, page2));
    page2.nCylinders = 4U;
    TEST_ASSERT_EQUAL(IGN_MODE_WASTEDCOP, validateSparkMode(IGN_MODE_WASTEDCOP, page2));
    page2.nCylinders = 6U;
    TEST_ASSERT_EQUAL(IGN_MODE_WASTEDCOP, validateSparkMode(IGN_MODE_WASTEDCOP, page2));
    page2.nCylinders = 8U;
    TEST_ASSERT_EQUAL(IGN_MODE_WASTEDCOP, validateSparkMode(IGN_MODE_WASTEDCOP, page2));

    page2 = {};
    page2.nCylinders = IGN_CHANNELS+1;
    page2.strokes = FOUR_STROKE;
    TEST_ASSERT_EQUAL(IGN_MODE_WASTED, validateSparkMode(IGN_MODE_SEQUENTIAL, page2));

    page2 = {};
    page2.nCylinders = 1;
    page2.strokes = TWO_STROKE;
    TEST_ASSERT_EQUAL(IGN_MODE_WASTED, validateSparkMode(IGN_MODE_SEQUENTIAL, page2));

    page2 = {};
    page2.nCylinders = 1;
    TEST_ASSERT_EQUAL(IGN_MODE_WASTED, validateSparkMode(IGN_MODE_ROTARY, page2));

    page2 = {};
    page2.nCylinders = 1;
    TEST_ASSERT_EQUAL(IGN_MODE_WASTED, validateSparkMode(IGN_MODE_WASTEDCOP, page2));
}

static void test_validateIgnitionSetup_oddfire(void)
{
    config2 page2 = {};
    config4 page4 = {};
    config13 page13 = {};

    page2.engineType = ODD_FIRE;
    page2.nCylinders = 11;

    validateIgnitionSetup(page2, page4, page13);

    TEST_ASSERT_EQUAL(EVEN_FIRE, page2.engineType);
}

static void test_validateIgnitionSetup_trims(void)
{
    config2 page2 = {};
    config4 page4 = {};
    config13 page13 = {};

    page4.sparkMode = IGN_MODE_SEQUENTIAL;
    memset(page13.ignTrim, -7, sizeof(page13.ignTrim));
    validateIgnitionSetup(page2, page4, page13);
    TEST_ASSERT_EACH_EQUAL_INT8 (-7, page13.ignTrim, _countof(page13.ignTrim));

    page4.sparkMode = IGN_MODE_SINGLE;
    validateIgnitionSetup(page2, page4, page13);
    TEST_ASSERT_EACH_EQUAL_INT8 (0, page13.ignTrim, _countof(page13.ignTrim));
}

static void test_validateIgnitionSetup_rotary(void)
{
    config2 page2 = {};
    config4 page4 = {};
    config13 page13 = {};

    page2.nCylinders = 4U;
    page4.sparkMode = IGN_MODE_ROTARY;
    page4.IgInv = GOING_HIGH;
    page2.strokes = TWO_STROKE;
    page2.engineType = ODD_FIRE;
    validateIgnitionSetup(page2, page4, page13);
    TEST_ASSERT_EQUAL(GOING_LOW, page4.IgInv);
    TEST_ASSERT_EQUAL(FOUR_STROKE, page2.strokes);
    TEST_ASSERT_EQUAL(EVEN_FIRE, page2.engineType);

    page2.nCylinders = 5U;
    validateIgnitionSetup(page2, page4, page13);
    TEST_ASSERT_EQUAL(IGN_MODE_WASTED, page4.sparkMode);
}

static void setup_ignition_channel_angles(void)
{
    for (uint8_t i = 0; i < _countof(ignitionSchedules); i++)
    {
        ignitionSchedules[i].channelDegrees = (CRANK_ANGLE_MAX_IGN/8U) * i;
    }
}    
 
static void test_setIgnitionChannels_mask_enables_and_disables_channels(void)
{
    test_context_t context;
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
        setIgnitionChannels(context.current, context.page4, 0U);

        // Enabled channels should be pending, disabled should remain OFF
        for (uint8_t i = 0; i < context.current.maxIgnOutputs; i++)
        {
            TEST_ASSERT_EQUAL_UINT8((i%2)==0 ? PENDING : OFF, (uint8_t)ignitionSchedules[i]._status);
        }
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
    TEST_ASSERT_EQUAL((std::min)((uint8_t)IGN_CHANNELS, page2.nCylinders), current.maxIgnOutputs);
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

static void test_changeIgnitionToFullSequential_running_schedule(void)
{
    resetIgnitionSchedulers();
    ignitionSchedules[0]._status = ScheduleStatus::RUNNING;
    for (uint8_t numCylinders=1; numCylinders<=8; ++numCylinders)
    {
        test_changeIgnitionToFullSequential_notapplied(numCylinders);
    }
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
    ignitionSchedules[0]._status = ScheduleStatus::RUNNING;
    for (uint8_t numCylinders=1; numCylinders<=8; ++numCylinders)
    {
        test_changeIgnitionToHalfSync_notapplied(numCylinders);
    }    
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
    for (auto& schedule: ignitionSchedules)
    {
        assert_isAnyIgnScheduleRunning(schedule);
    }    
}


static void test_setIgnitionChannels_fixed_cranking_override(void)
{
    test_context_t context;
    context.current.maxIgnOutputs = IGN_CHANNELS;
    context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
    CRANK_ANGLE_MAX_IGN = 720U;
    setup_ignition_channel_angles();
    context.current.crankRPM = 400;
    context.current.decoder.getFeatures = fakeGetFeatures;
    fakeDecoderFeatures.hasFixedCrankingTiming = false;
    context.page4.ignCranklock = true;
    context.current.rotationStatus = EngineRotationStatus::Cranking;
    context.current.schedulerCutState.ignitionChannels = 0b11111111;

    TEST_ASSERT_FALSE(context.current.isFixedCrankingIgnitionTimingActive(context.page4));
    resetIgnitionSchedulers();
    context.calculateIgnitionAngles();
    setIgnitionChannels(context.current, context.page4, 0U);

    int16_t originalAngles[IGN_CHANNELS];
    uint32_t originalDurations[IGN_CHANNELS];
    for (uint8_t i=0; i<_countof(ignitionSchedules); ++i)
    {
        originalAngles[i] = ignitionSchedules[i].chargeAngle;
        originalDurations[i] = ignitionSchedules[i]._duration;
    }

    context.current.setRpm(context.current.crankRPM-10);
    fakeDecoderFeatures.hasFixedCrankingTiming = true;
    TEST_ASSERT_TRUE(context.current.isFixedCrankingIgnitionTimingActive(context.page4));
    resetIgnitionSchedulers();
    context.calculateIgnitionAngles();
    setIgnitionChannels(context.current, context.page4, 0U);

    for (uint8_t i=0; i<_countof(ignitionSchedules); ++i)
    {
        TEST_ASSERT_EQUAL(originalAngles[i], ignitionSchedules[i].chargeAngle);
        TEST_ASSERT_NOT_EQUAL(originalDurations[i], ignitionSchedules[i]._duration);
    }

    context.current.setRpm(200);
    TEST_ASSERT_TRUE(context.current.isFixedCrankingIgnitionTimingActive(context.page4));
    resetIgnitionSchedulers();
    context.calculateIgnitionAngles();
    setIgnitionChannels(context.current, context.page4, 0U);

    for (uint8_t i=0; i<_countof(ignitionSchedules); ++i)
    {
            TEST_ASSERT_NOT_EQUAL(originalAngles[i], ignitionSchedules[i].chargeAngle);
            TEST_ASSERT_NOT_EQUAL(originalDurations[i], ignitionSchedules[i]._duration);\
    }
}

void testIgnitionController(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_validateSparkMode);
    RUN_TEST_P(test_validateIgnitionSetup_oddfire);
    RUN_TEST_P(test_validateIgnitionSetup_trims);
    RUN_TEST_P(test_validateIgnitionSetup_rotary);
    RUN_TEST_P(test_setIgnitionChannels_mask_enables_and_disables_channels);
    RUN_TEST_P(test_setIgnitionChannels_fixed_cranking_override);
    RUN_TEST_P(test_changeIgnitionToFullSequential);
    RUN_TEST_P(test_changeIgnitionToFullSequential_running_schedule);
    RUN_TEST_P(test_changeIgnitionToHalfSync);
    RUN_TEST_P(test_changeIgnitionToHalfSync_runningschedule);
    RUN_TEST_P(test_isAnyIgnScheduleRunning);
 }
}