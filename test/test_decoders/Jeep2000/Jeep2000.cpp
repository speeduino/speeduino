#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getCrankAngle(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile uint16_t toothCurrentCount;

  auto decoder = triggerSetup_Jeep2000();

  auto run_case = [&](uint8_t toothNum, int16_t expected, int trigAngle = 0) {
    toothLastToothTime = 2000;
    toothCurrentCount = toothNum;
    decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
    setAngleConverterRevolutionTime(2000);
    int16_t angle = decoder.pGetCrankAngle(toothLastToothTime + 100);
    TEST_ASSERT_EQUAL(expected, angle);
  };

  // timeToAngle(100) ~= 18 deg
  const int dt = 18;

  // toothAngles from triggerSetup_Jeep2000
  // indices 1..12 map to array values
  run_case(1, 174 + dt);
  run_case(2, 194 + dt);
  run_case(3, 214 + dt);
  run_case(4, 234 + dt);
  run_case(5, 294 + dt);
  run_case(6, 314 + dt);
  run_case(7, 334 + dt);
  run_case(8, 354 + dt);
  run_case(9, 414 + dt);
  run_case(10, 434 + dt);
  run_case(11, 454 + dt);
  run_case(12, 474 + dt);

  // cam tooth (0) special case -> 114 + dt
  run_case(0, 114 + dt);

  // trigger angle offset
  run_case(1, 174 + dt + 10, 10);
}

static void test_getRPM(void)
{
  auto decoder = triggerSetup_Jeep2000();
  TEST_ASSERT_NOT_EQUAL(0, decoder.getRPM());
}

void testJeep2000(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRPM);
  }
}