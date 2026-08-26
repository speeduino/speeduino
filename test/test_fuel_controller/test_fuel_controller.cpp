#include "../test_utils.h"
#include "scheduler_fuel_controller.h"
#include "units.h"
#include "../channel_test_helpers.h"
#include "../fake_decoder_status.h"
#include "decoder_builder.h"

extern uint8_t calulateNumSquirts(const statuses &current, const config2 &page2);
extern uint16_t calculateMaxInjAngle(const statuses &current, const config2 &page2);
extern uint8_t calcNumPrimaryInjectors(const statuses &current, const config2 &page2);
extern uint8_t calcNumSecondaryInjectors(uint16_t primary, const config2 &page2, const config10 &page10);
extern uint16_t calcAngularCylinderSeparation(const statuses &current, const config2 &page2);
extern uint16_t getOddfireAngle(const config2 &page2, uint8_t channel);
extern bool useEvenFire(const config2 &page2);
extern uint16_t getEvenFireAngle(const statuses &current, const config2 &page2, uint8_t channel);
extern uint16_t calcScheduleAngle(const statuses &current, const config2 &page2, uint8_t channel);
extern bool isAnyFuelScheduleRunning(void);
extern uint16_t lookupInjectorAngle(const statuses &current);
extern table2D_u8_u16_4 injectorAngleTable;
extern void setFuelChannelSchedule(FuelSchedule &schedule, uint8_t channel, uint16_t crankAngle, byte injChannelMask, uint16_t injAngle, injectorAngleCalcCache *pCache);
extern table2D_u8_u8_4 PrimingPulseTable;
extern uint16_t setFuelChannelSchedules(uint16_t crankAngle, byte injChannelMask, uint16_t injAngle);
extern bool changeToFullSequentialInjection(const statuses &current, const config2 &page2);
extern bool changeToSemiSequentialInjection(const statuses &current, const config2 &page2);
extern void validateInjectionSetup(config2 &page2, config6 &page6);

static void test_calulateNumSquirts_default_no_divider(void)
{
  config2 page2 = {};
  page2.divider = 0;
  page2.nCylinders = 4;
  page2.strokes = TWO_STROKE;
  page2.injType = INJ_TYPE_PORT;

  statuses current = {};
  current.injLayout = INJ_PAIRED;

  TEST_ASSERT_EQUAL_UINT8(2U, calulateNumSquirts(current, page2));
}

static void test_calulateNumSquirts_divider_calculation(void)
{
  config2 page2 = {};
  page2.divider = 2;
  page2.nCylinders = 8;
  page2.strokes = TWO_STROKE;
  page2.injType = INJ_TYPE_PORT;

  statuses current = {};
  current.injLayout = INJ_PAIRED;

  TEST_ASSERT_EQUAL_UINT8(4U, calulateNumSquirts(current, page2));
}

static void test_calulateNumSquirts_sequential_four_stroke_override(void)
{
  config2 page2 = {};
  page2.divider = 2;
  page2.nCylinders = 4;
  page2.strokes = FOUR_STROKE;
  page2.injType = INJ_TYPE_PORT;

  statuses current = {};
  current.injLayout = INJ_SEQUENTIAL;
  
  TEST_ASSERT_EQUAL_UINT8(1U, calulateNumSquirts(current, page2));
}

static void test_calulateNumSquirts_sequential_two_stroke_uses_divider(void)
{
  config2 page2 = {};
  page2.divider = 2;
  page2.nCylinders = 4;
  page2.strokes = TWO_STROKE;
  page2.injType = INJ_TYPE_PORT;

  statuses current = {};
  current.injLayout = INJ_SEQUENTIAL;

  TEST_ASSERT_EQUAL_UINT8(2U, calulateNumSquirts(current, page2));
}

static void test_calulateNumSquirts_three_cyl_port_semisequential_force_two(void)
{
  config2 page2 = {};
  page2.divider = 1;
  page2.nCylinders = 3;
  page2.strokes = FOUR_STROKE;
  page2.injType = INJ_TYPE_PORT;

  statuses current = {};
  current.injLayout = INJ_SEMISEQUENTIAL;

  TEST_ASSERT_EQUAL_UINT8(2U, calulateNumSquirts(current, page2));
}

static void test_calulateNumSquirts_three_cyl_port_paired_force_two(void)
{
  config2 page2 = {};
  page2.divider = 1;
  page2.nCylinders = 3;
  page2.strokes = FOUR_STROKE;
  page2.injType = INJ_TYPE_PORT;

  statuses current = {};
  current.injLayout = INJ_PAIRED;

  TEST_ASSERT_EQUAL_UINT8(2U, calulateNumSquirts(current, page2));
}

