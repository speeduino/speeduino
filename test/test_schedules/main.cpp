#include "../test_harness_device.h"
#include "../test_harness_native.h"
#include "init.h"

void runAllScheduleTests(void)
{
  extern void test_status_initial_off(void);
  extern void test_status_off_to_pending(void);
  extern void test_status_pending_to_running(void);
  extern void test_status_running_to_pending(void);
  extern void test_status_running_to_off(void);
  extern void test_accuracy_timeout(void);
  extern void test_accuracy_duration(void);
  extern void test_setSchedule(void);
  extern void testScheduleStateMachine(void);
  extern void test_schedule(void);
  extern void test_ignition_controller();
  extern void test_fuel_controller(void);

  initialiseAll();

  test_status_initial_off();
  test_status_off_to_pending();
  test_status_pending_to_running();
  test_status_running_to_pending();
  test_status_running_to_off();
  test_accuracy_timeout();
  test_accuracy_duration();
  test_setSchedule();
  testScheduleStateMachine();
  test_schedule();
  test_ignition_controller();
  test_fuel_controller();
}

TEST_HARNESS(runAllScheduleTests)
