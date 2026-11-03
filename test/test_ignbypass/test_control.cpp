#include "../test_utils.h"
#include "context.h"
#include "src/controllers/ignBypass/ignBypassControl_detail.h"

extern ignBypassController::details::state_t _state;

static void test_ignBypassControl_transition(EngineRotationStatus from, EngineRotationStatus to)
{
    test_context_t context;
    context.setupValidIgnBypass();
    context.cur.rotationStatus = from;
    context.initialise();

    TEST_ASSERT_EQUAL(from, _state.rotationStatus);
    context.cur.rotationStatus = to;
    ignBypassControl(context.cur);
    TEST_ASSERT_EQUAL(to, _state.rotationStatus);
}

static void test_ignBypassControl_stopped_to_cranking(void)
{
    test_ignBypassControl_transition(EngineRotationStatus::Stopped, EngineRotationStatus::Cranking);
    TEST_ASSERT_FALSE(_state.ignBypassPin._pin.isPinHigh());
}

static void test_ignBypassControl_stopped_to_running(void)
{
    test_ignBypassControl_transition(EngineRotationStatus::Stopped, EngineRotationStatus::Running);
    TEST_ASSERT_TRUE(_state.ignBypassPin._pin.isPinHigh());
}

static void test_ignBypassControl_cranking_to_running(void)
{
    test_ignBypassControl_transition(EngineRotationStatus::Cranking, EngineRotationStatus::Running);
    TEST_ASSERT_TRUE(_state.ignBypassPin._pin.isPinHigh());
}

static void test_ignBypassControl_cranking_to_stopped(void)
{
    test_ignBypassControl_transition(EngineRotationStatus::Cranking, EngineRotationStatus::Stopped);
    TEST_ASSERT_FALSE(_state.ignBypassPin._pin.isPinHigh());
}

static void test_ignBypassControl_running_to_stopped(void)
{
    test_ignBypassControl_transition(EngineRotationStatus::Running, EngineRotationStatus::Stopped);
    TEST_ASSERT_FALSE(_state.ignBypassPin._pin.isPinHigh());
}

static void test_ignBypassControl_running_to_cranking(void)
{
    test_ignBypassControl_transition(EngineRotationStatus::Running, EngineRotationStatus::Cranking);
    TEST_ASSERT_FALSE(_state.ignBypassPin._pin.isPinHigh());
}

void testControl(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_ignBypassControl_stopped_to_running);
        RUN_TEST_P(test_ignBypassControl_stopped_to_cranking);
        RUN_TEST_P(test_ignBypassControl_cranking_to_running);
        RUN_TEST_P(test_ignBypassControl_cranking_to_stopped);
        RUN_TEST_P(test_ignBypassControl_running_to_stopped);
        RUN_TEST_P(test_ignBypassControl_running_to_cranking);
    }
}