#include "decoders.h"
#include "../test_utils.h"
#include "globals.h"

static void assert_crank_not_current_rpm(decoder_t &decoder)
{
    currentStatus.setRpm(currentStatus.crankRPM/2U);
    currentStatus.revolutionTime = UINT32_MAX; // To trigger a change
    TEST_ASSERT_NOT_EQUAL(currentStatus.RPM, decoder.getRPM());
}

static void assert_crank_uses_current_rpm(decoder_t &decoder)
{
    currentStatus.setRpm(currentStatus.crankRPM/2U);
    currentStatus.revolutionTime = UINT32_MAX; // To trigger a change
    TEST_ASSERT_EQUAL(currentStatus.RPM, decoder.getRPM());
}

static void test_getRPM(void)
{
  extern uint16_t toothCurrentCount;
  extern decoder_status_t decoderStatus;
  extern uint32_t toothLastMinusOneToothTime;
  extern uint32_t toothLastToothTime;

  auto decoder = triggerSetup_ThirtySixMinus222();
  currentStatus.crankRPM = 400;

  decoderStatus.syncStatus = SyncStatus::Full;
  toothLastMinusOneToothTime = 1000;
  toothLastToothTime = toothLastMinusOneToothTime * 2;

  // Not cranking
  currentStatus.setRpm((currentStatus.crankRPM*2U)+111);
  currentStatus.revolutionTime = UINT32_MAX; // To trigger a change
  TEST_ASSERT_EQUAL(currentStatus.RPM, decoder.getRPM());

  // Cranking
  configPage2.nCylinders = 4;
  for (auto toothCount: { 19, 16, 34, })
  {
    toothCurrentCount = toothCount;
    assert_crank_uses_current_rpm(decoder);
  }
  for (auto toothCount: { 9, 12, 33, })
  {
    toothCurrentCount = toothCount;
    decoderStatus.toothAngleIsCorrect = true;
    assert_crank_not_current_rpm(decoder);
    decoderStatus.toothAngleIsCorrect = false;
    assert_crank_uses_current_rpm(decoder);
  }

  configPage2.nCylinders = 6;
  for (auto toothCount: { 9, 12, 33, })
  {
    toothCurrentCount = toothCount;
    assert_crank_uses_current_rpm(decoder);
  }
  for (auto toothCount: { 19, 16, 34, })
  {
    toothCurrentCount = toothCount;
    decoderStatus.toothAngleIsCorrect = true;
    assert_crank_not_current_rpm(decoder);
    decoderStatus.toothAngleIsCorrect = false;
    assert_crank_uses_current_rpm(decoder);
  }  

  configPage2.nCylinders = 2;
  for (auto toothCount: { 9, 12, 33, 19, 16, 34, })
  {
    toothCurrentCount = toothCount;
    decoderStatus.toothAngleIsCorrect = true;
    assert_crank_uses_current_rpm(decoder);
    decoderStatus.toothAngleIsCorrect = false;
    assert_crank_uses_current_rpm(decoder);
  }
}

void testThirtySixMinus22(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getRPM);
  }
}