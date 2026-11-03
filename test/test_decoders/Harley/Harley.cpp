#include "../../test_utils.h"
#include "src/decoders/harley.h"
#include "units.h"

static void test_getCrankAngle(void)
{
  statuses current={};
  config4 page4={};
  auto decoderState = decoders::harley::initialise(1000, current);

  // Make time->angle deterministic for tests
  CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
  setAngleConverterRevolutionTime(2000);
    
  // Base case: tooth 1 should map to 0 + triggerAngle
  page4.triggerAngle = 0;
  decoderState.toothLastToothTime = 10000;
  decoderState.toothCurrentCount = 1;
  {
    int16_t angle = decoders::harley::getCrankAngle(decoderState.toothLastToothTime + 100, decoderState, page4);
    int16_t expected = 0 + timeToAngle(100);
    TEST_ASSERT_EQUAL(expected, angle);
  }

  // Tooth 2 should map to 157 + triggerAngle
  decoderState.toothCurrentCount = 2;
  {
    int16_t angle = decoders::harley::getCrankAngle(decoderState.toothLastToothTime + 100, decoderState, page4);
    int16_t expected = 157 + timeToAngle(100);
    TEST_ASSERT_EQUAL(expected, angle);
  }

  // Tooth 3 behaves like tooth 1 (reference tooth)
  decoderState.toothCurrentCount = 3;
  {
    int16_t angle = decoders::harley::getCrankAngle(decoderState.toothLastToothTime + 100, decoderState, page4);
    int16_t expected = 0 + timeToAngle(100);
    TEST_ASSERT_EQUAL(expected, angle);
  }

  // Tooth 4 with a triggerAngle offset
  page4.triggerAngle = 10;
  decoderState.toothCurrentCount = 4;
  {
    int16_t angle = decoders::harley::getCrankAngle(decoderState.toothLastToothTime + 50, decoderState, page4);
    int16_t expected = 157 + 10 + timeToAngle(50);
    TEST_ASSERT_EQUAL(expected, angle);
  }

  // Wrap-around behaviour: large triggerAngle should reduce result by 720 when >=720
  page4.triggerAngle = 600;
  decoderState.toothCurrentCount = 2;
  {
    int16_t angle = decoders::harley::getCrankAngle(decoderState.toothLastToothTime + 10, decoderState, page4);
    int16_t expected = 157 + 600 + timeToAngle(10);
    if (expected >= 720) expected -= 720;
    TEST_ASSERT_EQUAL(expected, angle);
  }
}

static void test_getRevolutionTime(void)
{
  statuses current={};
  config4 page4={};
  auto decoderState = decoders::harley::initialise(1000, current);

  // Ensure sync present
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  // --- Cranking calculation below the configured RPM/10 threshold
  current.setRpm(0);
  page4.crankRPM = 100; // 1000 RPM
  current.crankRPM = RPM_MEDIUM.toUser(page4.crankRPM);
  current.revolutionTime = 12345UL;
  // Choose tempToothAngle = 120 and gap such that revTime = 20000 * 360 / 120 = 60000
  decoderState.triggerToothAngle = 120;
  decoderState.toothLastMinusOneToothTime = 1000UL;
  decoderState.toothLastToothTime = decoderState.toothLastMinusOneToothTime + 20000UL;
  decoderState.toothOneMinusOneTime = 0UL;
  decoderState.toothOneTime = 0UL;
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoders::harley::getRevolutionTime(current, decoderState));

  // --- Tooth #1 has a zero tooth angle: keep the published period.
  // The tooth #1 times aren't used: they can span a stall (E.g. the first tooth after a restart)
  decoderState.triggerToothAngle = 0;
  decoderState.toothOneMinusOneTime = 1000UL;
  decoderState.toothOneTime = decoderState.toothOneMinusOneTime + 5000000UL;
  TEST_ASSERT_EQUAL_UINT32(12345UL, decoders::harley::getRevolutionTime(current, decoderState));

  // --- Running path: should call stdGetRevolutionTime(CRANK_SPEED)
  current.setRpm(2000);
  page4.crankRPM = 10; // 100 RPM
  current.crankRPM = RPM_MEDIUM.toUser(page4.crankRPM);
  decoderState.toothOneMinusOneTime = 1000UL;
  decoderState.toothOneTime = decoderState.toothOneMinusOneTime + 60000UL; // revTime=60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoders::harley::getRevolutionTime(current, decoderState));

  // --- Fallback: when not synced, the speed is unknown
  decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(0UL, decoders::harley::getRevolutionTime(current, decoderState));
}

static void assert_getRevolutionTime_at_cranking_boundary(uint16_t rpm, uint32_t expected)
{
  statuses current={};
  config4 page4={};
  auto decoderState = decoders::harley::initialise(1000, current);

  page4.crankRPM = 40; // Stored in RPM/10, so the boundary is 400 RPM
  current.crankRPM = RPM_MEDIUM.toUser(page4.crankRPM);
  current.setRpm(rpm);
  current.startRevolutions = 1;
  current.revolutionTime = UINT32_MAX;
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  // Deliberately different results distinguish which calculation was selected:
  // last-tooth angle/gap -> 60000uS (1000 RPM); complete revolution -> 40000uS (1500 RPM).
  decoderState.triggerToothAngle = 120;
  decoderState.toothLastMinusOneToothTime = 1000UL;
  decoderState.toothLastToothTime = 21000UL;
  decoderState.toothOneMinusOneTime = 1000UL;
  decoderState.toothOneTime = 41000UL;
  TEST_ASSERT_EQUAL_UINT32(expected, decoders::harley::getRevolutionTime(current, decoderState));
}

