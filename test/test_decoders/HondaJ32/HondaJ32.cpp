#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getCrankAngle(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile uint8_t toothCurrentCount;

  auto decoder = triggerSetup_HondaJ32();

  auto run_case = [&](uint16_t toothNum, int16_t expected, int16_t triggerAngle = 0) {
    toothLastToothTime = 2000;
    toothCurrentCount = toothNum;
    decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = triggerAngle;
    setAngleConverterRevolutionTime(2000);
    int16_t angle = decoder.pGetCrankAngle(toothLastToothTime + 100);
    TEST_ASSERT_INT16_WITHIN_MESSAGE(2, expected, angle, "Crank Angle");
  };

  // Basic teeth
  // triggerToothAngle = 15 deg. timeToAngle(100) ~= 18 deg
  run_case(1, 15 + 18);
  run_case(2, 30 + 18);

  // Special teeth
  // tooth 14 -> 213 + 18
  run_case(14, 213 + 18);
  // tooth 22 -> 333 + 18
  run_case(22, 333 + 18);

  // Wrap-around: tooth 24 -> 24*15 + 18 = 378 -> subtract 360 => 18
  run_case(24, 18);

  // Tooth zero case
  run_case(0, 0 + 18);

  // Non-zero trigger angle offset
  run_case(1, 15 + 18 + 10, 10);
}

void testHondaJ32(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
  }
}