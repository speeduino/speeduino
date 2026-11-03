#include "../../test_utils.h"
#include "src/decoders/36minus222.h"
#include "../test_decoder_shared.h"
#include "atomic.h"
#include "scheduler_ignition_controller.h"

struct context_36minus222_t
{
  statuses current = {};
  config2 page2 = {};
  config4 page4 = {};
  decoders::shared::state_t decoderState;

  void init(void)
  {
    decoders::thirtysixminus222::initialise(page4);
  }

  void triggerPrimary(uint32_t curTime)
  {
    decoders::thirtysixminus222::triggerPrimary(curTime, current, decoderState, page2, page4);
  }

  auto getRevolutionTime()
  {
    return decoders::thirtysixminus222::getRevolutionTime(current, decoderState, page2, page4);
  }

};

static void assert_crank_per_tooth(context_36minus222_t &context)
{
    context.current.setRpm(context.current.crankRPM/2U);
    TEST_ASSERT_EQUAL_UINT32(36000UL, context.getRevolutionTime());
}

static void assert_crank_uses_current_revolution_time(context_36minus222_t &context)
{
    context.current.setRpm(context.current.crankRPM/2U);
    TEST_ASSERT_EQUAL_UINT32(context.current.revolutionTime, context.getRevolutionTime());
}

static void test_getRevolutionTime(void)
{
  context_36minus222_t context;
  context.init();
  context.current.crankRPM = 400;
  context.page4.StgCycles = 0;
  context.current.startRevolutions = 1;
  context.current.revolutionTime = 12345UL;

  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.decoderState.toothOneMinusOneTime = 1000UL;
  context.decoderState.toothOneTime = context.decoderState.toothOneMinusOneTime + 60000UL; // Full revolution: 60000uS
  context.decoderState.toothLastMinusOneToothTime = 1000;
  context.decoderState.toothLastToothTime = context.decoderState.toothLastMinusOneToothTime * 2; // Tooth gap of 1000uS * 36 teeth: 36000uS

  // Not cranking
  context.current.setRpm((context.current.crankRPM*2U)+111);
  TEST_ASSERT_EQUAL_UINT32(60000UL, context.getRevolutionTime());

  // Cranking
  context.page2.nCylinders = 4;
  for (auto toothCount: { 19, 16, 34, })
  {
    context.decoderState.toothCurrentCount = toothCount;
    assert_crank_uses_current_revolution_time(context);
  }
  for (auto toothCount: { 9, 12, 33, })
  {
    context.decoderState.toothCurrentCount = toothCount;
    context.decoderState.decoderStatus.toothAngleIsCorrect = true;
    assert_crank_per_tooth(context);
    context.decoderState.decoderStatus.toothAngleIsCorrect = false;
    assert_crank_uses_current_revolution_time(context);
  }

  context.page2.nCylinders = 6;
  for (auto toothCount: { 9, 12, 33, })
  {
    context.decoderState.toothCurrentCount = toothCount;
    assert_crank_uses_current_revolution_time(context);
  }
  for (auto toothCount: { 19, 16, 34, })
  {
    context.decoderState.toothCurrentCount = toothCount;
    context.decoderState.decoderStatus.toothAngleIsCorrect = true;
    assert_crank_per_tooth(context);
    context.decoderState.decoderStatus.toothAngleIsCorrect = false;
    assert_crank_uses_current_revolution_time(context);
  }

  context.page2.nCylinders = 2;
  for (auto toothCount: { 9, 12, 33, 19, 16, 34, })
  {
    context.decoderState.toothCurrentCount = toothCount;
    context.decoderState.decoderStatus.toothAngleIsCorrect = true;
    assert_crank_uses_current_revolution_time(context);
    context.decoderState.decoderStatus.toothAngleIsCorrect = false;
    assert_crank_uses_current_revolution_time(context);
  }
}

static void test_setEndTeeth(uint8_t cylinders, int16_t advance, const uint16_t (&expected)[3])
{
  context_36minus222_t context;
  context.init();

  context.page2.nCylinders = cylinders;
  context.current.advance = advance;

  decoders::thirtysixminus222::setEndTeeth(context.current, context.decoderState, context.page2);

  TEST_ASSERT_EQUAL(expected[0], context.decoderState.ignitionEndTeeth[0]);
  TEST_ASSERT_EQUAL(expected[1], context.decoderState.ignitionEndTeeth[1]);
  if (expected[2]!=99)
  {
    TEST_ASSERT_EQUAL(expected[2], context.decoderState.ignitionEndTeeth[2]);
  }
}

