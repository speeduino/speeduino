#include "../device_test_harness.h"

void runAllScheduleCalcTests(void)
{
  extern void test_calc_ign_timeout();
  extern void test_calc_inj_timeout();
  extern void test_adjust_crank_angle();

  test_calc_ign_timeout();
  test_calc_inj_timeout();
  test_adjust_crank_angle();
}

DEVICE_TEST(runAllScheduleCalcTests)
