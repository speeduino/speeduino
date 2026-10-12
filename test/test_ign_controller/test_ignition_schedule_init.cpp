#include "../test_utils.h"
#include "../channel_test_helpers.h"
#include "scheduler_ignition_controller.h"
#include "../fake_decoder_status.h"
#include "test_context.h"
#include "scheduledIO_ign.h"

extern void matchIgnitionSchedulersToSyncState(const config2 &page2, const config4 &page4, statuses &current);
extern void setCallbacks(uint8_t sparkMode, uint8_t numCylinders, uint8_t rotaryMode);
extern void resetIgnitionSchedulers(void);

auto setupContext(uint8_t boardId, uint8_t nCylinders, uint8_t strokes)
{
  test_context_t context(boardId);

  context.page2.nCylinders = nCylinders;
  context.page2.strokes = strokes;

  return context;
}

static void assert_ignition_channel(uint16_t angle, uint8_t channel, const IgnitionSchedule &schedule, const statuses &current)
{
  char msg[32];

  if (channel<current.maxIgnOutputs)
  {
    sprintf_P(msg, PSTR("channe%" PRIu8 "Degrees"), channel+1);
    TEST_ASSERT_EQUAL_MESSAGE(angle, schedule.channelDegrees, msg);
    sprintf_P(msg, PSTR("ign%" PRIu8 "StartFunction"), channel+1);
    TEST_ASSERT_TRUE_MESSAGE(schedule._callbacks.start!=nullCallback, msg);
    sprintf_P(msg, PSTR("ign%" PRIu8 "EndFunction"), channel+1);
    TEST_ASSERT_TRUE_MESSAGE(schedule._callbacks.end!=nullCallback, msg);
  }
}

static void assert_ignition_schedules(uint16_t crankAngle, uint16_t expectedOutputs, const uint16_t (&angle)[8], const statuses &current)
{
  char msg[48];

  strcpy_P(msg, PSTR("CRANK_ANGLE_MAX_IGN"));
  TEST_ASSERT_EQUAL_INT16_MESSAGE(crankAngle, CRANK_ANGLE_MAX_IGN, msg);
  strcpy_P(msg, PSTR("maxIgnOutputs"));
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(expectedOutputs, current.maxIgnOutputs, msg);

  for (uint8_t i = 0; i < _countof(ignitionSchedules); i++)
  {
    assert_ignition_channel(angle[i], i, ignitionSchedules[i], current);
  }
}

static void assert_cylinder1_stroke4_seq_even(const statuses &current)
{
  const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_ignition_schedules(720U, 1U, angle, current);
}

