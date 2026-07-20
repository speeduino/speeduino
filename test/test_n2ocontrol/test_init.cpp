#include "globals.h"
#include "auxiliaries.h"
#include "units.h"
#include "../test_utils.h"
#include "shared.h"
#include "src/pins/fastInputPin.h"
#include "src/pins/fastOutputPin.h"
#include "src/pins/outputPin.h"
#include "board_definition.h"

extern fastInputPin_t n2o_arming_pin;
extern boardOutputPin_t n2o_stage1_pin;
extern boardOutputPin_t n2o_stage2_pin;

static void test_newboard_reset(void)
{
    setup_n20_tune(NITROUS_STAGE1);

    configPage10.n2o_minTPS = 255;
    TEST_ASSERT_EQUAL(NITROUS_STAGE1, configPage10.n2o_enable);
    initialiseAuxPWM();
    TEST_ASSERT_EQUAL(NITROUS_OFF, configPage10.n2o_enable);
}

static void test_init_basic(void)
{
    setup_n20_tune(NITROUS_STAGE1);

    TEST_ASSERT_EQUAL(NITROUS_STAGE1, configPage10.n2o_enable);
    initialiseAuxPWM();
    TEST_ASSERT_EQUAL(NITROUS_STAGE1, configPage10.n2o_enable);
    TEST_ASSERT_EQUAL(NITROUS_OFF, currentStatus.nitrous_status);
}

static void test_n2o_armingpin(void)
{
    setup_n20_tune(NITROUS_STAGE1);
    n2o_arming_pin.setPin(NOT_A_PIN);
    initialiseAuxPWM();
    TEST_ASSERT_TRUE(n2o_arming_pin.isValid());

    // Reverse polarity - coverage only
    setup_n20_tune(NITROUS_STAGE1);
    configPage10.n2o_pin_polarity = !configPage10.n2o_pin_polarity;
    n2o_arming_pin.setPin(NOT_A_PIN);
    initialiseAuxPWM();
    TEST_ASSERT_TRUE(n2o_arming_pin.isValid());

    // Disabled
    setup_n20_tune(NITROUS_OFF);
    n2o_arming_pin.setPin(NOT_A_PIN);
    initialiseAuxPWM();
    TEST_ASSERT_FALSE(n2o_arming_pin.isValid());

}

static void test_n2o_stage_pins(void)
{
    setup_n20_tune(NITROUS_STAGE1);
    n2o_stage1_pin.setPin(NOT_A_PIN);
    initialiseAuxPWM();
    TEST_ASSERT_TRUE(n2o_stage1_pin.isValid()==(configPage10.n2o_stage1_pin!=NOT_A_PIN));
    TEST_ASSERT_TRUE(n2o_stage2_pin.isValid()==(configPage10.n2o_stage2_pin!=NOT_A_PIN));
}

void testInit(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_newboard_reset);
    RUN_TEST_P(test_init_basic);
    RUN_TEST_P(test_n2o_armingpin);
    RUN_TEST_P(test_n2o_stage_pins);
  }
}