#include "../../test_utils.h"
#include "src/decoders/webber.h"
#include "src/decoders/dualWheel.h"
#include "../../test_decoders/test_decoder_shared.h"

struct context_webber_t
{
  statuses current = {};
  config4 page4 = {};
  decoders::shared::state_t decoderState;

  void init(void)
  {
    page4.triggerTeeth = 4;    
    decoderState = decoders::dual_wheel::initialise(page4);
  }

  void triggerPrimary(uint32_t curTime)
  {
    decoders::webber::triggerPrimary(curTime, current, decoderState, page4);
  }

  void triggerSecondary(uint32_t curTime)
  {
    decoders::webber::triggerSecondary(curTime, current, decoderState, page4);
  }

  void setup_primaryfilter(uint32_t lastGap)
  {
    decoderState.toothLastToothTime = 100000;
    decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime - lastGap;
  }

  void setup_secondaryfilter(uint32_t lastGap)
  {
    decoderState.toothLastSecToothTime = 100000;
    decoderState.toothLastMinusOneSecToothTime = decoderState.toothLastSecToothTime - lastGap;
  }
};

static void test_triggerPrimary_filter_pass(void)
{
  context_webber_t context{};
    
  context.init();
  context.setup_primaryfilter(1000);
  context.decoderState.toothCurrentCount = 10;
  
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));

  // Verify increment of tooth count
  TEST_ASSERT_EQUAL(11, context.decoderState.toothCurrentCount);
  // Verify validTrigger flag set
  TEST_ASSERT_EQUAL(true, context.decoderState.decoderStatus.validTrigger);
}

static void test_triggerPrimary_filter_fail(void)
{
  context_webber_t context{};
  
  context.init();
  context.decoderState.toothLastToothTime = 100000;
  context.decoderState.toothCurrentCount = 10;
  
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState)-2);

  // Verify no increment and invalid flag
  TEST_ASSERT_EQUAL(10, context.decoderState.toothCurrentCount); 
  TEST_ASSERT_EQUAL(false, context.decoderState.decoderStatus.validTrigger);
}

static void test_triggerPrimary_filter_update(void)
{
  context_webber_t context{};
  
  context.init();
  context.setup_primaryfilter(1000);
  context.decoderState.toothCurrentCount = 10;
  
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState)+2000);

  // Verify filter time updated to 150% crank tooth (2000 + 1000 = 3000)
  TEST_ASSERT_EQUAL(4251, context.decoderState.triggerSecFilterTime);
}

static void setup_nosync(context_webber_t &context, uint16_t secondaryToothCount, uint16_t checkSyncToothCount)
{
  context.init();
  context.setup_primaryfilter(1000);
  context.decoderState.checkSyncToothCount = checkSyncToothCount; 
  context.decoderState.secondaryToothCount = secondaryToothCount;
  context.decoderState.toothCurrentCount = 10;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::None;
  context.decoderState.revolutionOne = true;
}

static void test_triggerPrimary_nosync_sync(void)
{
  context_webber_t context{};
  setup_nosync(context, 1, 3);

  // Trigger time with valid gap
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState)+1000);

  // Verify sync acquisition and count reset to 2 (Webber uses 4-cam, 16-crank setup)
  TEST_ASSERT_EQUAL(2, context.decoderState.toothCurrentCount); 
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_FALSE(context.decoderState.revolutionOne);
}

static void test_triggerPrimary_nosync_nosync(uint16_t secondaryToothCount, uint16_t checkSyncToothCount)
{
  context_webber_t context{};
  setup_nosync(context, secondaryToothCount, checkSyncToothCount);

  // Trigger time with valid gap
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState)+1000);

  TEST_ASSERT_EQUAL(SyncStatus::None, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_TRUE(context.decoderState.revolutionOne);
}

static void test_triggerPrimary_nosync_nosync(void)
{
  test_triggerPrimary_nosync_nosync(2, 3);
  test_triggerPrimary_nosync_nosync(1, 2);
}

static void test_triggerPrimary_revolution_wrap(void)
{
  context_webber_t context{};
  
  // Initial state: Just before wrap (16th tooth on 16-teeth engine)
  context.init();
  context.setup_primaryfilter(1000);
  context.decoderState.checkSyncToothCount = 4; 
  context.decoderState.secondaryToothCount = 1;
  context.decoderState.revolutionOne = false;
  context.decoderState.toothCurrentCount = 16; // Just before wrap
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState)+1000);

  // Verify wrap: count resets to 1, revolutionOne flips, startRevolutions increments
  TEST_ASSERT_EQUAL(1, context.decoderState.toothCurrentCount); 
  TEST_ASSERT_EQUAL(true, context.decoderState.revolutionOne); 
  TEST_ASSERT_EQUAL(true, context.current.startRevolutions);
}

