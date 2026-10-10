#include "decoders.h"
#include "../test_utils.h"
#include "globals.h"
#include "src/decoders/details/decoder_state.h"

extern decoders::detail::state_t _decoderState;

static void test_getRevolutionTime(void)
{
  auto decoder = triggerSetup_ThirtySixMinus21();
  currentStatus.crankRPM = 400;
  configPage4.StgCycles = 0;
  currentStatus.startRevolutions = 1;
  currentStatus.revolutionTime = 12345UL;

  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  _decoderState.toothOneMinusOneTime = 1000UL;
  _decoderState.toothOneTime = _decoderState.toothOneMinusOneTime + 60000UL; // Full revolution: 60000uS
  _decoderState.toothLastMinusOneToothTime = 1000;
  _decoderState.toothLastToothTime = _decoderState.toothLastMinusOneToothTime * 2; // Tooth gap of 1000uS * 36 teeth: 36000uS

  // Not cranking: full revolution
  currentStatus.setRpm((currentStatus.crankRPM*2U)+111);
  _decoderState.toothCurrentCount = 20;
  _decoderState.decoderStatus.toothAngleIsCorrect = true;
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());

  // Cranking: can't do a per tooth calculation after the missing tooth, so no change
  currentStatus.setRpm(currentStatus.crankRPM/2U);
  TEST_ASSERT_EQUAL_UINT32(12345UL, decoder.getRevolutionTime());

  _decoderState.toothCurrentCount = 1;
  _decoderState.decoderStatus.toothAngleIsCorrect = false;
  TEST_ASSERT_EQUAL_UINT32(12345UL, decoder.getRevolutionTime());

  // Cranking: per tooth
  _decoderState.decoderStatus.toothAngleIsCorrect = true;
  TEST_ASSERT_EQUAL_UINT32(36000UL, decoder.getRevolutionTime());
}

void testThirtySixMinus21(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getRevolutionTime);
  }
}