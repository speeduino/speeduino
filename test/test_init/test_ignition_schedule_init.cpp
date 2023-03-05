#include <Arduino.h>
#include <unity.h>
#include "globals.h"
#include "init.h"
#include "schedule_calcs.h"
#include "scheduledIO.h"
#include "../test_utils.h"
#include "storage.h"
#include "../test_schedules/channel_test_helpers.h"

void prepareForInitialiseAll(uint8_t boardId);

static void assert_ignition_channel(uint16_t angle, uint8_t channel, int channelInjDegrees, voidVoidCallback startFunction, voidVoidCallback endFunction)
{
  char msg[32];

  sprintf_P(msg, PSTR("channe%" PRIu8 "1InjDegrees"), channel+1);
  TEST_ASSERT_EQUAL_MESSAGE(angle, channelInjDegrees, msg);
  sprintf_P(msg, PSTR("ign%" PRIu8 "StartFunction"), channel+1);
  TEST_ASSERT_TRUE_MESSAGE(channel>=currentStatus.maxIgnOutputs || (startFunction!=nullCallback), msg);
  sprintf_P(msg, PSTR("ign%" PRIu8 "EndFunction"), channel+1);
  TEST_ASSERT_TRUE_MESSAGE(channel>=currentStatus.maxIgnOutputs || (endFunction!=nullCallback), msg);
}

static void assert_ignition_schedules(uint16_t crankAngle, uint16_t expectedOutputs, const uint16_t (&angle)[8])
{
  char msg[48];

  strcpy_P(msg, PSTR("CRANK_ANGLE_MAX_IGN"));
  TEST_ASSERT_EQUAL_INT16_MESSAGE(crankAngle, CRANK_ANGLE_MAX_IGN, msg);
  strcpy_P(msg, PSTR("maxIgnOutputs"));
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(expectedOutputs, currentStatus.maxIgnOutputs, msg);

  RUNIF_IGNCHANNEL1(assert_ignition_channel(angle[0], 0, ignitionSchedule1.channelIgnDegrees, ignitionSchedule1.pStartCallback, ignitionSchedule1.pEndCallback), {});
  RUNIF_IGNCHANNEL2(assert_ignition_channel(angle[1], 1, ignitionSchedule2.channelIgnDegrees, ignitionSchedule2.pStartCallback, ignitionSchedule2.pEndCallback), {});
  RUNIF_IGNCHANNEL3(assert_ignition_channel(angle[2], 2, ignitionSchedule3.channelIgnDegrees, ignitionSchedule3.pStartCallback, ignitionSchedule3.pEndCallback), {});
  RUNIF_IGNCHANNEL4(assert_ignition_channel(angle[3], 3, ignitionSchedule4.channelIgnDegrees, ignitionSchedule4.pStartCallback, ignitionSchedule4.pEndCallback), {});
  RUNIF_IGNCHANNEL5(assert_ignition_channel(angle[4], 4, ignitionSchedule5.channelIgnDegrees, ignitionSchedule5.pStartCallback, ignitionSchedule5.pEndCallback), {});
  RUNIF_IGNCHANNEL6(assert_ignition_channel(angle[5], 5, ignitionSchedule6.channelIgnDegrees, ignitionSchedule6.pStartCallback, ignitionSchedule6.pEndCallback), {});
  RUNIF_IGNCHANNEL7(assert_ignition_channel(angle[6], 6, ignitionSchedule7.channelIgnDegrees, ignitionSchedule7.pStartCallback, ignitionSchedule7.pEndCallback), {});
  RUNIF_IGNCHANNEL8(assert_ignition_channel(angle[7], 7, ignitionSchedule8.channelIgnDegrees, ignitionSchedule8.pStartCallback, ignitionSchedule8.pEndCallback), {});
}

static void assert_cylinder1_stroke4_seq_even(void)
{
  const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_ignition_schedules(720U, 1U, angle);
}

static void cylinder1_stroke4_seq_even(void)
{
  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  configPage2.engineType = EVEN_FIRE;
  initialiseAll(); //Run the main initialise function
  assert_cylinder1_stroke4_seq_even();
}