static void test_setEndTeeth(void)
{
  test_setEndTeeth(4, 9, { 36, 16, 99 });
  test_setEndTeeth(4, 10, { 35, 16, 99 });
  test_setEndTeeth(4, 19, { 35, 16, 99 });
  test_setEndTeeth(4, 20, { 34, 16, 99 });
  test_setEndTeeth(4, 29, { 34, 16, 99 });
  test_setEndTeeth(4, 30, { 31, 13, 99 });

  test_setEndTeeth(6, 9, { 36, 9, 23 });
  test_setEndTeeth(6, 10, { 35, 9, 22 });
  test_setEndTeeth(6, 19, { 35, 9, 22 });
  test_setEndTeeth(6, 20, { 34, 6, 21 });
  test_setEndTeeth(6, 29, { 34, 6, 21 });
  test_setEndTeeth(6, 30, { 33, 6, 20 });
  test_setEndTeeth(6, 39, { 33, 6, 20 });
  test_setEndTeeth(6, 40, { 31, 6, 19 });
}

static void test_triggerPrimary_gapcheck_with_pass(void)
{
  context_36minus222_t context;
  context.init();
  
  // Setup context for 4 cylinder engine
  context.page2.nCylinders = 4;

  // Initial state setup
  context.decoderState.toothLastMinusOneToothTime = 100000;
  context.decoderState.toothLastToothTime = 1000;
  context.decoderState.toothCurrentCount = 5;
  context.decoderState.toothSystemCount = 0;

  // Trigger with valid gap: CurGap = 1000
  context.triggerPrimary(100000 + 2000); // curTime = 102000

  TEST_ASSERT_EQUAL(6, context.decoderState.toothCurrentCount); // Incremented
  TEST_ASSERT_EQUAL(true, context.decoderState.decoderStatus.validTrigger); // Flag set
}

static void test_triggerPrimary_wrap_count(void)
{
  context_36minus222_t context;
  context.init();
  context.page2.nCylinders = 4;
  
  // Setup state to be over 36 teeth
  context.decoderState.toothCurrentCount = 37; // > 36
  context.decoderState.revolutionOne = true;

  context.triggerPrimary(100000 + 2000);
  
  TEST_ASSERT_EQUAL(1, context.decoderState.toothCurrentCount); // Reset to 1
  TEST_ASSERT_FALSE(context.decoderState.revolutionOne); // Flip tracker
  TEST_ASSERT_EQUAL(1, context.current.startRevolutions); // Counter incremented
}

static void test_triggerPrimary_sync_single_missing_tooth_h4(void)
{
  context_36minus222_t context;
  context.init();
  context.page2.nCylinders = 4;
  
  // Initial state: No missing tooth seen yet
  context.decoderState.toothSystemCount = 0; 
  context.decoderState.toothCurrentCount = 17; // Before missing tooth
  context.decoderState.toothLastToothTime = 100000;
  context.decoderState.toothLastMinusOneToothTime = context.decoderState.toothLastToothTime - 1000;

  // Trigger with big gap (indicating missing tooth)
  context.triggerPrimary(context.decoderState.toothLastToothTime + 5000); // curGap > targetGap (since targetGap is set based on previous gap)
  
  // Should enter the "missing tooth detected" state
  TEST_ASSERT_EQUAL(1, context.decoderState.toothSystemCount); // Set to 1
  TEST_ASSERT_EQUAL(20, context.decoderState.toothCurrentCount); // Incremented twice (17 -> 19)
  TEST_ASSERT_FALSE(context.decoderState.decoderStatus.toothAngleIsCorrect); // Angle is double
  TEST_ASSERT_EQUAL(0, context.decoderState.triggerFilterTime); // Filter reset
}

static void test_triggerPrimary_sync_single_missing_tooth_continues(uint8_t cylinders, uint16_t expectedCount)
{
  context_36minus222_t context;
  context.init();
  context.page2.nCylinders = cylinders;
  
  // Setup state after missing tooth detection
  context.decoderState.toothSystemCount = 1;
  context.decoderState.toothCurrentCount = 19; // After missing tooth (H4)
  
  // Trigger with regular gap (no missing tooth)
  context.triggerPrimary(100000 + 1000);
  
  // Should complete detection and reset
  TEST_ASSERT_EQUAL(expectedCount, context.decoderState.toothCurrentCount); // Reset to 35
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus); // Full sync acquired
  TEST_ASSERT_EQUAL(0, context.decoderState.toothSystemCount); // Reset system count
  TEST_ASSERT_TRUE(context.decoderState.decoderStatus.toothAngleIsCorrect);
}

