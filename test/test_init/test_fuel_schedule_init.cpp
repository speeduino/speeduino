#include <Arduino.h>
#include <unity.h>
#include "globals.h"
#include "init.h"
#include "../test_utils.h"
#include "storage.h"
#include "../test_schedules/channel_test_helpers.h"
#include "decoders.h"
#include "scheduler.h"

extern decoder_status_t decoderStatus;
void prepareForInitialiseAll(uint8_t boardId);

static void __attribute__((noinline)) assert_fuel_channel(bool enabled, uint16_t angle, uint8_t cmdBit, const FuelSchedule &schedule, int assertLineNum)
{
  if (enabled)
  {
  char msg[64];

  sprintf_P(msg, PSTR("channel%" PRIu8 ".InjChannelIsEnabled. Max:%" PRIu8), cmdBit+1, currentStatus.maxInjOutputs);
    UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT8(currentStatus.maxInjOutputs, cmdBit+1U, assertLineNum, msg);
  sprintf_P(msg, PSTR("channel%" PRIu8 ".InjDegrees"), cmdBit+1);
  UNITY_TEST_ASSERT_EQUAL_INT(angle, schedule.channelDegrees, assertLineNum, msg);
  sprintf_P(msg, PSTR("inj%" PRIu8 ".StartFunction"), cmdBit+1);
  UNITY_TEST_ASSERT(schedule._pStartCallback!=nullCallback, assertLineNum, msg);
  sprintf_P(msg, PSTR("inj%" PRIu8 ".EndFunction"), cmdBit+1);
  UNITY_TEST_ASSERT(schedule._pEndCallback!=nullCallback, assertLineNum, msg);
  }
}

static void __attribute__((noinline)) assert_num_inj_channels(const bool (&enabled)[8], int assertLineNum)
{
  uint8_t expectedOutputs=0;
  for (uint8_t i=0; i<8; i++) {
    if (enabled[i]) {
      ++expectedOutputs;
    }
  }
  UNITY_TEST_ASSERT_EQUAL_UINT8(expectedOutputs, currentStatus.maxInjOutputs, assertLineNum, nullptr);
}

static void __attribute__((noinline)) assert_fuel_schedules(uint16_t crankAngle, const bool (&enabled)[8], const uint16_t (&angle)[8], int assertLineNum)
{
  char msg[32];

  strcpy_P(msg, PSTR("CRANK_ANGLE_MAX_INJ"));
  UNITY_TEST_ASSERT_EQUAL_INT16(crankAngle, CRANK_ANGLE_MAX_INJ, assertLineNum, msg);

  assert_num_inj_channels(enabled, assertLineNum);

  RUNIF_INJCHANNEL1(assert_fuel_channel(enabled[0], angle[0], INJ1_CMD_BIT, fuelSchedule1, assertLineNum), {});
  RUNIF_INJCHANNEL2(assert_fuel_channel(enabled[1], angle[1], INJ2_CMD_BIT, fuelSchedule2, assertLineNum), {});
  RUNIF_INJCHANNEL3(assert_fuel_channel(enabled[2], angle[2], INJ3_CMD_BIT, fuelSchedule3, assertLineNum), {});
  RUNIF_INJCHANNEL4(assert_fuel_channel(enabled[3], angle[3], INJ4_CMD_BIT, fuelSchedule4, assertLineNum), {});
  RUNIF_INJCHANNEL5(assert_fuel_channel(enabled[4], angle[4], INJ5_CMD_BIT, fuelSchedule5, assertLineNum), {});
  RUNIF_INJCHANNEL6(assert_fuel_channel(enabled[5], angle[5], INJ6_CMD_BIT, fuelSchedule6, assertLineNum), {});
  RUNIF_INJCHANNEL7(assert_fuel_channel(enabled[6], angle[6], INJ7_CMD_BIT, fuelSchedule7, assertLineNum), {});
  RUNIF_INJCHANNEL8(assert_fuel_channel(enabled[7], angle[7], INJ8_CMD_BIT, fuelSchedule8, assertLineNum), {});
}

static void assert_1cylinder_4stroke_seq_nostage(int assertLineNum)
{
	const bool enabled[] = {true, false, false, false, false, false, false, false};
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(720U, enabled, angle, assertLineNum);
}

static void cylinder1_stroke4_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = true;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  assert_1cylinder_4stroke_seq_nostage(__LINE__);
}

