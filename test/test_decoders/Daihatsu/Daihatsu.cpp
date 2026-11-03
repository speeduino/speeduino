#include "../test_utils.h"
#include "src/decoders/daihatsu.h"
#include "../test_decoder_shared.h"

static void test_getRevolutionTime(void)
{
  config2 page2={};
  page2.nCylinders = 3;
  statuses current={};
  auto decoderState = decoders::daihatsu::initialise(page2);

  // Prepare state so the revolution time can be calculated
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  current.startRevolutions = 1; // not cranking
  current.setRpm(2000);
  current.revolutionTime = 12345UL;

  // Cam-speed: set times such that (toothOneTime - toothOneMinusOneTime) >> 1 == 60000us
  decoderState.toothOneMinusOneTime = 2000UL;
  decoderState.toothOneTime = decoderState.toothOneMinusOneTime + 120000UL; // >>1 -> 60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoders::daihatsu::getRevolutionTime(current, decoderState));

  // Fallback: when not synced, should return current.revolutionTime
  decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(12345UL, decoders::daihatsu::getRevolutionTime(current, decoderState));
}

struct daihatsu_context_t {
  config2 page2 = {};
  config4 page4 = {};
  statuses current = {};
  decoders::shared::state_t decoderState;
  
  void init(void)
  {
    decoderState = decoders::daihatsu::initialise(page2);
  }

  void triggerPrimary(uint32_t curTime)
  {
    decoders::daihatsu::triggerPrimary(curTime, current, decoderState, page2, page4);
  }
};

static auto setup_daihatsu_trigger(uint8_t cyls, SyncStatus syncStatus) {
  daihatsu_context_t context;

  context.page2.nCylinders = cyls;
  context.init();

  context.current.rotationStatus = EngineRotationStatus::Running;
  context.decoderState.toothLastToothTime = 100000;
  context.decoderState.toothOneMinusOneTime = 50000;
  context.decoderState.toothCurrentCount = 1;
  context.decoderState.curGap = 2000; // > triggerFilterTime (approx 35us max)
  context.decoderState.decoderStatus.syncStatus = syncStatus;

  return context;
}

static void test_triggerPrimary_rollscounters(void)
{
  auto context = setup_daihatsu_trigger(3, SyncStatus::None);

  auto nextTime = context.decoderState.toothLastToothTime+500UL;
  context.triggerPrimary(nextTime);

  TEST_ASSERT_EQUAL(1, context.decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL(100000UL, context.decoderState.toothLastMinusOneToothTime);
  TEST_ASSERT_EQUAL(nextTime, context.decoderState.toothLastToothTime);
}

static void test_triggerPrimary_syncFull_wrapsCounter(uint8_t cyls)
{
  auto context = setup_daihatsu_trigger(cyls, SyncStatus::Full);

  context.decoderState.toothCurrentCount = context.decoderState.triggerActualTeeth;
  context.triggerPrimary(context.decoderState.toothLastToothTime);

  TEST_ASSERT_EQUAL(1, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(1, context.current.startRevolutions);
}

static void test_triggerPrimary_syncFull_wrapsCounter(void)
{
  test_triggerPrimary_syncFull_wrapsCounter(3);
  test_triggerPrimary_syncFull_wrapsCounter(4);
}

static void test_triggerPrimary_syncFull_incrementsTooth(uint8_t cyls)
{
  auto context = setup_daihatsu_trigger(cyls, SyncStatus::Full);

  context.decoderState.toothCurrentCount = 1;
  context.triggerPrimary(context.decoderState.toothLastToothTime);

  TEST_ASSERT_EQUAL(2, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(0, context.current.startRevolutions);
}

static void test_triggerPrimary_syncFull_incrementsTooth(void)
{
  test_triggerPrimary_syncFull_incrementsTooth(3);
  test_triggerPrimary_syncFull_incrementsTooth(4);
}

static void test_triggerPrimary_syncNone_failsGap_noaction(uint16_t toothCount)
{
  auto context = setup_daihatsu_trigger(3, SyncStatus::None);

  context.decoderState.toothSystemCount = toothCount;
  context.triggerPrimary(context.decoderState.toothLastToothTime+50000UL);

  TEST_ASSERT_EQUAL(SyncStatus::None, context.decoderState.decoderStatus.syncStatus);
}

static void test_triggerPrimary_syncNone_failsGap_noaction(void)
{
  test_triggerPrimary_syncNone_failsGap_noaction(0);
  test_triggerPrimary_syncNone_failsGap_noaction(1);
  test_triggerPrimary_syncNone_failsGap_noaction(2);
  test_triggerPrimary_syncNone_failsGap_noaction(3);
  test_triggerPrimary_syncNone_failsGap_noaction(4);
}

static void test_triggerPrimary_syncNone_passesGap_syncFull(uint8_t cyls, uint16_t toothCount)
{
  auto context = setup_daihatsu_trigger(cyls, SyncStatus::None);

  context.decoderState.toothSystemCount = toothCount;
  context.triggerPrimary(context.decoderState.toothLastToothTime+1);

  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
}

static void test_triggerPrimary_syncNone_passesGap_syncFull(void)
{
  test_triggerPrimary_syncNone_passesGap_syncFull(3, 3);
  test_triggerPrimary_syncNone_passesGap_syncFull(3, 4);
  test_triggerPrimary_syncNone_passesGap_syncFull(4, 3);
  test_triggerPrimary_syncNone_passesGap_syncFull(4, 4);
}

void testDaihatsu(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_getRevolutionTime);
  RUN_TEST_P(test_triggerPrimary_rollscounters);
  RUN_TEST_P(test_triggerPrimary_syncFull_wrapsCounter);
  RUN_TEST_P(test_triggerPrimary_syncFull_incrementsTooth);
  RUN_TEST_P(test_triggerPrimary_syncNone_failsGap_noaction);
  RUN_TEST_P(test_triggerPrimary_syncNone_passesGap_syncFull);
}