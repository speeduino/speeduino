#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getCrankAngle(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile int toothCurrentCount;

  auto decoder = triggerSetup_Miata9905();

  auto run_case = [&](int toothNum, int16_t expected, int trigAngle = 0) {
    toothLastToothTime = 2000;
    toothCurrentCount = toothNum;
    decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
    setAngleConverterRevolutionTime(2000);
    int16_t angle = decoder.pGetCrankAngle(toothLastToothTime + 100);
    TEST_ASSERT_EQUAL(expected, angle);
  };

  // timeToAngle(100) ~= 18 deg when revolution time is 2000us
  const int dt = 18;

  // toothAngles from triggerSetup_Miata9905 (indices 1..8)
  run_case(1, 710 + dt - 720); // wraps to small positive angle (710+18=728 -> 8)
  run_case(2, 100 + dt);
  run_case(3, 170 + dt);
  run_case(4, 280 + dt);
  run_case(5, 350 + dt);
  run_case(6, 460 + dt);
  run_case(7, 530 + dt);
  run_case(8, 640 + dt);

  // trigger angle offset
  run_case(2, 100 + dt + 10, 10);
}

void testMiata9905(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
  }
}