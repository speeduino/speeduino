#include "decoders.h"
#include "../test_utils.h"
#include "globals.h"
#include "src/decoders/details/decoder_state.h"

extern decoders::detail::state_t _decoderState;

static void assert_crank_per_tooth(decoder_t &decoder)
{
    currentStatus.setRpm(currentStatus.crankRPM/2U);
    TEST_ASSERT_EQUAL_UINT32(36000UL, decoder.getRevolutionTime());
}

static void assert_crank_uses_current_revolution_time(decoder_t &decoder)
{
    currentStatus.setRpm(currentStatus.crankRPM/2U);
    TEST_ASSERT_EQUAL_UINT32(currentStatus.revolutionTime, decoder.getRevolutionTime());
}

static void test_getRevolutionTime(void)
{
  auto decoder = triggerSetup_ThirtySixMinus222();
  currentStatus.crankRPM = 400;
  configPage4.StgCycles = 0;
  currentStatus.startRevolutions = 1;
  currentStatus.revolutionTime = 12345UL;

  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  _decoderState.toothOneMinusOneTime = 1000UL;
  _decoderState.toothOneTime = _decoderState.toothOneMinusOneTime + 60000UL; // Full revolution: 60000uS
  _decoderState.toothLastMinusOneToothTime = 1000;
  _decoderState.toothLastToothTime = _decoderState.toothLastMinusOneToothTime * 2; // Tooth gap of 1000uS * 36 teeth: 36000uS

  // Not cranking
  currentStatus.setRpm((currentStatus.crankRPM*2U)+111);
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());

  // Cranking
  configPage2.nCylinders = 4;
  for (auto toothCount: { 19, 16, 34, })
  {
    _decoderState.toothCurrentCount = toothCount;
    assert_crank_uses_current_revolution_time(decoder);
  }
  for (auto toothCount: { 9, 12, 33, })
  {
    _decoderState.toothCurrentCount = toothCount;
    _decoderState.decoderStatus.toothAngleIsCorrect = true;
    assert_crank_per_tooth(decoder);
    _decoderState.decoderStatus.toothAngleIsCorrect = false;
    assert_crank_uses_current_revolution_time(decoder);
  }

  configPage2.nCylinders = 6;
  for (auto toothCount: { 9, 12, 33, })
  {
    _decoderState.toothCurrentCount = toothCount;
    assert_crank_uses_current_revolution_time(decoder);
  }
  for (auto toothCount: { 19, 16, 34, })
  {
    _decoderState.toothCurrentCount = toothCount;
    _decoderState.decoderStatus.toothAngleIsCorrect = true;
    assert_crank_per_tooth(decoder);
    _decoderState.decoderStatus.toothAngleIsCorrect = false;
    assert_crank_uses_current_revolution_time(decoder);
  }

  configPage2.nCylinders = 2;
  for (auto toothCount: { 9, 12, 33, 19, 16, 34, })
  {
    _decoderState.toothCurrentCount = toothCount;
    _decoderState.decoderStatus.toothAngleIsCorrect = true;
    assert_crank_uses_current_revolution_time(decoder);
    _decoderState.decoderStatus.toothAngleIsCorrect = false;
    assert_crank_uses_current_revolution_time(decoder);
  }
}

void testThirtySixMinus22(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getRevolutionTime);
  }
}