static void cylinder1_stroke4_seq_even(void)
{
  auto context = setupContext(3, 1, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  assert_cylinder1_stroke4_seq_even(context.current);
}

static void cylinder1_stroke4_wasted_even(void)
{
  auto context = setupContext(3, 1, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_WASTED;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_ignition_schedules(360U, 1U, angle, context.current);
}  

static void cylinder1_stroke4_seq_odd(void)
{
  auto context = setupContext(3, 1, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
  context.page2.engineType = ODD_FIRE;
  context.initialise();
  const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_ignition_schedules(720U, 1U, angle, context.current);
}

static void run_1_cylinder_4stroke_tests(void)
{
  RUN_TEST_P(cylinder1_stroke4_seq_even);
  RUN_TEST_P(cylinder1_stroke4_wasted_even);
  RUN_TEST_P(cylinder1_stroke4_seq_odd);
}

static void assert_cylinder2_stroke4_seq_even(const statuses &current)
{
  const uint16_t angle[] = {0,360,0,0,0,0,0,0};
  assert_ignition_schedules(720U, 2U, angle, current);
}

static void cylinder2_stroke4_seq_even(void)
{
  auto context = setupContext(3, 2, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  assert_cylinder2_stroke4_seq_even(context.current);
}

static void cylinder2_stroke4_wasted_even(void)
{
  auto context = setupContext(3, 2, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_WASTED;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_ignition_schedules(360U, 2U, angle, context.current);
}  

static void cylinder2_stroke4_seq_odd(void)
{
  auto context = setupContext(3, 2, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
  context.page2.engineType = ODD_FIRE;
  context.page2.oddfire[0] = 13;
  context.page2.oddfire[1] = 111;
  context.page2.oddfire[2] = 217;

  context.initialise();
  const uint16_t angle[] = {0,13,0,0,0,0,0,0};
  assert_ignition_schedules(720U, 2U, angle, context.current);
}

static void run_2_cylinder_4stroke_tests(void)
{
  RUN_TEST_P(cylinder2_stroke4_seq_even);
  RUN_TEST_P(cylinder2_stroke4_wasted_even);
  RUN_TEST_P(cylinder2_stroke4_seq_odd);
}

static void assert_cylinder3_stroke4_seq_even(const statuses &current)
{
  const uint16_t angle[] = {0,240,480,0,0,0,0,0};
  assert_ignition_schedules(720U, 3U, angle, current);
}

static void cylinder3_stroke4_seq_even(void)
{
  auto context = setupContext(3, 3, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  assert_cylinder3_stroke4_seq_even(context.current);
}

static void cylinder3_stroke4_wasted_even(void)
{
  auto context = setupContext(3, 3, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_WASTED;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  const uint16_t angle[] = {0,120,240,0,0,0,0,0};
  assert_ignition_schedules(360U, 3U, angle, context.current);
}  

static void cylinder3_stroke4_wasted_odd(void)
{
  auto context = setupContext(3, 3, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_WASTED;
  context.page2.engineType = ODD_FIRE;
  context.page2.oddfire[0] = 13;
  context.page2.oddfire[1] = 111;
  context.page2.oddfire[2] = 217;
  context.initialise();
  const uint16_t angle[] = {0,13,111,0,0,0,0,0};
  assert_ignition_schedules(360U, 3U, angle, context.current);
}  

static void cylinder3_stroke4_single_even(void)
{
  auto context = setupContext(3, 3, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_SINGLE;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  const uint16_t angle[] = {0,240,480,0,0,0,0,0};
  assert_ignition_schedules(720U, 3U, angle, context.current);
}  

static void run_3_cylinder_4stroke_tests(void)
{
  RUN_TEST_P(cylinder3_stroke4_seq_even);
  RUN_TEST_P(cylinder3_stroke4_wasted_even);
  RUN_TEST_P(cylinder3_stroke4_wasted_odd);
  RUN_TEST_P(cylinder3_stroke4_single_even);
}

static void assert_cylinder4_stroke4_seq_even(const statuses &current)
{
  const uint16_t angle[] = {0,180,360,540,0,0,0,0};
  assert_ignition_schedules(720U, 4U, angle, current);
}

static void cylinder4_stroke4_seq_even(void)
{
  auto context = setupContext(3, 4, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  assert_cylinder4_stroke4_seq_even(context.current);
}

static void cylinder4_stroke4_wasted_even(void)
{
  auto context = setupContext(3, 4, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_WASTED;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_ignition_schedules(360U, 2U, angle, context.current);
}  

static void cylinder4_stroke4_seq_odd(void)
{
  auto context = setupContext(3, 4, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
  context.page2.engineType = ODD_FIRE;
  context.page2.oddfire[0] = 13;
  context.page2.oddfire[1] = 111;
  context.page2.oddfire[2] = 699;
  context.initialise();
  const uint16_t angle[] = {0,13,111,699,0,0,0,0};
  assert_ignition_schedules(720U, 4U, angle, context.current);
}

static void cylinder4_stroke4_single_even(void)
{
  auto context = setupContext(3, 4, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_SINGLE;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_ignition_schedules(360U, 2U, angle, context.current);
}  

static void run_4_cylinder_4stroke_tests(void)
{
  RUN_TEST_P(cylinder4_stroke4_seq_even);
  RUN_TEST_P(cylinder4_stroke4_wasted_even);
  RUN_TEST_P(cylinder4_stroke4_seq_odd);
  RUN_TEST_P(cylinder4_stroke4_single_even);
}

static void assert_cylinder5_stroke4_seq_even(const statuses &current)
{
#if IGN_CHANNELS>=5
  const uint16_t angle[] = {0,144,288,432,576,0,0,0};
  assert_ignition_schedules(720U, 5U, angle, current);
#else
  const uint16_t angle[] = {0,72,144,216,288,0,0,0};
  assert_ignition_schedules(360U, 5U, angle, current);
#endif
}

static void cylinder5_stroke4_seq_even(void)
{
  auto context = setupContext(3, 5, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  assert_cylinder5_stroke4_seq_even(context.current);
}

static void cylinder5_stroke4_wasted_even(void)
{
  auto context = setupContext(3, 5, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_WASTED;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  const uint16_t angle[] = {0,72,144,216,288,0,0,0};
  assert_ignition_schedules(360U, 5U, angle, context.current);
}

static void cylinder5_stroke4_single_even(void)
{
  auto context = setupContext(3, 5, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_SINGLE;

  context.initialise();

  constexpr uint16_t angles[] = { 0, 144, 288, 432, 576, 0, 0, 0};
  assert_ignition_schedules(720, 5, angles, context.current);
}

static void run_5_cylinder_4stroke_tests(void)
{
  RUN_TEST_P(cylinder5_stroke4_seq_even);
  RUN_TEST_P(cylinder5_stroke4_wasted_even);
  RUN_TEST_P(cylinder5_stroke4_single_even);
}

static void assert_cylinder6_stroke4_seq_even(const statuses &current)
{
#if IGN_CHANNELS >= 6
  const uint16_t angle[] = {0,120,240,360,480,600,0,0};
  assert_ignition_schedules(720U, 6U, angle, current);
#else
  const uint16_t angle[] = {0,120,240,0,0,0,0,0};
  assert_ignition_schedules(360U, 3U, angle, current);
#endif
}

static void cylinder6_stroke4_seq_even(void)
{
  auto context = setupContext(3, 6, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  assert_cylinder6_stroke4_seq_even(context.current);
}

static void cylinder6_stroke4_wasted_even(void)
{
  auto context = setupContext(3, 6, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_WASTED;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  const uint16_t angle[] = {0,120,240,0,0,0,0,0};
  assert_ignition_schedules(360U, 3U, angle, context.current);
} 

static void run_6_cylinder_4stroke_tests(void)
{
  RUN_TEST_P(cylinder6_stroke4_seq_even);
  RUN_TEST_P(cylinder6_stroke4_wasted_even); 
}

static void assert_cylinder8_stroke4_seq_even(const statuses &current)
{
#if IGN_CHANNELS >= 8
  const uint16_t angle[] = {0,90,180,270,360,450,540,630};
  assert_ignition_schedules(720U, 8U, angle, current);
#else
  const uint16_t angle[] = {0,90,180,270,0,0,0,0};
  assert_ignition_schedules(360U, 4U, angle, current);
#endif
}

static void cylinder8_stroke4_seq_even(void)
{
  auto context = setupContext(3, 8, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  assert_cylinder8_stroke4_seq_even(context.current);
}

static void cylinder8_stroke4_wasted_even(void)
{
  auto context = setupContext(3, 8, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_WASTED;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  const uint16_t angle[] = {0,90,180,270,0,0,0,0};
  assert_ignition_schedules(360U, 4U, angle, context.current);
}  

static void run_8_cylinder_4stroke_tests(void)
{
  RUN_TEST_P(cylinder8_stroke4_seq_even);
  RUN_TEST_P(cylinder8_stroke4_wasted_even);
}

static void assert_4cylinder_half_sync(const statuses &current)
{
  const uint16_t angle[] = {0,180,360,540,0,0,0,0};
  assert_ignition_schedules(360U, 2U, angle, current);
}

struct partial_sync_context_t
{
  statuses current = {};
  config2 page2 = {};
  config4 page4 = {};

  partial_sync_context_t(void)
  {
    current.decoder = decoder_builder_t().setGetStatus(getFakeDecoderStatus).build();
  }

};

static partial_sync_context_t setupPartialSyncTest(uint8_t cylinders)
{
  auto initContext = setupContext(3, cylinders, FOUR_STROKE);
  initContext.page4.sparkMode = IGN_MODE_SEQUENTIAL;
  initContext.page2.engineType = EVEN_FIRE;
  initContext.initialise();

  partial_sync_context_t context;
  context.page2 = initContext.page2;
  context.page4 = initContext.page4;
  context.current = initContext.current;
  context.current.decoder = decoder_builder_t(context.current.decoder).setGetStatus(getFakeDecoderStatus).build();
  return context;
}

static void test_partial_sync_1_cylinder(void)
{
  auto context = setupPartialSyncTest(1);

  // Initial state
  assert_cylinder1_stroke4_seq_even(context.current);

  // No change for 1 cylinder
  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  matchIgnitionSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_cylinder1_stroke4_seq_even(context.current);

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  matchIgnitionSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_cylinder1_stroke4_seq_even(context.current);
}

static void test_partial_sync_2_cylinder(void)
{
  auto context = setupPartialSyncTest(2);
  
  // Initial state
  assert_cylinder2_stroke4_seq_even(context.current);

  // No change for 2 cylinder
  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  matchIgnitionSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_cylinder2_stroke4_seq_even(context.current);

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  matchIgnitionSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_cylinder2_stroke4_seq_even(context.current);
}

static void test_partial_sync_3_cylinder(void)
{
  auto context = setupPartialSyncTest(3);

  // Initial state
  assert_cylinder3_stroke4_seq_even(context.current);

  // No change for 3 cylinder
  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  matchIgnitionSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_cylinder3_stroke4_seq_even(context.current);

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  matchIgnitionSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_cylinder3_stroke4_seq_even(context.current);
}

static void test_partial_sync_4_cylinder(void)
{
  auto context = setupPartialSyncTest(4);
  
  // Initial state
  assert_cylinder4_stroke4_seq_even(context.current);

  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  matchIgnitionSchedulersToSyncState(context.page2, context.page4, context.current);
  TEST_ASSERT_EQUAL(360, CRANK_ANGLE_MAX_IGN);
  assert_4cylinder_half_sync(context.current);

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  matchIgnitionSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_cylinder4_stroke4_seq_even(context.current);

  // No sync => no change
  fakeDecoderStatus.syncStatus = SyncStatus::None;
  matchIgnitionSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_cylinder4_stroke4_seq_even(context.current);
}

static void test_partial_sync_5_cylinder(void)
{
  auto context = setupPartialSyncTest(5);

  // Initial state
  assert_cylinder5_stroke4_seq_even(context.current);

  // No change for 5 cylinder
  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  matchIgnitionSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_cylinder5_stroke4_seq_even(context.current);

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  matchIgnitionSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_cylinder5_stroke4_seq_even(context.current);
}

static void test_partial_sync_6_cylinder(void)
{
#if IGN_CHANNELS>=6
    auto context = setupPartialSyncTest(6);

  // Initial state
  assert_cylinder6_stroke4_seq_even(context.current);

  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  matchIgnitionSchedulersToSyncState(context.page2, context.page4, context.current);
  {
    const uint16_t angle[] = {0,120,240,360,480,600,0,0};
    assert_ignition_schedules(360U, 3U, angle, context.current);
  }

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  matchIgnitionSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_cylinder6_stroke4_seq_even(context.current);
#else
  TEST_IGNORE_MESSAGE("Skipping 6 cylinder partial sync test - not enough injectors");
#endif
}


static void test_partial_sync_8_cylinder(void)
{
#if IGN_CHANNELS>=8
  auto context = setupPartialSyncTest(8);

  // Initial state
  assert_cylinder8_stroke4_seq_even(context.current);

  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  matchIgnitionSchedulersToSyncState(context.page2, context.page4, context.current);
  {
    const uint16_t angle[] = {0,90,180,270,360,450,540,630};
    assert_ignition_schedules(360U, 4U, angle, context.current);
  }

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  matchIgnitionSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_cylinder8_stroke4_seq_even(context.current);
#else
  TEST_IGNORE_MESSAGE("Skipping 8 cylinder partial sync test - not enough injectors");
#endif
}

static void run_partial_sync_tests(void)
{
  RUN_TEST_P(test_partial_sync_1_cylinder);
  RUN_TEST_P(test_partial_sync_2_cylinder);
  RUN_TEST_P(test_partial_sync_3_cylinder);
  RUN_TEST_P(test_partial_sync_4_cylinder);
  RUN_TEST_P(test_partial_sync_5_cylinder);
  RUN_TEST_P(test_partial_sync_6_cylinder);
  RUN_TEST_P(test_partial_sync_8_cylinder);
}
static void assert_callbacks(IgnitionSchedule &schedule, IgnitionSchedule::callback_t start, IgnitionSchedule::callback_t end)
{
    TEST_ASSERT_EQUAL_PTR (start, schedule._callbacks.start);
    TEST_ASSERT_EQUAL_PTR (end, schedule._callbacks.end);
}

static void assert_callbacks(const Schedule::callback_pair_t (&callbacks)[8])
{
  uint8_t i = 0;
  for (auto& schedule: ignitionSchedules)
  {
    TEST_ASSERT_EQUAL_PTR (callbacks[i].start, schedule._callbacks.start);
    TEST_ASSERT_EQUAL_PTR (callbacks[i].end, schedule._callbacks.end);   
    ++i;
  }
}

static void assert_singlechannel_callbacks(void)
{
  Schedule::callback_pair_t callbacks[] = {
    { beginCoil1Charge, endCoil1Charge },
    { beginCoil1Charge, endCoil1Charge },
    { beginCoil1Charge, endCoil1Charge },
    { beginCoil1Charge, endCoil1Charge },
    { beginCoil1Charge, endCoil1Charge },
    { beginCoil1Charge, endCoil1Charge },
    { beginCoil1Charge, endCoil1Charge },
    { beginCoil1Charge, endCoil1Charge },
  };
  assert_callbacks(callbacks);
}

static void test_initialize_singlechannel_callbacks(void)
{
  for (uint8_t numCylinders = 0; numCylinders<IGN_CHANNELS; ++numCylinders)
  {
    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_SINGLE, numCylinders, 0U);
    assert_singlechannel_callbacks();
  }
}

static void assert_wastedCOP_1_to_3_callbacks(void)
{
    Schedule::callback_pair_t callbacks[] = {
      { beginCoil1Charge, endCoil1Charge },
      { beginCoil2Charge, endCoil2Charge },
      { beginCoil3Charge, endCoil3Charge },
      { beginCoil4Charge, endCoil4Charge },
      { beginCoil5Charge, endCoil5Charge },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
    };
    assert_callbacks(callbacks);
}

static void test_initialize_wastedCOP1_callbacks(void)
{
    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_WASTEDCOP, 1, 0U);
    assert_wastedCOP_1_to_3_callbacks();
}
    
static void test_initialize_wastedCOP2_callbacks(void)
{
    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_WASTEDCOP, 2, 0U);
    assert_wastedCOP_1_to_3_callbacks();
}

static void test_initialize_wastedCOP3_callbacks(void)
{
    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_WASTEDCOP, 3, 0U);
    assert_wastedCOP_1_to_3_callbacks();
}

static void test_initialize_wastedCOP4_callbacks(void)
{
    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_WASTEDCOP, 4, 0U);

    Schedule::callback_pair_t callbacks[] = {
      { beginCoil1and3Charge, endCoil1and3Charge },
      { beginCoil2and4Charge, endCoil2and4Charge },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
    };
    assert_callbacks(callbacks);
}

static void test_initialize_wastedCOP5_callbacks(void)
{
    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_WASTEDCOP, 5, 0U);
    assert_wastedCOP_1_to_3_callbacks();
}

static void test_initialize_wastedCOP6_callbacks(void)
{
    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_WASTEDCOP, 6, 0U);

    Schedule::callback_pair_t callbacks[] = {
      { beginCoil1and4Charge, endCoil1and4Charge },
      { beginCoil2and5Charge, endCoil2and5Charge },
      { beginCoil3and6Charge, endCoil3and6Charge },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
    };
    assert_callbacks(callbacks);
}

static void test_initialize_wastedCOP8_callbacks(void)
{
    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_WASTEDCOP, 8, 0U);

    Schedule::callback_pair_t callbacks[] = {
      { beginCoil1and5Charge, endCoil1and5Charge },
      { beginCoil2and6Charge, endCoil2and6Charge },
      { beginCoil3and7Charge, endCoil3and7Charge },
      { beginCoil4and8Charge, endCoil4and8Charge },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
    };
    assert_callbacks(callbacks);
}

static void assert_sequential_callbacks(void)
{
    Schedule::callback_pair_t callbacks[] = {
      { beginCoil1Charge, endCoil1Charge },
      { beginCoil2Charge, endCoil2Charge },
      { beginCoil3Charge, endCoil3Charge },
      { beginCoil4Charge, endCoil4Charge },
      { beginCoil5Charge, endCoil5Charge },
      { beginCoil6Charge, endCoil6Charge },
      { beginCoil7Charge, endCoil7Charge },
      { beginCoil8Charge, endCoil8Charge },
    };
    assert_callbacks(callbacks);
}

static void test_initialize_sequential_callbacks(void)
{
    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_SEQUENTIAL, 1, 0U);
    assert_sequential_callbacks();

    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_SEQUENTIAL, 2, 0U);
    assert_sequential_callbacks();

    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_SEQUENTIAL, 3, 0U);
    assert_sequential_callbacks();

    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_SEQUENTIAL, 4, 0U);
    assert_sequential_callbacks();

    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_SEQUENTIAL, 5, 0U);
    assert_sequential_callbacks();

    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_SEQUENTIAL, 6, 0U);
    assert_sequential_callbacks();

    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_SEQUENTIAL, 7, 0U);
    assert_sequential_callbacks();

    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_SEQUENTIAL, 8, 0U);
    assert_sequential_callbacks();
}

static void test_initialize_rotary_fc_callbacks(void)
{
    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_ROTARY, 0U, ROTARY_IGN_FC);

    Schedule::callback_pair_t callbacks[] = {
      { beginCoil1Charge, endCoil1Charge },
      { beginCoil1Charge, endCoil1Charge },
      { beginTrailingCoilCharge, endTrailingCoilCharge1 },
      { beginTrailingCoilCharge, endTrailingCoilCharge2 },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
    };
    assert_callbacks(callbacks);
}

static void test_initialize_rotary_fd_callbacks(void)
{
    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_ROTARY, 0U, ROTARY_IGN_FD);

    Schedule::callback_pair_t callbacks[] = {
      { beginCoil1Charge, endCoil1Charge },
      { beginCoil1Charge, endCoil1Charge },
      { beginCoil2Charge, endCoil2Charge },
      { beginCoil3Charge, endCoil3Charge },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
    };
    assert_callbacks(callbacks);
}

static void test_initialize_rotary_rx8_callbacks(void)
{
    resetIgnitionSchedulers();
    setCallbacks(IGN_MODE_ROTARY, 0U, ROTARY_IGN_RX8);

    Schedule::callback_pair_t callbacks[] = {
      { beginCoil1Charge, endCoil1Charge },
      { beginCoil2Charge, endCoil2Charge },
      { beginCoil3Charge, endCoil3Charge },
      { beginCoil4Charge, endCoil4Charge },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
      { nullCallback, nullCallback },
    };
    assert_callbacks(callbacks);
}

static void run_callback_tests(void)
{
  RUN_TEST_P(test_initialize_singlechannel_callbacks);
  RUN_TEST_P(test_initialize_wastedCOP1_callbacks);
  RUN_TEST_P(test_initialize_wastedCOP2_callbacks);
  RUN_TEST_P(test_initialize_wastedCOP3_callbacks);
  RUN_TEST_P(test_initialize_wastedCOP4_callbacks);
  RUN_TEST_P(test_initialize_wastedCOP5_callbacks);
  RUN_TEST_P(test_initialize_wastedCOP6_callbacks);
  RUN_TEST_P(test_initialize_wastedCOP8_callbacks);
  RUN_TEST_P(test_initialize_sequential_callbacks);
  RUN_TEST_P(test_initialize_rotary_fc_callbacks);
  RUN_TEST_P(test_initialize_rotary_fd_callbacks);
  RUN_TEST_P(test_initialize_rotary_rx8_callbacks);
}

static void test_rotary_fc(void)
{
  auto context = setupContext(3, 4, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_ROTARY;
  context.page10.rotaryType = ROTARY_IGN_FC;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  const uint16_t angles[] = {0,180,0,180,0,0,0,0};
  assert_ignition_schedules(360U, 4U, angles, context.current);
}

static void test_rotary_fd(void)
{
  auto context = setupContext(3, 4, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_ROTARY;
  context.page10.rotaryType = ROTARY_IGN_FD;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  const uint16_t angles[] = {0,180,0,180,0,0,0,0};
  assert_ignition_schedules(360U, 4U, angles, context.current);
}

static void test_rotary_rx8(void)
{
  auto context = setupContext(3, 4, FOUR_STROKE);
  context.page4.sparkMode = IGN_MODE_ROTARY;
  context.page10.rotaryType = ROTARY_IGN_RX8;
  context.page2.engineType = EVEN_FIRE;
  context.initialise();
  const uint16_t angles[] = {0,180,0,180,0,0,0,0};
  assert_ignition_schedules(360U, 4U, angles, context.current);
}

static void run_rotary_tests(void)
{
  RUN_TEST_P(test_rotary_fc);
  RUN_TEST_P(test_rotary_fd);
  RUN_TEST_P(test_rotary_rx8);
}

void testIgnitionScheduleInit(void)
{
  SET_UNITY_FILENAME() {
    run_1_cylinder_4stroke_tests();
    run_2_cylinder_4stroke_tests();
    run_3_cylinder_4stroke_tests();
    run_4_cylinder_4stroke_tests();
    run_5_cylinder_4stroke_tests();
    run_6_cylinder_4stroke_tests();
    run_8_cylinder_4stroke_tests();
    run_partial_sync_tests();
    run_callback_tests();
    run_rotary_tests();
  }
}