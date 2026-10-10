#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"
#include "src/decoders/decoder_state.h"

extern decoders::detail::state_t _decoderState;

static void test_getCrankAngle(void)
{  
  auto decoder = triggerSetup_GM7X();

  // Setup a simple timing window: last tooth at 2000us, previous at 1500us
  _decoderState.toothLastToothTime = 2000;
  _decoderState.toothLastMinusOneToothTime = _decoderState.toothLastToothTime - 500;
  _decoderState.decoderStatus.toothAngleIsCorrect = true;
  CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
  setAngleConverterRevolutionTime(2000);

  // 100us after the last tooth — use same delta for all checks
  const uint32_t callTime = _decoderState.toothLastToothTime + 100;

  // _decoderState.toothCurrentCount = 1 -> ((1-1)*60)+42 + timeToAngle(100) => 42 + ~18 = 60
  _decoderState.toothCurrentCount = 1;
  configPage4.triggerAngle = 0;
  TEST_ASSERT_EQUAL_INT16(60, decoder.pGetCrankAngle(callTime));

  // _decoderState.toothCurrentCount = 2 -> ((2-1)*60)+42 + ~18 = 102+18 = 120
  _decoderState.toothCurrentCount = 2;
  TEST_ASSERT_EQUAL_INT16(120, decoder.pGetCrankAngle(callTime));

  // _decoderState.toothCurrentCount = 3 -> special case = 112 + ~18 = 130
  _decoderState.toothCurrentCount = 3;
  TEST_ASSERT_EQUAL_INT16(130, decoder.pGetCrankAngle(callTime));

  // _decoderState.toothCurrentCount = 4 -> ((4-2)*60)+42 + ~18 = 162+18 = 180
  _decoderState.toothCurrentCount = 4;
  TEST_ASSERT_EQUAL_INT16(180, decoder.pGetCrankAngle(callTime));

  // Large tooth count to force wrap-around: e.g. 13 -> ((13-2)*60)+42 + ~18 = 702+18 = 720 -> wraps to 0
  _decoderState.toothCurrentCount = 13;
  TEST_ASSERT_EQUAL_INT16(0, decoder.pGetCrankAngle(callTime));

  // Non-zero triggerAngle modifies base result
  _decoderState.toothCurrentCount = 1;
  configPage4.triggerAngle = 10;
  TEST_ASSERT_EQUAL_INT16(70, decoder.pGetCrankAngle(callTime));
}

static void test_getRevolutionTime(void)
{
  auto decoder = triggerSetup_GM7X();

  // Standard calculation: the time between the last 2 tooth #1
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.startRevolutions = 1; // not cranking
  _decoderState.toothOneMinusOneTime = 1000UL;
  _decoderState.toothOneTime = _decoderState.toothOneMinusOneTime + 60000UL;
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());
}

void testGM7X(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRevolutionTime);
  }
}