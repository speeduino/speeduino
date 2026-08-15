#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getCrankAngle(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile int toothCurrentCount;

  auto decoder = triggerSetup_MazdaAU();

  auto run_case = [&](int toothNum, int16_t expected, int trigAngle = 0) {
    toothLastToothTime = 2000;
    toothCurrentCount = toothNum;
    decoderStatus.toothAngleIsCorrect = true;
    decoderStatus.syncStatus = SyncStatus::Full;
    configPage4.triggerAngle = trigAngle;
    setAngleConverterRevolutionTime(2000);
    int16_t angle = decoder.pGetCrankAngle(toothLastToothTime + 100);
    TEST_ASSERT_INT16_WITHIN_MESSAGE(2, expected, angle, "MazdaAU Crank Angle");
  };

  // timeToAngle(100) ~= 18 deg
  const int dt = 18;

  // toothAngles from triggerSetup_MazdaAU: [348,96,168,276]
  run_case(1, 348 + dt);
  run_case(2, 96 + dt);
  run_case(3, 168 + dt);
  run_case(4, 276 + dt);

  // trigger angle offset
  run_case(2, 96 + dt + 10, 10);
}

void testMazdaAU(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
  }
}