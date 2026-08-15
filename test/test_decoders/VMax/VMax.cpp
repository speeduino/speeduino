#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getCrankAngle(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile int secondaryToothCount;

  auto decoder = triggerSetup_Vmax();

  auto run_case = [&](int secTooth, int delta, int trigAngle, int16_t expected) {
    toothLastToothTime = 2000;
    secondaryToothCount = secTooth;
    decoderStatus.syncStatus = SyncStatus::Full;
    decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
    setAngleConverterRevolutionTime(2000);
    int16_t angle = decoder.pGetCrankAngle(toothLastToothTime + delta);
    TEST_ASSERT_INT16_WITHIN_MESSAGE(2, expected, angle, "VMax Crank Angle");
  };

  // timeToAngle(100) ~= 18 deg
  const int dt = 18;

  run_case(1, 100, 0, 0 + dt);
  run_case(2, 100, 0, 40 + dt);
  run_case(3, 100, 0, 110 + dt);
  run_case(4, 100, 0, 180 + dt);
  run_case(5, 100, 0, 220 + dt);
  run_case(6, 100, 0, 290 + dt);

  // trigger angle offset
  run_case(3, 100, 10, 110 + dt + 10);
}

void testVMax(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
  }
}