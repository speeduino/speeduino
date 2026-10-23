#include "../../test_utils.h"
#include "src/decoders/gm7x.h"

static void test_getCrankAngle(void)
{
  config4 page4 = {};
  auto decoderState = decoders::gm7x::initialise();

  // Setup a simple timing window: last tooth at 2000us, previous at 1500us
  decoderState.toothLastToothTime = 2000;
  decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime - 500;
  decoderState.decoderStatus.toothAngleIsCorrect = true;
  CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
  setAngleConverterRevolutionTime(2000);

  // 100us after the last tooth — use same delta for all checks
  const uint32_t callTime = decoderState.toothLastToothTime + 100;

  // decoderState.toothCurrentCount = 1 -> ((1-1)*60)+42 + timeToAngle(100) => 42 + ~18 = 60
  decoderState.toothCurrentCount = 1;
  page4.triggerAngle = 0;
  TEST_ASSERT_EQUAL_INT16(60, decoders::gm7x::getCrankAngle(callTime, decoderState, page4));

  // decoderState.toothCurrentCount = 2 -> ((2-1)*60)+42 + ~18 = 102+18 = 120
  decoderState.toothCurrentCount = 2;
  TEST_ASSERT_EQUAL_INT16(120, decoders::gm7x::getCrankAngle(callTime, decoderState, page4));

  // decoderState.toothCurrentCount = 3 -> special case = 112 + ~18 = 130
  decoderState.toothCurrentCount = 3;
  TEST_ASSERT_EQUAL_INT16(130, decoders::gm7x::getCrankAngle(callTime, decoderState, page4));

  // decoderState.toothCurrentCount = 4 -> ((4-2)*60)+42 + ~18 = 162+18 = 180
  decoderState.toothCurrentCount = 4;
  TEST_ASSERT_EQUAL_INT16(180, decoders::gm7x::getCrankAngle(callTime, decoderState, page4));

  // Large tooth count to force wrap-around: e.g. 13 -> ((13-2)*60)+42 + ~18 = 702+18 = 720 -> wraps to 0
  decoderState.toothCurrentCount = 13;
  TEST_ASSERT_EQUAL_INT16(0, decoders::gm7x::getCrankAngle(callTime, decoderState, page4));

  // Non-zero triggerAngle modifies base result
  decoderState.toothCurrentCount = 1;
  page4.triggerAngle = 10;
  TEST_ASSERT_EQUAL_INT16(70, decoders::gm7x::getCrankAngle(callTime, decoderState, page4));
}

void testGM7XCrankAngle(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_getCrankAngle);
}