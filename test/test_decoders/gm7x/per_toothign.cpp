#include "../../test_utils.h"
#include "src/decoders/gm7x.h"
#include "scheduler_ignition_controller.h"

static void test_set_end_teeth_below_advance_threshold(void)
{
    statuses current{};
    decoders::shared::state_t decoderState{};

    current.advance = 17;
    decoders::gm7x::setEndTeeth(decoderState, current);

    TEST_ASSERT_EQUAL_UINT16(7, decoderState.ignitionEndTeeth[0]);
    TEST_ASSERT_EQUAL_UINT16(2, decoderState.ignitionEndTeeth[1]);
    TEST_ASSERT_EQUAL_UINT16(5, decoderState.ignitionEndTeeth[2]);
}

static void test_set_end_teeth_at_advance_threshold(void)
{
    statuses current{};
    decoders::shared::state_t decoderState{};

    current.advance = 18;
    decoders::gm7x::setEndTeeth(decoderState, current);

    TEST_ASSERT_EQUAL_UINT16(6, decoderState.ignitionEndTeeth[0]);
    TEST_ASSERT_EQUAL_UINT16(1, decoderState.ignitionEndTeeth[1]);
    TEST_ASSERT_EQUAL_UINT16(4, decoderState.ignitionEndTeeth[2]);
}

void testGM7XPerToothIgn(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_set_end_teeth_below_advance_threshold);
    RUN_TEST_P(test_set_end_teeth_at_advance_threshold);
}