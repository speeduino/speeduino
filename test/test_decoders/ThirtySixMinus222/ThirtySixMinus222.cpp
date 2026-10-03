#include "decoders.h"
#include "../test_utils.h"
#include "globals.h"

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
  extern uint16_t toothCurrentCount;
  extern decoder_status_t decoderStatus;
  extern uint32_t toothLastMinusOneToothTime;
  extern uint32_t toothLastToothTime;
  extern volatile unsigned long toothOneTime;
  extern volatile unsigned long toothOneMinusOneTime;

  auto decoder = triggerSetup_ThirtySixMinus222();
  currentStatus.crankRPM = 400;
  configPage4.StgCycles = 0;
  currentStatus.startRevolutions = 1;
  currentStatus.revolutionTime = 12345UL;

  decoderStatus.syncStatus = SyncStatus::Full;
  toothOneMinusOneTime = 1000UL;
  toothOneTime = toothOneMinusOneTime + 60000UL; // Full revolution: 60000uS
  toothLastMinusOneToothTime = 1000;
  toothLastToothTime = toothLastMinusOneToothTime * 2; // Tooth gap of 1000uS * 36 teeth: 36000uS

  // Not cranking
  currentStatus.setRpm((currentStatus.crankRPM*2U)+111);
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());

  // Cranking
  configPage2.nCylinders = 4;
  for (auto toothCount: { 19, 16, 34, })
  {
    toothCurrentCount = toothCount;
    assert_crank_uses_current_revolution_time(decoder);
  }
  for (auto toothCount: { 9, 12, 33, })
  {
    toothCurrentCount = toothCount;
    decoderStatus.toothAngleIsCorrect = true;
    assert_crank_per_tooth(decoder);
    decoderStatus.toothAngleIsCorrect = false;
    assert_crank_uses_current_revolution_time(decoder);
  }

  configPage2.nCylinders = 6;
  for (auto toothCount: { 9, 12, 33, })
  {
    toothCurrentCount = toothCount;
    assert_crank_uses_current_revolution_time(decoder);
  }
  for (auto toothCount: { 19, 16, 34, })
  {
    toothCurrentCount = toothCount;
    decoderStatus.toothAngleIsCorrect = true;
    assert_crank_per_tooth(decoder);
    decoderStatus.toothAngleIsCorrect = false;
    assert_crank_uses_current_revolution_time(decoder);
  }

  configPage2.nCylinders = 2;
  for (auto toothCount: { 9, 12, 33, 19, 16, 34, })
  {
    toothCurrentCount = toothCount;
    decoderStatus.toothAngleIsCorrect = true;
    assert_crank_uses_current_revolution_time(decoder);
    decoderStatus.toothAngleIsCorrect = false;
    assert_crank_uses_current_revolution_time(decoder);
  }
}

void testThirtySixMinus22(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getRevolutionTime);
  }
}