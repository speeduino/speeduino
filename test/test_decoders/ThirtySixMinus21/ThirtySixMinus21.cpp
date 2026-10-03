#include "decoders.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getRevolutionTime(void)
{
  extern uint16_t toothCurrentCount;
  extern decoder_status_t decoderStatus;
  extern uint32_t toothLastMinusOneToothTime;
  extern uint32_t toothLastToothTime;
  extern volatile unsigned long toothOneTime;
  extern volatile unsigned long toothOneMinusOneTime;

  auto decoder = triggerSetup_ThirtySixMinus21();
  currentStatus.crankRPM = 400;
  configPage4.StgCycles = 0;
  currentStatus.startRevolutions = 1;
  currentStatus.revolutionTime = 12345UL;

  decoderStatus.syncStatus = SyncStatus::Full;
  toothOneMinusOneTime = 1000UL;
  toothOneTime = toothOneMinusOneTime + 60000UL; // Full revolution: 60000uS
  toothLastMinusOneToothTime = 1000;
  toothLastToothTime = toothLastMinusOneToothTime * 2; // Tooth gap of 1000uS * 36 teeth: 36000uS

  // Not cranking: full revolution
  currentStatus.setRpm((currentStatus.crankRPM*2U)+111);
  toothCurrentCount = 20;
  decoderStatus.toothAngleIsCorrect = true;
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());

  // Cranking: can't do a per tooth calculation after the missing tooth, so no change
  currentStatus.setRpm(currentStatus.crankRPM/2U);
  TEST_ASSERT_EQUAL_UINT32(12345UL, decoder.getRevolutionTime());

  toothCurrentCount = 1;
  decoderStatus.toothAngleIsCorrect = false;
  TEST_ASSERT_EQUAL_UINT32(12345UL, decoder.getRevolutionTime());

  // Cranking: per tooth
  decoderStatus.toothAngleIsCorrect = true;
  TEST_ASSERT_EQUAL_UINT32(36000UL, decoder.getRevolutionTime());
}

void testThirtySixMinus21(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getRevolutionTime);
  }
}