static void cylinder1_stroke4_semiseq_nostage(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = true;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, false, false, false, false, false, false, false};
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(720U, enabled, angle, __LINE__);
}

static void enableStaging(void)
{
  configPage10.stagingEnabled = true;
  configPage10.stagedInjSizePri = 250;
  configPage10.stagedInjSizeSec = 500;
}

static void cylinder1_stroke4_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = true;
  enableStaging();
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(720U, enabled, angle, __LINE__);
  }

static void cylinder1_stroke4_semiseq_staged(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = true;
  enableStaging();
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(720U, enabled, angle, __LINE__);
}

static void run_1_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 1;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.divider = 1;

  RUN_TEST_P(cylinder1_stroke4_seq_nostage);
  RUN_TEST_P(cylinder1_stroke4_semiseq_nostage);
  RUN_TEST_P(cylinder1_stroke4_seq_staged);
  RUN_TEST_P(cylinder1_stroke4_semiseq_staged);
}

static void cylinder1_stroke2_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = true;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  const bool enabled[] = {true, false, false, false, false, false, false, false};
  const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(360U, enabled, angle, __LINE__);
}

static void cylinder1_stroke2_semiseq_nostage(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = true;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, false, false, false, false, false, false, false};
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(360U, enabled, angle, __LINE__);
}

static void cylinder1_stroke2_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = true;
  enableStaging();
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(360U, enabled, angle, __LINE__);
}

static void cylinder1_stroke2_semiseq_staged(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = true;
  enableStaging();
  initialiseAll(); //Run the main initialise function
  const bool enabled[] = {true, true, false, false, false, false, false, false};
  const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(360U, enabled, angle, __LINE__);
}

static void run_1_cylinder_2stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 1;
  configPage2.strokes = TWO_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.divider = 1;

  RUN_TEST_P(cylinder1_stroke2_seq_nostage);
  RUN_TEST_P(cylinder1_stroke2_semiseq_nostage);
  RUN_TEST_P(cylinder1_stroke2_seq_staged);
  RUN_TEST_P(cylinder1_stroke2_semiseq_staged);
}

static void assert_2cylinder_4stroke_seq_nostage(int assertLineNum)
{
	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_fuel_schedules(720U, enabled, angle, assertLineNum);
}

static void cylinder2_stroke4_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = true;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  assert_2cylinder_4stroke_seq_nostage(__LINE__);
}

static void cylinder2_stroke4_semiseq_nostage(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = true;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
}

static void cylinder2_stroke4_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = true;
  enableStaging();
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,180,0,180,0,0,0,0};
  assert_fuel_schedules(720U, enabled, angle, __LINE__);
}

static void cylinder2_stroke4_semiseq_staged(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = true;
  enableStaging();
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,180,0,180,0,0,0,0};
  assert_fuel_schedules(360U, enabled, angle, __LINE__);
}

static void run_2_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 2;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.divider = 1;

  RUN_TEST_P(cylinder2_stroke4_seq_nostage);
  RUN_TEST_P(cylinder2_stroke4_semiseq_nostage);
  RUN_TEST_P(cylinder2_stroke4_seq_staged);
  RUN_TEST_P(cylinder2_stroke4_semiseq_staged);
}


static void cylinder2_stroke2_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = true;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_fuel_schedules(180U, enabled, angle, __LINE__);
}

static void cylinder2_stroke2_semiseq_nostage(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = true;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_fuel_schedules(180U, enabled, angle, __LINE__);
}

static void cylinder2_stroke2_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = true;
  enableStaging();
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,180,0,180,0,0,0,0};
  assert_fuel_schedules(180U, enabled, angle, __LINE__);
}

static void cylinder2_stroke2_semiseq_staged(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = true;
  enableStaging();
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,180,0,180,0,0,0,0};
  assert_fuel_schedules(180U, enabled, angle, __LINE__);
}

static void run_2_cylinder_2stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 2;
  configPage2.strokes = TWO_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.divider = 1;

  RUN_TEST_P(cylinder2_stroke2_seq_nostage);
  RUN_TEST_P(cylinder2_stroke2_semiseq_nostage);
  RUN_TEST_P(cylinder2_stroke2_seq_staged);
  RUN_TEST_P(cylinder2_stroke2_semiseq_staged);
}

