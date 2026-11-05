#include "../../test_utils.h"
#include "src/decoders/renix.h"
#include "../../test_decoders/test_decoder_shared.h"
#include "scheduler_ignition_controller.h"
#include "atomic.h"

constexpr uint32_t BASE_1ST_TOOTH_TIME = 100000UL;

struct context_renix_t
{
  statuses current = {};
  config2 page2 = {};
  config4 page4 = {};
  decoders::shared::state_t decoderState;

  void init(uint8_t cylinders)
  {
    page2.nCylinders = cylinders;
    decoderState = decoders::renix::initialise(page2, page4);
  }

  void triggerPrimary(uint32_t curTime)
  {
    decoders::renix::triggerPrimary(curTime, current, decoderState, page2, page4);
  }

  void setup_primaryfilter(uint32_t lastGap, uint32_t filter)
  {
    decoderState.toothLastToothTime = BASE_1ST_TOOTH_TIME;
    decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime - lastGap;
    decoderState.toothLastToothRisingTime = BASE_1ST_TOOTH_TIME;
    decoderState.toothLastSecToothRisingTime = decoderState.toothLastToothRisingTime - lastGap;
    decoderState.triggerFilterTime = filter;
  }
};

static context_renix_t test_setup_renix44(void)
{
  context_renix_t context;
  context.init(4);
  return context;
}

static context_renix_t test_setup_renix66()
{
  context_renix_t context;
  context.init(6);
  return context;
}

//************************************** Begin the new ignition setEndTooth tests *************************************/

static void assert_setEndTeeth(uint8_t expected, context_renix_t &context, IgnitionSchedule &schedule, uint8_t index, int8_t advance)
{
    schedule.dischargeAngle = 180 + advance; 
    decoders::renix::setEndTeeth(context.decoderState, context.page4);
    TEST_ASSERT_EQUAL(expected, context.decoderState.ignitionEndTeeth[index]);
}

