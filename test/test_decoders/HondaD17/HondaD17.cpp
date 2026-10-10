#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"
#include "src/decoders/decoder_state.h"

extern decoders::detail::state_t _decoderState;

static void test_getCrankAngle(void)
{      
  auto decoder = triggerSetup_HondaD17();

  // Use deterministic time->angle conversion
  CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
  _decoderState.revolutionOne = false;
  setAngleConverterRevolutionTime(2000);

  // Setup common deterministic state
  configPage4.triggerAngle = 0;
  _decoderState.toothLastToothTime = 5000;

  // _decoderState.toothCurrentCount == 1 -> angle = 0 + triggerAngle + timeToAngle(dt)
  _decoderState.toothCurrentCount = 1;
  TEST_ASSERT_EQUAL(18, decoder.pGetCrankAngle(_decoderState.toothLastToothTime + 100));

  // _decoderState.toothCurrentCount == 2 -> angle = 1*triggerToothAngle + triggerAngle + timeToAngle(dt)
  _decoderState.toothCurrentCount = 2;
  TEST_ASSERT_EQUAL(48, decoder.pGetCrankAngle(_decoderState.toothLastToothTime + 100));

  // _decoderState.toothCurrentCount == 0 -> treated as 13th tooth -> use 11*triggerToothAngle
  _decoderState.toothCurrentCount = 0;
  TEST_ASSERT_EQUAL(339, decoder.pGetCrankAngle(_decoderState.toothLastToothTime + 50));

  // Trigger angle offset and wrap-around behavior
  configPage4.triggerAngle = 500;
  _decoderState.toothCurrentCount = 12; // base = 11*triggerToothAngle = 330
  TEST_ASSERT_EQUAL(112, decoder.pGetCrankAngle(_decoderState.toothLastToothTime + 10));
}

static void test_getRevolutionTime(void)
{
  auto decoder = triggerSetup_HondaD17();

  // Standard calculation: the time between the last 2 tooth #1
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.startRevolutions = 1; // not cranking
  _decoderState.toothOneMinusOneTime = 1000UL;
  _decoderState.toothOneTime = _decoderState.toothOneMinusOneTime + 60000UL;
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());
}

void testHondaD17(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRevolutionTime);
  }
}