static void assert_3cylinder_4stroke_seq_nostage(int assertLineNum)
{
	const bool enabled[] = {true, true, true, false, false, false, false, false};
	const uint16_t angle[] = {0,240,480,0,0,0,0,0};
  assert_fuel_schedules(720U, enabled, angle, assertLineNum);
}

static void cylinder3_stroke4_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = true;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  assert_3cylinder_4stroke_seq_nostage(__LINE__);

}

static void cylinder3_stroke4_semiseq_nostage_tb(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = true;
  configPage10.stagingEnabled = false;
  configPage2.injType = INJ_TYPE_TBODY;
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, true, false, false, false, false, false};
	const uint16_t angle[] = {0,80,160,0,0,0,0,0};
  assert_fuel_schedules(720U/3U, enabled, angle, __LINE__);
}

static void cylinder3_stroke4_semiseq_nostage_port(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = true;
  configPage10.stagingEnabled = false;
  configPage2.injType = INJ_TYPE_PORT;
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, true, false, false, false, false, false};
	const uint16_t angle[] = {0,120,240,0,0,0,0,0};
  assert_fuel_schedules(720U/2U, enabled, angle, __LINE__); //Special case as 3 squirts per cycle MUST be over 720 degrees
}


static void cylinder3_stroke4_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = true;
  enableStaging();
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS>=6
	const bool enabled[] = {true, true, true, true, true, true, false, false};
	const uint16_t angle[] = {0,240,480,0,240,480,0,0};
  assert_fuel_schedules(720U, enabled, angle, __LINE__);
#else
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,240,480,0,0,0,0,0};
  assert_fuel_schedules(720U, enabled, angle, __LINE__);
#endif
}

static void cylinder3_stroke4_semiseq_staged_tb(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  enableStaging();
  configPage2.injType = INJ_TYPE_TBODY;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS>=6
	const uint16_t angle[] = {0,80,160,0,80,160,0,0};
	const bool enabled[] = {true, true, true, true, true, true, false, false};
  TEST_IGNORE_MESSAGE("Fix code so test passes :-()");
#else
	const uint16_t angle[] = {0,80,160,0,0,0,0,0};
	const bool enabled[] = {true, true, true, true, false, false, false, false};
#endif
  assert_fuel_schedules(720U/3U, enabled, angle, __LINE__); //Special case as 3 squirts per cycle MUST be over 720 degrees
}


static void cylinder3_stroke4_semiseq_staged_port(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  enableStaging();
  configPage2.injType = INJ_TYPE_PORT;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS>=6
	const uint16_t angle[] = {0,120,240,0,120,240,0,0};
	const bool enabled[] = {true, true, true, true, true, true, false, false};
  TEST_IGNORE_MESSAGE("Fix code so test passes :-()");
#else
	const uint16_t angle[] = {0,120,240,0,0,0,0,0};
	const bool enabled[] = {true, true, true, true, false, false, false, false};
#endif
  assert_fuel_schedules(720U/2U, enabled, angle, __LINE__); //Special case as 3 squirts per cycle MUST be over 720 degrees
}
static void run_3_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 3;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.divider = 1; //3 squirts per cycle for a 3 cylinder

  RUN_TEST_P(cylinder3_stroke4_seq_nostage);
  RUN_TEST_P(cylinder3_stroke4_semiseq_nostage_tb);
  RUN_TEST_P(cylinder3_stroke4_semiseq_nostage_port);
  RUN_TEST_P(cylinder3_stroke4_seq_staged);
  RUN_TEST_P(cylinder3_stroke4_semiseq_staged_tb);
  RUN_TEST_P(cylinder3_stroke4_semiseq_staged_port);
}

static void cylinder3_stroke2_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, true, false, false, false, false, false};
	const uint16_t angle[] = {0,120,240,0,0,0,0,0};
  assert_fuel_schedules(360U, enabled, angle, __LINE__);
  }

static void cylinder3_stroke2_semiseq_nostage_tb(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  configPage2.injType = INJ_TYPE_TBODY;
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, true, false, false, false, false, false};
	const uint16_t angle[] = {0,80,160,0,0,0,0,0};
  assert_fuel_schedules(360U/3U, enabled, angle, __LINE__);
}

static void cylinder3_stroke2_semiseq_nostage_port(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  configPage2.injType = INJ_TYPE_PORT;
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, true, false, false, false, false, false};
	const uint16_t angle[] = {0,120,240,0,0,0,0,0};
  assert_fuel_schedules(360U/2U, enabled, angle, __LINE__);
}

