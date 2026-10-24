#include "../test_utils.h"
#include "src/decoders/4g63.h"

static void test_getRevolutionTime(void)
{
    config2 page2={};
    config4 page4={};
    statuses current={};
  auto decoderState = decoders::fourg63::initialise(1000, current, page2);

  // Cranking path: current.RPM < current.crankRPM
  current.setRpm(0);
  current.crankRPM = 400;
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  // Simulate two tooth times 500us apart -> delta = 500
  decoderState.toothLastToothTime = 2000;
  decoderState.toothLastMinusOneToothTime = 1500;

  // For a 70 degree tooth gap (one of the 4G63 cranking gaps)
  decoderState.triggerToothAngle = 70;
  TEST_ASSERT_EQUAL_UINT32(2571UL, decoders::fourg63::getRevolutionTime(current, decoderState, page4)); // 500 * 360 / 70 = 2571

  // For a 110 degree tooth gap
  decoderState.triggerToothAngle = 110;
  TEST_ASSERT_EQUAL_UINT32(1636UL, decoders::fourg63::getRevolutionTime(current, decoderState, page4)); // 500 * 360 / 110 = 1636

  // Running path: should return stdGetRevolutionTime(CAM_SPEED) -> defaults to current.revolutionTime when no revolution update
  current.setRpm(1000);
  current.revolutionTime = 60000UL;
  // Clear toothOne* so stdGetRevolutionTime can't calculate the revolution time
  decoderState.toothOneTime = 0;
  decoderState.toothOneMinusOneTime = 0;
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoders::fourg63::getRevolutionTime(current, decoderState, page4));

  // No sync: the speed is unknown
  decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(0UL, decoders::fourg63::getRevolutionTime(current, decoderState, page4));
}

  static void test_getRevolutionTime_uses_published_period_without_tooth_interval(void)
  {
    config4 page4{};
    config2 page2{};
    statuses current{};
    auto decoderState = decoders::fourg63::initialise(1000, current, page2);

    current.setRpm(0);
    current.crankRPM = 400;
    current.revolutionTime = 123456UL;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.triggerToothAngle = 70;
    decoderState.toothLastToothTime = 0;
    decoderState.toothLastMinusOneToothTime = 0;

    TEST_ASSERT_EQUAL_UINT32(123456UL, decoders::fourg63::getRevolutionTime(current, decoderState, page4));
  }

void test4G63RevTime(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_getRevolutionTime);
    RUN_TEST_P(test_getRevolutionTime_uses_published_period_without_tooth_interval);
}