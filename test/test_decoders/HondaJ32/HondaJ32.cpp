#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"
#include "crankMaths.h"
#include "src/decoders/details/decoder_state.h"

extern decoders::detail::state_t _decoderState;

static void test_getCrankAngle(void)
{
  auto decoder = triggerSetup_HondaJ32();

  auto run_case = [&](uint16_t toothNum, int16_t expected, int16_t triggerAngle = 0) {
    _decoderState.toothLastToothTime = 2000;
    _decoderState.toothCurrentCount = toothNum;
    _decoderState.decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = triggerAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 360;
    setAngleConverterRevolutionTime(2000);
    TEST_ASSERT_EQUAL(expected, decoder.pGetCrankAngle(_decoderState.toothLastToothTime + 100));  
  };

  // Basic teeth
  // triggerToothAngle = 15 deg. timeToAngle(100) ~= 18 deg
  run_case(1, 15 + 18);
  run_case(2, 30 + 18);

  // Special teeth
  // tooth 14 -> 213 + 18
  run_case(14, 213 + 18);
  // tooth 22 -> 333 + 18
  run_case(22, 333 + 18);

  // Wrap-around: tooth 24 -> 24*15 + 18 = 378 -> subtract 360 => 18
  run_case(24, 18);

  // Tooth zero case
  run_case(0, 0 + 18);

  // Non-zero trigger angle offset
  run_case(1, 15 + 18 + 10, 10);
}

static void test_getRevolutionTime(void)
{
  auto decoder = triggerSetup_HondaJ32();
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.revolutionTime = 12345UL;
  // No tooth #1 history yet: keep the published period
  TEST_ASSERT_EQUAL_UINT32(12345UL, decoder.getRevolutionTime());

  // The time between the last 2 tooth #1 (as recorded by the trigger handler)
  _decoderState.toothOneMinusOneTime = 1000UL;
  _decoderState.toothOneTime = _decoderState.toothOneMinusOneTime + 60000UL;
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());

  // No sync: the tooth #1 times may be stale, so keep the published period
  _decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(12345UL, decoder.getRevolutionTime());
}

void testHondaJ32(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRevolutionTime);
  }
}