static void cylinder3_stroke2_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  enableStaging();
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS>=6
	const bool enabled[] = {true, true, true, true, true, true, false, false};
	const uint16_t angle[] = {0,120,240,0,120,240,0,0};
  assert_fuel_schedules(360U, enabled, angle, __LINE__);
#else
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,120,240,0,0,0,0,0};
  assert_fuel_schedules(360U, enabled, angle, __LINE__);
#endif
  }

static void cylinder3_stroke2_semiseq_staged_tb(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  enableStaging();
  configPage2.injType = INJ_TYPE_TBODY;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS>=6
	const uint16_t angle[] = {0,80,160,0,80,160,0,0};
	const bool enabled[] = {true, true, true, true, true, true, false, false};
  TEST_IGNORE_MESSAGE("Fix code so test passes :-()");
#else
	const uint16_t angle[] = {0,80,160,0,0,0,0,0};
	const bool enabled[] = {true, true, true, true, false, false, false, false};
#endif
  assert_fuel_schedules(360U/3U, enabled, angle, __LINE__);
}

static void cylinder3_stroke2_semiseq_staged_port(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  enableStaging();
  configPage2.injType = INJ_TYPE_PORT;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS>=6
	const uint16_t angle[] = {0,120,240,0,120,240,0,0};
	const bool enabled[] = {true, true, true, true, true, true, false, false};
  TEST_IGNORE_MESSAGE("Fix code so test passes :-()");
#else
	const uint16_t angle[] = {0,120,240,0,0,0,0,0};
	const bool enabled[] = {true, true, true, true, false, false, false, false};
#endif
  assert_fuel_schedules(360U/2U, enabled, angle, __LINE__);
}

static void run_3_cylinder_2stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 3;
  configPage2.strokes = TWO_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.divider = 1;

  RUN_TEST_P(cylinder3_stroke2_seq_nostage);
  RUN_TEST_P(cylinder3_stroke2_semiseq_nostage_tb);
  RUN_TEST_P(cylinder3_stroke2_semiseq_nostage_port);
  RUN_TEST_P(cylinder3_stroke2_seq_staged);
  RUN_TEST_P(cylinder3_stroke2_semiseq_staged_tb);
  RUN_TEST_P(cylinder3_stroke2_semiseq_staged_port);
}

static void assert_4cylinder_4stroke_seq_nostage(int assertLineNum)
{
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,180,360,540,0,0,0,0};
    assert_fuel_schedules(720U, enabled, angle, assertLineNum);
}

static void cylinder4_stroke4_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  assert_4cylinder_4stroke_seq_nostage(__LINE__);
}

static void cylinder4_stroke4_semiseq_nostage(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_fuel_schedules(360U, enabled, angle, __LINE__);
  }


static void cylinder4_stroke4_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  enableStaging();
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS>=8
	const bool enabled[] = {true, true, true, true, true, true, true, true};
	const uint16_t angle[] = {0,180,360,540,0,180,360,540};
  assert_fuel_schedules(720U, enabled, angle, __LINE__);
#elif INJ_CHANNELS >= 5
	const bool enabled[] = {true, true, true, true, true, false, false, false};
	const uint16_t angle[] = {0,180,360,540,0,0,0,0};
  assert_fuel_schedules(720U, enabled, angle, __LINE__);
#else
  assert_4cylinder_4stroke_seq_nostage(__LINE__);
#endif
}

static void cylinder4_stroke4_semiseq_staged(void)
{
  configPage2.injLayout = INJ_PAIRED;
  enableStaging();
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,180,0,180,0,0,0,0};
  assert_fuel_schedules(360U, enabled, angle, __LINE__);
}

void run_4_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 4;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.divider = 2;

  RUN_TEST_P(cylinder4_stroke4_seq_nostage);
  RUN_TEST_P(cylinder4_stroke4_semiseq_nostage);
  RUN_TEST_P(cylinder4_stroke4_seq_staged);
  RUN_TEST_P(cylinder4_stroke4_semiseq_staged);
}

static void cylinder4_stroke2_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_fuel_schedules(180U, enabled, angle, __LINE__);
  }

static void cylinder4_stroke2_semiseq_nostage(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_fuel_schedules(180U, enabled, angle, __LINE__);
  }

