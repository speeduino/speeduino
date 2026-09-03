#include "../test_utils.h"
#include "../channel_test_helpers.h"
#include "scheduler_ignition_controller.h"
#include "../fake_decoder_status.h"
#include "decoder_builder.h"
#include "src/pins/pinMapping.h"

extern void matchIgnitionSchedulersToSyncState(const config2 &page2, const config4 &page4, statuses &current);

struct test_context_t
{
  statuses current = {};
  config2 page2;
  config4 page4;
  config10 page10;
  config13 page13;
  pinNumbers_t pins;

  test_context_t(uint8_t boardId)
  {
    page2.pinMapping = boardId;
    pins = getPinMapping(boardId);
  }

  void initialise(void)
  {
    ::initialiseIgnitionSchedules(current, page2, page4, page10, page13, pins);
  }
};

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
    TEST_ASSERT_TRUE_MESSAGE(schedule._pStartCallback!=nullCallback, msg);
    sprintf_P(msg, PSTR("ign%" PRIu8 "EndFunction"), channel+1);
    TEST_ASSERT_TRUE_MESSAGE(schedule._pEndCallback!=nullCallback, msg);
  }
}

static void assert_ignition_schedules(uint16_t crankAngle, uint16_t expectedOutputs, const uint16_t (&angle)[8], const statuses &current)
{
  char msg[48];

  strcpy_P(msg, PSTR("CRANK_ANGLE_MAX_IGN"));
  TEST_ASSERT_EQUAL_INT16_MESSAGE(crankAngle, CRANK_ANGLE_MAX_IGN, msg);
  strcpy_P(msg, PSTR("maxIgnOutputs"));
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(expectedOutputs, current.maxIgnOutputs, msg);

  RUNIF_IGNCHANNEL1(assert_ignition_channel(angle[0], 0, ignitionSchedule1, current), {});
  RUNIF_IGNCHANNEL2(assert_ignition_channel(angle[1], 1, ignitionSchedule2, current), {});
  RUNIF_IGNCHANNEL3(assert_ignition_channel(angle[2], 2, ignitionSchedule3, current), {});
  RUNIF_IGNCHANNEL4(assert_ignition_channel(angle[3], 3, ignitionSchedule4, current), {});
  RUNIF_IGNCHANNEL5(assert_ignition_channel(angle[4], 4, ignitionSchedule5, current), {});
  RUNIF_IGNCHANNEL6(assert_ignition_channel(angle[5], 5, ignitionSchedule6, current), {});
  RUNIF_IGNCHANNEL7(assert_ignition_channel(angle[6], 6, ignitionSchedule7, current), {});
  RUNIF_IGNCHANNEL8(assert_ignition_channel(angle[7], 7, ignitionSchedule8, current), {});
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
  const uint16_t angle[] = {0,144,288,432,576,0,0,0};
  assert_ignition_schedules(720U, 5U, angle, current);
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

static void run_5_cylinder_4stroke_tests(void)
{
  RUN_TEST_P(cylinder5_stroke4_seq_even);
  RUN_TEST_P(cylinder5_stroke4_wasted_even);
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
  }
}