static void test_calulateNumSquirts_three_cyl_tbody_semisequential_uses_divider(void)
{
  config2 page2 = {};
  page2.divider = 1;
  page2.nCylinders = 3;
  page2.strokes = FOUR_STROKE;
  page2.injType = INJ_TYPE_TBODY;

  statuses current = {};
  current.injLayout = INJ_SEMISEQUENTIAL;

  TEST_ASSERT_EQUAL_UINT8(3U, calulateNumSquirts(current, page2));
}

static void test_calulateNumSquirts_clamps_minimum_to_one(void)
{
  config2 page2 = {};
  page2.divider = 10;
  page2.nCylinders = 1;
  page2.strokes = TWO_STROKE;
  page2.injType = INJ_TYPE_PORT;

  statuses current = {};
  current.injLayout = INJ_PAIRED;

  TEST_ASSERT_EQUAL_UINT8(1U, calulateNumSquirts(current, page2));
}

static void test_calculateMaxInjAngle_four_stroke_default_two_squirts(void)
{
  config2 page2 = {};
  statuses current = {};
  
  current.nSquirts = 2;
  page2.strokes = FOUR_STROKE;
  page2.injLayout = INJ_PAIRED;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT16(360U, calculateMaxInjAngle(current, page2));
}

static void test_calculateMaxInjAngle_two_stroke_default_two_squirts(void)
{
  config2 page2 = {};
  statuses current = {};
  
  current.nSquirts = 2;
  page2.strokes = TWO_STROKE;
  page2.injLayout = INJ_PAIRED;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT16(180U, calculateMaxInjAngle(current, page2));
}

static void test_calculateMaxInjAngle_three_cyl_sequential_two_stroke_special(void)
{
  config2 page2 = {};
  statuses current = {};
  
  current.nSquirts = 1;
  current.injLayout = INJ_SEQUENTIAL;

  page2.nCylinders = 3;
  page2.strokes = TWO_STROKE;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT16(360U, calculateMaxInjAngle(current, page2));
}

static void test_calculateMaxInjAngle_three_cyl_sequential_four_stroke_special(void)
{
  config2 page2 = {};
  statuses current = {};
  
  current.nSquirts = 1;
  current.injLayout = INJ_SEQUENTIAL;

  page2.nCylinders = 3;
  page2.strokes = FOUR_STROKE;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT16(720U, calculateMaxInjAngle(current, page2));
}

static void test_calculateMaxInjAngle_three_cyl_port_paired_four_stroke_special(void)
{
  config2 page2 = {};
  statuses current = {};
  
  current.nSquirts = 2;
  page2.nCylinders = 3;
  page2.strokes = FOUR_STROKE;
  page2.injLayout = INJ_PAIRED;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT16(360U, calculateMaxInjAngle(current, page2));
}

static void test_calculateMaxInjAngle_four_stroke_three_squirts_tracks_720(void)
{
  config2 page2 = {};
  statuses current = {};
  
  current.nSquirts = 3;
  page2.strokes = FOUR_STROKE;
  page2.injLayout = INJ_PAIRED;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT16(240U, calculateMaxInjAngle(current, page2));
}

static void test_calculateMaxInjAngle_four_stroke_five_squirts_tracks_720(void)
{
  config2 page2 = {};
  statuses current = {};
  
  current.nSquirts = 5;
  page2.strokes = FOUR_STROKE;
  page2.injLayout = INJ_PAIRED;
  page2.injType = INJ_TYPE_PORT;

  TEST_ASSERT_EQUAL_UINT16(144U, calculateMaxInjAngle(current, page2));
}

static void assert_calcNumPrimaryInjectors_nonsequential(config2 &page2)
{
  statuses current = {};
  current.injLayout = page2.injLayout;

  page2.nCylinders = 0;
  TEST_ASSERT_EQUAL(1, calcNumPrimaryInjectors(current, page2));
  page2.nCylinders = 2;
  TEST_ASSERT_EQUAL(2, calcNumPrimaryInjectors(current, page2));
  page2.nCylinders = 5;
  TEST_ASSERT_EQUAL(INJ_CHANNELS>=5 ? 5 : INJ_CHANNELS, calcNumPrimaryInjectors(current, page2));
  page2.nCylinders = 6;
  TEST_ASSERT_EQUAL(3, calcNumPrimaryInjectors(current, page2));
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
    statuses current = {};
    current.injLayout = INJ_SEQUENTIAL;
    
    page2.nCylinders = 0;
    TEST_ASSERT_EQUAL(1, calcNumPrimaryInjectors(current, page2));
    
    page2.nCylinders = 2;
    TEST_ASSERT_EQUAL(2, calcNumPrimaryInjectors(current, page2));
    
    page2.nCylinders = 5;
    TEST_ASSERT_EQUAL(INJ_CHANNELS>=5 ? 5 : INJ_CHANNELS, calcNumPrimaryInjectors(current, page2));

    page2.nCylinders = 6;
    TEST_ASSERT_EQUAL(INJ_CHANNELS>=6 ? 6 : 4, calcNumPrimaryInjectors(current, page2));

    page2.nCylinders = INJ_CHANNELS+1;
    TEST_ASSERT_EQUAL(INJ_CHANNELS, calcNumPrimaryInjectors(current, page2));
}

