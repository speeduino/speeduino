#include "../../test_utils.h"
#include "src/decoders/st170.h"
#include "scheduler_ignition_controller.h"
#include "decoder_builder.h"

struct context_st170_t
{
  statuses current = {};
  config4 page4 = {};
  config6 page6 = {};
  config10 page10 = {};
  decoders::shared::state_t decoderState;

  void init(void)
  {
    decoderState = decoders::st170::initialise(page4);
  }

  void triggerSecondary(uint32_t curTime)
  {
    decoders::st170::triggerSecondary(curTime, current, decoderState, page4, page6, page10);
  }

  uint32_t getRevolutionTime()
  {
    return decoders::st170::getRevolutionTime(current, decoderState, page4);
  }

  void setup_secondaryfilter(uint32_t lastGap)
  {
    decoderState.toothLastSecToothTime = 100000;
    decoderState.toothLastMinusOneSecToothTime = decoderState.toothLastSecToothTime - lastGap;
  }
};

static void test_getRevolutionTime(void)
{
  context_st170_t context;

  context.page4.StgCycles = 0;
  context.current.crankRPM = 400;
  context.current.startRevolutions = 1;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.current.revolutionTime = 12345UL;
  context.decoderState.toothOneMinusOneTime = 1000UL;
  context.decoderState.toothOneTime = context.decoderState.toothOneMinusOneTime + 60000UL; // Full revolution: 60000uS
  context.decoderState.toothLastMinusOneToothTime = 1000UL;
  context.decoderState.toothLastToothTime = context.decoderState.toothLastMinusOneToothTime + 1000UL; // Tooth gap of 1000uS * 36 teeth: 36000uS

  // Running: full revolution
  context.current.setRpm(context.current.crankRPM*2);
  TEST_ASSERT_EQUAL_UINT32(60000UL, context.getRevolutionTime());

  // Cranking: per tooth
  context.decoderState.toothCurrentCount = 2;
  context.current.setRpm(context.current.crankRPM/2);
  TEST_ASSERT_EQUAL_UINT32(36000UL, context.getRevolutionTime());

  // Can't do a per tooth calculation at tooth #1, so no change
  context.decoderState.toothCurrentCount = 1;
  TEST_ASSERT_EQUAL_UINT32(12345UL, context.getRevolutionTime());
}

