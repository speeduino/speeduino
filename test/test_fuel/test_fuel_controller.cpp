#include "../test_utils.h"
#include "scheduler_fuel_controller.h"

extern uint8_t calulateNumSquirts(const config2 &page2);
extern uint16_t calculateMaxInjAngle(uint8_t squirtsPerCycle, const config2 &page2);
extern uint8_t calcNumPrimaryInjectors(config2 &page2);
extern uint8_t calcNumSecondaryInjectors(uint16_t primary, const config2 &page2, config10 &page10);

static void test_calulateNumSquirts_default_no_divider(void)
{
  config2 page2 = {};
  page2.divider = 0;
  page2.nCylinders = 4;
  page2.injLayout = INJ_PAIRED;
  page2.strokes = TWO_STROKE;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT8(2U, calulateNumSquirts(page2));
}

static void test_calulateNumSquirts_divider_calculation(void)
{
  config2 page2 = {};
  page2.divider = 2;
  page2.nCylinders = 8;
  page2.injLayout = INJ_PAIRED;
  page2.strokes = TWO_STROKE;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT8(4U, calulateNumSquirts(page2));
}

static void test_calulateNumSquirts_sequential_four_stroke_override(void)
{
  config2 page2 = {};
  page2.divider = 2;
  page2.nCylinders = 4;
  page2.injLayout = INJ_SEQUENTIAL;
  page2.strokes = FOUR_STROKE;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT8(1U, calulateNumSquirts(page2));
}

static void test_calulateNumSquirts_sequential_two_stroke_uses_divider(void)
{
  config2 page2 = {};
  page2.divider = 2;
  page2.nCylinders = 4;
  page2.injLayout = INJ_SEQUENTIAL;
  page2.strokes = TWO_STROKE;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT8(2U, calulateNumSquirts(page2));
}

static void test_calulateNumSquirts_three_cyl_port_semisequential_force_two(void)
{
  config2 page2 = {};
  page2.divider = 1;
  page2.nCylinders = 3;
  page2.injLayout = INJ_SEMISEQUENTIAL;
  page2.strokes = FOUR_STROKE;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT8(2U, calulateNumSquirts(page2));
}

static void test_calulateNumSquirts_three_cyl_port_paired_force_two(void)
{
  config2 page2 = {};
  page2.divider = 1;
  page2.nCylinders = 3;
  page2.injLayout = INJ_PAIRED;
  page2.strokes = FOUR_STROKE;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT8(2U, calulateNumSquirts(page2));
}

static void test_calulateNumSquirts_three_cyl_tbody_semisequential_uses_divider(void)
{
  config2 page2 = {};
  page2.divider = 1;
  page2.nCylinders = 3;
  page2.injLayout = INJ_SEMISEQUENTIAL;
  page2.strokes = FOUR_STROKE;
  page2.injType = INJ_TYPE_TBODY;

  TEST_ASSERT_EQUAL_UINT8(3U, calulateNumSquirts(page2));
}

static void test_calulateNumSquirts_clamps_minimum_to_one(void)
{
  config2 page2 = {};
  page2.divider = 10;
  page2.nCylinders = 1;
  page2.injLayout = INJ_PAIRED;
  page2.strokes = TWO_STROKE;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT8(1U, calulateNumSquirts(page2));
}

static void test_calculateMaxInjAngle_four_stroke_default_two_squirts(void)
{
  config2 page2 = {};
  page2.strokes = FOUR_STROKE;
  page2.injLayout = INJ_PAIRED;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT16(360U, calculateMaxInjAngle(2, page2));
}

static void test_calculateMaxInjAngle_two_stroke_default_two_squirts(void)
{
  config2 page2 = {};
  page2.strokes = TWO_STROKE;
  page2.injLayout = INJ_PAIRED;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT16(180U, calculateMaxInjAngle(2, page2));
}

static void test_calculateMaxInjAngle_three_cyl_sequential_two_stroke_special(void)
{
  config2 page2 = {};
  page2.nCylinders = 3;
  page2.strokes = TWO_STROKE;
  page2.injLayout = INJ_SEQUENTIAL;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT16(360U, calculateMaxInjAngle(1, page2));
}

static void test_calculateMaxInjAngle_three_cyl_sequential_four_stroke_special(void)
{
  config2 page2 = {};
  page2.nCylinders = 3;
  page2.strokes = FOUR_STROKE;
  page2.injLayout = INJ_SEQUENTIAL;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT16(720U, calculateMaxInjAngle(1, page2));
}

static void test_calculateMaxInjAngle_three_cyl_port_paired_four_stroke_special(void)
{
  config2 page2 = {};
  page2.nCylinders = 3;
  page2.strokes = FOUR_STROKE;
  page2.injLayout = INJ_PAIRED;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT16(360U, calculateMaxInjAngle(2, page2));
}

static void test_calculateMaxInjAngle_four_stroke_three_squirts_tracks_720(void)
{
  config2 page2 = {};
  page2.strokes = FOUR_STROKE;
  page2.injLayout = INJ_PAIRED;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT16(240U, calculateMaxInjAngle(3, page2));
}

static void test_calculateMaxInjAngle_four_stroke_five_squirts_tracks_720(void)
{
  config2 page2 = {};
  page2.strokes = FOUR_STROKE;
  page2.injLayout = INJ_PAIRED;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT16(144U, calculateMaxInjAngle(5, page2));
}