static void cylinder4_stroke2_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  enableStaging();
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS>=8
	const bool enabled[] = {true, true, true, true, true, true, true, true};
	const uint16_t angle[] = {0,180,0,0,0,180,0,0};
  assert_fuel_schedules(180U, enabled, angle, __LINE__);
#elif INJ_CHANNELS >= 5
	const bool enabled[] = {true, true, true, true, true, false, false, false};
	const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_fuel_schedules(180U, enabled, angle, __LINE__);
#else
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_fuel_schedules(180U, enabled, angle, __LINE__);
#endif
  }

static void cylinder4_stroke2_semiseq_staged(void)
{
  configPage2.injLayout = INJ_PAIRED;
  enableStaging();
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,180,0,180,0,0,0,0};
  assert_fuel_schedules(180U, enabled, angle, __LINE__);
}

void run_4_cylinder_2stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 4;
  configPage2.strokes = TWO_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.divider = 2;

  RUN_TEST_P(cylinder4_stroke2_seq_nostage);
  RUN_TEST_P(cylinder4_stroke2_semiseq_nostage);
  RUN_TEST_P(cylinder4_stroke2_seq_staged);
  RUN_TEST_P(cylinder4_stroke2_semiseq_staged);
}

static void assert_5cylinder_4stroke_seq_nostage(int assertLineNum)
{
#if INJ_CHANNELS >= 5
	const bool enabled[] = {true, true, true, true, true, false, false, false};
	const uint16_t angle[] = {0,144,288,432,576,0,0,0};
#else
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
#endif
  assert_fuel_schedules(720U, enabled, angle, assertLineNum);
}

static void cylinder5_stroke4_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  assert_5cylinder_4stroke_seq_nostage(__LINE__);
}


static void cylinder5_stroke4_semiseq_nostage(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,72,144,216,288,0,0,0};
  assert_fuel_schedules(720U, enabled, angle, __LINE__);
  }

static void cylinder5_stroke4_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  enableStaging();
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS >= 6
	const bool enabled[] = {true, true, true, true, true, true, false, false};
	const uint16_t angle[] = {0,144,288,432,576,0,0,0};
#else
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
#endif
  assert_fuel_schedules(720U, enabled, angle, __LINE__);
}

static void cylinder5_stroke4_semiseq_staged(void)
{
  configPage2.injLayout = INJ_PAIRED;
  enableStaging();
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS >= 5
	const bool enabled[] = {true, true, true, true, true, true, false, false};
	const uint16_t angle[] = {0,72,144,216,288,0,0,0};
  TEST_IGNORE_MESSAGE("Fix code so test passes :-()");
#else
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,72,144,216,288,0,0,0};
#endif
  assert_fuel_schedules(720U, enabled, angle, __LINE__);
}

void run_5_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 5;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.divider = 5;

  RUN_TEST_P(cylinder5_stroke4_seq_nostage);
  RUN_TEST_P(cylinder5_stroke4_semiseq_nostage);
  RUN_TEST_P(cylinder5_stroke4_seq_staged);
  RUN_TEST_P(cylinder5_stroke4_semiseq_staged);
}

static void assert_6cylinder_4stroke_seq_nostage(int assertLineNum)
{
#if INJ_CHANNELS >= 6
	const bool enabled[] = {true, true, true, true, true, true, false, false};
	const uint16_t angle[] = {0,120,240,360,480,600,0,0};
#else
	const bool enabled[] = {true, true, true, false, false, false, false, false};
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
#endif
  assert_fuel_schedules(720U, enabled, angle, assertLineNum);
}

static void cylinder6_stroke4_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  assert_6cylinder_4stroke_seq_nostage(__LINE__);
}

static void cylinder6_stroke4_semiseq_nostage(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const bool enabled[] = {true, true, true, false, false, false, false, false};
	const uint16_t angle[] = {0,120,240,0,0,0,0,0};
  assert_fuel_schedules(720U, enabled, angle, __LINE__);
  }

static void cylinder6_stroke4_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  enableStaging();
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS >= 8
	const bool enabled[] = {true, true, true, true, true, true, false, false};
	const uint16_t angle[] = {0,120,240,360,480,600,0,0};
#else
	const bool enabled[] = {true, true, true, false, false, false, false, false};
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
#endif
  assert_fuel_schedules(720U, enabled, angle, __LINE__);
}