static void cylinder1_stroke4_wasted_even(void)
{
  configPage4.sparkMode = IGN_MODE_WASTED;
  configPage2.engineType = EVEN_FIRE;
  initialiseAll(); //Run the main initialise function
  const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_ignition_schedules(360U, 1U, angle);
}  

static void cylinder1_stroke4_seq_odd(void)
{
  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  configPage2.engineType = ODD_FIRE;
  initialiseAll(); //Run the main initialise function
  const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_ignition_schedules(720U, 1U, angle);
}

static void run_1_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 1;
  configPage2.strokes = FOUR_STROKE;

  RUN_TEST_P(cylinder1_stroke4_seq_even);
  RUN_TEST_P(cylinder1_stroke4_wasted_even);
  RUN_TEST_P(cylinder1_stroke4_seq_odd);
}

static void assert_cylinder2_stroke4_seq_even(void)
{
  const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_ignition_schedules(720U, 2U, angle);
}

static void cylinder2_stroke4_seq_even(void)
{
  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  configPage2.engineType = EVEN_FIRE;
  initialiseAll(); //Run the main initialise function
  assert_cylinder2_stroke4_seq_even();
}

static void cylinder2_stroke4_wasted_even(void)
{
  configPage4.sparkMode = IGN_MODE_WASTED;
  configPage2.engineType = EVEN_FIRE;
  initialiseAll(); //Run the main initialise function
  const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_ignition_schedules(360U, 2U, angle);
}  

static void cylinder2_stroke4_seq_odd(void)
{
  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  configPage2.engineType = ODD_FIRE;
  configPage2.oddfire2 = 13;
  configPage2.oddfire3 = 111;
  configPage2.oddfire4 = 217;

  initialiseAll(); //Run the main initialise function
  const uint16_t angle[] = {0,13,0,0,0,0,0,0};
  assert_ignition_schedules(720U, 2U, angle);
}

static void run_2_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 2;
  configPage2.strokes = FOUR_STROKE;

  RUN_TEST_P(cylinder2_stroke4_seq_even);
  RUN_TEST_P(cylinder2_stroke4_wasted_even);
  RUN_TEST_P(cylinder2_stroke4_seq_odd);
}

static void assert_cylinder3_stroke4_seq_even(void)
{
  const uint16_t angle[] = {0,240,480,0,0,0,0,0};
  assert_ignition_schedules(720U, 3U, angle);
}

static void cylinder3_stroke4_seq_even(void)
{
  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  configPage2.engineType = EVEN_FIRE;
  initialiseAll(); //Run the main initialise function
  assert_cylinder3_stroke4_seq_even();
}

static void cylinder3_stroke4_wasted_even(void)
{
  configPage4.sparkMode = IGN_MODE_WASTED;
  configPage2.engineType = EVEN_FIRE;
  initialiseAll(); //Run the main initialise function
  const uint16_t angle[] = {0,120,240,0,0,0,0,0};
  assert_ignition_schedules(360U, 3U, angle);
}  

static void cylinder3_stroke4_wasted_odd(void)
{
  configPage4.sparkMode = IGN_MODE_WASTED;
  configPage2.engineType = ODD_FIRE;
  configPage2.oddfire2 = 13;
  configPage2.oddfire3 = 111;
  configPage2.oddfire4 = 217;
  initialiseAll(); //Run the main initialise function
  const uint16_t angle[] = {0,13,111,0,0,0,0,0};
  assert_ignition_schedules(360U, 3U, angle);
}  

static void run_3_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 3;
  configPage2.strokes = FOUR_STROKE;

  RUN_TEST_P(cylinder3_stroke4_seq_even);
  RUN_TEST_P(cylinder3_stroke4_wasted_even);
  RUN_TEST_P(cylinder3_stroke4_wasted_odd);
}

static void assert_cylinder4_stroke4_seq_even(void)
{
  const uint16_t angle[] = {0,180,360,540,0,0,0,0};
  assert_ignition_schedules(720U, 4U, angle);
}

static void cylinder4_stroke4_seq_even(void)
{
  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  configPage2.engineType = EVEN_FIRE;
  initialiseAll(); //Run the main initialise function
  assert_cylinder4_stroke4_seq_even();
}