static void test_calcNumSecondaryInjectors_stagingdisabled(void)
{
    config2 page2 = {};
    config10 page10 = {};

    page10.stagingEnabled = false;
    TEST_ASSERT_EQUAL(0, calcNumSecondaryInjectors(INJ_CHANNELS/2, page2, page10));
}

static void test_calcNumSecondaryInjectors_stagingenabled_nospareinjectors(void)
{
    config2 page2 = {};
    config10 page10 = {};

    page10.stagingEnabled = true;
    TEST_ASSERT_EQUAL(0, calcNumSecondaryInjectors(INJ_CHANNELS, page2, page10));

}

static void test_calcNumSecondaryInjectors_stagingenabled_mirrorprimary(void)
{
    config2 page2 = {};
    config10 page10 = {};

    page10.stagingEnabled = true;
    TEST_ASSERT_EQUAL(INJ_CHANNELS/2, calcNumSecondaryInjectors(INJ_CHANNELS/2, page2, page10));
}

static void test_calcNumSecondaryInjectors_stagingenabled_1spare(void)
{
    config2 page2 = {};
    config10 page10 = {};

    page10.stagingEnabled = true;
    TEST_ASSERT_EQUAL(1, calcNumSecondaryInjectors(INJ_CHANNELS-1, page2, page10));
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
  page2.oddfire[0] = 5;
  page2.oddfire[1] = 15;
  page2.oddfire[2] = 25;
  return page2;
}