static void test_triggerSecondary_startup(void)
{
  context_st170_t context;
  context.init();
  context.decoderState.triggerSecFilterTime = 1;

  context.triggerSecondary(100000);

  TEST_ASSERT_EQUAL_UINT32(0, context.decoderState.curGap2);
  TEST_ASSERT_EQUAL_UINT32(100000, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL_UINT16(0, context.decoderState.secondaryToothCount);
}

static void test_triggerSecondary_filter_rejects_pulse(void)
{
  context_st170_t context;
  context.init();
  context.setup_secondaryfilter(1000);
  context.decoderState.triggerSecFilterTime = 1001;
  context.decoderState.targetGap2 = 1234;
  context.decoderState.secondaryToothCount = 2;

  context.triggerSecondary(101000); // Gap 1000, below the filter threshold.

  TEST_ASSERT_EQUAL_UINT32(1000, context.decoderState.curGap2);
  TEST_ASSERT_EQUAL_UINT32(100000, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL_UINT32(1234, context.decoderState.targetGap2);
  TEST_ASSERT_EQUAL_UINT16(2, context.decoderState.secondaryToothCount);
}

static void test_triggerSecondary_regular_tooth(void)
{
  context_st170_t context;
  context.init();
  context.setup_secondaryfilter(1000);
  context.decoderState.triggerSecFilterTime = 100;
  context.decoderState.secondaryToothCount = 2;

  context.triggerSecondary(101000); // Gap 1000; below the 1500 target gap.

  TEST_ASSERT_EQUAL_UINT32(1500, context.decoderState.targetGap2);
  TEST_ASSERT_EQUAL_UINT32(100000, context.decoderState.toothLastMinusOneSecToothTime);
  TEST_ASSERT_EQUAL_UINT32(101000, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL_UINT32(250, context.decoderState.triggerSecFilterTime);
  TEST_ASSERT_EQUAL_UINT16(3, context.decoderState.secondaryToothCount);
}

static void test_triggerSecondary_long_gap_resets_pattern(void)
{
  context_st170_t context;
  context.init();
  context.setup_secondaryfilter(1000);
  context.decoderState.triggerSecFilterTime = 100;
  context.decoderState.secondaryToothCount = 3;

  context.triggerSecondary(101500); // Gap equals the 1500 target gap.

  TEST_ASSERT_EQUAL_UINT16(1, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL_UINT8(1, context.decoderState.revolutionOne);
  TEST_ASSERT_EQUAL_UINT32(0, context.decoderState.triggerSecFilterTime);
  TEST_ASSERT_EQUAL_UINT32(101500, context.decoderState.toothLastSecToothTime);
}

static void test_triggerSecondary_count_five_resets_pattern(void)
{
  context_st170_t context;
  context.init();
  context.setup_secondaryfilter(1000);
  context.decoderState.triggerSecFilterTime = 100;
  context.decoderState.secondaryToothCount = 5;

  context.triggerSecondary(101000); // Regular gap, but count is already 5.

  TEST_ASSERT_EQUAL_UINT16(1, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL_UINT8(1, context.decoderState.revolutionOne);
  TEST_ASSERT_EQUAL_UINT32(0, context.decoderState.triggerSecFilterTime);
}


static void setup_vvt_closed_loop(context_st170_t &context)
{
  context.init();
  context.setup_secondaryfilter(1000);
  context.decoderState.triggerSecFilterTime = 1;

  context.page6.vvtEnabled = 1;
  context.page6.vvtMode = VVT_MODE_CLOSED_LOOP;
  context.page4.ANGLEFILTER_VVT = 0;
  context.page10.vvtCL0DutyAng = 25;
  static const auto getCrankAngle = [](uint32_t) -> int16_t { return 90; };
  context.current.decoder = decoder_builder_t().setGetCrankAngle(getCrankAngle).build();
}

static void test_triggerSecondary_vvt_fail_conditions(void)
{
  context_st170_t context;

  setup_vvt_closed_loop(context);
  context.page6.vvtEnabled = false;
  context.triggerSecondary(101500);
  TEST_ASSERT_EQUAL_INT16(0, context.current.vvt1.angle);

  setup_vvt_closed_loop(context);
  context.decoderState.revolutionOne = false;
  context.triggerSecondary(100001);
  TEST_ASSERT_EQUAL_INT16(0, context.current.vvt1.angle);  

  setup_vvt_closed_loop(context);
  context.decoderState.secondaryToothCount = 2;
  context.triggerSecondary(100001);
  TEST_ASSERT_EQUAL_INT16(0, context.current.vvt1.angle);  

  setup_vvt_closed_loop(context);
  context.page6.vvtMode = VVT_MODE_OPEN_LOOP;
  context.triggerSecondary(100001);
  TEST_ASSERT_EQUAL_INT16(0, context.current.vvt1.angle);  
}

static void test_triggerSecondary_vvt_closed_loop(void)
{
  context_st170_t context;
  setup_vvt_closed_loop(context);

  // Gap (1500) meets the 1.5x target and enters the VVT update branch.
  context.triggerSecondary(101500);

  TEST_ASSERT_EQUAL_UINT16(1, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL_UINT8(1, context.decoderState.revolutionOne);
  TEST_ASSERT_EQUAL_INT16(155, context.current.vvt1.angle);
}

static void set_discharge_angles(uint16_t angle)
{
  ignitionSchedule1.dischargeAngle = angle;
#if (IGN_CHANNELS >= 2)
  ignitionSchedule2.dischargeAngle = angle;
#endif
#if (IGN_CHANNELS >= 3)
  ignitionSchedule3.dischargeAngle = angle;
#endif
#if (IGN_CHANNELS >= 4)
  ignitionSchedule4.dischargeAngle = angle;
#endif
}

static void test_setEndTeeth_non_sequential_and_sequential(void)
{
  context_st170_t context;
  context.init();

  // Keep the test away from the tooth-range boundaries.
  set_discharge_angles(180);
  context.page4.sparkMode = IGN_MODE_WASTED;
  decoders::st170::setEndTeeth(context.decoderState, context.page4);
  const uint16_t nonSequentialTooth = context.decoderState.ignitionEndTeeth[0];

  TEST_ASSERT_TRUE(nonSequentialTooth >= 1);
  TEST_ASSERT_TRUE(nonSequentialTooth <= 35);

  context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
  context.page4.TrigSpeed = CRANK_SPEED;
  set_discharge_angles(180 + 360);
  decoders::st170::setEndTeeth(context.decoderState, context.page4);
  TEST_ASSERT_EQUAL_UINT16(nonSequentialTooth + 36,
                           context.decoderState.ignitionEndTeeth[0]);

#if (IGN_CHANNELS >= 2)
  TEST_ASSERT_EQUAL_UINT16(nonSequentialTooth + 36,
                           context.decoderState.ignitionEndTeeth[1]);
#endif
#if (IGN_CHANNELS >= 3)
  TEST_ASSERT_EQUAL_UINT16(nonSequentialTooth + 36,
                           context.decoderState.ignitionEndTeeth[2]);
#endif
#if (IGN_CHANNELS >= 4)
  TEST_ASSERT_EQUAL_UINT16(nonSequentialTooth + 36,
                           context.decoderState.ignitionEndTeeth[3]);
#endif
}

void testFordST170()
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_getRevolutionTime);
  RUN_TEST(test_triggerSecondary_startup);
  RUN_TEST(test_triggerSecondary_filter_rejects_pulse);
  RUN_TEST(test_triggerSecondary_regular_tooth);
  RUN_TEST(test_triggerSecondary_long_gap_resets_pattern);
  RUN_TEST(test_triggerSecondary_count_five_resets_pattern);
  RUN_TEST(test_triggerSecondary_vvt_closed_loop);
  RUN_TEST(test_triggerSecondary_vvt_fail_conditions);
  RUN_TEST(test_setEndTeeth_non_sequential_and_sequential);
}
