#include "../test_utils.h"
#include "context.h"
#include "src/controllers/ignBypass/ignBypassControl_detail.h"

extern ignBypassController::details::state_t _state;

static void test_initialiseIgnBypass_disabled(void)
{
    test_context_t context;
    context.setupValidIgnBypass();

    // Test with ignBypassEnabled set to false
    context.p4.ignBypassEnabled = false;
    context.initialise();
    TEST_ASSERT_FALSE(_state.ignBypassPin.isValid());
    
    // Should have no effect if the pin is not set as an output
    context.cur.rotationStatus = EngineRotationStatus::Cranking;
    TEST_ASSERT_NOT_EQUAL(context.cur.rotationStatus, _state.rotationStatus);
    ignBypassControl(context.cur);
    TEST_ASSERT_NOT_EQUAL(context.cur.rotationStatus, _state.rotationStatus);
}

static void test_initialiseIgnBypass_invalid_pin(void)
{
    test_context_t context;
    context.setupValidIgnBypass();

    // Test with ignBypassEnabled set to true with invalid pin number
    context.pins.setCoilPin(0, context.pins.pinIgnBypass); //...that conflicts with a coil pin to simulate an invalid output pin
    context.initialise();
    TEST_ASSERT_FALSE(_state.ignBypassPin.isValid());
    TEST_ASSERT_FALSE(context.p4.ignBypassEnabled);

    // Should have no effect if the pin is not set as an output
    context.cur.rotationStatus = EngineRotationStatus::Cranking;
    TEST_ASSERT_NOT_EQUAL(context.cur.rotationStatus, _state.rotationStatus);
    ignBypassControl(context.cur);
    TEST_ASSERT_NOT_EQUAL(context.cur.rotationStatus, _state.rotationStatus);
}

static void test_initialiseIgnBypass_enabled(void)
{
    test_context_t context;

    context.setupValidIgnBypass();
    context.initialise();
    TEST_ASSERT_TRUE(_state.ignBypassPin.isValid());
}

void testInit(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_initialiseIgnBypass_disabled);
        RUN_TEST_P(test_initialiseIgnBypass_enabled);
        RUN_TEST_P(test_initialiseIgnBypass_invalid_pin);
    }
}