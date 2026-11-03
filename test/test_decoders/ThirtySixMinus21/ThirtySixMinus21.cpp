#include "../../test_utils.h"
#include "src/decoders/36minus21.h"
#include "../test_decoder_shared.h"
#include "atomic.h"
#include "scheduler_ignition_controller.h"

struct context_36minus21_t
{
  statuses current = {};
  config2 page2 = {};
  config4 page4 = {};
  decoders::shared::state_t decoderState;

  void init(void)
  {
    decoderState = decoders::thirtysixminus21::initialise(page4);
  }

  void triggerPrimary(uint32_t curTime)
  {
    decoders::thirtysixminus21::triggerPrimary(curTime, current, decoderState, page2, page4);
  }

  uint32_t getRevolutionTime()
  {
    return decoders::thirtysixminus21::getRevolutionTime(current, decoderState, page4);
  }
};

static void test_getRevolutionTime(void)
{
  context_36minus21_t context;
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

  // Not cranking: full revolution
  context.current.setRpm((context.current.crankRPM*2U)+111);
  context.decoderState.toothCurrentCount = 20;
  context.decoderState.decoderStatus.toothAngleIsCorrect = true;
  TEST_ASSERT_EQUAL_UINT32(60000UL, context.getRevolutionTime());

  // Cranking: can't do a per tooth calculation after the missing tooth, so no change
  context.current.setRpm(context.current.crankRPM/2U);
  TEST_ASSERT_EQUAL_UINT32(12345UL, context.getRevolutionTime());

  context.decoderState.toothCurrentCount = 1;
  context.decoderState.decoderStatus.toothAngleIsCorrect = false;
  TEST_ASSERT_EQUAL_UINT32(12345UL, context.getRevolutionTime());

  // Cranking: per tooth
  context.decoderState.decoderStatus.toothAngleIsCorrect = true;
  TEST_ASSERT_EQUAL_UINT32(36000UL, context.getRevolutionTime());
}

static void test_setEndTeeth(void)
{
  context_36minus21_t context;
  context.init();
  decoders::thirtysixminus21::setEndTeeth(context.decoderState);

  TEST_ASSERT_EQUAL(10, context.decoderState.ignitionEndTeeth[0]);
  TEST_ASSERT_EQUAL(28, context.decoderState.ignitionEndTeeth[1]);
}

static void test_triggerPrimary_gapcheck_with_pass(void)
{
  context_36minus21_t context;
  context.init();

  context.decoderState.toothLastToothTime = 1000;
  
  // Test call
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));

  TEST_ASSERT_EQUAL(1, context.decoderState.toothCurrentCount); // Incremented from 0
  TEST_ASSERT_EQUAL(true, context.decoderState.decoderStatus.validTrigger); // Valid trigger set
}

static void test_triggerPrimary_gapcheck_fail(void)
{
  context_36minus21_t context;
  context.init();

  context.decoderState.toothLastToothTime = 10000;
  
  // Test call
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState)-2);

  TEST_ASSERT_EQUAL(0, context.decoderState.toothCurrentCount); // Not incremented
  TEST_ASSERT_FALSE(context.decoderState.decoderStatus.validTrigger); // Not valid trigger
}

static void test_triggerPrimary_wrap_count_normal(uint16_t toothCurrentCount)
{
  context_36minus21_t context;
  context.init();

  // Trigger a pulse that completes the rotation
  context.decoderState.toothLastToothTime = 10000;
  // context.decoderState.toothLastMinusOneToothTime = context.decoderState.toothLastToothTime - 1000;
  context.decoderState.toothCurrentCount = toothCurrentCount;
  context.decoderState.revolutionOne = false;
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState)-2);

  // Verify wrap count logic
  TEST_ASSERT_EQUAL(1, context.decoderState.toothCurrentCount); // Reset to 1
  TEST_ASSERT_EQUAL(true, context.decoderState.revolutionOne); // Flipped
  TEST_ASSERT_EQUAL(1, context.current.startRevolutions); // Revolution counter incremented
}

static void test_triggerPrimary_wrap_count_normal(void)
{
  test_triggerPrimary_wrap_count_normal(1);
  test_triggerPrimary_wrap_count_normal(37);  
}

static void test_triggerPrimary_single_missing_tooth_detection(void)
{
  context_36minus21_t context;
  context.init();

  // Tooth before gap
  context.decoderState.toothLastToothTime = 10000;
  context.decoderState.toothLastMinusOneToothTime = context.decoderState.toothLastToothTime - 2000;
  context.decoderState.revolutionOne = false;
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState)+3000);
  
  // Verify detection of single gap
  TEST_ASSERT_EQUAL(20, context.decoderState.toothCurrentCount); // Sync acquired state
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_FALSE(context.decoderState.decoderStatus.toothAngleIsCorrect); // Double tooth angle warning
  TEST_ASSERT_EQUAL(0, context.decoderState.triggerFilterTime); // Filter disabled
}

static void test_triggerPrimary_double_missing_tooth_detection(void)
{
  context_36minus21_t context;
  
  // Setup state for double missing tooth detection
  // Last gap was 1000. 3x=3000. 1.5x=1500.
  context.init();

  // Tooth before first gap
  context.decoderState.toothLastToothTime = 100000 - 3000;
  context.decoderState.toothLastMinusOneToothTime = context.decoderState.toothLastToothTime - 2000;
  
  // Trigger a very large gap (> 3000)
  context.triggerPrimary(context.decoderState.toothLastToothTime +7000); // CurGap = 7000
  
  // Verify detection of double gap
  TEST_ASSERT_EQUAL(1, context.decoderState.toothCurrentCount); // Sync acquired state (reset)
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_FALSE(context.decoderState.decoderStatus.toothAngleIsCorrect); // Double tooth angle warning
  TEST_ASSERT_EQUAL(0, context.decoderState.triggerFilterTime); // Filter disabled
}

static void setup_perTooth(context_36minus21_t &context)
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

static void assert_pertoothign(context_36minus21_t &context)
{
  auto newCrankAngle = ( (context.decoderState.toothCurrentCount) * context.decoderState.triggerToothAngle );
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));
  TEST_ASSERT_UINT32_WITHIN(10, ignitionSchedule1._counter + angleToTimerTicks(ignitionSchedule1.chargeAngle - newCrankAngle), ignitionSchedule1._compare);
}

static void test_primaryTrigger_applies_per_tooth_timing(void)
{
  context_36minus21_t context;
  context.init();
  context.page2.perToothIgn = true;

  setAngleConverterRevolutionTime(6000000UL);
  context.current.setRpm(3000);

  ATOMIC() {
    stopIgnitionSchedulers();

    setup_perTooth(context);
    assert_pertoothign(context);
  }
}


void testThirtySixMinus21(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST(test_triggerPrimary_gapcheck_with_pass);
  RUN_TEST(test_triggerPrimary_gapcheck_fail);
  RUN_TEST(test_triggerPrimary_wrap_count_normal);
  RUN_TEST(test_triggerPrimary_single_missing_tooth_detection);
  RUN_TEST(test_triggerPrimary_double_missing_tooth_detection);
  RUN_TEST(test_primaryTrigger_applies_per_tooth_timing);

  RUN_TEST_P(test_getRevolutionTime);
  RUN_TEST_P(test_setEndTeeth);
}