static void assert_calcNumPrimaryInjectors_nonsequential(config2 &page2)
{
    page2.nCylinders = 0;
    TEST_ASSERT_EQUAL(1, calcNumPrimaryInjectors(page2));
    page2.nCylinders = 2;
    TEST_ASSERT_EQUAL(2, calcNumPrimaryInjectors(page2));
    page2.nCylinders = 5;
    TEST_ASSERT_EQUAL(INJ_CHANNELS>=5 ? 5 : INJ_CHANNELS, calcNumPrimaryInjectors(page2));
    page2.nCylinders = 6;
    TEST_ASSERT_EQUAL(3, calcNumPrimaryInjectors(page2));
}

static void test_calcNumPrimaryInjectors_nonsequential(void)
{
    config2 page2 = {};
    page2.injLayout = INJ_PAIRED;
    assert_calcNumPrimaryInjectors_nonsequential(page2);
    page2.injLayout = INJ_SEMISEQUENTIAL;
    assert_calcNumPrimaryInjectors_nonsequential(page2);
    page2.injLayout = INJ_BANKED;
    assert_calcNumPrimaryInjectors_nonsequential(page2);
}

static void test_calcNumPrimaryInjectors_sequential(void)
{
    config2 page2 = {};
    page2.injLayout = INJ_SEQUENTIAL;
    
    page2.nCylinders = 0;
    TEST_ASSERT_EQUAL(1, calcNumPrimaryInjectors(page2));
    
    page2.nCylinders = 2;
    TEST_ASSERT_EQUAL(2, calcNumPrimaryInjectors(page2));
    
    page2.nCylinders = 5;
    TEST_ASSERT_EQUAL(INJ_CHANNELS>=5 ? 5 : INJ_CHANNELS, calcNumPrimaryInjectors(page2));

    page2.nCylinders = 6;
    TEST_ASSERT_EQUAL(INJ_CHANNELS>=6 ? 6 : 4, calcNumPrimaryInjectors(page2));

    page2.nCylinders = INJ_CHANNELS+1;
    TEST_ASSERT_EQUAL(INJ_CHANNELS, calcNumPrimaryInjectors(page2));
}

static void test_calcNumSecondaryInjectors_stagingdisabled(void)
{
    config2 page2 = {};
    config10 page10 = {};

    page10.stagingEnabled = false;
    TEST_ASSERT_EQUAL(0, calcNumSecondaryInjectors(INJ_CHANNELS/2, page2, page10));
    TEST_ASSERT_FALSE(page10.stagingEnabled);
}

static void test_calcNumSecondaryInjectors_stagingenabled_nospareinjectors(void)
{
    config2 page2 = {};
    config10 page10 = {};

    page10.stagingEnabled = true;
    TEST_ASSERT_EQUAL(0, calcNumSecondaryInjectors(INJ_CHANNELS, page2, page10));
    TEST_ASSERT_FALSE(page10.stagingEnabled);
}

static void test_calcNumSecondaryInjectors_stagingenabled_mirrorprimary(void)
{
    config2 page2 = {};
    config10 page10 = {};

    page10.stagingEnabled = true;
    TEST_ASSERT_EQUAL(INJ_CHANNELS/2, calcNumSecondaryInjectors(INJ_CHANNELS/2, page2, page10));
    TEST_ASSERT_TRUE(page10.stagingEnabled);
}

static void test_calcNumSecondaryInjectors_stagingenabled_1spare(void)
{
    config2 page2 = {};
    config10 page10 = {};

    page10.stagingEnabled = true;
    TEST_ASSERT_EQUAL(1, calcNumSecondaryInjectors(INJ_CHANNELS-1, page2, page10));
    TEST_ASSERT_TRUE(page10.stagingEnabled);
}

void testFuelController(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_calulateNumSquirts_default_no_divider);
    RUN_TEST_P(test_calulateNumSquirts_divider_calculation);
    RUN_TEST_P(test_calulateNumSquirts_sequential_four_stroke_override);
    RUN_TEST_P(test_calulateNumSquirts_sequential_two_stroke_uses_divider);
    RUN_TEST_P(test_calulateNumSquirts_three_cyl_port_semisequential_force_two);
    RUN_TEST_P(test_calulateNumSquirts_three_cyl_port_paired_force_two);
    RUN_TEST_P(test_calulateNumSquirts_three_cyl_tbody_semisequential_uses_divider);
    RUN_TEST_P(test_calulateNumSquirts_clamps_minimum_to_one);
    RUN_TEST_P(test_calculateMaxInjAngle_four_stroke_default_two_squirts);
    RUN_TEST_P(test_calculateMaxInjAngle_two_stroke_default_two_squirts);
    RUN_TEST_P(test_calculateMaxInjAngle_three_cyl_sequential_two_stroke_special);
    RUN_TEST_P(test_calculateMaxInjAngle_three_cyl_sequential_four_stroke_special);
    RUN_TEST_P(test_calculateMaxInjAngle_three_cyl_port_paired_four_stroke_special);
    RUN_TEST_P(test_calculateMaxInjAngle_four_stroke_three_squirts_tracks_720);
    RUN_TEST_P(test_calculateMaxInjAngle_four_stroke_five_squirts_tracks_720);
    RUN_TEST_P(test_calcNumPrimaryInjectors_nonsequential);
    RUN_TEST_P(test_calcNumPrimaryInjectors_sequential);
    RUN_TEST_P(test_calcNumSecondaryInjectors_stagingdisabled);
    RUN_TEST_P(test_calcNumSecondaryInjectors_stagingenabled_nospareinjectors);
    RUN_TEST_P(test_calcNumSecondaryInjectors_stagingenabled_mirrorprimary);
    RUN_TEST_P(test_calcNumSecondaryInjectors_stagingenabled_1spare);
  }
}