static void test_getOddfireAngle(void)
{
  config2 page2 = setup_oddfire();
  TEST_ASSERT_EQUAL(0, getOddfireAngle(page2, 0));
  TEST_ASSERT_EQUAL(0, getOddfireAngle(page2, 1));
  TEST_ASSERT_EQUAL(0, getOddfireAngle(page2, 5));

  TEST_ASSERT_EQUAL(page2.oddfire[0], getOddfireAngle(page2, 2));
  TEST_ASSERT_EQUAL(page2.oddfire[1], getOddfireAngle(page2, 3));
  TEST_ASSERT_EQUAL(page2.oddfire[2], getOddfireAngle(page2, 4));
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

static void set_all_schedules_off(void)
{
    RUNIF_INJCHANNEL1( { fuelSchedule1._status = ScheduleStatus::OFF; }, {});
    RUNIF_INJCHANNEL2( { fuelSchedule2._status = ScheduleStatus::OFF; }, {});
    RUNIF_INJCHANNEL3( { fuelSchedule3._status = ScheduleStatus::OFF; }, {});
    RUNIF_INJCHANNEL4( { fuelSchedule4._status = ScheduleStatus::OFF; }, {});
    RUNIF_INJCHANNEL5( { fuelSchedule5._status = ScheduleStatus::OFF; }, {});
    RUNIF_INJCHANNEL6( { fuelSchedule6._status = ScheduleStatus::OFF; }, {});
    RUNIF_INJCHANNEL7( { fuelSchedule7._status = ScheduleStatus::OFF; }, {});
    RUNIF_INJCHANNEL8( { fuelSchedule8._status = ScheduleStatus::OFF; }, {});
}

static void test_isAnyFuelScheduleRunning(void)
{
  set_all_schedules_off();
  TEST_ASSERT_FALSE(isAnyFuelScheduleRunning());

  set_all_schedules_off();
  RUNIF_INJCHANNEL1( { fuelSchedule1._status = ScheduleStatus::RUNNING; }, {});
  RUNIF_INJCHANNEL1( { TEST_ASSERT_TRUE(isAnyFuelScheduleRunning()); }, {});
  
  set_all_schedules_off();
  RUNIF_INJCHANNEL2( { fuelSchedule2._status = ScheduleStatus::RUNNING; }, {});
  RUNIF_INJCHANNEL2( { TEST_ASSERT_TRUE(isAnyFuelScheduleRunning()); }, { TEST_ASSERT_FALSE(isAnyFuelScheduleRunning()); });

  set_all_schedules_off();
  RUNIF_INJCHANNEL3( { fuelSchedule3._status = ScheduleStatus::RUNNING; }, {});
  RUNIF_INJCHANNEL3( { TEST_ASSERT_TRUE(isAnyFuelScheduleRunning()); }, { TEST_ASSERT_FALSE(isAnyFuelScheduleRunning()); });

  set_all_schedules_off();
  RUNIF_INJCHANNEL4( { fuelSchedule4._status = ScheduleStatus::RUNNING; }, {});
  RUNIF_INJCHANNEL4( { TEST_ASSERT_TRUE(isAnyFuelScheduleRunning()); }, { TEST_ASSERT_FALSE(isAnyFuelScheduleRunning()); });

  set_all_schedules_off();
  RUNIF_INJCHANNEL5( { fuelSchedule5._status = ScheduleStatus::RUNNING; }, {});
  RUNIF_INJCHANNEL5( { TEST_ASSERT_TRUE(isAnyFuelScheduleRunning()); }, { TEST_ASSERT_FALSE(isAnyFuelScheduleRunning()); });

  set_all_schedules_off();
  RUNIF_INJCHANNEL6( { fuelSchedule6._status = ScheduleStatus::RUNNING; }, {});
  RUNIF_INJCHANNEL6( { TEST_ASSERT_TRUE(isAnyFuelScheduleRunning()); }, { TEST_ASSERT_FALSE(isAnyFuelScheduleRunning()); });

  set_all_schedules_off();
  RUNIF_INJCHANNEL7( { fuelSchedule7._status = ScheduleStatus::RUNNING; }, {});
  RUNIF_INJCHANNEL7( { TEST_ASSERT_TRUE(isAnyFuelScheduleRunning()); }, { TEST_ASSERT_FALSE(isAnyFuelScheduleRunning()); });

  set_all_schedules_off();
  RUNIF_INJCHANNEL8( { fuelSchedule8._status = ScheduleStatus::RUNNING; }, {});
  RUNIF_INJCHANNEL8( { TEST_ASSERT_TRUE(isAnyFuelScheduleRunning()); }, { TEST_ASSERT_FALSE(isAnyFuelScheduleRunning()); });
}

using raw_counter_t = std::remove_reference<FuelSchedule::counter_t>::type;
using raw_compare_t = std::remove_reference<FuelSchedule::compare_t>::type;

static void setup_setFuelChannelSchedule(FuelSchedule &schedule)
{
  CRANK_ANGLE_MAX_INJ = 720U;
  setAngleConverterRevolutionTime(1000000UL);

  schedule._counter = 5U;
  schedule.pw = 1000U;
  schedule.channelDegrees = 0U;
}

static void test_setFuelChannelSchedule_ignores_zero_pw(void)
{
  raw_counter_t counter = {0};
  raw_compare_t compare = {0};
  FuelSchedule schedule(counter, compare);
  setup_setFuelChannelSchedule(schedule);

  injectorAngleCalcCache cache = {};
  schedule.pw = 0;
  setFuelChannelSchedule(schedule, UINT8_C(1), 100U, 0xFF, 180U, &cache);

  TEST_ASSERT_EQUAL(OFF, schedule._status);
  TEST_ASSERT_EQUAL(0U, schedule._duration);
  TEST_ASSERT_EQUAL(0U, schedule._compare);
}

static void test_setFuelChannelSchedule_ignores_zero_timeout(void)
{
  raw_counter_t counter = {0};
  raw_compare_t compare = {0};
  FuelSchedule schedule(counter, compare);
  setup_setFuelChannelSchedule(schedule);

  injectorAngleCalcCache cache = {};
  setFuelChannelSchedule(schedule, UINT8_C(1), 0U, 0xFF, 0U, &cache);

  TEST_ASSERT_EQUAL(OFF, schedule._status);
  TEST_ASSERT_EQUAL(0U, schedule._duration);
  TEST_ASSERT_EQUAL(0U, schedule._compare);
}

static void test_setFuelChannelSchedule_ignores_disabled_channel(void)
{
  raw_counter_t counter = {0};
  raw_compare_t compare = {0};
  FuelSchedule schedule(counter, compare);
  setup_setFuelChannelSchedule(schedule);

  injectorAngleCalcCache cache = {};
  setFuelChannelSchedule(schedule, UINT8_C(1), 0U, 0U, 180U, &cache);

  TEST_ASSERT_EQUAL(OFF, schedule._status);
  TEST_ASSERT_EQUAL(0U, schedule._duration);
  TEST_ASSERT_EQUAL(0U, schedule._compare);
}

static void test_setFuelChannelSchedule_starts_pending_when_enabled(void)
{
  raw_counter_t counter = {5U};
  raw_compare_t compare = {0};
  FuelSchedule schedule(counter, compare);
  setup_setFuelChannelSchedule(schedule);
  
  injectorAngleCalcCache cache = {};
  setFuelChannelSchedule(schedule, UINT8_C(1), 300U, 0xFF, 355U, &cache);

  TEST_ASSERT_EQUAL(PENDING, schedule._status);
  TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(1000U), schedule._duration);
  TEST_ASSERT_GREATER_THAN(0U, schedule._compare);
}

