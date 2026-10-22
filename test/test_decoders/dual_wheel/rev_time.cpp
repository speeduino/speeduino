#include "../../test_utils.h"
#include "src/decoders/dualWheel.h"
#include "scheduler_ignition_controller.h"

using namespace decoders::shared;

static config4 setup_dualwheel_12_1(void)
{
    config4 page4 = {};

    //Setup a 12-1 wheel
    page4.triggerTeeth = 12;
    page4.TrigSpeed = CRANK_SPEED;
    page4.trigPatternSec = SEC_TRIGGER_SINGLE;

    return page4;
}

static void test_getRevolutionTime(void)
{
  auto page4 = setup_dualwheel_12_1();
  auto decoderState = decoders::dual_wheel::initialise(page4);
  statuses current = {};

  current.startRevolutions = 1;
  current.crankRPM = 400;
  current.revolutionTime = 12345UL;
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothOneMinusOneTime = 1000UL;
  decoderState.toothOneTime = decoderState.toothOneMinusOneTime + 60000UL; // Full revolution: 60000uS
  decoderState.toothLastMinusOneToothTime = 1000UL;
  decoderState.toothLastToothTime = decoderState.toothLastMinusOneToothTime + 3000UL; // Tooth gap of 3000uS * 12 teeth: 36000uS

  // Running: full revolution
  current.setRpm(current.crankRPM*2);
  ;
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoders::dual_wheel::getRevolutionTime(current, decoderState, page4));

  // Cranking: per tooth
  current.setRpm(current.crankRPM/2);
  TEST_ASSERT_EQUAL_UINT32(36000UL, decoders::dual_wheel::getRevolutionTime(current, decoderState, page4));

  // Without full sync the speed is unknown
  decoderState.decoderStatus.syncStatus = SyncStatus::Partial;
  TEST_ASSERT_EQUAL_UINT32(0UL, decoders::dual_wheel::getRevolutionTime(current, decoderState, page4));

  decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(0UL, decoders::dual_wheel::getRevolutionTime(current, decoderState, page4));
}

void testDualWheelRevTime(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_getRevolutionTime);
}