// Helper setup function for secondary trigger tests
static void setup_secondary_trigger(context_webber_t &context, SyncStatus syncStatus, uint16_t checkCount, uint16_t secCount)
{
  context.init();
  context.setup_secondaryfilter(1000);
  context.decoderState.decoderStatus.syncStatus = syncStatus;
  context.decoderState.toothCurrentCount = 13;
  context.decoderState.checkSyncToothCount = checkCount;
  context.decoderState.secondaryToothCount = secCount;
  context.decoderState.revolutionOne = true;
  context.decoderState.triggerSecFilterTime = 3333;
  context.decoderState.curGap = 1000;
}

static void setup_secondary_trigger_synced(context_webber_t &context, uint16_t checkCount, uint16_t secCount)
{
  setup_secondary_trigger(context, SyncStatus::Full, checkCount, secCount);
}

static void setup_secondary_trigger_unsynced(context_webber_t &context, uint16_t checkCount, uint16_t secCount)
{
  setup_secondary_trigger(context, SyncStatus::None, checkCount, secCount);
}

static void test_triggerSecondary_gap_fail(void)
{
  context_webber_t context{};
  setup_secondary_trigger_unsynced(context, 1, 1);

  auto nextTime = calcNextTimePassSecondaryFilter(context.decoderState)-2;
  context.triggerSecondary(nextTime);

  TEST_ASSERT_EQUAL(100000, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_NOT_EQUAL(nextTime, context.decoderState.toothLastToothTime);
  TEST_ASSERT_NOT_EQUAL(nextTime, context.decoderState.toothLastMinusOneToothTime);
  TEST_ASSERT_EQUAL(1, context.decoderState.checkSyncToothCount);
  TEST_ASSERT_EQUAL(1, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL(1500, context.decoderState.triggerSecFilterTime);
  TEST_ASSERT_EQUAL(SyncStatus::None, context.decoderState.decoderStatus.syncStatus);
}

static void test_triggerSecondary_gap_pass(SyncStatus syncStatus, uint16_t checkCount, uint16_t secCount)
{
  context_webber_t context{};
  setup_secondary_trigger(context, syncStatus, checkCount, secCount);

  auto nextTime = calcNextTimePassSecondaryFilter(context.decoderState);
  context.triggerSecondary(nextTime);

  TEST_ASSERT_EQUAL(nextTime, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_NOT_EQUAL(nextTime, context.decoderState.toothLastToothTime);
  TEST_ASSERT_NOT_EQUAL(nextTime, context.decoderState.toothLastMinusOneToothTime);
  TEST_ASSERT_EQUAL(1, context.decoderState.checkSyncToothCount);
  TEST_ASSERT_EQUAL(6, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL(1500, context.decoderState.triggerSecFilterTime);
  TEST_ASSERT_EQUAL(syncStatus, context.decoderState.decoderStatus.syncStatus);
}

static void test_triggerSecondary_gap_pass(void)
{
  test_triggerSecondary_gap_pass(SyncStatus::Full, 5, 5);
  test_triggerSecondary_gap_pass(SyncStatus::None, 5, 5);
}

static void assert_triggerSecondary_3_2(const decoders::shared::state_t &decoderState)
{
  TEST_ASSERT_EQUAL(4000, decoderState.triggerSecFilterTime);
  TEST_ASSERT_EQUAL(1, decoderState.secondaryToothCount);
  TEST_ASSERT_TRUE(decoderState.revolutionOne);
}

static void test_triggerSecondary_unsynced_3_2(void)
{
  context_webber_t context{};
  setup_secondary_trigger_unsynced(context, 3, 2);
  auto nextTime = calcNextTimePassSecondaryFilter(context.decoderState);
  context.triggerSecondary(nextTime);

  TEST_ASSERT_EQUAL(nextTime, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL(nextTime, context.decoderState.toothLastToothTime);
  TEST_ASSERT_EQUAL(nextTime - 1500000, context.decoderState.toothLastMinusOneToothTime);
  TEST_ASSERT_EQUAL(3, context.decoderState.checkSyncToothCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(context.page4.triggerTeeth-1, context.decoderState.toothCurrentCount);
  assert_triggerSecondary_3_2(context.decoderState);
}

static void test_triggerSecondary_synced_3_2_no_syncloss(uint16_t toothCurrentCount, uint16_t startRevolutions)
{
  context_webber_t context{};
  setup_secondary_trigger_synced(context, 3, 2);
  context.page4.useResync = false;
  context.current.startRevolutions = startRevolutions;
  context.decoderState.toothCurrentCount = toothCurrentCount;
  auto nextTime = calcNextTimePassSecondaryFilter(context.decoderState);
  context.triggerSecondary(nextTime);

  TEST_ASSERT_EQUAL(0, context.current.syncLossCounter);
  TEST_ASSERT_EQUAL(3, context.decoderState.checkSyncToothCount);
  TEST_ASSERT_EQUAL(toothCurrentCount, context.decoderState.toothCurrentCount);
  assert_triggerSecondary_3_2(context.decoderState);
}

static void test_triggerSecondary_synced_3_2_no_syncloss(void)
{
  test_triggerSecondary_synced_3_2_no_syncloss(1, 1);
  test_triggerSecondary_synced_3_2_no_syncloss(3, 1);
  test_triggerSecondary_synced_3_2_no_syncloss(3, 3);
}

static void test_triggerSecondary_synced_3_2_syncloss(void)
{
  context_webber_t context{};
  setup_secondary_trigger_synced(context, 3, 2);
  context.page4.useResync = false;
  context.current.startRevolutions = 5;
  auto nextTime = calcNextTimePassSecondaryFilter(context.decoderState);
  context.triggerSecondary(nextTime);

  TEST_ASSERT_EQUAL(1, context.current.syncLossCounter);
  TEST_ASSERT_EQUAL(3, context.decoderState.checkSyncToothCount);
  TEST_ASSERT_EQUAL(13, context.decoderState.toothCurrentCount);
  assert_triggerSecondary_3_2(context.decoderState);
}

static void test_triggerSecondary_synced_3_2_syncloss_resync(void)
{
  context_webber_t context{};
  setup_secondary_trigger_synced(context, 3, 2);
  context.page4.useResync = true;
  context.current.startRevolutions = 5;
  auto nextTime = calcNextTimePassSecondaryFilter(context.decoderState);
  context.triggerSecondary(nextTime);

  TEST_ASSERT_EQUAL(1, context.current.syncLossCounter);
  TEST_ASSERT_EQUAL(3, context.decoderState.checkSyncToothCount);
  TEST_ASSERT_EQUAL(context.page4.triggerTeeth-1, context.decoderState.toothCurrentCount);
  assert_triggerSecondary_3_2(context.decoderState);
}

static void test_triggerSecondary_unsynced_n_0(void)
{
  context_webber_t context{};
  setup_secondary_trigger_unsynced(context, 3, 0);
  auto nextTime = calcNextTimePassSecondaryFilter(context.decoderState);
  context.triggerSecondary(nextTime);

  TEST_ASSERT_EQUAL(nextTime, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL(nextTime, context.decoderState.toothLastToothTime);
  TEST_ASSERT_EQUAL(nextTime - 1500000, context.decoderState.toothLastMinusOneToothTime);
  TEST_ASSERT_EQUAL(3, context.decoderState.checkSyncToothCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(1, context.decoderState.toothCurrentCount);
  TEST_ASSERT_TRUE(context.decoderState.revolutionOne);
}

void testWebber(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST(test_triggerPrimary_filter_pass);
  RUN_TEST(test_triggerPrimary_filter_fail);
  RUN_TEST(test_triggerPrimary_filter_update);
  RUN_TEST(test_triggerPrimary_nosync_sync);
  RUN_TEST(test_triggerPrimary_nosync_nosync);
  RUN_TEST(test_triggerPrimary_revolution_wrap);

  RUN_TEST(test_triggerSecondary_gap_fail);
  RUN_TEST(test_triggerSecondary_gap_pass);
  RUN_TEST(test_triggerSecondary_unsynced_3_2);
  RUN_TEST(test_triggerSecondary_synced_3_2_no_syncloss);
  RUN_TEST(test_triggerSecondary_synced_3_2_syncloss);
  RUN_TEST(test_triggerSecondary_synced_3_2_syncloss_resync);
  RUN_TEST(test_triggerSecondary_unsynced_n_0);
}
