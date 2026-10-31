#include "../test_utils.h"
#include "src/decoders/420a.h"
#include "../test_decoder_shared.h"
#include "atomic.h"
#include "scheduler_ignition_controller.h"

struct context_420a_t
{
  statuses current = {};
  config2 page2 = {};
  config4 page4 = {};
  decoders::shared::state_t decoderState = decoders::dsm420a::initialise();

  context_420a_t()
  {
    current.decoder.primary._pin.setPin(2);
  }

  void triggerPrimary(uint32_t curTime)
  {
    decoders::dsm420a::triggerPrimary(curTime, current, decoderState, page2, page4);
  }

  uint32_t getRevolutionTime()
  {
    return decoders::dsm420a::getRevolutionTime(current, decoderState);
  }
};

static void test_getRevolutionTime(void)
{
  context_420a_t context;

  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.current.startRevolutions = 1; // not cranking
  context.decoderState.toothOneMinusOneTime = 1000UL;
  context.decoderState.toothOneTime = context.decoderState.toothOneMinusOneTime + 120000UL; // Cam speed: >>1 -> 60000

  // Running & cranking both use the standard calculation
  context.current.crankRPM = 400;
  context.current.setRpm(context.current.crankRPM*2);
  TEST_ASSERT_EQUAL_UINT32(60000UL, context.getRevolutionTime());

  context.current.setRpm(context.current.crankRPM/2);
  TEST_ASSERT_EQUAL_UINT32(60000UL, context.getRevolutionTime());
}

static void test_setEndTeeth(void)
{
  context_420a_t context;

  decoders::dsm420a::setEndTeeth(context.current, context.decoderState);
  TEST_ASSERT_EQUAL(1, context.decoderState.ignitionEndTeeth[0]);
  TEST_ASSERT_EQUAL(5, context.decoderState.ignitionEndTeeth[1]);
  TEST_ASSERT_EQUAL(9, context.decoderState.ignitionEndTeeth[2]);
  TEST_ASSERT_EQUAL(13, context.decoderState.ignitionEndTeeth[3]);

  context.current.advance = 9;

  decoders::dsm420a::setEndTeeth(context.current, context.decoderState);
  TEST_ASSERT_EQUAL(16, context.decoderState.ignitionEndTeeth[0]);
  TEST_ASSERT_EQUAL(4, context.decoderState.ignitionEndTeeth[1]);
  TEST_ASSERT_EQUAL(8, context.decoderState.ignitionEndTeeth[2]);
  TEST_ASSERT_EQUAL(12, context.decoderState.ignitionEndTeeth[3]);
}

static void test_triggerPrimary_valid_trigger(void)
{
  context_420a_t context{};
  context.decoderState.toothLastToothTime = 3333333; 
  context.decoderState.toothCurrentCount = 7;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  // Trigger time
  uint32_t triggerTime = calcNextTimePassPrimaryFilter(context.decoderState)+1000;
  
  context.triggerPrimary(triggerTime);

  TEST_ASSERT_EQUAL(8, context.decoderState.toothCurrentCount); // Incremented
  TEST_ASSERT_EQUAL(true, context.decoderState.decoderStatus.validTrigger); // Flag set
  TEST_ASSERT_EQUAL(triggerTime, context.decoderState.toothLastToothTime);
}

static void test_triggerPrimary_filter_fail(void)
{
  context_420a_t context{};
  context.decoderState.toothLastToothTime = 3333333; 
  context.decoderState.toothCurrentCount = 7;
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState)-2);

  TEST_ASSERT_EQUAL(7, context.decoderState.toothCurrentCount); // Not incremented
  TEST_ASSERT_EQUAL(false, context.decoderState.decoderStatus.validTrigger); // Flag not set
}

static void test_triggerPrimary_rotation_wrap(void)
{
  context_420a_t context{};

  // Setup state just before wrap (Tooth #16)
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.decoderState.toothLastToothTime = 3333333; 
  context.decoderState.toothOneMinusOneTime = context.decoderState.toothLastToothTime - 4000;
  context.decoderState.toothCurrentCount = 16;

  // Trigger a pulse that completes the rotation (> 16 teeth)
  uint32_t triggerTime = calcNextTimePassPrimaryFilter(context.decoderState) + 2000;
  
  context.triggerPrimary(triggerTime);

  // Verify wrap logic
  TEST_ASSERT_EQUAL(1, context.decoderState.toothCurrentCount); // Reset to 1
  TEST_ASSERT_EQUAL(triggerTime, context.decoderState.toothOneTime); // Update tooth one time
  TEST_ASSERT_EQUAL(1, context.current.startRevolutions); // Counter incremented
}

static void setup_perTooth(context_420a_t &context)
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

