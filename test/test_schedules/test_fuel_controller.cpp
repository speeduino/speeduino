#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "channel_test_helpers.h"

static void set_all_statuses(ScheduleStatus status)
{
    RUNIF_INJCHANNEL1({ fuelSchedule1.Status=status; }, {});
    RUNIF_INJCHANNEL2({ fuelSchedule2.Status=status; }, {});
    RUNIF_INJCHANNEL3({ fuelSchedule3.Status=status; }, {});
    RUNIF_INJCHANNEL4({ fuelSchedule4.Status=status; }, {});
    RUNIF_INJCHANNEL5({ fuelSchedule5.Status=status; }, {});
    RUNIF_INJCHANNEL6({ fuelSchedule6.Status=status; }, {});
    RUNIF_INJCHANNEL7({ fuelSchedule7.Status=status; }, {});
    RUNIF_INJCHANNEL8({ fuelSchedule8.Status=status; }, {});
}

static void assert_all_statuses(ScheduleStatus status)
{
    RUNIF_INJCHANNEL1({ TEST_ASSERT_EQUAL(status, fuelSchedule1.Status); }, {});
    RUNIF_INJCHANNEL2({ TEST_ASSERT_EQUAL(status, fuelSchedule2.Status); }, {});
    RUNIF_INJCHANNEL3({ TEST_ASSERT_EQUAL(status, fuelSchedule3.Status); }, {});
    RUNIF_INJCHANNEL4({ TEST_ASSERT_EQUAL(status, fuelSchedule4.Status); }, {});
    RUNIF_INJCHANNEL5({ TEST_ASSERT_EQUAL(status, fuelSchedule5.Status); }, {});
    RUNIF_INJCHANNEL6({ TEST_ASSERT_EQUAL(status, fuelSchedule6.Status); }, {});
    RUNIF_INJCHANNEL7({ TEST_ASSERT_EQUAL(status, fuelSchedule7.Status); }, {});
    RUNIF_INJCHANNEL8({ TEST_ASSERT_EQUAL(status, fuelSchedule8.Status); }, {});
}

static void test_disableAllFuelSchedules(void)
{
    stopFuelSchedulers();
    set_all_statuses(PENDING);
    disableAllFuelSchedules();
    assert_all_statuses(OFF);
}

void test_fuel_controller(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_disableAllFuelSchedules);
  }
}