static void test_lookupInjectorAngle_clamp_max_inj(void)
{
  statuses current = {};
  populate_2dtable(&injectorAngleTable, (uint16_t)720, RPM_COARSE.toRaw(1000));

  current.setRpm(1000);
  CRANK_ANGLE_MAX_INJ = injectorAngleTable.values[0]/2U;

  TEST_ASSERT_LESS_THAN(injectorAngleTable.values[0], CRANK_ANGLE_MAX_INJ);
  TEST_ASSERT_EQUAL(CRANK_ANGLE_MAX_INJ, lookupInjectorAngle(current));
}

constexpr uint8_t PRIMING_PULSE_WIDTH = 5;

static void assert_isPriming(FuelSchedule &schedule, bool priming)
{
  if (priming)
  {
    TEST_ASSERT_EQUAL(PENDING, schedule._status);
    TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(PRIMING_PULSE_WIDTH*500U), schedule._duration);
  }
  else
  {
    TEST_ASSERT_EQUAL(OFF, schedule._status);
  }
}

static void test_beginInjectorPriming_floodclear(void)
{
  stopFuelSchedulers();

  for (uint8_t channel = 0; channel<INJ_CHANNELS; ++channel)
  {
    statuses current = {};
    config4 page4 = {};
    page4.floodClear = 90;
    current.TPS = page4.floodClear+1;
    set_all_schedules_off();

    current.injOutputs.primary = channel;
    beginInjectorPriming(current, page4);

    RUNIF_INJCHANNEL1( { assert_isPriming(fuelSchedule1, false); }, {});
    RUNIF_INJCHANNEL2( { assert_isPriming(fuelSchedule2, false); }, {});
    RUNIF_INJCHANNEL3( { assert_isPriming(fuelSchedule3, false); }, {});
    RUNIF_INJCHANNEL4( { assert_isPriming(fuelSchedule4, false); }, {});
    RUNIF_INJCHANNEL5( { assert_isPriming(fuelSchedule5, false); }, {});
    RUNIF_INJCHANNEL6( { assert_isPriming(fuelSchedule6, false); }, {});
    RUNIF_INJCHANNEL7( { assert_isPriming(fuelSchedule7, false); }, {});
    RUNIF_INJCHANNEL8( { assert_isPriming(fuelSchedule8, false); }, {});
  }
}

static void test_beginInjectorPriming(void)
{
  constexpr int16_t COOLANT_TEMP = 200;
  stopFuelSchedulers();
  populate_2dtable(&PrimingPulseTable, PRIMING_PULSE_WIDTH, temperatureAddOffset(COOLANT_TEMP));

  for (uint8_t channel = 0; channel<INJ_CHANNELS; ++channel)
  {
    statuses current = {};
    config4 page4 = {};
    page4.floodClear = 100;
    current.TPS = page4.floodClear/2;
    current.coolant = COOLANT_TEMP;
    set_all_schedules_off();

    current.injOutputs.primary = channel;
    beginInjectorPriming(current, page4);

    RUNIF_INJCHANNEL1( { assert_isPriming(fuelSchedule1, channel>=1); }, {});
    RUNIF_INJCHANNEL2( { assert_isPriming(fuelSchedule2, channel>=2); }, {});
    RUNIF_INJCHANNEL3( { assert_isPriming(fuelSchedule3, channel>=3); }, {});
    RUNIF_INJCHANNEL4( { assert_isPriming(fuelSchedule4, channel>=4); }, {});
    RUNIF_INJCHANNEL5( { assert_isPriming(fuelSchedule5, channel>=5); }, {});
    RUNIF_INJCHANNEL6( { assert_isPriming(fuelSchedule6, channel>=6); }, {});
    RUNIF_INJCHANNEL7( { assert_isPriming(fuelSchedule7, channel>=7); }, {});
    RUNIF_INJCHANNEL8( { assert_isPriming(fuelSchedule8, channel>=8); }, {});
  }
}

static void test_setFuelChannelSchedules_returnsInjectionAngle(void)
{
  TEST_ASSERT_EQUAL(355U, setFuelChannelSchedules(0, 0, 355));
}

