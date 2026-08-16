#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getCrankAngle(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile int toothCurrentCount;

  // Configure a 4-tooth distributor (4-cylinder cam-spaced)
  configPage2.nCylinders = 4;
  configPage4.triggerTeeth = 4;

  auto decoder = triggerSetup_BasicDistributor();

  auto run_case = [&](int toothCount, int delta, int trigAngle, int16_t expected) {
    toothLastToothTime = 2000;
    toothCurrentCount = toothCount;
    decoderStatus.syncStatus = SyncStatus::Full;
    decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
    setAngleConverterRevolutionTime(2000);
    int16_t angle = decoder.pGetCrankAngle(toothLastToothTime + delta);
    TEST_ASSERT_EQUAL(expected, angle);
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

void testBasicDistributor(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
  }
}