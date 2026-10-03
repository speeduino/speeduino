#include "../test_utils.h"
#include "shared.h"
#include "units.h"
#include "src/controllers/nitrous/nitrousController_state.h"

extern nitrous::detail::state_t _n2oState;

static void test_newboard_reset(void)
{
    auto context = setup_n20_tune(NITROUS_BOTH);

    context.page10.n2o_minTPS = 255;
    context.init();
    TEST_ASSERT_EQUAL(NITROUS_OFF, context.page10.n2o_enable);
    TEST_ASSERT_EQUAL(NITROUS_OFF, context.current.nitrous_status);
    TEST_ASSERT_FALSE(_n2oState.stage1Pin.isValid());
    TEST_ASSERT_FALSE(_n2oState.stage2Pin.isValid());
    TEST_ASSERT_FALSE(_n2oState.armingPin.isValid());
}

static void test_init_basic(void)
{
    auto context = setup_n20_tune(NITROUS_STAGE1);

    context.init();
    TEST_ASSERT_EQUAL(NITROUS_STAGE1, context.page10.n2o_enable);
    TEST_ASSERT_EQUAL(NITROUS_OFF, context.current.nitrous_status);
    TEST_ASSERT_TRUE(_n2oState.stage1Pin.isValid());
    TEST_ASSERT_FALSE(_n2oState.stage2Pin.isValid());
    TEST_ASSERT_TRUE(_n2oState.armingPin.isValid());

    context = setup_n20_tune(NITROUS_STAGE2);
    context.init();
    TEST_ASSERT_EQUAL(NITROUS_BOTH, context.page10.n2o_enable);
    TEST_ASSERT_EQUAL(NITROUS_OFF, context.current.nitrous_status);
    TEST_ASSERT_TRUE(_n2oState.stage1Pin.isValid());
    TEST_ASSERT_TRUE(_n2oState.stage2Pin.isValid());
    TEST_ASSERT_TRUE(_n2oState.armingPin.isValid());
}

static void test_n2o_armingpin_polarity(void)
{
    auto context = setup_n20_tune(NITROUS_STAGE1);
    _n2oState.armingPin.setPin(NOT_A_PIN);
    context.init();
    TEST_ASSERT_TRUE(_n2oState.armingPin.isValid());

    // Reverse polarity - coverage only
    context = setup_n20_tune(NITROUS_STAGE1);
    context.page10.n2o_pin_polarity = !context.page10.n2o_pin_polarity;
    _n2oState.armingPin.setPin(NOT_A_PIN);
    context.init();
    TEST_ASSERT_TRUE(_n2oState.armingPin.isValid());
}

static void test_invalid_pins(void)
{
    auto context = setup_n20_tune(NITROUS_BOTH);
    context.page10.n2o_arming_pin = NOT_A_PIN;
    context.init();
    TEST_ASSERT_EQUAL(NITROUS_OFF, context.page10.n2o_enable);
    TEST_ASSERT_EQUAL(NITROUS_OFF, context.current.nitrous_status);
    TEST_ASSERT_FALSE(_n2oState.stage1Pin.isValid());
    TEST_ASSERT_FALSE(_n2oState.stage2Pin.isValid());
    TEST_ASSERT_FALSE(_n2oState.armingPin.isValid());

    context = setup_n20_tune(NITROUS_BOTH);
    context.page10.n2o_stage1_pin = NOT_A_PIN;
    context.init();
    TEST_ASSERT_EQUAL(NITROUS_OFF, context.page10.n2o_enable);
    TEST_ASSERT_EQUAL(NITROUS_OFF, context.current.nitrous_status);
    TEST_ASSERT_FALSE(_n2oState.stage1Pin.isValid());
    TEST_ASSERT_FALSE(_n2oState.stage2Pin.isValid());
    TEST_ASSERT_FALSE(_n2oState.armingPin.isValid());    

    context = setup_n20_tune(NITROUS_BOTH);
    context.page10.n2o_stage2_pin = NOT_A_PIN;
    context.init();
    TEST_ASSERT_EQUAL(NITROUS_OFF, context.page10.n2o_enable);
    TEST_ASSERT_EQUAL(NITROUS_OFF, context.current.nitrous_status);
    TEST_ASSERT_FALSE(_n2oState.stage1Pin.isValid());
    TEST_ASSERT_FALSE(_n2oState.stage2Pin.isValid());
    TEST_ASSERT_FALSE(_n2oState.armingPin.isValid());
}

void testInit(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_newboard_reset);
    RUN_TEST_P(test_init_basic);
    RUN_TEST_P(test_n2o_armingpin_polarity);
    RUN_TEST_P(test_invalid_pins);
  }
}