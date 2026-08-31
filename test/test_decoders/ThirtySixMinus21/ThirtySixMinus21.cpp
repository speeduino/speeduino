#include "decoders.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getRPM(void)
{
  extern uint16_t toothCurrentCount;
  extern decoder_status_t decoderStatus;
  extern uint32_t toothLastMinusOneToothTime;
  extern uint32_t toothLastToothTime;

  auto decoder = triggerSetup_ThirtySixMinus21();
  currentStatus.crankRPM = 400;

  decoderStatus.syncStatus = SyncStatus::Full;
  toothLastMinusOneToothTime = 1000;
  toothLastToothTime = toothLastMinusOneToothTime * 2;

  // Not cranking
  currentStatus.setRpm((currentStatus.crankRPM*2U)+111);
  toothCurrentCount = 20;
  currentStatus.revolutionTime = UINT32_MAX; // To trigger a change
  TEST_ASSERT_EQUAL(currentStatus.RPM, decoder.getRPM());

  // Cranking
  currentStatus.setRpm(currentStatus.crankRPM/2U);

  currentStatus.revolutionTime = UINT32_MAX; // To trigger a change
  TEST_ASSERT_NOT_EQUAL(0, decoder.getRPM());

  toothCurrentCount = 1;
  decoderStatus.toothAngleIsCorrect = false;
  currentStatus.revolutionTime = UINT32_MAX; // To trigger a change
  TEST_ASSERT_EQUAL(currentStatus.RPM, decoder.getRPM());

  decoderStatus.toothAngleIsCorrect = true;
  currentStatus.revolutionTime = UINT32_MAX; // To trigger a change
  TEST_ASSERT_NOT_EQUAL(currentStatus.RPM, decoder.getRPM());
}

void testThirtySixMinus21(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getRPM);
  }
}