static void setup_changeToFullSequentialInjection(statuses &current, config2 &page2, uint8_t nCylinders)
{
  current.injLayout = INJ_SEMISEQUENTIAL;
  current.decoder = decoder_builder_t().setGetStatus(getFakeDecoderStatus).build();
  page2.nCylinders = nCylinders;
  page2.injLayout = INJ_SEQUENTIAL;
  fakeDecoderStatus.syncStatus=SyncStatus::Full;
}

static void test_changeToFullSequentialInjection(void)
{
  statuses current = {};
  config2 page2 = {};

  setup_changeToFullSequentialInjection(current, page2, 1);
  TEST_ASSERT_FALSE(changeToFullSequentialInjection(current, page2));
  setup_changeToFullSequentialInjection(current, page2, 2);
  TEST_ASSERT_FALSE(changeToFullSequentialInjection(current, page2));
  setup_changeToFullSequentialInjection(current, page2, 3);
  TEST_ASSERT_FALSE(changeToFullSequentialInjection(current, page2));
  setup_changeToFullSequentialInjection(current, page2, 4);
  TEST_ASSERT_TRUE(changeToFullSequentialInjection(current, page2));
  setup_changeToFullSequentialInjection(current, page2, 5);
  TEST_ASSERT_FALSE(changeToFullSequentialInjection(current, page2));
  setup_changeToFullSequentialInjection(current, page2, 6);
  TEST_ASSERT_TRUE(changeToFullSequentialInjection(current, page2));
  setup_changeToFullSequentialInjection(current, page2, 8);
  TEST_ASSERT_TRUE(changeToFullSequentialInjection(current, page2));

  setup_changeToFullSequentialInjection(current, page2, 4);
  page2.injLayout = INJ_SEMISEQUENTIAL;
  TEST_ASSERT_FALSE(changeToFullSequentialInjection(current, page2));

  setup_changeToFullSequentialInjection(current, page2, 4);
  fakeDecoderStatus.syncStatus=SyncStatus::Partial;
  TEST_ASSERT_FALSE(changeToFullSequentialInjection(current, page2));

  setup_changeToFullSequentialInjection(current, page2, 4);
  current.injLayout = INJ_SEQUENTIAL;
  TEST_ASSERT_FALSE(changeToFullSequentialInjection(current, page2));
}

static void setup_changeToSemiSequentialInjection(statuses &current, config2 &page2, uint8_t nCylinders)
{
  current.injLayout = INJ_SEQUENTIAL;
  current.decoder = decoder_builder_t().setGetStatus(getFakeDecoderStatus).build();
  page2.injLayout = INJ_SEQUENTIAL;
  page2.nCylinders = nCylinders;
  fakeDecoderStatus.syncStatus=SyncStatus::Partial;
}

static void test_changeToSemiSequentialInjection(void)
{
  config2 page2 = {};
  statuses current = {};

  setup_changeToSemiSequentialInjection(current, page2, 1);
  TEST_ASSERT_FALSE(changeToSemiSequentialInjection(current, page2));
  setup_changeToSemiSequentialInjection(current, page2, 2);
  TEST_ASSERT_FALSE(changeToSemiSequentialInjection(current, page2));
  setup_changeToSemiSequentialInjection(current, page2, 3);
  TEST_ASSERT_FALSE(changeToSemiSequentialInjection(current, page2));
  setup_changeToSemiSequentialInjection(current, page2, 4);
  TEST_ASSERT_TRUE(changeToSemiSequentialInjection(current, page2));
  setup_changeToSemiSequentialInjection(current, page2, 5);
  TEST_ASSERT_FALSE(changeToSemiSequentialInjection(current, page2));
  setup_changeToSemiSequentialInjection(current, page2, 6);
  TEST_ASSERT_TRUE(changeToSemiSequentialInjection(current, page2));
  setup_changeToSemiSequentialInjection(current, page2, 8);
  TEST_ASSERT_TRUE(changeToSemiSequentialInjection(current, page2));

  setup_changeToSemiSequentialInjection(current, page2, 4);
  page2.injLayout = INJ_SEMISEQUENTIAL;
  TEST_ASSERT_FALSE(changeToSemiSequentialInjection(current, page2));

  setup_changeToSemiSequentialInjection(current, page2, 4);
  fakeDecoderStatus.syncStatus=SyncStatus::Full;
  TEST_ASSERT_FALSE(changeToSemiSequentialInjection(current, page2));

  setup_changeToSemiSequentialInjection(current, page2, 4);
  current.injLayout = INJ_SEMISEQUENTIAL;
  TEST_ASSERT_FALSE(changeToSemiSequentialInjection(current, page2));
}

