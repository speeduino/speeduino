#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "type_traits.h"

using raw_counter_t = type_traits::remove_reference<IgnitionSchedule::counter_t>::type;
using raw_compare_t = type_traits::remove_reference<IgnitionSchedule::compare_t>::type;

static void test_disable(void)
{
  raw_counter_t counter = {0};
  raw_compare_t compare = {0};
  Schedule schedule(counter, compare);

  schedule.Status = OFF;
  disableSchedule(schedule);
  TEST_ASSERT_EQUAL(OFF, schedule.Status);

  schedule.Status = PENDING;
  disableSchedule(schedule);
  TEST_ASSERT_EQUAL(OFF, schedule.Status);

  // We do *not* stop a schedule part way through running.
  // I.e. injector has opened (but not clsosed) or coil is charging (but not discharged).
  schedule.Status = RUNNING;
  disableSchedule(schedule);
  TEST_ASSERT_EQUAL(RUNNING, schedule.Status);

  schedule.Status = RUNNING_WITHNEXT;
  disableSchedule(schedule);
  TEST_ASSERT_EQUAL(RUNNING, schedule.Status);
}

void test_schedule(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_disable);
  }
}