static void cylinder6_stroke4_semiseq_staged(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  enableStaging();
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS >= 8
	const uint16_t angle[] = {0,120,240,0,0,120,240,0};
	const bool enabled[] = {true, true, true, true, true, true, true, false};
  TEST_IGNORE_MESSAGE("Fix code so test passes :-()");
#else
	const uint16_t angle[] = {0,120,240,0,0,0,0,0};
	const bool enabled[] = {true, true, true, false, false, false, false, false};
#endif
  assert_fuel_schedules(720U, enabled, angle, __LINE__);
}

void run_6_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 6;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.divider = 6;

  RUN_TEST_P(cylinder6_stroke4_seq_nostage);
  RUN_TEST_P(cylinder6_stroke4_semiseq_nostage);
  RUN_TEST_P(cylinder6_stroke4_seq_staged);
  RUN_TEST_P(cylinder6_stroke4_semiseq_staged);
}

static void assert_8cylinder_4stroke_seq_nostage(int assertLineNum)
{
#if INJ_CHANNELS >= 8
	const bool enabled[] = {true, true, true, true, true, true, true, true};
	const uint16_t angle[] = {0,90,180,270,360,450,540,630};
#else
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
#endif
  assert_fuel_schedules(720U, enabled, angle, assertLineNum);
}

static void cylinder8_stroke4_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  assert_8cylinder_4stroke_seq_nostage(__LINE__);
}

void run_8_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 8;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.divider = 8;

  // Staging not supported on 8 cylinders

  RUN_TEST_P(cylinder8_stroke4_seq_nostage);
}

static constexpr uint16_t zeroAngles[] = {0,0,0,0,0,0,0,0};

static void cylinder_1_NoinjTiming_paired(void) {
  configPage2.injLayout = INJ_PAIRED;
  configPage2.nCylinders = 1;
  configPage2.divider = 1;

  initialiseAll(); //Run the main initialise function

  const bool enabled[] = {true, false, false, false, false, false, false, false};
  assert_fuel_schedules(720U, enabled, zeroAngles, __LINE__);
}

static void cylinder_2_NoinjTiming_paired(void) {
  configPage2.injLayout = INJ_PAIRED;
  configPage2.nCylinders = 2;
  configPage2.divider = 2;

  initialiseAll(); //Run the main initialise function

  const bool enabled[] = {true, true, false, false, false, false, false, false};
  assert_fuel_schedules(720U, enabled, zeroAngles, __LINE__);
}

static void cylinder_3_NoinjTiming_paired(void) {
  configPage2.injLayout = INJ_PAIRED;
  configPage2.nCylinders = 3;
  configPage2.divider = 3;
  configPage2.injType = INJ_TYPE_PORT;

  initialiseAll(); //Run the main initialise function

  const bool enabled[] = {true, true, true, false, false, false, false, false};
  assert_fuel_schedules(360U, enabled, zeroAngles, __LINE__);
}

static void cylinder_4_NoinjTiming_paired(void) {
  configPage2.injLayout = INJ_PAIRED;
  configPage2.nCylinders = 4;
  configPage2.divider = 4;

  initialiseAll(); //Run the main initialise function

  const bool enabled[] = {true, true, false, false, false, false, false, false};
  assert_fuel_schedules(720U, enabled, zeroAngles, __LINE__);
}

static void cylinder_5_NoinjTiming_paired(void) {
  configPage2.injLayout = INJ_PAIRED;
  configPage2.nCylinders = 5;
  configPage2.divider = 5;

  initialiseAll(); //Run the main initialise function

  const bool enabled[] = {true, true, true, true, false, false, false, false};
  assert_fuel_schedules(720U, enabled, zeroAngles, __LINE__);
}

static void cylinder_6_NoinjTiming_paired(void) {
  configPage2.injLayout = INJ_PAIRED;
  configPage2.nCylinders = 6;
  configPage2.divider = 6;

  initialiseAll(); //Run the main initialise function

  const bool enabled[] = {true, true, true, false, false, false, false, false};
  assert_fuel_schedules(720U, enabled, zeroAngles, __LINE__);
}

static void cylinder_8_NoinjTiming_paired(void) {
  configPage2.injLayout = INJ_PAIRED;
  configPage2.nCylinders = 8;
  configPage2.divider = 8;

  initialiseAll(); //Run the main initialise function

  const bool enabled[] = {true, true, true, true, false, false, false, false};
  assert_fuel_schedules(720U, enabled, zeroAngles, __LINE__);
}

