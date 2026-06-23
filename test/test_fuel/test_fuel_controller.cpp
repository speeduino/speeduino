#include "../test_utils.h"
#include "scheduler_fuel_controller.h"

extern uint8_t calulateNumSquirts(const config2 &page2);

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
  }
}