static void assert_validateInjectionSetup_injLayout_semi(uint8_t nCylinders, uint8_t expectedLayout)
{
  config2 page2 = {};
  config6 page6 = {};
  page2.nInjectors = nCylinders;
  page2.injLayout = INJ_SEMISEQUENTIAL;
  page2.nCylinders = nCylinders;
  validateInjectionSetup(page2, page6);
  char buffer[100];
  snprintf(buffer, sizeof(buffer), "nCylinders=%" PRIu8 ", channels=%" PRIu8, nCylinders, (uint8_t)INJ_CHANNELS);
  TEST_ASSERT_EQUAL_MESSAGE(expectedLayout, page2.injLayout, buffer);
}

static void assert_validateInjectionSetup_injLayout_semi_notenoughinjectors(uint8_t nCylinders)
{
  config2 page2 = {};
  config6 page6 = {};
  page2.nInjectors = nCylinders-1;
  page2.nCylinders = nCylinders;
  page2.injLayout = INJ_SEMISEQUENTIAL;
  validateInjectionSetup(page2, page6);
  TEST_ASSERT_EQUAL(INJ_PAIRED, page2.injLayout);
}

static void test_validateInjectionSetup_nInjector_clamp(void)
{
  config2 page2 = {};
  config6 page6 = {};

  page2.nInjectors = 0;  
  validateInjectionSetup(page2, page6);
  TEST_ASSERT_EQUAL(1, page2.nInjectors);

  page2.nInjectors = INJ_CHANNELS - 1;
  validateInjectionSetup(page2, page6);
  TEST_ASSERT_EQUAL(INJ_CHANNELS - 1, page2.nInjectors);

  page2.nInjectors = INJ_CHANNELS;
  validateInjectionSetup(page2, page6);
  TEST_ASSERT_EQUAL(INJ_CHANNELS, page2.nInjectors);

  page2.nInjectors = INJ_CHANNELS + 1;
  validateInjectionSetup(page2, page6);
  TEST_ASSERT_EQUAL(INJ_CHANNELS, page2.nInjectors);
}

static void test_validateInjectionSetup_injLayout(void)
{
  config2 page2 = {};
  config6 page6 = {};

  page2.nInjectors = INJ_CHANNELS;
  page2.injLayout = INJ_SEQUENTIAL;

  page2.nCylinders = page2.nInjectors;
  validateInjectionSetup(page2, page6);
  TEST_ASSERT_EQUAL(INJ_SEQUENTIAL, page2.injLayout);

  page2.nCylinders = page2.nInjectors-1;
  validateInjectionSetup(page2, page6);
  TEST_ASSERT_EQUAL(INJ_SEQUENTIAL, page2.injLayout);

  page2.nCylinders = page2.nInjectors+1;
  validateInjectionSetup(page2, page6);
  TEST_ASSERT_EQUAL(INJ_PAIRED, page2.injLayout);

  assert_validateInjectionSetup_injLayout_semi(1, INJ_PAIRED);
  assert_validateInjectionSetup_injLayout_semi(2, INJ_PAIRED);
  assert_validateInjectionSetup_injLayout_semi(3, INJ_PAIRED);
  assert_validateInjectionSetup_injLayout_semi(4, INJ_CHANNELS>=4 ? INJ_SEMISEQUENTIAL : INJ_PAIRED);
  assert_validateInjectionSetup_injLayout_semi(5, INJ_CHANNELS>=5 ? INJ_SEMISEQUENTIAL : INJ_PAIRED);
  assert_validateInjectionSetup_injLayout_semi(6, INJ_CHANNELS>=6 ? INJ_SEMISEQUENTIAL : INJ_PAIRED);
  assert_validateInjectionSetup_injLayout_semi(8, INJ_CHANNELS>=8 ? INJ_SEMISEQUENTIAL : INJ_PAIRED);

  assert_validateInjectionSetup_injLayout_semi_notenoughinjectors(1);
  assert_validateInjectionSetup_injLayout_semi_notenoughinjectors(2);
  assert_validateInjectionSetup_injLayout_semi_notenoughinjectors(3);
  assert_validateInjectionSetup_injLayout_semi_notenoughinjectors(4);
  assert_validateInjectionSetup_injLayout_semi_notenoughinjectors(5);
  assert_validateInjectionSetup_injLayout_semi_notenoughinjectors(6);
  assert_validateInjectionSetup_injLayout_semi_notenoughinjectors(8);
}

