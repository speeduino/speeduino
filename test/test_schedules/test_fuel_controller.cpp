#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "channel_test_helpers.h"
#include "units.h"

extern bool isAnyFuelScheduleRunning(void);
extern uint16_t lookupInjectorAngle(const statuses &current);
extern table2D_u8_u16_4 injectorAngleTable;

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
    RUN_TEST_P(test_lookupInjectorAngle_clamp_max_inj);
  }
}
