#include "../test_utils.h"
#include "src/decoders/basicDistributor.h"

static void test_getCrankAngle(void)
{
  // Configure a 4-tooth distributor (4-cylinder cam-spaced)
  config2 page2={};
  config4 page4={};
  page2.nCylinders = 4;
  page4.triggerTeeth = 4;
  auto decoderState = decoders::basic_distributor::initialise(page2);

  auto run_case = [&](int toothCount, int delta, int trigAngle, int16_t expected) {
    decoderState.toothLastToothTime = 2000;
    decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime - (5*delta);
    decoderState.toothCurrentCount = toothCount;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.decoderStatus.toothAngleIsCorrect = true;
    page4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
    TEST_ASSERT_EQUAL(expected, decoders::basic_distributor::getCrankAngle(decoderState.toothLastToothTime + delta, decoderState, page4));
  };

  // timeToAngleIntervalTooth(100) ~ 18 deg when revolution time = 2000
  const int dt = 36;

  run_case(1, 100, 0, 0 + dt);
  run_case(2, 100, 0, 180 + dt);
  run_case(3, 100, 0, 360 + dt);
  run_case(4, 100, 0, 540 + dt);

  // trigger offset
  run_case(1, 100, 30, 30 + dt);
}

void testBasicDistribCrankAngle(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_getCrankAngle);
}