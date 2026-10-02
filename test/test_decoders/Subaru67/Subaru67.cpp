#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"
#include "crankMaths.h"
#include "src/decoders/details/decoder_state.h"

extern decoders::detail::state_t _decoderState;
static void test_getCrankAngle(void)
{  
  auto decoder = triggerSetup_Subaru67();

  auto setup_case = [&](int toothNum, int trigAngle) {
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    _decoderState.toothLastMinusOneToothTime = 1500;
    _decoderState.toothLastToothTime = 2000; // toothTime = 500
    _decoderState.triggerToothAngle = 90; // scale interval-based angle to make delta noticeable
    _decoderState.toothCurrentCount = toothNum;
    _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    _decoderState.decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
  };

  auto run_case = [&](int toothNum, int trigAngle, int delta, int16_t expected) {
    setup_case(toothNum, trigAngle);
    TEST_ASSERT_EQUAL(expected, decoder.pGetCrankAngle(_decoderState.toothLastToothTime + delta));
  };

  // dt = timeToAngleIntervalTooth(100) = 100*90/500 = 18
  const int dt = 18;

  // toothAngles: [710,83,115,170,263,295,350,443,475,530,623,655]
  run_case(1, 0, 100,  (710 + dt) - 720); // wraps to small positive angle
  run_case(2, 0, 100,  83 + dt);
  run_case(3, 0, 100,  115 + dt);
  run_case(4, 0, 100,  170 + dt);
  run_case(5, 0, 100,  263 + dt);
  run_case(6, 0, 100,  295 + dt);
  run_case(7, 0, 100,  350 + dt);
  run_case(8, 0, 100,  443 + dt);
  run_case(9, 0, 100,  475 + dt);
  run_case(10,0, 100,  530 + dt);
  run_case(11,0, 100,  623 + dt);
  run_case(12,0, 100,  655 + dt);

  // trigger angle offset
  run_case(2, 10, 100,  83 + dt + 10);

  // Zero if not full sync
  setup_case(1, 0);
  _decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL(0, decoder.pGetCrankAngle(_decoderState.toothLastToothTime + 100));
  setup_case(1, 0);
  _decoderState.decoderStatus.syncStatus = SyncStatus::Partial;
  TEST_ASSERT_EQUAL(0, decoder.pGetCrankAngle(_decoderState.toothLastToothTime + 100));
}

static void test_getRevolutionTime(void)
{
  auto decoder = triggerSetup_Subaru67();

  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.revolutionTime = 12345UL;
  _decoderState.toothOneMinusOneTime = 1000UL;
  _decoderState.toothOneTime = _decoderState.toothOneMinusOneTime + 120000UL; // Cam speed: >>1 -> 60000

  // No speed until the first revolution has completed
  currentStatus.startRevolutions = 0;
  TEST_ASSERT_EQUAL_UINT32(0UL, decoder.getRevolutionTime());

  currentStatus.startRevolutions = 1;
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());
}

void testSubaru67(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRevolutionTime);
  }
}