static void cylinder4_stroke4_wasted_even(void)
{
  configPage4.sparkMode = IGN_MODE_WASTED;
  configPage2.engineType = EVEN_FIRE;
  initialiseAll(); //Run the main initialise function
  const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_ignition_schedules(360U, 2U, angle);
}  

static void cylinder4_stroke4_seq_odd(void)
{
  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  configPage2.engineType = ODD_FIRE;
  configPage2.oddfire2 = 13;
  configPage2.oddfire3 = 111;
  configPage2.oddfire4 = 217;
  initialiseAll(); //Run the main initialise function
  const uint16_t angle[] = {0,13,111,217,0,0,0,0};
  assert_ignition_schedules(360U, 4U, angle);
}


static void run_4_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 4;
  configPage2.strokes = FOUR_STROKE;

  RUN_TEST_P(cylinder4_stroke4_seq_even);
  RUN_TEST_P(cylinder4_stroke4_wasted_even);
  RUN_TEST_P(cylinder4_stroke4_seq_odd);
}

static void assert_cylinder5_stroke4_seq_even(void)
{
  const uint16_t angle[] = {0,144,288,432,576,0,0,0};
  assert_ignition_schedules(720U, 5U, angle);
}

static void cylinder5_stroke4_seq_even(void)
{
  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  configPage2.engineType = EVEN_FIRE;
  initialiseAll(); //Run the main initialise function
  assert_cylinder5_stroke4_seq_even();
}

static void cylinder5_stroke4_wasted_even(void)
{
  configPage4.sparkMode = IGN_MODE_WASTED;
  configPage2.engineType = EVEN_FIRE;
  initialiseAll(); //Run the main initialise function
  const uint16_t angle[] = {0,72,144,216,288,0,0,0};
  assert_ignition_schedules(360U, 5U, angle);
}  

static void run_5_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 5;
  configPage2.strokes = FOUR_STROKE;

  RUN_TEST_P(cylinder5_stroke4_seq_even);
  RUN_TEST_P(cylinder5_stroke4_wasted_even);
}

static void assert_cylinder6_stroke4_seq_even(void)
{
#if IGN_CHANNELS >= 6
  const uint16_t angle[] = {0,120,240,360,480,600,0,0};
  assert_ignition_schedules(720U, 6U, angle);
#else
  const uint16_t angle[] = {0,120,240,0,0,0,0,0};
  assert_ignition_schedules(360U, 3U, angle);
#endif
}

static void cylinder6_stroke4_seq_even(void)
{
  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  configPage2.engineType = EVEN_FIRE;
  initialiseAll(); //Run the main initialise function
  assert_cylinder6_stroke4_seq_even();
}

static void cylinder6_stroke4_wasted_even(void)
{
  configPage4.sparkMode = IGN_MODE_WASTED;
  configPage2.engineType = EVEN_FIRE;
  initialiseAll(); //Run the main initialise function
  const uint16_t angle[] = {0,120,240,0,0,0,0,0};
  assert_ignition_schedules(360U, 3U, angle);
} 

static void run_6_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 6;
  configPage2.strokes = FOUR_STROKE;

  RUN_TEST_P(cylinder6_stroke4_seq_even);
  RUN_TEST_P(cylinder6_stroke4_wasted_even); 
}

static void assert_cylinder8_stroke4_seq_even(void)
{
#if IGN_CHANNELS >= 8
  const uint16_t angle[] = {0,90,180,270,360,450,540,630};
  assert_ignition_schedules(720U, 8U, angle);
#else
  const uint16_t angle[] = {0,90,180,270,0,0,0,0};
  assert_ignition_schedules(360U, 4U, angle);
#endif
}

static void cylinder8_stroke4_seq_even(void)
{
  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  configPage2.engineType = EVEN_FIRE;
  initialiseAll(); //Run the main initialise function
  assert_cylinder8_stroke4_seq_even();
}

static void cylinder8_stroke4_wasted_even(void)
{
  configPage4.sparkMode = IGN_MODE_WASTED;
  configPage2.engineType = EVEN_FIRE;
  initialiseAll(); //Run the main initialise function
  const uint16_t angle[] = {0,90,180,270,0,0,0,0};
  assert_ignition_schedules(360U, 4U, angle);
}  