static void test_triggerPrimary_sync_single_missing_tooth_continues(void)
{
  test_triggerPrimary_sync_single_missing_tooth_continues(4, 35);
  test_triggerPrimary_sync_single_missing_tooth_continues(6, 34);
}

static void test_triggerPrimary_sync_double_missing_tooth(uint8_t cylinders, uint16_t expectedCount)
{
  context_36minus222_t context;
  context.page2.nCylinders = cylinders;
  context.init();
  
  // Initial state
  context.decoderState.toothSystemCount = 0;
  context.decoderState.toothCurrentCount = 17; // Before first missing tooth
  context.decoderState.toothLastToothTime = 100000;
  context.decoderState.toothLastMinusOneToothTime = context.decoderState.toothLastToothTime - 1000;
  
  // 1. Detect first missing tooth
  context.triggerPrimary(100000 + 5000);
  TEST_ASSERT_EQUAL(1, context.decoderState.toothSystemCount); 
  TEST_ASSERT_EQUAL(20, context.decoderState.toothCurrentCount);
  
  // 2. Detect second missing tooth (big gap again)
  context.decoderState.toothLastMinusOneToothTime = 100000 + 5000; 
  context.decoderState.toothLastToothTime = 190000; // Gap 100000
  
  context.triggerPrimary(100000 + 200000); // CurGap = 100000, TargetGap = 100000
  
  // Should trigger full sync detection
  TEST_ASSERT_EQUAL(0, context.decoderState.toothSystemCount); // Reset to 0
  TEST_ASSERT_EQUAL(expectedCount, context.decoderState.toothCurrentCount); // Sync at 19
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
}

static void test_triggerPrimary_sync_double_missing_tooth_h4(void)
{
  test_triggerPrimary_sync_double_missing_tooth(4, 19);
}

static void test_triggerPrimary_sync_double_missing_tooth_h6(void)
{
  test_triggerPrimary_sync_double_missing_tooth(6, 12);
}

static void setup_perTooth(context_36minus222_t &context)
{
  context.decoderState.toothCurrentCount = 7;
  context.decoderState.triggerToothAngle = 13;
  context.decoderState.ignitionEndTeeth[0] = context.decoderState.toothCurrentCount+1;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  context.page2.perToothIgn = true;

  context.current.rotationStatus = EngineRotationStatus::Running;
  context.current.startRevolutions = 99;

  ignitionSchedule1._status = PENDING;
  ignitionSchedule1._counter = 100;
  ignitionSchedule1._compare = 100;
  ignitionSchedule1.chargeAngle = 577;
}

static void assert_pertoothign(context_36minus222_t &context)
{
  auto newCrankAngle = ( (context.decoderState.toothCurrentCount) * context.decoderState.triggerToothAngle );
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));
  TEST_ASSERT_UINT32_WITHIN(10, ignitionSchedule1._counter + angleToTimerTicks(ignitionSchedule1.chargeAngle - newCrankAngle), ignitionSchedule1._compare);
}

static void test_primary_applies_per_tooth_timing(void)
{
  context_36minus222_t context;
  context.init();
  context.page2.perToothIgn = true;

  setAngleConverterRevolutionTime(6000000UL);
  context.current.setRpm(3000);

  ATOMIC() {
    stopIgnitionSchedulers();

    setup_perTooth(context);
    assert_pertoothign(context);

    // setup_perTooth(context);
    // context.decoderState.toothCurrentCount = 8;
    // context.decoderState.ignitionEndTeeth[0] = context.decoderState.toothCurrentCount+1-6;
    // assert_pertoothign(context);

    // setup_perTooth(context);
    // context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
    // assert_pertoothign(context);
  }
}

void testThirtySixMinus22(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_getRevolutionTime);
  RUN_TEST_P(test_setEndTeeth);

  RUN_TEST(test_triggerPrimary_gapcheck_with_pass);
  RUN_TEST(test_triggerPrimary_wrap_count);
  RUN_TEST(test_triggerPrimary_sync_single_missing_tooth_h4);
  RUN_TEST(test_triggerPrimary_sync_single_missing_tooth_continues);
  RUN_TEST(test_triggerPrimary_sync_double_missing_tooth_h4);
  RUN_TEST(test_triggerPrimary_sync_double_missing_tooth_h6);
  RUN_TEST(test_primary_applies_per_tooth_timing);
}