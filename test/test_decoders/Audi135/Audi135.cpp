#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"
#include "src/decoders/decoder_state.h"

extern decoders::detail::state_t _decoderState;

static void test_getCrankAngle(void)
{
  // Configure and create decoder
  auto decoder = triggerSetup_Audi135();

  auto run_case = [&](int toothCount, bool revOne, int delta, int trigAngle, int16_t expected) {
    _decoderState.toothLastToothTime = 2000;
    _decoderState.toothCurrentCount = toothCount;
    _decoderState.revolutionOne = revOne;
    _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    _decoderState.decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
    TEST_ASSERT_EQUAL(expected, decoder.pGetCrankAngle(_decoderState.toothLastToothTime + delta));
  };

  // timeToAngle(100) ~= 18 deg when revolution time = 2000
  const int dt = 18;

  // Basic checks
  run_case(1, false, 100, 0, 0 + dt);
  run_case(3, false, 100, 0, (3 - 1) * 8 + dt); // tooth 3 -> 16 + dt
  run_case(10, false, 100, 0, (10 - 1) * 8 + dt); // tooth 10 -> 72 + dt

  // Last-tooth and zero-case (zero treated as 45)
  run_case(45, false, 100, 0, (45 - 1) * 8 + dt);
  run_case(0, false, 100, 0, (45 - 1) * 8 + dt);

  // trigger angle offset
  run_case(3, false, 100, 5, (3 - 1) * 8 + 5 + dt);

  // sequential/revolution cases
  run_case(1, true, 100, 0, 360 + 0 + dt);
  // Case where adding 360 wraps above 720: tooth 45 + revOne -> (352+dt+360)-720
  {
    int raw = (45 - 1) * 8 + dt + 360;
    int wrapped = raw >= 720 ? raw - 720 : raw;
    run_case(45, true, 100, 0, wrapped);
  }
}

static void test_getRevolutionTime(void)
{
  auto decoder = triggerSetup_Audi135();

  // Standard calculation: the time between the last 2 tooth #1
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.startRevolutions = 1; // not cranking
  _decoderState.toothOneMinusOneTime = 1000UL;
  _decoderState.toothOneTime = _decoderState.toothOneMinusOneTime + 60000UL;
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());
}

void testAudi135(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRevolutionTime);
  }
}