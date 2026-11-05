#include "../../test_utils.h"
#include "test_context.h"
#include "scheduler_ignition_controller.h"

static void test_newIgn(uint16_t dischargeAngle, int16_t triggerAngle, uint8_t channel, uint16_t expectedTooth, IgnitionSchedule &schedule)
{
  context_k6a_t ctx;
  ctx.page4.sparkMode = IGN_MODE_SINGLE;
  ctx.page4.triggerAngle = triggerAngle;
  ctx.init();
  schedule.dischargeAngle = dischargeAngle;
  CRANK_ANGLE_MAX_IGN = 360;
    
  ctx.setEndTeeth();
  TEST_ASSERT_EQUAL(expectedTooth, ctx.decoderState.ignitionEndTeeth[channel]);
}

static void test_newIgn_channel1(void)
{
  test_newIgn(360-10, 0, 0, 3, ignitionSchedule1);
  test_newIgn(360-10, 90, 0, 3, ignitionSchedule1);
  test_newIgn(360-10, 180, 0, 1, ignitionSchedule1);
  test_newIgn(360-10, 270, 0, 1, ignitionSchedule1);
  test_newIgn(360-10, 360, 0, 3, ignitionSchedule1);
  test_newIgn(360-10, -90, 0, 1, ignitionSchedule1);
  test_newIgn(360-10, -180, 0, 1, ignitionSchedule1);
  test_newIgn(360-10, -270, 0, 3, ignitionSchedule1);
  test_newIgn(360-10, -360, 0, 3, ignitionSchedule1);
}

static void test_newIgn_channel2()
{
  test_newIgn(180-10, 0, 1, 1, ignitionSchedule2);
  test_newIgn(180-10, 90, 1, 1, ignitionSchedule2);
  test_newIgn(180-10, 180, 1, 3, ignitionSchedule2);
  test_newIgn(180-10, 270, 1, 3, ignitionSchedule2);
  test_newIgn(180-10, 360, 1, 1, ignitionSchedule2);
  test_newIgn(180-10, -90, 1, 3, ignitionSchedule2);
  test_newIgn(180-10, -180, 1, 3, ignitionSchedule2);
  test_newIgn(180-10, -270, 1, 1, ignitionSchedule2);
  test_newIgn(180-10, -360, 1, 1, ignitionSchedule2);
}

void testSuzukiK6AsetEndTeeth()
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_newIgn_channel1);
    RUN_TEST_P(test_newIgn_channel2);
  }
}