#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"
#include "src/decoders/decoder_state.h"

extern decoders::detail::state_t _decoderState;

static void test_getCrankAngle(void)
{
  auto run_case = [&](decoder_t &decoder, int toothCount, int trigAngle, int delta, int16_t expected) {
    _decoderState.toothLastToothTime = 2000;
    _decoderState.revolutionOne = false;
    _decoderState.toothCurrentCount = toothCount;
    _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    _decoderState.decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
    TEST_ASSERT_EQUAL(expected, decoder.pGetCrankAngle(_decoderState.toothLastToothTime + delta));
  };

  const int dt = 18; // timeToAngle(100) ~= 18 deg with revolution time 2000

  // 4-cylinder setup (triggerActualTeeth = 5)
  configPage2.nCylinders = 4;
  decoder_t dec4 = triggerSetup_Daihatsu();
  run_case(dec4, 1, 0, 100, 0 + dt);
  run_case(dec4, 2, 0, 100, 30 + dt);
  run_case(dec4, 3, 0, 100, 180 + dt);
  run_case(dec4, 4, 0, 100, 360 + dt);
  run_case(dec4, 5, 0, 100, 540 + dt);

  // trigger angle offset
  run_case(dec4, 2, 10, 100, 30 + 10 + dt);

  // 3-cylinder setup (triggerActualTeeth = 4)
  configPage2.nCylinders = 3;
  decoder_t dec3 = triggerSetup_Daihatsu();
  run_case(dec3, 1, 0, 100, 0 + dt);
  run_case(dec3, 2, 0, 100, 30 + dt);
  run_case(dec3, 3, 0, 100, 240 + dt);
  run_case(dec3, 4, 0, 100, 480 + dt);
}

static void test_getRevolutionTime(void)
{
  auto decoder = triggerSetup_Daihatsu();

  // Prepare state so the revolution time can be calculated
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.startRevolutions = 1; // not cranking
  currentStatus.setRpm(2000);
  currentStatus.revolutionTime = 12345UL;

  // Cam-speed: set times such that (toothOneTime - toothOneMinusOneTime) >> 1 == 60000us
  _decoderState.toothOneMinusOneTime = 2000UL;
  _decoderState.toothOneTime = _decoderState.toothOneMinusOneTime + 120000UL; // >>1 -> 60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());

  // Fallback: when not synced, should return currentStatus.revolutionTime
  _decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(12345UL, decoder.getRevolutionTime());
}

void testDaihatsu(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRevolutionTime);
  }
}