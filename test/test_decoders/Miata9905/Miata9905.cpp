#include "../test_utils.h"
#include "src/decoders/miata9905.h"

static void test_getCrankAngle(void)
{
  statuses current={};
  config4 page4={};
  auto decoderState = decoders::miata9905::initialise(1000, current);

  auto run_case = [&](int toothNum, int16_t expected, int trigAngle = 0) {
    decoderState.toothLastToothTime = 2000;
    decoderState.toothCurrentCount = toothNum;
    // decoderState.decoderStatus.toothAngleIsCorrect = true;
    page4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
    TEST_ASSERT_EQUAL(expected, decoders::miata9905::getCrankAngle(decoderState.toothLastToothTime + 100, decoderState, page4));
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
  statuses current={};
  auto decoderState = decoders::miata9905::initialise(1000, current);

  // --- Cranking branch: current.RPM < current.crankRPM and sync full
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  current.setRpm(0);
  current.crankRPM = 400;
  current.revolutionTime = 99999UL;
  // triggerToothAngle=90 (from decoder setup), gap=15000 -> revTime = 15000 * 360 / 90 = 60000
  decoderState.toothLastMinusOneToothTime = 1000UL;
  decoderState.toothLastToothTime = decoderState.toothLastMinusOneToothTime + 15000UL;
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoders::miata9905::getRevolutionTime(current, decoderState));

  // Missing tooth history while cranking: keep the published period
  decoderState.toothLastToothTime = 0;
  decoderState.toothLastMinusOneToothTime = 0;
  TEST_ASSERT_EQUAL_UINT32(99999UL, decoders::miata9905::getRevolutionTime(current, decoderState));

  // --- Running path: should call stdGetRevolutionTime(CAM_SPEED)
  current.setRpm(2000);
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothOneMinusOneTime = 2000UL;
  decoderState.toothOneTime = decoderState.toothOneMinusOneTime + 120000UL; // >>1 -> 60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoders::miata9905::getRevolutionTime(current, decoderState));

  // --- Fallback: when the revolution time can't be calculated, stdGetRevolutionTime returns current.revolutionTime
  decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(99999UL, decoders::miata9905::getRevolutionTime(current, decoderState));
}

static void test_initialise(void)
{
    {
        statuses current{};
        current.initialisationComplete = true;
        auto subject = decoders::miata9905::initialise(5000, current);
        TEST_ASSERT_EQUAL(0, subject.toothLastToothTime);
    }

    {
        statuses current{};
        current.initialisationComplete = false;
        auto subject = decoders::miata9905::initialise(5000, current);
        TEST_ASSERT_EQUAL(5000, subject.toothLastToothTime);
    }
}

static void resetEndTeeth(decoders::shared::state_t &decoderState)
{
  decoderState.ignitionEndTeeth[0] = 99;
  decoderState.ignitionEndTeeth[1] = 99;
  decoderState.ignitionEndTeeth[2] = 99;
  decoderState.ignitionEndTeeth[3] = 99;
}

static void test_setEndTeeth(void)
{
  statuses current = {};
  config4 page4 = {};
  auto decoderState = decoders::miata9905::initialise(1000, current);

  // Case 1: Sequential mode, Advance >= 10
  resetEndTeeth(decoderState);
  page4.sparkMode = IGN_MODE_SEQUENTIAL;
  current.advance = 15;
  decoders::miata9905::setEndTeeth(current, decoderState, page4);
  TEST_ASSERT_EQUAL(8, decoderState.ignitionEndTeeth[0]);
  TEST_ASSERT_EQUAL(2, decoderState.ignitionEndTeeth[1]);
  TEST_ASSERT_EQUAL(4, decoderState.ignitionEndTeeth[2]);
  TEST_ASSERT_EQUAL(6, decoderState.ignitionEndTeeth[3]);

  // Case 2: Sequential mode, 0 < Advance < 10
  resetEndTeeth(decoderState);
  current.advance = 5;
  decoders::miata9905::setEndTeeth(current, decoderState, page4);
  TEST_ASSERT_EQUAL(1, decoderState.ignitionEndTeeth[0]);
  TEST_ASSERT_EQUAL(3, decoderState.ignitionEndTeeth[1]);
  TEST_ASSERT_EQUAL(5, decoderState.ignitionEndTeeth[2]);
  TEST_ASSERT_EQUAL(7, decoderState.ignitionEndTeeth[3]);

  // Case 2.5: Sequential mode, Advance < 0
  resetEndTeeth(decoderState);
  current.advance = -5;
  decoders::miata9905::setEndTeeth(current, decoderState, page4);
  TEST_ASSERT_EQUAL(99, decoderState.ignitionEndTeeth[0]);
  TEST_ASSERT_EQUAL(99, decoderState.ignitionEndTeeth[1]);
  TEST_ASSERT_EQUAL(99, decoderState.ignitionEndTeeth[2]);
  TEST_ASSERT_EQUAL(99, decoderState.ignitionEndTeeth[3]);

  // Case 3: Non-Sequential mode, Advance >= 10
  resetEndTeeth(decoderState);
  page4.sparkMode = IGN_MODE_WASTED;
  current.advance = 12;
  decoders::miata9905::setEndTeeth(current, decoderState, page4);
  TEST_ASSERT_EQUAL(4, decoderState.ignitionEndTeeth[0]);
  TEST_ASSERT_EQUAL(2, decoderState.ignitionEndTeeth[1]);
  TEST_ASSERT_EQUAL(4, decoderState.ignitionEndTeeth[2]); // Not used
  TEST_ASSERT_EQUAL(2, decoderState.ignitionEndTeeth[3]); // Not used

  // Case 4: Non-Sequential mode, 0 < Advance < 10
  resetEndTeeth(decoderState);
  current.advance = 3;
  decoders::miata9905::setEndTeeth(current, decoderState, page4);
  TEST_ASSERT_EQUAL(1, decoderState.ignitionEndTeeth[0]);
  TEST_ASSERT_EQUAL(3, decoderState.ignitionEndTeeth[1]);
  TEST_ASSERT_EQUAL(1, decoderState.ignitionEndTeeth[2]); // Not used
  TEST_ASSERT_EQUAL(3, decoderState.ignitionEndTeeth[3]); // Not used

  // Case 4: Non-Sequential mode, Advance < 0
  resetEndTeeth(decoderState);
  current.advance = -5;
  decoders::miata9905::setEndTeeth(current, decoderState, page4);
  TEST_ASSERT_EQUAL(99, decoderState.ignitionEndTeeth[0]);
  TEST_ASSERT_EQUAL(99, decoderState.ignitionEndTeeth[1]);
  TEST_ASSERT_EQUAL(99, decoderState.ignitionEndTeeth[2]);
  TEST_ASSERT_EQUAL(99, decoderState.ignitionEndTeeth[3]);
}

void testMiata9905(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_getCrankAngle);
  RUN_TEST_P(test_getRevolutionTime);
  RUN_TEST_P(test_initialise);

  RUN_TEST_P(test_setEndTeeth);

  extern void testMiata9905Triggers(void);
  testMiata9905Triggers();
}