static void test_setEndTeeth_44_channel_1()
{
    auto context = test_setup_renix44();
    context.page4.sparkMode = IGN_MODE_SINGLE;

    context.page4.triggerAngle = 0; //No trigger offset
    assert_setEndTeeth(4, context, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(1, context, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(1, context, ignitionSchedule1, 0, 10);

    context.page4.triggerAngle = 90;
    assert_setEndTeeth(3, context, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(4, context, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(4, context, ignitionSchedule1, 0, 10);

    context.page4.triggerAngle = 180;
    assert_setEndTeeth(3, context, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(3, context, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(3, context, ignitionSchedule1, 0, 10);

    context.page4.triggerAngle = 270;
    assert_setEndTeeth(2, context, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(2, context, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(3, context, ignitionSchedule1, 0, 10);

    context.page4.triggerAngle = 360;
    assert_setEndTeeth(1, context, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(1, context, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(2, context, ignitionSchedule1, 0, 10);

    context.page4.triggerAngle = -90;
    assert_setEndTeeth(1, context, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(2, context, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(2, context, ignitionSchedule1, 0, 10);

    context.page4.triggerAngle = -180;
    assert_setEndTeeth(2, context, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(3, context, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(3, context, ignitionSchedule1, 0, 10);

    context.page4.triggerAngle = -270;
    assert_setEndTeeth(3, context, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(4, context, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(4, context, ignitionSchedule1, 0, 10);

    context.page4.triggerAngle = -360;
    assert_setEndTeeth(4, context, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(1, context, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(1, context, ignitionSchedule1, 0, 10);
}

static void test_setEndTeeth_66_channel_1()
{
    auto context = test_setup_renix66();
    context.page4.sparkMode = IGN_MODE_SINGLE;

    context.page4.triggerAngle = 0; //No trigger offset
    assert_setEndTeeth(1, context, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(2, context, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(2, context, ignitionSchedule1, 0, 10);

    context.page4.triggerAngle = 90;
    assert_setEndTeeth(6, context, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(6, context, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(6, context, ignitionSchedule1, 0, 10);

    context.page4.triggerAngle = 180;
    assert_setEndTeeth(5, context, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(5, context, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(5, context, ignitionSchedule1, 0, 10);

    context.page4.triggerAngle = 270;
    assert_setEndTeeth(4, context, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(4, context, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(4, context, ignitionSchedule1, 0, 10);

    context.page4.triggerAngle = 360;
    assert_setEndTeeth(2, context, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(2, context, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(3, context, ignitionSchedule1, 0, 10);

    context.page4.triggerAngle = -90;
    assert_setEndTeeth(3, context, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(3, context, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(3, context, ignitionSchedule1, 0, 10);

    context.page4.triggerAngle = -180;
    assert_setEndTeeth(4, context, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(5, context, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(5, context, ignitionSchedule1, 0, 10);

    context.page4.triggerAngle = -270;
    assert_setEndTeeth(6, context, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(6, context, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(6, context, ignitionSchedule1, 0, 10);

    context.page4.triggerAngle = -360;
    assert_setEndTeeth(1, context, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(2, context, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(2, context, ignitionSchedule1, 0, 10);
}

static void test_triggerPrimary_rejects_filtered_pulse(void)
{
  auto context = test_setup_renix44();
  context.decoderState.toothLastToothRisingTime = 2000;
  context.decoderState.triggerFilterTime = 1000;

  context.triggerPrimary(2500); // Gap 500, below filter.

  TEST_ASSERT_EQUAL_UINT32(500, context.decoderState.curGap);
  TEST_ASSERT_EQUAL_UINT32(2000, context.decoderState.toothLastToothRisingTime);
  TEST_ASSERT_EQUAL_UINT8(1, context.decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL_UINT8(0, context.current.syncLossCounter);
}

static void test_triggerPrimary_startup_advances_tooth(void)
{
  auto context = test_setup_renix44();
  context.decoderState.toothLastToothRisingTime = 1000;
  context.decoderState.toothLastSecToothRisingTime = 0;
  context.decoderState.triggerFilterTime = 100;

  context.triggerPrimary(2000);

  TEST_ASSERT_EQUAL_UINT8(2, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT8(1, context.decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL_UINT32(2000, context.decoderState.toothLastToothRisingTime);
  TEST_ASSERT_EQUAL_UINT32(2000, context.decoderState.toothLastToothTime);
}

static void test_triggerPrimary_correct_missing_tooth_position(void)
{
  auto context = test_setup_renix44();
  context.decoderState.toothSystemCount = 9;
  context.decoderState.toothLastToothRisingTime = 2000;
  context.decoderState.toothLastSecToothRisingTime = 1500;
  context.decoderState.toothLastToothTime = 1000;
  context.decoderState.toothCurrentCount = 2;
  context.decoderState.triggerFilterTime = 100;

  context.triggerPrimary(3000); // Gap 1000 matches the 2x target.

  TEST_ASSERT_EQUAL_UINT8(3, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT8(1, context.decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL_UINT32(3000, context.decoderState.toothLastToothRisingTime);
  TEST_ASSERT_EQUAL_UINT32(3000, context.decoderState.toothLastToothTime);
  TEST_ASSERT_EQUAL_UINT8(0, context.current.syncLossCounter);
}

static void test_triggerPrimary_wrong_missing_tooth_position_loses_sync(void)
{
  auto context = test_setup_renix44();
  context.decoderState.toothSystemCount = 1;
  context.decoderState.toothLastToothRisingTime = 2000;
  context.decoderState.toothLastSecToothRisingTime = 1500;
  context.decoderState.toothLastToothTime = 1000;
  context.decoderState.toothCurrentCount = 4;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.decoderState.triggerFilterTime = 100;

  context.triggerPrimary(3000);

  TEST_ASSERT_EQUAL(SyncStatus::None, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT8(1, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT8(1, context.decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL_UINT8(1, context.current.syncLossCounter);
}

static void test_triggerPrimary_four_cylinder_revolution_wrap(void)
{
  auto context = test_setup_renix44();
  context.decoderState.toothSystemCount = 9;
  context.decoderState.toothLastToothRisingTime = 2000;
  context.decoderState.toothLastSecToothRisingTime = 1500;
  context.decoderState.toothLastToothTime = 1000;
  context.decoderState.toothCurrentCount = 4;
  context.decoderState.triggerFilterTime = 100;

  context.triggerPrimary(3000);

  TEST_ASSERT_EQUAL_UINT8(1, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT8(1, context.decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT8(1, context.current.startRevolutions);
  TEST_ASSERT_TRUE(context.decoderState.revolutionOne);
  TEST_ASSERT_EQUAL_UINT32(3000, context.decoderState.toothOneTime);
}

static void setup_perTooth(context_renix_t &context)
{
  setAngleConverterRevolutionTime(6000000UL);
  context.current.setRpm(3000);

  context.decoderState.toothCurrentCount = 7;
  context.decoderState.triggerToothAngle = 13;
  context.decoderState.toothSystemCount = 11;
  context.decoderState.ignitionEndTeeth[0] = context.decoderState.toothCurrentCount+1;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.setup_primaryfilter(1000, 500);
  context.page2.perToothIgn = true;

  context.current.rotationStatus = EngineRotationStatus::Running;
  context.current.startRevolutions = 99;

  ignitionSchedule1._status = PENDING;
  ignitionSchedule1._counter = 100;
  ignitionSchedule1._compare = 100;
  ignitionSchedule1.chargeAngle = 577;
}

static void assert_pertoothign(context_renix_t &context, uint16_t newCrankAngle)
{
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));
  TEST_ASSERT_UINT32_WITHIN(10, ignitionSchedule1._counter + angleToTimerTicks(ignitionSchedule1.chargeAngle - newCrankAngle), ignitionSchedule1._compare);
}

static void assert_pertoothign360(context_renix_t &context)
{
  auto newCrankAngle = context.page4.triggerAngle + ( (context.decoderState.toothCurrentCount) * context.decoderState.triggerToothAngle );
  assert_pertoothign(context, newCrankAngle);
}

static void assert_pertoothign720(context_renix_t &context)
{
  auto newCrankAngle = 360 + context.page4.triggerAngle + ( (context.decoderState.toothCurrentCount) * context.decoderState.triggerToothAngle );
  assert_pertoothign(context, newCrankAngle);
}

static void test_primary_applies_per_tooth_timing(void)
{
  auto context = test_setup_renix66();

  ATOMIC() {
    stopIgnitionSchedulers();

    setup_perTooth(context);
    assert_pertoothign360(context);

    context = test_setup_renix66();
    setup_perTooth(context);
    context.page4.TrigSpeed = CRANK_SPEED;
    context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
    context.decoderState.revolutionOne = true;
    context.decoderState.ignitionEndTeeth[0] = context.page4.triggerTeeth + context.decoderState.toothCurrentCount+1;
    assert_pertoothign720(context);
  }
}

void testRenix()
{
  SET_UNITY_FILENAME() {
      RUN_TEST_P(test_setEndTeeth_44_channel_1);
      RUN_TEST_P(test_setEndTeeth_66_channel_1);

      RUN_TEST(test_triggerPrimary_rejects_filtered_pulse);
      RUN_TEST(test_triggerPrimary_startup_advances_tooth);
      RUN_TEST(test_triggerPrimary_correct_missing_tooth_position);
      RUN_TEST(test_triggerPrimary_wrong_missing_tooth_position_loses_sync);
      RUN_TEST(test_triggerPrimary_four_cylinder_revolution_wrap);
      RUN_TEST(test_primary_applies_per_tooth_timing);
    }           
}