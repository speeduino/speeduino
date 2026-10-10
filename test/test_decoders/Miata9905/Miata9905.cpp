#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"
#include "crankMaths.h"
#include "src/decoders/decoder_state.h"

extern decoders::detail::state_t _decoderState;

static void test_getCrankAngle(void)
{
  auto decoder = triggerSetup_Miata9905();

  auto run_case = [&](int toothNum, int16_t expected, int trigAngle = 0) {
    _decoderState.toothLastToothTime = 2000;
    _decoderState.toothCurrentCount = toothNum;
    _decoderState.decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
    TEST_ASSERT_EQUAL(expected, decoder.pGetCrankAngle(_decoderState.toothLastToothTime + 100));
  };

  // timeToAngle(100) ~= 18 deg when revolution time is 2000us
  const int dt = 18;

  // toothAngles from triggerSetup_Miata9905 (indices 1..8)
  run_case(1, 710 + dt - 720); // wraps to small positive angle (710+18=728 -> 8)
  run_case(2, 100 + dt);
  run_case(3, 170 + dt);
  run_case(4, 280 + dt);
  run_case(5, 350 + dt);
  run_case(6, 460 + dt);
  run_case(7, 530 + dt);
  run_case(8, 640 + dt);

  // trigger angle offset
  run_case(2, 100 + dt + 10, 10);
}

static void test_getRevolutionTime(void)
{
  auto decoder = triggerSetup_Miata9905();

  // --- Cranking branch: currentStatus.RPM < currentStatus.crankRPM and sync full
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.setRpm(0);
  currentStatus.crankRPM = 400;
  currentStatus.revolutionTime = 99999UL;
  // triggerToothAngle=90 (from decoder setup), gap=15000 -> revTime = 15000 * 360 / 90 = 60000
  _decoderState.toothLastMinusOneToothTime = 1000UL;
  _decoderState.toothLastToothTime = _decoderState.toothLastMinusOneToothTime + 15000UL;
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());

  // Missing tooth history while cranking: keep the published period
  _decoderState.toothLastToothTime = 0;
  _decoderState.toothLastMinusOneToothTime = 0;
  TEST_ASSERT_EQUAL_UINT32(99999UL, decoder.getRevolutionTime());

  // --- Running path: should call stdGetRevolutionTime(CAM_SPEED)
  currentStatus.setRpm(2000);
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  _decoderState.toothOneMinusOneTime = 2000UL;
  _decoderState.toothOneTime = _decoderState.toothOneMinusOneTime + 120000UL; // >>1 -> 60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());

  // --- Fallback: when the revolution time can't be calculated, stdGetRevolutionTime returns currentStatus.revolutionTime
  _decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(99999UL, decoder.getRevolutionTime());
}

void testMiata9905(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRevolutionTime);
  }
}