static void test_getRevolutionTime_below_cranking_boundary(void)
{
  assert_getRevolutionTime_at_cranking_boundary(399, 60000UL);
}

static void test_getRevolutionTime_at_cranking_boundary(void)
{
  assert_getRevolutionTime_at_cranking_boundary(400, 40000UL);
}

static void test_getRevolutionTime_above_cranking_boundary(void)
{
  assert_getRevolutionTime_at_cranking_boundary(401, 40000UL);
}

struct trigger_context_harley_t
{
  statuses current = {};
  config4 page4 = {};
  decoders::shared::state_t decoderState;

  void init(uint32_t curTime)
  {
    decoderState = decoders::harley::initialise(curTime, current);
    current.decoder.primary._pin.setPin(2);
  }

  void triggerPrimary(uint32_t curTime)
  {
    decoders::harley::triggerPrimary(curTime, current, decoderState, page4);
  }
};


static void test_triggerPrimary_filter_fail_early(void)
{
  trigger_context_harley_t context{};
  uint32_t startTime = 100000;
  context.init(startTime);

  // Simulate small gap (< triggerFilterTime = 1000)
  uint32_t shortTime = startTime + 500; // Gap = 500
  
  context.triggerPrimary(shortTime);
  
  TEST_ASSERT_EQUAL(0, context.decoderState.toothCurrentCount); // Should not increment/change
  TEST_ASSERT_FALSE(context.decoderState.decoderStatus.validTrigger);
}

static void test_triggerPrimary_valid_trigger_start_of_cycle(void)
{
  trigger_context_harley_t context{};
  uint32_t startTime = 100000;
  context.init(startTime);

  // Setup state to indicate start of a cycle (e.g., Tooth #1 seen after a long gap)
  context.decoderState.toothLastToothTime = startTime - 2000; // CurGap > targetGap
  context.decoderState.curGap = 2000;
  context.decoderState.targetGap = 1000; // Mocked target gap
  context.decoderState.toothCurrentCount = 10;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.current.decoder.primary._pin._pin.setPinHigh();

  // Trigger time
  uint32_t triggerTime = startTime + 1000;
  
  context.triggerPrimary(triggerTime);
  
  // Verify reset to 1 and sync status
  TEST_ASSERT_EQUAL(1, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(0, context.decoderState.triggerToothAngle);
  TEST_ASSERT_TRUE(context.decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL(startTime + 1000, context.decoderState.toothLastToothTime);
  TEST_ASSERT_EQUAL(1, context.current.startRevolutions);
}

static void test_triggerPrimary_valid_trigger_mid_cycle(void)
{
  trigger_context_harley_t context{};
  uint32_t startTime = 100000;
  context.init(startTime);

  // Setup state to indicate mid cycle
  context.decoderState.toothLastToothTime = startTime;
  context.decoderState.curGap = 500; // Gap maintained
  context.decoderState.targetGap = 1000; // Mocked target gap
  context.decoderState.toothCurrentCount = 2;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.current.decoder.primary._pin._pin.setPinHigh();

  // Trigger time
  uint32_t triggerTime = startTime + 499;
  
  context.triggerPrimary(triggerTime);
  
  // Verify increment to 2 and angle 157
  TEST_ASSERT_EQUAL(2, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(157, context.decoderState.triggerToothAngle);
  TEST_ASSERT_TRUE(context.decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL(triggerTime, context.decoderState.toothLastToothTime);
  TEST_ASSERT_EQUAL(1, context.current.startRevolutions);
}

static void test_triggerPrimary_invalid_pin_low(void)
{
  trigger_context_harley_t context{};
  uint32_t startTime = 100000;
  context.init(startTime);

  // Setup state to indicate sync is full
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.decoderState.toothCurrentCount = 5;
  context.decoderState.targetGap = 1000; 
  context.decoderState.curGap = 2000; // Passed filter

  // Mock pin low
  context.current.decoder.primary._pin._pin.setPinLow();

  // Trigger time
  uint32_t triggerTime = startTime + 1000;
  
  context.triggerPrimary(triggerTime);
  
  // Verify sync loss and reset
  TEST_ASSERT_EQUAL(SyncStatus::None, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(0, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(1, context.current.syncLossCounter);
}

static void test_triggerPrimary_invalid_pin_low_no_sync(void)
{
  trigger_context_harley_t context{};
  uint32_t startTime = 100000;
  context.init(startTime);

  // Setup state to indicate no sync
  context.decoderState.decoderStatus.syncStatus = SyncStatus::None;
  context.decoderState.toothCurrentCount = 5;
  context.decoderState.targetGap = 1000; 
  context.decoderState.curGap = 2000; // Passed filter

  // Mock pin low
  context.current.decoder.primary._pin._pin.setPinLow();

  // Trigger time
  uint32_t triggerTime = startTime + 1000;
  
  context.triggerPrimary(triggerTime);
  
  // Verify sync status remains None and sync counter does not increment
  TEST_ASSERT_EQUAL(SyncStatus::None, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(0, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(0, context.current.syncLossCounter);
}

void testHarley(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_getCrankAngle);
  RUN_TEST_P(test_getRevolutionTime);
  RUN_TEST_P(test_getRevolutionTime_below_cranking_boundary);
  RUN_TEST_P(test_getRevolutionTime_at_cranking_boundary);
  RUN_TEST_P(test_getRevolutionTime_above_cranking_boundary);

  RUN_TEST(test_triggerPrimary_filter_fail_early);
  RUN_TEST(test_triggerPrimary_valid_trigger_start_of_cycle);
  RUN_TEST(test_triggerPrimary_valid_trigger_mid_cycle);
  RUN_TEST(test_triggerPrimary_invalid_pin_low);
  RUN_TEST(test_triggerPrimary_invalid_pin_low_no_sync);
}