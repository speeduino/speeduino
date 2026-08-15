#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getCrankAngle(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile unsigned long toothLastMinusOneToothTime;
  extern volatile uint16_t toothCurrentCount;

  auto decoder = triggerSetup_GM7X();

  // Setup a simple timing window: last tooth at 2000us, previous at 1500us
  toothLastToothTime = 2000;
  toothLastMinusOneToothTime = toothLastToothTime - 500;
  decoderStatus.toothAngleIsCorrect = true;
  setAngleConverterRevolutionTime(2000);

  // 100us after the last tooth — use same delta for all checks
  const uint32_t callTime = toothLastToothTime + 100;

  // toothCurrentCount = 1 -> ((1-1)*60)+42 + timeToAngle(100) => 42 + ~18 = 60
  toothCurrentCount = 1;
  configPage4.triggerAngle = 0;
  TEST_ASSERT_EQUAL_INT16(60, decoder.pGetCrankAngle(callTime));

  // toothCurrentCount = 2 -> ((2-1)*60)+42 + ~18 = 102+18 = 120
  toothCurrentCount = 2;
  TEST_ASSERT_EQUAL_INT16(120, decoder.pGetCrankAngle(callTime));

  // toothCurrentCount = 3 -> special case = 112 + ~18 = 130
  toothCurrentCount = 3;
  TEST_ASSERT_EQUAL_INT16(130, decoder.pGetCrankAngle(callTime));

  // toothCurrentCount = 4 -> ((4-2)*60)+42 + ~18 = 162+18 = 180
  toothCurrentCount = 4;
  TEST_ASSERT_EQUAL_INT16(180, decoder.pGetCrankAngle(callTime));

  // Large tooth count to force wrap-around: e.g. 13 -> ((13-2)*60)+42 + ~18 = 702+18 = 720 -> wraps to 0
  toothCurrentCount = 13;
  TEST_ASSERT_EQUAL_INT16(0, decoder.pGetCrankAngle(callTime));

  // Non-zero triggerAngle modifies base result
  toothCurrentCount = 1;
  configPage4.triggerAngle = 10;
  TEST_ASSERT_EQUAL_INT16(70, decoder.pGetCrankAngle(callTime));
}

void testGM7X(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
  }
}