#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "scheduler_fuel_controller.h"
#include "channel_test_helpers.h"
#include "units.h"
#include "type_traits.h"

extern bool isAnyFuelScheduleRunning(void);
extern uint16_t lookupInjectorAngle(const statuses &current);
extern table2D_u8_u16_4 injectorAngleTable;
extern void setFuelChannelSchedule(FuelSchedule &schedule, uint8_t channel, uint16_t crankAngle, byte injChannelMask, uint16_t injAngle, injectorAngleCalcCache *pCache);

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

using raw_counter_t = type_traits::remove_reference<FuelSchedule::counter_t>::type;
using raw_compare_t = type_traits::remove_reference<FuelSchedule::compare_t>::type;

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
  setFuelChannelSchedule(schedule, UINT8_C(1), 100U, (byte)(1U << INJ1_CMD_BIT), 180U, &cache);

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
  setFuelChannelSchedule(schedule, UINT8_C(1), 0U, (byte)(1U << INJ1_CMD_BIT), 0U, &cache);

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
  setFuelChannelSchedule(schedule, UINT8_C(1), 300U, (byte)(1U << INJ1_CMD_BIT), 355U, &cache);

  TEST_ASSERT_EQUAL(PENDING, schedule._status);
  TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(1000U), schedule._duration);
  TEST_ASSERT_GREATER_THAN(0U, schedule._compare);
}

static void test_lookupInjectorAngle_clamp_max_inj(void)
{
  statuses current = {};
  populate_2dtable(&injectorAngleTable, (uint16_t)720, RPM_COARSE.toRaw(1000));

  setRpm(current, 1000);
  CRANK_ANGLE_MAX_INJ = injectorAngleTable.values[0]/2U;

  TEST_ASSERT_LESS_THAN(injectorAngleTable.values[0], CRANK_ANGLE_MAX_INJ);
  TEST_ASSERT_EQUAL(CRANK_ANGLE_MAX_INJ, lookupInjectorAngle(current));
}

void test_fuel_controller(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_isAnyFuelScheduleRunning);
    RUN_TEST_P(test_setFuelChannelSchedule_ignores_zero_pw);
    RUN_TEST_P(test_setFuelChannelSchedule_ignores_disabled_channel);
    RUN_TEST_P(test_setFuelChannelSchedule_starts_pending_when_enabled);
    RUN_TEST_P(test_setFuelChannelSchedule_ignores_zero_timeout);
    RUN_TEST_P(test_lookupInjectorAngle_clamp_max_inj);
  }
}
