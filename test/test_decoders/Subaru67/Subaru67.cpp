#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"
#include "crankMaths.h"

static void test_getCrankAngle(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile uint32_t toothLastToothTime;
  extern volatile unsigned long toothLastMinusOneToothTime;
  extern volatile uint16_t triggerToothAngle;
  extern volatile int toothCurrentCount;

  auto decoder = triggerSetup_Subaru67();

  auto setup_case = [&](int toothNum, int trigAngle) {
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    toothLastMinusOneToothTime = 1500;
    toothLastToothTime = 2000; // toothTime = 500
    triggerToothAngle = 90; // scale interval-based angle to make delta noticeable
    toothCurrentCount = toothNum;
    decoderStatus.syncStatus = SyncStatus::Full;
    decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
  };

  auto run_case = [&](int toothNum, int trigAngle, int delta, int16_t expected) {
    setup_case(toothNum, trigAngle);
    TEST_ASSERT_EQUAL(expected, decoder.pGetCrankAngle(toothLastToothTime + delta));
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
  decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL(0, decoder.pGetCrankAngle(toothLastToothTime + 100));
  setup_case(1, 0);
  decoderStatus.syncStatus = SyncStatus::Partial;
  TEST_ASSERT_EQUAL(0, decoder.pGetCrankAngle(toothLastToothTime + 100));
}

static void test_getRPM(void)
{
  auto decoder = triggerSetup_Subaru67();

  currentStatus.startRevolutions = 0;
  TEST_ASSERT_EQUAL(0, decoder.getRPM());

  currentStatus.startRevolutions = 1;
  TEST_ASSERT_NOT_EQUAL(0, decoder.getRPM());
}

void testSubaru67(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRPM);
  }
}