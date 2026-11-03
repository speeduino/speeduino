#include "../../test_utils.h"
#include "src/decoders/subaru6_7.h"

static void test_getCrankAngle(void)
{
  config4 page4={};
  auto decoderState = decoders::subaru67::initialise();

  auto setup_case = [&](int toothNum, int trigAngle) {
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    decoderState.toothLastMinusOneToothTime = 1500;
    decoderState.toothLastToothTime = 2000; // toothTime = 500
    decoderState.triggerToothAngle = 90; // scale interval-based angle to make delta noticeable
    decoderState.toothCurrentCount = toothNum;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.decoderStatus.toothAngleIsCorrect = true;
    page4.triggerAngle = trigAngle;
  };

  auto run_case = [&](int toothNum, int trigAngle, int delta, int16_t expected) {
    setup_case(toothNum, trigAngle);
    TEST_ASSERT_EQUAL(expected, decoders::subaru67::getCrankAngle(decoderState.toothLastToothTime + delta, decoderState, page4));
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
  decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL(0, decoders::subaru67::getCrankAngle(decoderState.toothLastToothTime + 100, decoderState, page4));
  setup_case(1, 0);
  decoderState.decoderStatus.syncStatus = SyncStatus::Partial;
  TEST_ASSERT_EQUAL(0, decoders::subaru67::getCrankAngle(decoderState.toothLastToothTime + 100, decoderState, page4));
}

static void test_getRevolutionTime(void)
{
  statuses current={};
  auto decoderState = decoders::subaru67::initialise();

  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  current.revolutionTime = 12345UL;
  decoderState.toothOneMinusOneTime = 1000UL;
  decoderState.toothOneTime = decoderState.toothOneMinusOneTime + 120000UL; // Cam speed: >>1 -> 60000

  // No speed until the first revolution has completed
  current.startRevolutions = 0;
  TEST_ASSERT_EQUAL_UINT32(0UL, decoders::subaru67::getRevolutionTime(current, decoderState));

  current.startRevolutions = 1;
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoders::subaru67::getRevolutionTime(current, decoderState));
}


static void test_setEndTeeth_sequential_advanced(void)
{
  statuses current;
  config4 page4;
  auto decoderState = decoders::subaru67::initialise();
  
  // Set up configs
  page4.sparkMode = IGN_MODE_SEQUENTIAL;
  current.advance = 15; // >= 10
  
  // Execute function
  decoders::subaru67::setEndTeeth(current, decoderState, page4);
  
  // Verify results (Sequential, Advanced)
  TEST_ASSERT_EQUAL(12, decoderState.ignitionEndTeeth[0]);
  TEST_ASSERT_EQUAL(3, decoderState.ignitionEndTeeth[1]);
  TEST_ASSERT_EQUAL(6, decoderState.ignitionEndTeeth[2]);
  TEST_ASSERT_EQUAL(9, decoderState.ignitionEndTeeth[3]);
}

static void test_setEndTeeth_sequential_retarded(void)
{
  statuses current;
  config4 page4;
  auto decoderState = decoders::subaru67::initialise();
  
  // Set up configs
  page4.sparkMode = IGN_MODE_SEQUENTIAL;
  current.advance = 5; // < 10
  
  // Execute function
  decoders::subaru67::setEndTeeth(current, decoderState, page4);
  
  // Verify results (Sequential, Retarded)
  TEST_ASSERT_EQUAL(1, decoderState.ignitionEndTeeth[0]);
  TEST_ASSERT_EQUAL(4, decoderState.ignitionEndTeeth[1]);
  TEST_ASSERT_EQUAL(7, decoderState.ignitionEndTeeth[2]);
  TEST_ASSERT_EQUAL(10, decoderState.ignitionEndTeeth[3]);
}

static void test_setEndTeeth_non_sequential_advanced(void)
{
  statuses current;
  config4 page4;
  auto decoderState = decoders::subaru67::initialise();
  
  // Set up configs
  page4.sparkMode = IGN_MODE_WASTED;
  current.advance = 15; // >= 10
  
  // Execute function
  decoders::subaru67::setEndTeeth(current, decoderState, page4);
  
  // Verify results (Non-Sequential, Advanced)
  TEST_ASSERT_EQUAL(6, decoderState.ignitionEndTeeth[0]);
  TEST_ASSERT_EQUAL(3, decoderState.ignitionEndTeeth[1]);
}

static void test_setEndTeeth_non_sequential_retarded(void)
{
  statuses current;
  config4 page4;
  auto decoderState = decoders::subaru67::initialise();
  
  // Set up configs
  page4.sparkMode = IGN_MODE_WASTED;
  current.advance = 5; // < 10
  
  // Execute function
  decoders::subaru67::setEndTeeth(current, decoderState, page4);
  
  // Verify results (Non-Sequential, Retarded)
  TEST_ASSERT_EQUAL(1, decoderState.ignitionEndTeeth[0]);
  TEST_ASSERT_EQUAL(4, decoderState.ignitionEndTeeth[1]);
}

void testSubaru67(void)
{
  extern void testSubaru67Triggers(void);

  testSubaru67Triggers();

  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRevolutionTime);

    RUN_TEST(test_setEndTeeth_sequential_advanced);
    RUN_TEST(test_setEndTeeth_sequential_retarded);
    RUN_TEST(test_setEndTeeth_non_sequential_advanced);
    RUN_TEST(test_setEndTeeth_non_sequential_retarded);    
  }
}