static void assert_pertoothign(context_420a_t &context)
{
  int16_t newCrankAngle = context.decoderState.toothAngles[context.decoderState.toothCurrentCount] + context.page4.triggerAngle;
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));
  TEST_ASSERT_UINT32_WITHIN(10, ignitionSchedule1._counter + angleToTimerTicks(ignitionSchedule1.chargeAngle - newCrankAngle), ignitionSchedule1._compare);
}

static void test_triggerPrimary_per_tooth_timing_check(void)
{
  context_420a_t context;

  setAngleConverterRevolutionTime(6000000UL);
  context.current.setRpm(3000);

  ATOMIC() {
    stopIgnitionSchedulers();

    setup_perTooth(context);
    assert_pertoothign(context);
  }
}

static void test_triggerSecondary_sync_success_primary_high(void)
{
  context_420a_t context{};
  
  // Initial state: Not synced
  context.decoderState.decoderStatus.syncStatus = SyncStatus::None;
  context.decoderState.toothCurrentCount = 1; 
  context.current.syncLossCounter = 0;
  
  // Mock primary signal HIGH (Falling edge of secondary signal while primary is HIGH)
  context.current.decoder.primary._pin._pin.setPinHigh();
  
  // Trigger secondary
  decoders::dsm420a::triggerSecondary(context.current, context.decoderState);

  // Verify sync acquisition
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(13, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(0, context.current.syncLossCounter);
}

static void test_triggerSecondary_sync_success_primary_low(void)
{
  context_420a_t context{};
  
  // Initial state: Not synced
  context.decoderState.decoderStatus.syncStatus = SyncStatus::None;
  context.decoderState.toothCurrentCount = 1;
  context.current.syncLossCounter = 0;

  // Mock primary signal LOW (Falling edge of secondary signal while primary is LOW)
  context.current.decoder.primary._pin._pin.setPinLow();
  
  // Trigger secondary
  decoders::dsm420a::triggerSecondary(context.current, context.decoderState);

  // Verify sync acquisition
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(5, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(0, context.current.syncLossCounter);
}

static void test_triggerSecondary_maintenance_primary_high(void)
{
  context_420a_t context{};
  
  // Initial state: Synced and count matches expected (13)
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.decoderState.toothCurrentCount = 13;
  context.current.syncLossCounter = 0;

  // Mock primary signal HIGH
  context.current.decoder.primary._pin._pin.setPinHigh();
  
  // Trigger secondary
  decoders::dsm420a::triggerSecondary(context.current, context.decoderState);

  // Verify maintenance
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(13, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(0, context.current.syncLossCounter);
}

static void test_triggerSecondary_sync_loss_primary_high(void)
{
  context_420a_t context{};
  
  // Initial state: Synced but count is wrong (e.g., skipped teeth)
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.decoderState.toothCurrentCount = 10; // Incorrect count
  context.current.syncLossCounter = 0;

  // Mock primary signal HIGH
  context.current.decoder.primary._pin._pin.setPinHigh();
  
  // Trigger secondary
  decoders::dsm420a::triggerSecondary(context.current, context.decoderState);

  // Verify sync loss action
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(13, context.decoderState.toothCurrentCount); // Reset to expected
  TEST_ASSERT_EQUAL(1, context.current.syncLossCounter); // Counter incremented
}

static void test_triggerSecondary_sync_loss_primary_low(void)
{
  context_420a_t context{};
  
  // Initial state: Synced but count is wrong (e.g., skipped teeth)
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.decoderState.toothCurrentCount = 8; // Incorrect count
  context.current.syncLossCounter = 0;

  // Mock primary signal LOW
  context.current.decoder.primary._pin._pin.setPinLow();
  
  // Trigger secondary
  decoders::dsm420a::triggerSecondary(context.current, context.decoderState);

  // Verify sync loss action
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(5, context.decoderState.toothCurrentCount); // Reset to expected
  TEST_ASSERT_EQUAL(1, context.current.syncLossCounter); // Counter incremented
}

void test420a(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getRevolutionTime);
    RUN_TEST_P(test_setEndTeeth);

    RUN_TEST(test_triggerPrimary_valid_trigger);
    RUN_TEST(test_triggerPrimary_filter_fail);
    RUN_TEST(test_triggerPrimary_rotation_wrap);
    RUN_TEST(test_triggerPrimary_per_tooth_timing_check);

    RUN_TEST(test_triggerSecondary_sync_success_primary_high);
    RUN_TEST(test_triggerSecondary_sync_success_primary_low);
    RUN_TEST(test_triggerSecondary_maintenance_primary_high);
    RUN_TEST(test_triggerSecondary_sync_loss_primary_high);
    RUN_TEST(test_triggerSecondary_sync_loss_primary_low);
  }
}