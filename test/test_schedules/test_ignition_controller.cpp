#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "channel_test_helpers.h"

static void set_all_statuses(ScheduleStatus status)
{
    RUNIF_IGNCHANNEL1({ ignitionSchedule1.Status=status; }, {});
    RUNIF_IGNCHANNEL2({ ignitionSchedule2.Status=status; }, {});
    RUNIF_IGNCHANNEL3({ ignitionSchedule3.Status=status; }, {});
    RUNIF_IGNCHANNEL4({ ignitionSchedule4.Status=status; }, {});
    RUNIF_IGNCHANNEL5({ ignitionSchedule5.Status=status; }, {});
    RUNIF_IGNCHANNEL6({ ignitionSchedule6.Status=status; }, {});
    RUNIF_IGNCHANNEL7({ ignitionSchedule7.Status=status; }, {});
    RUNIF_IGNCHANNEL8({ ignitionSchedule8.Status=status; }, {});
}

static void assert_all_statuses(ScheduleStatus status)
{
    RUNIF_IGNCHANNEL1({ TEST_ASSERT_EQUAL(status, ignitionSchedule1.Status); }, {});
    RUNIF_IGNCHANNEL2({ TEST_ASSERT_EQUAL(status, ignitionSchedule2.Status); }, {});
    RUNIF_IGNCHANNEL3({ TEST_ASSERT_EQUAL(status, ignitionSchedule3.Status); }, {});
    RUNIF_IGNCHANNEL4({ TEST_ASSERT_EQUAL(status, ignitionSchedule4.Status); }, {});
    RUNIF_IGNCHANNEL5({ TEST_ASSERT_EQUAL(status, ignitionSchedule5.Status); }, {});
    RUNIF_IGNCHANNEL6({ TEST_ASSERT_EQUAL(status, ignitionSchedule6.Status); }, {});
    RUNIF_IGNCHANNEL7({ TEST_ASSERT_EQUAL(status, ignitionSchedule7.Status); }, {});
    RUNIF_IGNCHANNEL8({ TEST_ASSERT_EQUAL(status, ignitionSchedule8.Status); }, {});
}

static void test_disableAllIgnSchedules(void)
{
    stopIgnitionSchedulers();
    set_all_statuses(PENDING);
    disableAllIgnSchedules();
    assert_all_statuses(OFF);
}

void test_ignition_controller(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_disableAllIgnSchedules);
  }
}