static void run_8_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 8;
  configPage2.strokes = FOUR_STROKE;

  RUN_TEST_P(cylinder8_stroke4_seq_even);
  RUN_TEST_P(cylinder8_stroke4_wasted_even);
}

static void setupPartialSyncTest(uint8_t cylinders)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = cylinders;
  configPage2.strokes = FOUR_STROKE;
  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  configPage2.engineType = EVEN_FIRE;
  initialiseAll(); //Run the main initialise function
}

static void test_partial_sync_1_cylinder(void)
{
  setupPartialSyncTest(1);

  // Initial state
  assert_cylinder1_stroke4_seq_even();

  changeFullToHalfSync(configPage2, configPage4, currentStatus);
  {
    const uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_ignition_schedules(360U, 1U, angle);
  }

  changeHalfToFullSync(configPage2, configPage4, currentStatus);
  assert_cylinder1_stroke4_seq_even();
}

static void test_partial_sync_2_cylinder(void)
{
  setupPartialSyncTest(2);

  // Initial state
  assert_cylinder2_stroke4_seq_even();

  changeFullToHalfSync(configPage2, configPage4, currentStatus);
  {
    const uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_ignition_schedules(360U, 2U, angle);
  }

  changeHalfToFullSync(configPage2, configPage4, currentStatus);
  assert_cylinder2_stroke4_seq_even();
}

static void test_partial_sync_3_cylinder(void)
{
  setupPartialSyncTest(3);

  // Initial state
  assert_cylinder3_stroke4_seq_even();

  changeFullToHalfSync(configPage2, configPage4, currentStatus);
  {
    // TODO: This must be wrong!
    const uint16_t angle[] = {0,240,480,0,0,0,0,0};
    assert_ignition_schedules(360U, 3U, angle);
  }

  changeHalfToFullSync(configPage2, configPage4, currentStatus);
  assert_cylinder3_stroke4_seq_even();
}

static void test_partial_sync_4_cylinder(void)
{
  setupPartialSyncTest(4);

  // Initial state
  assert_cylinder4_stroke4_seq_even();

  changeFullToHalfSync(configPage2, configPage4, currentStatus);
  { 
    const uint16_t angle[] = {0,180,360,540,0,0,0,0};
    assert_ignition_schedules(360U, 2U, angle);
  }

  changeHalfToFullSync(configPage2, configPage4, currentStatus);
  assert_cylinder4_stroke4_seq_even();
}

static void test_partial_sync_5_cylinder(void)
{
  setupPartialSyncTest(5);

  // Initial state
  assert_cylinder5_stroke4_seq_even();

  changeFullToHalfSync(configPage2, configPage4, currentStatus);
  {
    // TODO: This must be wrong!
    const uint16_t angle[] = {0,144,288,432,576,0,0,0};
    assert_ignition_schedules(360U, 5U, angle);
  }

  changeHalfToFullSync(configPage2, configPage4, currentStatus);
  assert_cylinder5_stroke4_seq_even();
}

static void test_partial_sync_6_cylinder(void)
{
#if IGN_CHANNELS>=6
  setupPartialSyncTest(6);

  // Initial state
  assert_cylinder6_stroke4_seq_even();

  changeFullToHalfSync(configPage2, configPage4, currentStatus);
  {
    const uint16_t angle[] = {0,120,240,360,480,600,0,0};
    assert_ignition_schedules(360U, 3U, angle);
  }

  changeHalfToFullSync(configPage2, configPage4, currentStatus);
  assert_cylinder6_stroke4_seq_even();
#else
  TEST_IGNORE_MESSAGE("Skipping 6 cylinder partial sync test - not enough injectors");
#endif
}


static void test_partial_sync_8_cylinder(void)
{
#if IGN_CHANNELS>=8
  setupPartialSyncTest(8);

  // Initial state
  assert_cylinder8_stroke4_seq_even();

  changeFullToHalfSync(configPage2, configPage4, currentStatus);
  {
    const uint16_t angle[] = {0,90,180,270,360,450,540,630};
    assert_ignition_schedules(360U, 4U, angle);
  }

  changeHalfToFullSync(configPage2, configPage4, currentStatus);
  assert_cylinder8_stroke4_seq_even();
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

void testIgnitionScheduleInit()
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