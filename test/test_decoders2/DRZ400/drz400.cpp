#include "../../test_utils.h"
#include "src/decoders/drz400.h"

struct context_drz400_t
{
  statuses current = {};
  config4 page4 = {};
  decoders::shared::state_t decoderState;

  void init(void)
  {
    decoderState = decoders::drz400::initialise(page4);
  }

  void triggerSecondary(uint32_t curTime)
  {
    decoders::drz400::triggerSecondary(curTime, current, decoderState, page4);
  }

  void setup_secondaryfilter(uint32_t lastGap)
  {
    decoderState.toothLastSecToothTime = 100000;
    decoderState.toothLastMinusOneSecToothTime = decoderState.toothLastSecToothTime - lastGap;
  }
};

static void test_triggerSecondary_acquires_sync(void)
{
  context_drz400_t context{};
  context.page4.triggerTeeth = 4;
  context.page4.TrigSpeed = CAM_SPEED;
  context.init();

  context.decoderState.toothLastSecToothTime = 100000;
  context.decoderState.triggerSecFilterTime = 100;

  context.triggerSecondary(100100);

  TEST_ASSERT_EQUAL_UINT32(100100, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL_UINT8(4, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT16(1, context.current.syncLossCounter);
}

static void test_triggerSecondary_already_synced(void)
{
  context_drz400_t context{};
  context.page4.triggerTeeth = 4;
  context.init();

  context.decoderState.toothLastSecToothTime = 100000;
  context.decoderState.triggerSecFilterTime = 100;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.decoderState.toothCurrentCount = 3;

  context.triggerSecondary(100100);

  TEST_ASSERT_EQUAL_UINT32(100100, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL_UINT8(6, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT16(0, context.current.syncLossCounter);
}

static void test_triggerSecondary_rejects_filtered_pulse(void)
{
  context_drz400_t context{};
  context.page4.triggerTeeth = 4;
  context.init();

  context.decoderState.toothLastSecToothTime = 100000;
  context.decoderState.triggerSecFilterTime = 101;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::None;
  context.decoderState.toothOneTime = 3000;
  context.decoderState.toothOneMinusOneTime = 1000;

  context.triggerSecondary(100100); // Gap is 100, below the filter.

  TEST_ASSERT_EQUAL_UINT32(100000, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL(SyncStatus::None, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT16(0, context.current.syncLossCounter);
  TEST_ASSERT_EQUAL_UINT32(1000, context.decoderState.triggerSecFilterTime);
}

void testDRZ400(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST(test_triggerSecondary_acquires_sync);
  RUN_TEST(test_triggerSecondary_already_synced);
  RUN_TEST(test_triggerSecondary_rejects_filtered_pulse);
}