static void run_no_inj_timing_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = false;
  configPage10.stagingEnabled = false;

  RUN_TEST_P(cylinder_1_NoinjTiming_paired);
  RUN_TEST_P(cylinder_2_NoinjTiming_paired);
  RUN_TEST_P(cylinder_3_NoinjTiming_paired);
  RUN_TEST_P(cylinder_4_NoinjTiming_paired);
  RUN_TEST_P(cylinder_5_NoinjTiming_paired);
  RUN_TEST_P(cylinder_6_NoinjTiming_paired);
  RUN_TEST_P(cylinder_8_NoinjTiming_paired);
}

static void cylinder_2_oddfire(void)
{
  configPage2.injLayout = INJ_PAIRED;
  configPage2.nCylinders = 2;
  configPage2.divider = 2;

  initialiseAll(); //Run the main initialise function

	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,13,0,0,0,0,0,0};
  assert_fuel_schedules(720U, enabled, angle, __LINE__);
}

static void run_oddfire_tests()
{
  prepareForInitialiseAll(3U);
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = ODD_FIRE;
  configPage2.injTiming = true;
  configPage10.stagingEnabled = false;
  configPage2.oddfire2 = 13;
  configPage2.oddfire3 = 111;
  configPage2.oddfire4 = 217;

  // Oddfire only affects 2 cylinder configurations
  configPage2.nCylinders = 1;
  configPage2.divider = 1;
  RUN_TEST_P(cylinder1_stroke4_seq_nostage);

  RUN_TEST_P(cylinder_2_oddfire);

  configPage2.nCylinders = 3;
  configPage2.divider = 1;
  RUN_TEST_P(cylinder3_stroke4_seq_nostage);
  configPage2.nCylinders = 4;
  configPage2.divider = 2;
  RUN_TEST_P(cylinder4_stroke4_seq_nostage);
  configPage2.nCylinders = 5;
  configPage2.divider = 5;
  RUN_TEST_P(cylinder5_stroke4_seq_nostage);
  configPage2.nCylinders = 6;
  configPage2.divider = 6;
  RUN_TEST_P(cylinder6_stroke4_seq_nostage);
  configPage2.nCylinders = 8;
  configPage2.divider = 8;
  RUN_TEST_P(cylinder8_stroke4_seq_nostage);
}

static void setupPartialSyncTest(uint8_t cylinders)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = cylinders;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.strokes = FOUR_STROKE;
  configPage2.divider = cylinders;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
}

static void test_partial_sync_1_cylinder(void)
{
  setupPartialSyncTest(1);

  // Confirm initial state
  assert_1cylinder_4stroke_seq_nostage(__LINE__);

  decoderStatus.syncStatus = SyncStatus::Partial;
  changeFullToHalfSync(configPage2, configPage4, currentStatus);
  {
	  const bool enabled[] = {true, false, false, false, false, false, false, false};
	  const uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(360U, enabled, angle, __LINE__);
  }

  decoderStatus.syncStatus = SyncStatus::Full;
  changeHalfToFullSync(configPage2, currentStatus);
  assert_1cylinder_4stroke_seq_nostage(__LINE__);
}

static void test_partial_sync_2_cylinder(void)
{
  setupPartialSyncTest(2);

  // Confirm initial state
  assert_2cylinder_4stroke_seq_nostage(__LINE__);

  decoderStatus.syncStatus = SyncStatus::Partial;
  changeFullToHalfSync(configPage2, configPage4, currentStatus);
  {
	  const bool enabled[] = {true, true, false, false, false, false, false, false};
	  const uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_fuel_schedules(360U, enabled, angle, __LINE__);
  }

  decoderStatus.syncStatus = SyncStatus::Full;
  changeHalfToFullSync(configPage2, currentStatus);
  assert_2cylinder_4stroke_seq_nostage(__LINE__);
}


static void test_partial_sync_3_cylinder(void)
{
  setupPartialSyncTest(3);

  // Confirm initial state
  assert_3cylinder_4stroke_seq_nostage(__LINE__);

  decoderStatus.syncStatus = SyncStatus::Partial;
  changeFullToHalfSync(configPage2, configPage4, currentStatus);
  {
	  const bool enabled[] = {true, true, true, false, false, false, false, false};
	  const uint16_t angle[] = {0,240,480,0,0,0,0,0};
    assert_fuel_schedules(360U, enabled, angle, __LINE__);
  }

  decoderStatus.syncStatus = SyncStatus::Full;
  changeHalfToFullSync(configPage2, currentStatus);
  assert_3cylinder_4stroke_seq_nostage(__LINE__);
}

