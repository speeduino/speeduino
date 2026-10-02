#include "../test_utils.h"
#include "src/decoders/details/perToothIgnition.h"
#include "scheduler_ignition_controller.h"

using namespace decoders::detail;

static decoder_features_t test_get_decoder_features(void)
{
    decoder_features_t features;
    features.hasFixedCrankingTiming = true;
    return features;
}

static void prepare_ign_schedule(IgnitionSchedule & schedule)
{
    schedule._status = PENDING;
    schedule._counter = 1000;
    schedule._compare = 1000;
    schedule.chargeAngle = 300;
}

static void prepare_per_tooth_schedule(statuses &current)
{
    setAngleConverterRevolutionTime(6000000UL);
    current.rotationStatus = EngineRotationStatus::Running;
    current.startRevolutions = 10;
    prepare_ign_schedule(ignitionSchedule1);
#if IGN_CHANNELS >= 2
    prepare_ign_schedule(ignitionSchedule2);
#endif
#if IGN_CHANNELS >= 3
    prepare_ign_schedule(ignitionSchedule3);
#endif
#if IGN_CHANNELS >= 4
    prepare_ign_schedule(ignitionSchedule4);
#endif
#if IGN_CHANNELS >= 5
    prepare_ign_schedule(ignitionSchedule5);
#endif
#if IGN_CHANNELS >= 6
    prepare_ign_schedule(ignitionSchedule6);
#endif
#if IGN_CHANNELS >= 7
    prepare_ign_schedule(ignitionSchedule7);
#endif
#if IGN_CHANNELS >= 8
    prepare_ign_schedule(ignitionSchedule8);
#endif
}

static void test_per_tooth_timing_adjusts_matching_pending_channel(void)
{
    statuses current;
    state_t decoderState{};
    config4 page4{};

    prepare_per_tooth_schedule(current);
    constexpr uint8_t CURRENT_TOOTH = 4;

    for (uint8_t i = 0; i<_countof(decoderState.ignitionEndTeeth); ++i)
    {
        decoderState.ignitionEndTeeth[i] = CURRENT_TOOTH+i;
        checkPerToothTiming(current, decoderState, page4, 270, CURRENT_TOOTH+i);
    }

    TEST_ASSERT_NOT_EQUAL_UINT32(1000, ignitionSchedule1._compare);
#if IGN_CHANNELS >= 2
    TEST_ASSERT_NOT_EQUAL_UINT32(1000, ignitionSchedule2._compare);
#endif
#if IGN_CHANNELS >= 3
    TEST_ASSERT_NOT_EQUAL_UINT32(1000, ignitionSchedule3._compare);
#endif
#if IGN_CHANNELS >= 4
    TEST_ASSERT_NOT_EQUAL_UINT32(1000, ignitionSchedule4._compare);
#endif
#if IGN_CHANNELS >= 5
    TEST_ASSERT_NOT_EQUAL_UINT32(1000, ignitionSchedule5._compare);
#endif
#if IGN_CHANNELS >= 6
    TEST_ASSERT_NOT_EQUAL_UINT32(1000, ignitionSchedule6._compare);
#endif
#if IGN_CHANNELS >= 7
    TEST_ASSERT_NOT_EQUAL_UINT32(1000, ignitionSchedule7._compare);
#endif
#if IGN_CHANNELS >= 8
    TEST_ASSERT_NOT_EQUAL_UINT32(1000, ignitionSchedule8._compare);
#endif
}

static void test_per_tooth_timing_ignores_nonmatching_tooth(void)
{
    statuses current;
    state_t decoderState{};
    config4 page4{};

    prepare_per_tooth_schedule(current);
    constexpr uint8_t CURRENT_TOOTH = 4;
    decoderState.ignitionEndTeeth[0] = CURRENT_TOOTH;

    checkPerToothTiming(current, decoderState, page4, 100, CURRENT_TOOTH-1);

    TEST_ASSERT_EQUAL_UINT32(1000, ignitionSchedule1._compare);
}

static void test_per_tooth_timing_ignores_stopped_engine(void)
{
    statuses current;
    state_t decoderState{};
    config4 page4{};

    prepare_per_tooth_schedule(current);
    current.rotationStatus = EngineRotationStatus::Stopped;
    constexpr uint8_t CURRENT_TOOTH = 4;
    decoderState.ignitionEndTeeth[0] = CURRENT_TOOTH;

    checkPerToothTiming(current, decoderState, page4, 100, CURRENT_TOOTH);

    TEST_ASSERT_EQUAL_UINT32(1000, ignitionSchedule1._compare);
}

static void test_per_tooth_timing_ignores_fixed_cranking_timing(void)
{
    statuses current;
    state_t decoderState{};
    config4 page4{};

    prepare_per_tooth_schedule(current);
    current.rotationStatus = EngineRotationStatus::Cranking;
    current.decoder.getFeatures = &test_get_decoder_features;
    page4.ignCranklock = true;
    constexpr uint8_t CURRENT_TOOTH = 4;
    decoderState.ignitionEndTeeth[0] = CURRENT_TOOTH;

    checkPerToothTiming(current, decoderState, page4, 100, CURRENT_TOOTH);

    TEST_ASSERT_EQUAL_UINT32(1000, ignitionSchedule1._compare);
}

void testPerToothIgnition(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_per_tooth_timing_adjusts_matching_pending_channel);
    RUN_TEST_P(test_per_tooth_timing_ignores_nonmatching_tooth);
    RUN_TEST_P(test_per_tooth_timing_ignores_stopped_engine);
    RUN_TEST_P(test_per_tooth_timing_ignores_fixed_cranking_timing);
}

