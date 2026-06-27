#include "../test_utils.h"
#include "scheduler_fuel_controller.h"

extern uint8_t calulateNumSquirts(const config2 &page2);
extern uint16_t calculateMaxInjAngle(uint8_t squirtsPerCycle, const config2 &page2);
extern uint8_t calcNumPrimaryInjectors(config2 &page2);
extern uint8_t calcNumSecondaryInjectors(uint16_t primary, const config2 &page2, config10 &page10);
extern uint16_t calcAngularCylinderSeparation(const statuses &current, const config2 &page2);
extern uint16_t getOddfireAngle(const config2 &page2, uint8_t channel);
extern bool useEvenFire(const config2 &page2);
extern uint16_t getEvenFireAngle(const statuses &current, const config2 &page2, uint8_t channel);
extern uint16_t calcScheduleAngle(const statuses &current, const config2 &page2, uint8_t channel);

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

static void test_calcAngularCylinderSeparation(void)
{
  statuses current = {};
  config2 page2 = {};
  page2.injLayout = INJ_SEQUENTIAL;

  current.injOutputs.primary = 1;
  page2.nCylinders = 1;
  CRANK_ANGLE_MAX_INJ = 360;
  TEST_ASSERT_EQUAL(0, calcAngularCylinderSeparation(current, page2));

  current.injOutputs.primary = 2;
  CRANK_ANGLE_MAX_INJ = 360;
  TEST_ASSERT_EQUAL(180, calcAngularCylinderSeparation(current, page2));
}

static void assert_calcAngularCylinderSeparation_specialcases(const statuses &current, config2 &page2)
{
  page2.nCylinders = 5;
  TEST_ASSERT_EQUAL(360/5, calcAngularCylinderSeparation(current, page2));
  page2.nCylinders = 6;
  TEST_ASSERT_EQUAL(720/6, calcAngularCylinderSeparation(current, page2));
  
}
static void test_calcAngularCylinderSeparation_specialcases(void)
{
  statuses current = {};
  config2 page2 = {};
  current.injOutputs.primary = 1;
  page2.strokes = FOUR_STROKE;

  page2.injLayout = INJ_SEMISEQUENTIAL;
  assert_calcAngularCylinderSeparation_specialcases(current, page2);
  page2.injLayout = INJ_PAIRED;
  assert_calcAngularCylinderSeparation_specialcases(current, page2);
  page2.strokes = TWO_STROKE;
  page2.injLayout = INJ_SEQUENTIAL;
  assert_calcAngularCylinderSeparation_specialcases(current, page2);
}

static config2 setup_oddfire(void)
{
  config2 page2 = {};
  page2.engineType = ODD_FIRE;
  page2.injTiming = true;
  page2.nCylinders = 2;
  page2.oddfire2 = 5;
  page2.oddfire3 = 15;
  page2.oddfire4 = 25;
  return page2;
}

static void test_getOddfireAngle(void)
{
  config2 page2 = setup_oddfire();
  TEST_ASSERT_EQUAL(0, getOddfireAngle(page2, 0));
  TEST_ASSERT_EQUAL(0, getOddfireAngle(page2, 1));
  TEST_ASSERT_EQUAL(0, getOddfireAngle(page2, 5));

  TEST_ASSERT_EQUAL(page2.oddfire2, getOddfireAngle(page2, 2));
  TEST_ASSERT_EQUAL(page2.oddfire3, getOddfireAngle(page2, 3));
  TEST_ASSERT_EQUAL(page2.oddfire4, getOddfireAngle(page2, 4));
}

static config2 setup_evenfire(void)
{
  config2 page2 = {};
  page2.engineType = EVEN_FIRE;
  page2.nCylinders = 2;
  return page2;
}

static void test_useEvenFire(void)
{
  config2 page2 = setup_evenfire();
  TEST_ASSERT_TRUE(useEvenFire(page2));
  page2 = setup_oddfire();
  TEST_ASSERT_FALSE(useEvenFire(page2));
  page2.engineType = EVEN_FIRE;
  TEST_ASSERT_TRUE(useEvenFire(page2));
}

static void test_getEvenFireAngle(void)
{
  statuses current = {};
  config2 page2 = {};
  CRANK_ANGLE_MAX_INJ = 360;

  page2.nCylinders = 2;
  current.injOutputs.primary = 4;
  TEST_ASSERT_EQUAL(180, getEvenFireAngle(current, page2, 2));
  TEST_ASSERT_EQUAL(270, getEvenFireAngle(current, page2, 4));

  page2.nCylinders = 4;
  current.injOutputs.primary = 4;
  TEST_ASSERT_EQUAL(90, getEvenFireAngle(current, page2, 2));
}

static void test_calcScheduleAngle(void)
{
  statuses current = {};
  config2 page2 = setup_evenfire();

  CRANK_ANGLE_MAX_INJ = 360;
  current.injOutputs.primary = 8;

  page2.injTiming = false;
  TEST_ASSERT_EQUAL(0, calcScheduleAngle(current, page2, 4));
  page2.injTiming = true;
  TEST_ASSERT_NOT_EQUAL(0, calcScheduleAngle(current, page2, 4));

  // Test injector limits clamp
  TEST_ASSERT_UINT16_WITHIN(CRANK_ANGLE_MAX_INJ-1, 0, calcScheduleAngle(current, page2, 16));

  // Test oddfire path
  TEST_ASSERT_NOT_EQUAL(0, calcScheduleAngle(current, setup_oddfire(), 2));
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
    RUN_TEST_P(test_calcAngularCylinderSeparation);
    RUN_TEST_P(test_calcAngularCylinderSeparation_specialcases);
    RUN_TEST_P(test_getOddfireAngle);
    RUN_TEST_P(test_useEvenFire);
    RUN_TEST_P(test_getEvenFireAngle);
    RUN_TEST_P(test_calcScheduleAngle);
  }
}