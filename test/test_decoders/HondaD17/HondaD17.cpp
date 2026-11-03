#include "../test_utils.h"
#include "src/decoders/hondad17.h"

static void test_getCrankAngle(void)
{
  config4 page4={};
  auto decoderState = decoders::hondad17::initialise();

  // Use deterministic time->angle conversion
  CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
  decoderState.revolutionOne = false;
  setAngleConverterRevolutionTime(2000);

  // Setup common deterministic state
  page4.triggerAngle = 0;
  decoderState.toothLastToothTime = 5000;

  // decoderState.toothCurrentCount == 1 -> angle = 0 + triggerAngle + timeToAngle(dt)
  decoderState.toothCurrentCount = 1;
  TEST_ASSERT_EQUAL(18, decoders::hondad17::getCrankAngle(decoderState.toothLastToothTime + 100, decoderState, page4));

  // decoderState.toothCurrentCount == 2 -> angle = 1*triggerToothAngle + triggerAngle + timeToAngle(dt)
  decoderState.toothCurrentCount = 2;
  TEST_ASSERT_EQUAL(48, decoders::hondad17::getCrankAngle(decoderState.toothLastToothTime + 100, decoderState, page4));

  // decoderState.toothCurrentCount == 0 -> treated as 13th tooth -> use 11*triggerToothAngle
  decoderState.toothCurrentCount = 0;
  TEST_ASSERT_EQUAL(339, decoders::hondad17::getCrankAngle(decoderState.toothLastToothTime + 50, decoderState, page4));

  // Trigger angle offset and wrap-around behavior
  page4.triggerAngle = 500;
  decoderState.toothCurrentCount = 12; // base = 11*triggerToothAngle = 330
  TEST_ASSERT_EQUAL(112, decoders::hondad17::getCrankAngle(decoderState.toothLastToothTime + 10, decoderState, page4));
}

static void test_primary_updates_regular_unsynced_tooth(void)
{
  statuses current{};
  auto decoderState = decoders::hondad17::initialise();
  decoderState.curGap = 400;
  decoderState.toothLastMinusOneToothTime = 800;
  decoderState.toothLastToothTime = 1000;
  decoderState.toothCurrentCount = 4;

  decoders::hondad17::triggerPrimary(1500, current, decoderState);

  TEST_ASSERT_EQUAL_UINT32(400, decoderState.lastGap);
  TEST_ASSERT_EQUAL_UINT32(500, decoderState.curGap);
  TEST_ASSERT_EQUAL_UINT32(200, decoderState.targetGap);
  TEST_ASSERT_EQUAL_UINT16(5, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(SyncStatus::None, decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastMinusOneToothTime);
  TEST_ASSERT_EQUAL_UINT32(1500, decoderState.toothLastToothTime);
}

static void test_primary_acquires_sync_on_short_gap(void)
{
  statuses current{};
  auto decoderState = decoders::hondad17::initialise();
  decoderState.curGap = 1000;
  decoderState.toothLastToothTime = 1000;
  decoderState.toothCurrentCount = 4;

  decoders::hondad17::triggerPrimary(1200, current, decoderState);

  TEST_ASSERT_EQUAL_UINT32(500, decoderState.targetGap);
  TEST_ASSERT_EQUAL_UINT16(0, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastToothTime);
}

static void test_primary_skips_timestamp_update_on_synced_tooth_13(void)
{
  statuses current{};
  auto decoderState = decoders::hondad17::initialise();
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothCurrentCount = 12;
  decoderState.toothLastToothTime = 1000;
  decoderState.toothLastMinusOneToothTime = 800;

  decoders::hondad17::triggerPrimary(1200, current, decoderState);

  TEST_ASSERT_EQUAL_UINT16(0, decoderState.toothCurrentCount);
  TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastToothTime);
  TEST_ASSERT_EQUAL_UINT32(800, decoderState.toothLastMinusOneToothTime);
  TEST_ASSERT_EQUAL_UINT32(0, current.startRevolutions);
}

static void test_primary_starts_new_revolution_at_tooth_one(void)
{
  statuses current{};
  auto decoderState = decoders::hondad17::initialise();
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothCurrentCount = 0;
  decoderState.toothLastToothTime = 1000;
  decoderState.toothOneMinusOneTime = 200;
  decoderState.toothOneTime = 600;

  decoders::hondad17::triggerPrimary(1200, current, decoderState);

  TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(1, current.startRevolutions);
  TEST_ASSERT_EQUAL_UINT32(600, decoderState.toothOneMinusOneTime);
  TEST_ASSERT_EQUAL_UINT32(1200, decoderState.toothOneTime);
  TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastMinusOneToothTime);
  TEST_ASSERT_EQUAL_UINT32(1200, decoderState.toothLastToothTime);
}

void testHondaD17(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_getCrankAngle);
  RUN_TEST_P(test_primary_updates_regular_unsynced_tooth);
  RUN_TEST_P(test_primary_acquires_sync_on_short_gap);
  RUN_TEST_P(test_primary_skips_timestamp_update_on_synced_tooth_13);
  RUN_TEST_P(test_primary_starts_new_revolution_at_tooth_one);
}