static void assert_validateInjectionSetup_no_oddfire(uint8_t nCylinders)
{
  config2 page2 = {};
  config6 page6 = {};
  page2.nInjectors = INJ_CHANNELS;
  page2.injLayout = INJ_SEQUENTIAL;
  page2.engineType = ODD_FIRE;
  page2.nCylinders = 5;
  validateInjectionSetup(page2, page6);
  TEST_ASSERT_EQUAL(EVEN_FIRE, page2.engineType);
}

static void test_validateInjectionSetup_oddfire(void)
{
  config2 page2 = {};
  config6 page6 = {};

  page2.nInjectors = INJ_CHANNELS;
  page2.injLayout = INJ_SEQUENTIAL;
  page2.engineType = ODD_FIRE;
  page2.nCylinders = 2;

  validateInjectionSetup(page2, page6);
  TEST_ASSERT_EQUAL(ODD_FIRE, page2.engineType);

  assert_validateInjectionSetup_no_oddfire(1);
  assert_validateInjectionSetup_no_oddfire(3);
  assert_validateInjectionSetup_no_oddfire(4);
  assert_validateInjectionSetup_no_oddfire(5);
  assert_validateInjectionSetup_no_oddfire(6);
  assert_validateInjectionSetup_no_oddfire(8);
}

static void test_validateInjectionSetup_trim(void)
{
  config2 page2 = {};
  config6 page6 = {};

  page2.nInjectors = INJ_CHANNELS;

  page2.injLayout = INJ_SEQUENTIAL;
  page6.fuelTrimEnabled = true;
  validateInjectionSetup(page2, page6);
  TEST_ASSERT_TRUE(page6.fuelTrimEnabled);

  page2.injLayout = INJ_SEMISEQUENTIAL;
  page6.fuelTrimEnabled = true;
  validateInjectionSetup(page2, page6);
  TEST_ASSERT_FALSE(page6.fuelTrimEnabled);

  page2.injLayout = INJ_PAIRED;
  page6.fuelTrimEnabled = true;
  validateInjectionSetup(page2, page6);
  TEST_ASSERT_FALSE(page6.fuelTrimEnabled);
}

static void assert_injTiming_untouched(uint8_t injLayout)
{
  config2 page2 = {};
  config6 page6 = {};
  page2.nInjectors = INJ_CHANNELS;
  page2.nCylinders = page2.nInjectors;
  page2.injLayout = injLayout;
  page2.injTiming = true;
  validateInjectionSetup(page2, page6);
  TEST_ASSERT_TRUE(page2.injTiming);
  TEST_ASSERT_EQUAL(injLayout, page2.injLayout);

  page2.injTiming = false;
  validateInjectionSetup(page2, page6);
  TEST_ASSERT_FALSE(page2.injTiming);
  TEST_ASSERT_EQUAL(injLayout, page2.injLayout);
}

static void test_validateInjectionSetup_injtiming(void)
{
  config2 page2 = {};
  config6 page6 = {};

  page2.nInjectors = INJ_CHANNELS;
  page2.nCylinders = page2.nInjectors;

  page2.injLayout = INJ_SEQUENTIAL;
  page2.injTiming = false;
  validateInjectionSetup(page2, page6);
  TEST_ASSERT_TRUE(page2.injTiming);
  TEST_ASSERT_EQUAL(INJ_SEQUENTIAL, page2.injLayout);

  assert_injTiming_untouched(INJ_SEMISEQUENTIAL);
  assert_injTiming_untouched(INJ_PAIRED);
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
    RUN_TEST_P(test_isAnyFuelScheduleRunning);
    RUN_TEST_P(test_setFuelChannelSchedule_ignores_zero_pw);
    RUN_TEST_P(test_setFuelChannelSchedule_ignores_disabled_channel);
    RUN_TEST_P(test_setFuelChannelSchedule_starts_pending_when_enabled);
    RUN_TEST_P(test_setFuelChannelSchedule_ignores_zero_timeout);
    RUN_TEST_P(test_lookupInjectorAngle_clamp_max_inj);
    RUN_TEST_P(test_beginInjectorPriming_floodclear);
    RUN_TEST_P(test_beginInjectorPriming);
    RUN_TEST_P(test_setFuelChannelSchedules_returnsInjectionAngle);
    RUN_TEST_P(test_changeToFullSequentialInjection);
    RUN_TEST_P(test_changeToSemiSequentialInjection);
    RUN_TEST_P(test_validateInjectionSetup_nInjector_clamp);
    RUN_TEST_P(test_validateInjectionSetup_injLayout);
    RUN_TEST_P(test_validateInjectionSetup_oddfire);
    RUN_TEST_P(test_validateInjectionSetup_trim);
    RUN_TEST_P(test_validateInjectionSetup_injtiming);
 }
}