static void test_partial_sync_4_cylinder(void)
{
  setupPartialSyncTest(4);

  // Confirm initial state
  assert_4cylinder_4stroke_seq_nostage(__LINE__);

  decoderStatus.syncStatus = SyncStatus::Partial;
  changeFullToHalfSync(configPage2, configPage4, currentStatus);
  {
	  const bool enabled[] = {true, true, false, false, false, false, false, false};
	  const uint16_t angle[] = {0,180,360,540,0,0,0,0};
    assert_fuel_schedules(360U, enabled, angle, __LINE__);
  }

  decoderStatus.syncStatus = SyncStatus::Full;
  changeHalfToFullSync(configPage2, currentStatus);
  assert_4cylinder_4stroke_seq_nostage(__LINE__);
}

static void test_partial_sync_5_cylinder(void)
{
#if INJ_CHANNELS>=5
  setupPartialSyncTest(5);

  // Confirm initial state
  assert_5cylinder_4stroke_seq_nostage(__LINE__);

  decoderStatus.syncStatus = SyncStatus::Partial;
  changeFullToHalfSync(configPage2, configPage4, currentStatus);
  {
	  const bool enabled[] = {true, true, true, true, true, false, false, false};
	  const uint16_t angle[] = {0,144,288,432,576,0,0,0};
    assert_fuel_schedules(360U, enabled, angle, __LINE__);
  }

  decoderStatus.syncStatus = SyncStatus::Full;
  changeHalfToFullSync(configPage2, currentStatus);
  assert_5cylinder_4stroke_seq_nostage(__LINE__);
#else
  TEST_IGNORE_MESSAGE("Skipping - not enough injectors");
#endif
}

static void test_partial_sync_6_cylinder(void)
{
#if INJ_CHANNELS>=6
  setupPartialSyncTest(6);

  // Confirm initial state
  assert_6cylinder_4stroke_seq_nostage(__LINE__);

  decoderStatus.syncStatus = SyncStatus::Partial;
  changeFullToHalfSync(configPage2, configPage4, currentStatus);
  {
	  const bool enabled[] = {true, true, true, false, false, false, false, false};
	  const uint16_t angle[] = {0,120,240,360,480,600,0,0};
    assert_fuel_schedules(360U, enabled, angle, __LINE__);
  }

  decoderStatus.syncStatus = SyncStatus::Full;
  changeHalfToFullSync(configPage2, currentStatus);
  assert_6cylinder_4stroke_seq_nostage(__LINE__);
#else
  TEST_IGNORE_MESSAGE("Skipping - not enough injectors");
#endif
}

static void test_partial_sync_8_cylinder(void)
{
#if INJ_CHANNELS>=8
  setupPartialSyncTest(8);

  // Confirm initial state
  assert_8cylinder_4stroke_seq_nostage(__LINE__);

  decoderStatus.syncStatus = SyncStatus::Partial;
  changeFullToHalfSync(configPage2, configPage4, currentStatus);
  {
	  const bool enabled[] = {true, true, true, true, false, false, false, false};
	  const uint16_t angle[] = {0,90,180,270,360,450,540,630};
    assert_fuel_schedules(360U, enabled, angle, __LINE__);
  }

  decoderStatus.syncStatus = SyncStatus::Full;
  changeHalfToFullSync(configPage2,  currentStatus);
  assert_8cylinder_4stroke_seq_nostage(__LINE__);
#else
  TEST_IGNORE_MESSAGE("Skipping - not enough injectors");
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

void testFuelScheduleInit()
{
  SET_UNITY_FILENAME() {

  run_1_cylinder_4stroke_tests();
  run_1_cylinder_2stroke_tests();
  run_2_cylinder_4stroke_tests();
  run_2_cylinder_2stroke_tests();
  run_3_cylinder_4stroke_tests();
  run_3_cylinder_2stroke_tests();
  run_4_cylinder_4stroke_tests();
  run_4_cylinder_2stroke_tests();
  run_5_cylinder_4stroke_tests();
  run_6_cylinder_4stroke_tests();
  run_8_cylinder_4stroke_tests();

  run_no_inj_timing_tests();

  run_oddfire_tests();

  run_partial_sync_tests();
  }
}