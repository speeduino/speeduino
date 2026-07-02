#include <unity.h>
#include "../test_utils.h"
#include "../channel_test_helpers.h"
#include "../fake_decoder_status.h"
#include "decoder_builder.h"
#include "scheduler_fuel_controller.h"
#include "src/pins/pinNumbers_t.h"

extern decoder_status_t fakeDecoderStatus;
extern void matchFuelSchedulersToSyncState(const config2 &page2, const config4 &page4, statuses &current);

static void __attribute__((noinline)) assert_fuel_channel(const statuses &current, bool enabled, uint16_t angle, uint8_t cmdBit, const FuelSchedule &schedule, int assertLineNum)
{
  if (enabled)
  {
    char msg[64];

    sprintf_P(msg, PSTR("channel%" PRIu8 ".InjChannelIsEnabled. Max:%" PRIu8), cmdBit+1, current.injOutputs.getTotalInjectors());
    UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT8(current.injOutputs.getTotalInjectors(), cmdBit+1U, assertLineNum, msg);
    sprintf_P(msg, PSTR("channel%" PRIu8 ".InjDegrees"), cmdBit+1);
    UNITY_TEST_ASSERT_EQUAL_INT(angle, schedule.channelDegrees, assertLineNum, msg);
    sprintf_P(msg, PSTR("inj%" PRIu8 ".StartFunction"), cmdBit+1);
    UNITY_TEST_ASSERT(schedule._pStartCallback!=nullCallback, assertLineNum, msg);
    sprintf_P(msg, PSTR("inj%" PRIu8 ".EndFunction"), cmdBit+1);
    UNITY_TEST_ASSERT(schedule._pEndCallback!=nullCallback, assertLineNum, msg);
    sprintf_P(msg, PSTR("injAngle"));
    UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT16(CRANK_ANGLE_MAX_INJ, angle, assertLineNum, msg);
  }
}

static void __attribute__((noinline)) assert_num_inj_channels(const statuses &current, const bool (&enabled)[8], int assertLineNum)
{
  uint8_t expectedOutputs=0;
  for (uint8_t i=0; i<8; i++) {
    if (enabled[i]) {
      ++expectedOutputs;
    }
  }
  UNITY_TEST_ASSERT_EQUAL_UINT8(expectedOutputs, current.injOutputs.getTotalInjectors(), assertLineNum, nullptr);
}

static void __attribute__((noinline)) assert_fuel_schedules(const statuses &current, uint16_t crankAngle, const bool (&enabled)[8], const uint16_t (&angle)[8], int assertLineNum)
{
  char msg[32];

  strcpy_P(msg, PSTR("CRANK_ANGLE_MAX_INJ"));
  UNITY_TEST_ASSERT_EQUAL_INT16(crankAngle, CRANK_ANGLE_MAX_INJ, assertLineNum, msg);

  assert_num_inj_channels(current, enabled, assertLineNum);

  RUNIF_INJCHANNEL1(assert_fuel_channel(current, enabled[0], angle[0], fuelSchedule1, assertLineNum), {});
  RUNIF_INJCHANNEL2(assert_fuel_channel(current, enabled[1], angle[1], fuelSchedule2, assertLineNum), {});
  RUNIF_INJCHANNEL3(assert_fuel_channel(current, enabled[2], angle[2], fuelSchedule3, assertLineNum), {});
  RUNIF_INJCHANNEL4(assert_fuel_channel(current, enabled[3], angle[3], fuelSchedule4, assertLineNum), {});
  RUNIF_INJCHANNEL5(assert_fuel_channel(current, enabled[4], angle[4], fuelSchedule5, assertLineNum), {});
  RUNIF_INJCHANNEL6(assert_fuel_channel(current, enabled[5], angle[5], fuelSchedule6, assertLineNum), {});
  RUNIF_INJCHANNEL7(assert_fuel_channel(current, enabled[6], angle[6], fuelSchedule7, assertLineNum), {});
  RUNIF_INJCHANNEL8(assert_fuel_channel(current, enabled[7], angle[7], fuelSchedule8, assertLineNum), {});
}

struct init_context_t
{
  statuses current = {};
  config2 page2 = {};
  config4 page4 = {};
  config10 page10 = {};
  pinNumbers_t pins = {};

  init_context_t(void)
  {
    for (uint8_t channel=0; channel<INJ_CHANNELS; ++channel)
    {
      pins.setInjectorPin(channel, 11+channel);
    }
  }

  void initialise(void)
  {
    initialiseFuelSchedules(current, page2, page4, page10, pins);
  }
};

static void assert_injlayout(uint8_t layout, const init_context_t &context)
{
  TEST_ASSERT_EQUAL(layout, context.current.injLayout);
  TEST_ASSERT_EQUAL(layout, context.page2.injLayout);
}

static void assert_1channel_0stage_over720(int assertLineNum, const init_context_t &context)
{
	const bool enabled[] = {true, false, false, false, false, false, false, false};
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(context.current, 720U, enabled, angle, assertLineNum);
  TEST_ASSERT_FALSE(context.page10.stagingEnabled);
}

static void assert_1channel_1stage_over720(int assertLineNum, const init_context_t &context)
{
	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(context.current, 720U, enabled, angle, assertLineNum);
  TEST_ASSERT_TRUE(context.page10.stagingEnabled);
}

static void enableStaging(init_context_t &context)
{
  context.page10.stagingEnabled = true;
  context.page10.stagedInjSizePri = 250;
  context.page10.stagedInjSizeSec = 500;
}

static init_context_t setup1_cylinder_4stroke(void)
{
  init_context_t context;
  context.page2.nCylinders = 1;
  context.page2.strokes = FOUR_STROKE;
  context.page2.engineType = EVEN_FIRE;
  context.page2.divider = 1;
  context.page2.injTiming = true;
  return context;
}

static void cylinder1_stroke4_seq_nostage(void)
{
  auto context = setup1_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page10.stagingEnabled = false;

  context.initialise();
  
  assert_1channel_0stage_over720(__LINE__, context);
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void cylinder1_stroke4_semiseq_nostage(void)
{
  auto context = setup1_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page10.stagingEnabled = false;

  context.initialise();
  
  assert_1channel_0stage_over720(__LINE__, context);
  assert_injlayout(INJ_SEMISEQUENTIAL, context);
}

static void cylinder1_stroke4_seq_staged(void)
{
  auto context = setup1_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page2.injTiming = true;
  enableStaging(context);
  
  context.initialise();
  
  assert_1channel_1stage_over720(__LINE__, context);
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void cylinder1_stroke4_semiseq_staged(void)
{
  auto context = setup1_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page2.injTiming = true;
  enableStaging(context);

  context.initialise();
  
  assert_1channel_1stage_over720(__LINE__, context);
  assert_injlayout(INJ_SEMISEQUENTIAL, context);
}

static void run_1_cylinder_4stroke_tests(void)
{
  RUN_TEST_P(cylinder1_stroke4_seq_nostage);
  RUN_TEST_P(cylinder1_stroke4_semiseq_nostage);
  RUN_TEST_P(cylinder1_stroke4_seq_staged);
  RUN_TEST_P(cylinder1_stroke4_semiseq_staged);
}

static void assert_1channel_0stage_over360(int assertLineNum, const init_context_t &context)
{
	const bool enabled[] = {true, false, false, false, false, false, false, false};
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(context.current, 360U, enabled, angle, assertLineNum);
  TEST_ASSERT_FALSE(context.page10.stagingEnabled);
}

static void assert_1channel_1stage_over360(int assertLineNum, const init_context_t &context)
{
	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(context.current, 360U, enabled, angle, assertLineNum);
  TEST_ASSERT_TRUE(context.page10.stagingEnabled);
}

static init_context_t setup1_cylinder_2stroke(void)
{
  init_context_t context;
  context.page2.nCylinders = 1;
  context.page2.strokes = TWO_STROKE;
  context.page2.engineType = EVEN_FIRE;
  context.page2.divider = 1;
  context.page2.injTiming = true;
  return context;
}

static void cylinder1_stroke2_seq_nostage(void)
{
  auto context = setup1_cylinder_2stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page10.stagingEnabled = false;

  context.initialise();
  
  assert_1channel_0stage_over360(__LINE__, context);
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void cylinder1_stroke2_semiseq_nostage(void)
{
  auto context = setup1_cylinder_2stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page10.stagingEnabled = false;

  context.initialise();
  
  assert_1channel_0stage_over360(__LINE__, context);
  assert_injlayout(INJ_SEMISEQUENTIAL, context);
}

static void cylinder1_stroke2_seq_staged(void)
{
  auto context = setup1_cylinder_2stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  enableStaging(context);

  context.initialise();
	
  assert_1channel_1stage_over360(__LINE__, context);
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void cylinder1_stroke2_semiseq_staged(void)
{
  auto context = setup1_cylinder_2stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  enableStaging(context);

  context.initialise();

  assert_1channel_1stage_over360(__LINE__, context);
  assert_injlayout(INJ_SEMISEQUENTIAL, context);
}

static void run_1_cylinder_2stroke_tests(void)
{
  RUN_TEST_P(cylinder1_stroke2_seq_nostage);
  RUN_TEST_P(cylinder1_stroke2_semiseq_nostage);
  RUN_TEST_P(cylinder1_stroke2_seq_staged);
  RUN_TEST_P(cylinder1_stroke2_semiseq_staged);
}

static init_context_t setup2_cylinder_4stroke(void)
{
  init_context_t context;
  context.page2.nCylinders = 2;
  context.page2.strokes = FOUR_STROKE;
  context.page2.engineType = EVEN_FIRE;
  context.page2.divider = 1;
  context.page2.injTiming = true;
  return context;
}

static void assert_2channel_0stage_over720(int assertLineNum, const init_context_t &context)
{
	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_fuel_schedules(context.current, 720U, enabled, angle, assertLineNum);
  TEST_ASSERT_FALSE(context.page10.stagingEnabled);
}

static void cylinder2_stroke4_seq_nostage(void)
{
  auto context = setup2_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page10.stagingEnabled = false;

  context.initialise();

  assert_2channel_0stage_over720(__LINE__, context);
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void assert_2chennel_0stage_over360(int assertLineNum, const init_context_t &context)
{
	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_fuel_schedules(context.current, 360U, enabled, angle, assertLineNum);
  TEST_ASSERT_FALSE(context.page10.stagingEnabled);
}

static void cylinder2_stroke4_semiseq_nostage(void)
{
  auto context = setup2_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page10.stagingEnabled = false;

  context.initialise();

  assert_2chennel_0stage_over360(__LINE__, context);
  assert_injlayout(INJ_SEMISEQUENTIAL, context);
}

static void assert_2channel_2stage_over720(int assertLineNum, const init_context_t &context)
{
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,180,0,360,0,0,0,0};
  assert_fuel_schedules(context.current, 720U, enabled, angle, assertLineNum);
  TEST_ASSERT_TRUE(context.page10.stagingEnabled);
}

static void cylinder2_stroke4_seq_staged(void)
{
  auto context = setup2_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  enableStaging(context);

  context.initialise();

  assert_2channel_2stage_over720(__LINE__, context);
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void assert_2channel_2staged_over360(int assertLineNum, const init_context_t &context)
{
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,180,0,180,0,0,0,0};
  assert_fuel_schedules(context.current, 360U, enabled, angle, assertLineNum);
  TEST_ASSERT_TRUE(context.page10.stagingEnabled);
}

static void cylinder2_stroke4_semiseq_staged(void)
{
  auto context = setup2_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  enableStaging(context);

  context.initialise();
  
  assert_2channel_2staged_over360(__LINE__, context);
  assert_injlayout(INJ_SEMISEQUENTIAL, context);
}

static void run_2_cylinder_4stroke_tests(void)
{
  RUN_TEST_P(cylinder2_stroke4_seq_nostage);
  RUN_TEST_P(cylinder2_stroke4_semiseq_nostage);
  RUN_TEST_P(cylinder2_stroke4_seq_staged);
  RUN_TEST_P(cylinder2_stroke4_semiseq_staged);
}

static init_context_t setup_2_cylinder_2stroke(void)
{
  init_context_t context;
  context.page2.nCylinders = 2;
  context.page2.strokes = TWO_STROKE;
  context.page2.engineType = EVEN_FIRE;
  context.page2.divider = 1;
  context.page2.injTiming = true;
  return context;
} 

static void assert_2channel_0stage_over180(int assertLineNum, const init_context_t &context)
{
	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(context.current, 180U, enabled, angle, __LINE__);
  TEST_ASSERT_FALSE(context.page10.stagingEnabled);
}

static void cylinder2_stroke2_seq_nostage(void)
{
  auto context = setup_2_cylinder_2stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page10.stagingEnabled = false;

  context.initialise();
  
  assert_2channel_0stage_over180(__LINE__, context);
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void cylinder2_stroke2_semiseq_nostage(void)
{
  auto context = setup_2_cylinder_2stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page10.stagingEnabled = false;

  context.initialise();
  
  assert_2channel_0stage_over180(__LINE__, context);
  assert_injlayout(INJ_SEMISEQUENTIAL, context);
}

static void assert_2channel_2stage_over180(int assertLineNum, const init_context_t &context)
{
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,0,0,90,0,0,0,0};
  assert_fuel_schedules(context.current, 180U, enabled, angle, __LINE__);
  TEST_ASSERT_TRUE(context.page10.stagingEnabled);
}

static void cylinder2_stroke2_seq_staged(void)
{
  auto context = setup_2_cylinder_2stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  enableStaging(context);
  
  context.initialise();
	  
  assert_2channel_2stage_over180(__LINE__, context);
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void cylinder2_stroke2_semiseq_staged(void)
{
  auto context = setup_2_cylinder_2stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  enableStaging(context);
 
  context.initialise();
  
  assert_2channel_2stage_over180(__LINE__, context);
  assert_injlayout(INJ_SEMISEQUENTIAL, context);
}

static void run_2_cylinder_2stroke_tests(void)
{
  RUN_TEST_P(cylinder2_stroke2_seq_nostage);
  RUN_TEST_P(cylinder2_stroke2_semiseq_nostage);
  RUN_TEST_P(cylinder2_stroke2_seq_staged);
  RUN_TEST_P(cylinder2_stroke2_semiseq_staged);
}

static init_context_t setup_3_cylinder_4stroke(void)
{
  init_context_t context;
  context.page2.nCylinders = 3;
  context.page2.strokes = FOUR_STROKE;
  context.page2.engineType = EVEN_FIRE;
  context.page2.divider = 1; //3 squirts per cycle for a 3 cylinder
  context.page2.injTiming = true;
  return context;
}

static void assert_3cylinder_4stroke_seq_nostage(int assertLineNum, const init_context_t &context)
{
	const bool enabled[] = {true, INJ_CHANNELS>=2, INJ_CHANNELS>=3, false, false, false, false, false};
	const uint16_t angle[] = {0,240,480,0,0,0,0,0};
  assert_fuel_schedules(context.current, 720U, enabled, angle, assertLineNum);
  TEST_ASSERT_EQUAL(INJ_SEQUENTIAL, context.current.injLayout);
  TEST_ASSERT_FALSE(context.page10.stagingEnabled);
}

static void cylinder3_stroke4_seq_nostage(void)
{
  auto context = setup_3_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page2.injTiming = true;
  context.page10.stagingEnabled = false;
  context.initialise();
  assert_3cylinder_4stroke_seq_nostage(__LINE__, context);
}

static void cylinder3_stroke4_semiseq_nostage_tb(void)
{
  auto context = setup_3_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page2.injTiming = true;
  context.page10.stagingEnabled = false;
  context.page2.injType = INJ_TYPE_TBODY;
  context.initialise();
	const bool enabled[] = {true, INJ_CHANNELS>=2, INJ_CHANNELS>=3, false, false, false, false, false};
	const uint16_t angle[] = {0,80,160,0,0,0,0,0};
  assert_fuel_schedules(context.current, 720U/3U, enabled, angle, __LINE__);
  TEST_ASSERT_FALSE(context.page10.stagingEnabled);
}

static void assert_3cylinder_semiseq_nostage(int assertLineNum, const init_context_t &context)
{
 	const bool enabled[] = {true, INJ_CHANNELS>=2, INJ_CHANNELS>=3, false, false, false, false, false};
	const uint16_t angle[] = {0,120,240,0,0,0,0,0};
  assert_fuel_schedules(context.current, 360U, enabled, angle, assertLineNum); //Special case as 3 squirts per cycle MUST be over 720 degrees
  TEST_ASSERT_EQUAL(INJ_SEMISEQUENTIAL, context.current.injLayout);
  TEST_ASSERT_FALSE(context.page10.stagingEnabled);
}

static void cylinder3_stroke4_semiseq_nostage_port(void)
{
  auto context = setup_3_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page2.injTiming = true;
  context.page10.stagingEnabled = false;
  context.page2.injType = INJ_TYPE_PORT;
  context.initialise();
  assert_3cylinder_semiseq_nostage(__LINE__, context);
}

static void cylinder3_stroke4_seq_staged(void)
{
  auto context = setup_3_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page2.injTiming = true;
  enableStaging(context);
  context.initialise();
	const bool enabled[] = {true, INJ_CHANNELS>=2, INJ_CHANNELS>=3, INJ_CHANNELS>=4, INJ_CHANNELS>=6, INJ_CHANNELS>=6, false, false};
	const uint16_t angle[] = {0,240,480,0,240,480,0,0};
  assert_fuel_schedules(context.current, 720U, enabled, angle, __LINE__);
  TEST_ASSERT_EQUAL(INJ_SEQUENTIAL, context.current.injLayout);
  TEST_ASSERT_TRUE(context.page10.stagingEnabled);
}

static void cylinder3_stroke4_semiseq_staged_tb(void)
{
  auto context = setup_3_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  enableStaging(context);
  context.page2.injType = INJ_TYPE_TBODY;
  context.initialise();
	const uint16_t angle[] = {0,80,160,0,80,160,0,0};
	const bool enabled[] = {true, INJ_CHANNELS>=3, INJ_CHANNELS>=3, INJ_CHANNELS>=4, INJ_CHANNELS>=6, INJ_CHANNELS>=6, false, false};
  assert_fuel_schedules(context.current, 720U/3U, enabled, angle, __LINE__); //Special case as 3 squirts per cycle MUST be over 720 degrees
  TEST_ASSERT_EQUAL(INJ_SEMISEQUENTIAL, context.current.injLayout);
  TEST_ASSERT_TRUE(context.page10.stagingEnabled);
}


static void cylinder3_stroke4_semiseq_staged_port(void)
{
  auto context = setup_3_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  enableStaging(context);
  context.page2.injType = INJ_TYPE_PORT;
  context.initialise();
	const uint16_t angle[] = {0,120,240,0,120,240,0,0};
	const bool enabled[] = {true, INJ_CHANNELS>=3, INJ_CHANNELS>=3, INJ_CHANNELS>=4, INJ_CHANNELS>=6, INJ_CHANNELS>=6, false, false};
  assert_fuel_schedules(context.current, 720U/2U, enabled, angle, __LINE__); //Special case as 3 squirts per cycle MUST be over 720 degrees
}

static void run_3_cylinder_4stroke_tests(void)
{
  RUN_TEST_P(cylinder3_stroke4_seq_nostage);
  RUN_TEST_P(cylinder3_stroke4_semiseq_nostage_tb);
  RUN_TEST_P(cylinder3_stroke4_semiseq_nostage_port);
  RUN_TEST_P(cylinder3_stroke4_seq_staged);
  RUN_TEST_P(cylinder3_stroke4_semiseq_staged_tb);
  RUN_TEST_P(cylinder3_stroke4_semiseq_staged_port);
}

static init_context_t setup_3_cylinder_2stroke(void)
{
  init_context_t context;
  context.page2.nCylinders = 3;
  context.page2.strokes = TWO_STROKE;
  context.page2.engineType = EVEN_FIRE;
  context.page2.injTiming = true;
  context.page2.divider = 1;
  return context;
}

static void cylinder3_stroke2_seq_nostage(void)
{
  auto context = setup_3_cylinder_2stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page10.stagingEnabled = false;
  context.initialise();
	const bool enabled[] = {true, true, true, false, false, false, false, false};
	const uint16_t angle[] = {0,120,240,0,0,0,0,0};
  assert_fuel_schedules(context.current, 360U, enabled, angle, __LINE__);
}

static void cylinder3_stroke2_semiseq_nostage_tb(void)
{
  auto context = setup_3_cylinder_2stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page10.stagingEnabled = false;
  context.page2.injType = INJ_TYPE_TBODY;
  context.initialise();
	const bool enabled[] = {true, true, true, false, false, false, false, false};
	const uint16_t angle[] = {0,40,80,0,0,0,0,0};
  assert_fuel_schedules(context.current, 360U/3U, enabled, angle, __LINE__);
}

static void cylinder3_stroke2_semiseq_nostage_port(void)
{
  auto context = setup_3_cylinder_2stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page10.stagingEnabled = false;
  context.page2.injType = INJ_TYPE_PORT;
  context.initialise();
	const bool enabled[] = {true, true, true, false, false, false, false, false};
	const uint16_t angle[] = {0,60,120,0,0,0,0,0};
  assert_fuel_schedules(context.current, 360U/2U, enabled, angle, __LINE__);
}

static void cylinder3_stroke2_seq_staged(void)
{
  auto context = setup_3_cylinder_2stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  enableStaging(context);
  context.initialise();
	const bool enabled[] = {true, true, true, INJ_CHANNELS>=4, INJ_CHANNELS>=6, INJ_CHANNELS>=6, false, false};
	const uint16_t angle[] = {0,120,240,0,120,240,0,0};
  assert_fuel_schedules(context.current, 360U, enabled, angle, __LINE__);
}

static void cylinder3_stroke2_semiseq_staged_tb(void)
{
  auto context = setup_3_cylinder_2stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  enableStaging(context);
  context.page2.injType = INJ_TYPE_TBODY;
  context.initialise();
	const uint16_t angle[] = {0,40,80,0,40,80,0,0};
	const bool enabled[] = {true, true, true, INJ_CHANNELS>=4, INJ_CHANNELS>=6, INJ_CHANNELS>=6, false, false};
  assert_fuel_schedules(context.current, 360U/3U, enabled, angle, __LINE__);
}

static void cylinder3_stroke2_semiseq_staged_port(void)
{
  auto context = setup_3_cylinder_2stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  enableStaging(context);
  context.page2.injType = INJ_TYPE_PORT;
  context.initialise();
	const uint16_t angle[] = {0,60,120,0,60,120,0,0};
	const bool enabled[] = {true, true, true, INJ_CHANNELS>=4, INJ_CHANNELS>=6, INJ_CHANNELS>=6, false, false};
  assert_fuel_schedules(context.current, 360U/2U, enabled, angle, __LINE__);
}

static void run_3_cylinder_2stroke_tests(void)
{
  RUN_TEST_P(cylinder3_stroke2_seq_nostage);
  RUN_TEST_P(cylinder3_stroke2_semiseq_nostage_tb);
  RUN_TEST_P(cylinder3_stroke2_semiseq_nostage_port);
  RUN_TEST_P(cylinder3_stroke2_seq_staged);
  RUN_TEST_P(cylinder3_stroke2_semiseq_staged_tb);
  RUN_TEST_P(cylinder3_stroke2_semiseq_staged_port);
}

static init_context_t setup_4_cylinder_4stroke(void)
{
  init_context_t context;
  context.page2.nCylinders = 4;
  context.page2.strokes = FOUR_STROKE;
  context.page2.engineType = EVEN_FIRE;
  context.page2.injTiming = true;
  context.page2.divider = 2;
  return context;
}

static void assert_4cylinder_4stroke_seq_nostage(int assertLineNum, const statuses &current)
{
  const bool enabled[] = {true, true, true, true, false, false, false, false};
  const uint16_t angle[] = {0,180,360,540,0,0,0,0};
  assert_fuel_schedules(current, 720U, enabled, angle, assertLineNum);
  TEST_ASSERT_EQUAL(INJ_SEQUENTIAL, current.injLayout);
}

static void assert_4cylinder_4stroke_paired_nostage(int assertLineNum, const statuses &current)
{
	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_fuel_schedules(current, 360U, enabled, angle, assertLineNum);
}

static void cylinder4_stroke4_seq_nostage(void)
{
  auto context = setup_4_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page10.stagingEnabled = false;
  context.initialise();
  assert_4cylinder_4stroke_seq_nostage(__LINE__, context.current);
}

static void assert_4cylinder_4stroke_semiseq_nostage(int assertLineNum, const statuses &current)
{
	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_fuel_schedules(current, 360U, enabled, angle, assertLineNum);
}

static void cylinder4_stroke4_seq_staged(void)
{
  auto context = setup_4_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  enableStaging(context);
  context.initialise();
	const bool enabled[] = {true, true, true, true, INJ_CHANNELS>=5, INJ_CHANNELS>=8, INJ_CHANNELS>=8, INJ_CHANNELS>=8};
	const uint16_t angle[] = {0,180,360,540,0,180,360,540};
  assert_fuel_schedules(context.current, 720U, enabled, angle, __LINE__);
}

static void cylinder4_stroke4_paired_nostage(void)  
{
  auto context = setup_4_cylinder_4stroke();
  context.page2.injLayout = INJ_PAIRED;
  context.page10.stagingEnabled = false;
  context.initialise();
  assert_4cylinder_4stroke_paired_nostage(__LINE__, context.current);
}

static void cylinder4_stroke4_paired_staged(void)  
{
  auto context = setup_4_cylinder_4stroke();
  context.page2.injLayout = INJ_PAIRED;
  enableStaging(context);
  context.initialise();
  assert_2channel_2staged_over360(__LINE__, context);
}

static void cylinder4_stroke4_semiseq_nostage(uint8_t pairMode)
{
  auto context = setup_4_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page4.inj4cylPairing = pairMode;
  context.page10.stagingEnabled = false;
  context.initialise();
  assert_4cylinder_4stroke_paired_nostage(__LINE__, context.current);
}

static void cylinder4_stroke4_semiseq_pair1324_nostage(void)
{
  cylinder4_stroke4_semiseq_nostage(INJ_PAIR_13_24);
}

static void cylinder4_stroke4_semiseq_pair1423_nostage(void)
{
  cylinder4_stroke4_semiseq_nostage(INJ_PAIR_14_23);
}

static void cylinder4_stroke4_semiseq_staged(uint8_t pairMode)
{
  auto context = setup_4_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page4.inj4cylPairing = pairMode;
  enableStaging(context);
  context.initialise();
  // assert_2channel_2staged_over360(__LINE__, context);
  TEST_IGNORE_MESSAGE("TODO: make this work");
}

static void cylinder4_stroke4_semiseq_pair1324_staged(void)
{
  cylinder4_stroke4_semiseq_staged(INJ_PAIR_13_24);
}

static void cylinder4_stroke4_semiseq_pair1423_staged(void)
{
  cylinder4_stroke4_semiseq_staged(INJ_PAIR_14_23);
}

static void run_4_cylinder_4stroke_tests(void)
{
  RUN_TEST_P(cylinder4_stroke4_seq_nostage);
  RUN_TEST_P(cylinder4_stroke4_paired_nostage);
  RUN_TEST_P(cylinder4_stroke4_semiseq_pair1324_nostage);
  RUN_TEST_P(cylinder4_stroke4_semiseq_pair1423_nostage);
  RUN_TEST_P(cylinder4_stroke4_seq_staged);
  RUN_TEST_P(cylinder4_stroke4_paired_staged);
  RUN_TEST_P(cylinder4_stroke4_semiseq_pair1324_staged);
  RUN_TEST_P(cylinder4_stroke4_semiseq_pair1423_staged);
}

static init_context_t setup_4_cylinder_2stroke(void)
{
  init_context_t context;
  context.page2.nCylinders = 4;
  context.page2.strokes = TWO_STROKE;
  context.page2.engineType = EVEN_FIRE;
  context.page2.injTiming = true;
  context.page2.divider = 2;
  return context;
}

static void cylinder4_stroke2_seq_nostage(void)
{
  auto context = setup_4_cylinder_2stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page10.stagingEnabled = false;
  context.initialise();
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,45,90,135,0,0,0,0};
  assert_fuel_schedules(context.current, 180U, enabled, angle, __LINE__);
}

static void cylinder4_stroke2_semiseq_nostage(void)
{
  auto context = setup_4_cylinder_2stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page10.stagingEnabled = false;
  context.initialise();
	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,90,0,0,0,0,0,0};
  assert_fuel_schedules(context.current, 180U, enabled, angle, __LINE__);
}

static void cylinder4_stroke2_seq_staged(void)
{
  auto context = setup_4_cylinder_2stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  enableStaging(context);
  context.initialise();
	const bool enabled[] = {true, INJ_CHANNELS>=2, INJ_CHANNELS>=3, INJ_CHANNELS>=4, INJ_CHANNELS>=5, INJ_CHANNELS>=6, INJ_CHANNELS>=7, INJ_CHANNELS>=8};
	const uint16_t angle[] = {0,45,90,135,0,45,90,135};
  assert_fuel_schedules(context.current, 180U, enabled, angle, __LINE__);
}

static void cylinder4_stroke2_semiseq_staged(void)
{
  auto context = setup_4_cylinder_2stroke();
  context.page2.injLayout = INJ_PAIRED;
  enableStaging(context);
  context.initialise();
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,90,0,90,0,0,0,0};
  assert_fuel_schedules(context.current, 180U, enabled, angle, __LINE__);
}

void run_4_cylinder_2stroke_tests(void)
{
  RUN_TEST_P(cylinder4_stroke2_seq_nostage);
  RUN_TEST_P(cylinder4_stroke2_semiseq_nostage);
  RUN_TEST_P(cylinder4_stroke2_seq_staged);
  RUN_TEST_P(cylinder4_stroke2_semiseq_staged);
}

static init_context_t setup_5_cylinder_4stroke(void)
{
  init_context_t context;
  context.page2.nCylinders = 5;
  context.page2.strokes = FOUR_STROKE;
  context.page2.engineType = EVEN_FIRE;
  context.page2.injTiming = true;
  context.page2.divider = 5;
  return context;
}

static void assert_5cylinder_4stroke_seq_nostage(int assertLineNum, const statuses &current)
{
	const bool enabled[] = {true, true, true, true, INJ_CHANNELS>=5, false, false, false};
#if INJ_CHANNELS>=5
  const uint16_t angle[] = {0,144,288,432,576,0,0,0};
  TEST_ASSERT_EQUAL(INJ_SEQUENTIAL, current.injLayout);
#else
  const uint16_t angle[] = {0,72,144,216,0,0,0,0};
  TEST_ASSERT_EQUAL(INJ_PAIRED, current.injLayout);
#endif
  assert_fuel_schedules(current, 720U, enabled, angle, assertLineNum);
}

static void cylinder5_stroke4_seq_nostage(void)
{
  auto context = setup_5_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page10.stagingEnabled = false;
  context.initialise();
  assert_5cylinder_4stroke_seq_nostage(__LINE__, context.current);
}

static void cylinder5_stroke4_semiseq_nostage(void)
{
  auto context = setup_5_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page10.stagingEnabled = false;
  context.initialise();
	const bool enabled[] = {true, true, true, true, INJ_CHANNELS>=5, false, false, false};
	const uint16_t angle[] = {0,72,144,216,288,0,0,0};
  assert_fuel_schedules(context.current, 720U, enabled, angle, __LINE__);
}

static void cylinder5_stroke4_seq_staged(void)
{
  auto context = setup_5_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  enableStaging(context);
  context.initialise();
	const bool enabled[] = {true, true, true, true, INJ_CHANNELS>=5, INJ_CHANNELS>=6, false, false};
#if INJ_CHANNELS>=5
	const uint16_t angle[] = {0,144,288,432,576,0,0,0};
#else
	const uint16_t angle[] = {0,72,144,216,288,0,0,0};
#endif
  assert_fuel_schedules(context.current, 720U, enabled, angle, __LINE__);
}

static void cylinder5_stroke4_semiseq_staged(void)
{
  auto context = setup_5_cylinder_4stroke();
  context.page2.injLayout = INJ_PAIRED;
  enableStaging(context);
  context.initialise();
	const bool enabled[] = {true, true, true, true, INJ_CHANNELS>=5, INJ_CHANNELS>=6, false, false};
	const uint16_t angle[] = {0,72,144,216,288,360,0,0};
  assert_fuel_schedules(context.current, 720U, enabled, angle, __LINE__);
}

static void run_5_cylinder_4stroke_tests(void)
{
  RUN_TEST_P(cylinder5_stroke4_seq_nostage);
  RUN_TEST_P(cylinder5_stroke4_semiseq_nostage);
  RUN_TEST_P(cylinder5_stroke4_seq_staged);
  RUN_TEST_P(cylinder5_stroke4_semiseq_staged);
}

static init_context_t setup_6_cylinder_4stroke(void)
{
  init_context_t context;
  context.page2.nCylinders = 6;
  context.page2.strokes = FOUR_STROKE;
  context.page2.engineType = EVEN_FIRE;
  context.page2.injTiming = true;
  context.page2.divider = 6;
  return context;
}

static void assert_6cylinder_4stroke_seq_nostage(int assertLineNum, const init_context_t &context)
{
#if INJ_CHANNELS >= 6
	const bool enabled[] = {true, true, true, true, true, true, false, false};
	const uint16_t angle[] = {0,120,240,360,480,600,0,0};
  TEST_ASSERT_EQUAL(INJ_SEQUENTIAL, context.current.injLayout);
#else
	const bool enabled[] = {true, true, true, false, false, false, false, false};
	const uint16_t angle[] = {0,120,240,360,480,600,0,0};
  TEST_ASSERT_EQUAL(INJ_PAIRED, context.current.injLayout);
#endif
  TEST_ASSERT_FALSE(context.page10.stagingEnabled);
  assert_fuel_schedules(context.current, 720U, enabled, angle, assertLineNum);
}

static void cylinder6_stroke4_seq_nostage(void)
{
  auto context = setup_6_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page10.stagingEnabled = false;
  context.initialise();
  assert_6cylinder_4stroke_seq_nostage(__LINE__, context);
}

static void cylinder6_stroke4_semiseq_nostage(void)
{
  auto context = setup_6_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page10.stagingEnabled = false;
  context.initialise();
	const bool enabled[] = {true, true, true, false, false, false, false, false};
	const uint16_t angle[] = {0,120,240,0,0,0,0,0};
  assert_fuel_schedules(context.current, 720U, enabled, angle, __LINE__);
  TEST_ASSERT_FALSE(context.page10.stagingEnabled);
}

static void cylinder6_stroke4_seq_staged(void)
{
  auto context = setup_6_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  enableStaging(context);
  context.initialise();
  TEST_ASSERT_FALSE(context.page10.stagingEnabled);
	const uint16_t angle[] = {0,120,240,360,480,600,0,0};
	const bool enabled[] = {true, true, true, INJ_CHANNELS>=6, INJ_CHANNELS>=6, INJ_CHANNELS>=6, false, false};
  assert_fuel_schedules(context.current, 720U, enabled, angle, __LINE__);
}

static void cylinder6_stroke4_semiseq_staged(void)
{
  auto context = setup_6_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  enableStaging(context);
  context.initialise();
#if INJ_CHANNELS >= 7
	const uint16_t angle[] = {0,120,240,360,0,120,240,0};
	const bool enabled[] = {true, true, true, true, true, true, false, false};
  TEST_ASSERT_TRUE(context.page10.stagingEnabled);
  TEST_IGNORE_MESSAGE("Fix code so test passes :-()");
#else
	const uint16_t angle[] = {0,120,240,0,0,0,0,0};
	const bool enabled[] = {true, true, true, false, false, false, false, false};
  TEST_ASSERT_FALSE(context.page10.stagingEnabled);
#endif
  assert_fuel_schedules(context.current, 720U, enabled, angle, __LINE__);
}

static void run_6_cylinder_4stroke_tests(void)
{
  RUN_TEST_P(cylinder6_stroke4_seq_nostage);
  RUN_TEST_P(cylinder6_stroke4_semiseq_nostage);
  RUN_TEST_P(cylinder6_stroke4_seq_staged);
  RUN_TEST_P(cylinder6_stroke4_semiseq_staged);
}

static init_context_t setup_8_cylinder_4stroke(void)
{
  init_context_t context;
  context.page2.nCylinders = 8;
  context.page2.strokes = FOUR_STROKE;
  context.page2.engineType = EVEN_FIRE;
  context.page2.injTiming = true;
  context.page2.divider = 8;
  return context;
}

static void assert_8cylinder_4stroke_seq_nostage(int assertLineNum, const statuses &current)
{
#if INJ_CHANNELS >= 8
	const bool enabled[] = {true, true, true, true, true, true, true, true};
	const uint16_t angle[] = {0,90,180,270,360,450,540,630};
  TEST_ASSERT_EQUAL(INJ_SEQUENTIAL, current.injLayout);
#else
	const bool enabled[] = {true, true, true, true, false, false, false, false};
	const uint16_t angle[] = {0,180,360,540,0,0,0,0};
  TEST_ASSERT_EQUAL(INJ_PAIRED, current.injLayout);
#endif
  assert_fuel_schedules(current, 720U, enabled, angle, assertLineNum);
}

static void cylinder8_stroke4_seq_nostage(void)
{
  auto context = setup_8_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page10.stagingEnabled = false;
  context.initialise();
  assert_8cylinder_4stroke_seq_nostage(__LINE__, context.current);
}

static void assert_8cylinder_4stroke_paired_nostage(int assertLineNum, const statuses &current)
{
	const uint16_t angle[] = {0,90,180,270,360,450,540,630};
	const bool enabled[] = {true, true, true, true, false, false, false, false};
  assert_fuel_schedules(current, 360U, enabled, angle, assertLineNum);
}

static void cylinder8_stroke4_paired_nostage(void)
{
  auto context = setup_8_cylinder_4stroke();
  context.page2.divider = context.page2.nCylinders/2U;
  context.page2.injLayout = INJ_PAIRED;
  context.page10.stagingEnabled = false;
  context.initialise();
  assert_8cylinder_4stroke_paired_nostage(__LINE__, context.current);
}

static void run_8_cylinder_4stroke_tests(void)
{
  // Staging not supported on 8 cylinders

  RUN_TEST_P(cylinder8_stroke4_seq_nostage);
  RUN_TEST_P(cylinder8_stroke4_paired_nostage);
}

static init_context_t setup_no_inj_timing(void)
{
  init_context_t context;
  context.page2.strokes = FOUR_STROKE;
  context.page2.engineType = EVEN_FIRE;
  context.page2.injTiming = false;
  context.page10.stagingEnabled = false;
  return context;
}

static constexpr uint16_t zeroAngles[] = {0,0,0,0,0,0,0,0};

static void cylinder_1_NoinjTiming_paired(void) {
  auto context = setup_no_inj_timing();
  context.page2.injLayout = INJ_PAIRED;
  context.page2.nCylinders = 1;
  context.page2.divider = 1;

  context.initialise();

  const bool enabled[] = {true, false, false, false, false, false, false, false};
  assert_fuel_schedules(context.current, 720U, enabled, zeroAngles, __LINE__);
}

static void cylinder_2_NoinjTiming_paired(void) {
  auto context = setup_no_inj_timing();
  context.page2.injLayout = INJ_PAIRED;
  context.page2.nCylinders = 2;
  context.page2.divider = 2;

  context.initialise();

  const bool enabled[] = {true, true, false, false, false, false, false, false};
  assert_fuel_schedules(context.current, 720U, enabled, zeroAngles, __LINE__);
}

static void cylinder_3_NoinjTiming_paired(void) {
  auto context = setup_no_inj_timing();
  context.page2.injLayout = INJ_PAIRED;
  context.page2.nCylinders = 3;
  context.page2.divider = 3;
  context.page2.injType = INJ_TYPE_PORT;

  context.initialise();

  const bool enabled[] = {true, true, true, false, false, false, false, false};
  assert_fuel_schedules(context.current, 360U, enabled, zeroAngles, __LINE__);
}

static void cylinder_4_NoinjTiming_paired(void) {
  auto context = setup_no_inj_timing();
  context.page2.injLayout = INJ_PAIRED;
  context.page2.nCylinders = 4;
  context.page2.divider = 4;

  context.initialise();

  const bool enabled[] = {true, true, false, false, false, false, false, false};
  assert_fuel_schedules(context.current, 720U, enabled, zeroAngles, __LINE__);
}

static void cylinder_5_NoinjTiming_paired(void) {
  auto context = setup_no_inj_timing();
  context.page2.injLayout = INJ_PAIRED;
  context.page2.nCylinders = 5;
  context.page2.divider = 5;

  context.initialise();

  const bool enabled[] = {true, true, true, true, INJ_CHANNELS>=5, false, false, false};
  assert_fuel_schedules(context.current, 720U, enabled, zeroAngles, __LINE__);
}

static void cylinder_6_NoinjTiming_paired(void) {
  auto context = setup_no_inj_timing();
  context.page2.injLayout = INJ_PAIRED;
  context.page2.nCylinders = 6;
  context.page2.divider = 6;

  context.initialise();

  const bool enabled[] = {true, true, true, false, false, false, false, false};
  assert_fuel_schedules(context.current, 720U, enabled, zeroAngles, __LINE__);
}

static void cylinder_8_NoinjTiming_paired(void) {
  auto context = setup_no_inj_timing();
  context.page2.injLayout = INJ_PAIRED;
  context.page2.nCylinders = 8;
  context.page2.divider = 8;

  context.initialise();

  const bool enabled[] = {true, true, true, true, false, false, false, false};
  assert_fuel_schedules(context.current, 720U, enabled, zeroAngles, __LINE__);
}

static void run_no_inj_timing_tests(void)
{
  RUN_TEST_P(cylinder_1_NoinjTiming_paired);
  RUN_TEST_P(cylinder_2_NoinjTiming_paired);
  RUN_TEST_P(cylinder_3_NoinjTiming_paired);
  RUN_TEST_P(cylinder_4_NoinjTiming_paired);
  RUN_TEST_P(cylinder_5_NoinjTiming_paired);
  RUN_TEST_P(cylinder_6_NoinjTiming_paired);
  RUN_TEST_P(cylinder_8_NoinjTiming_paired);
}

static init_context_t setup_oddfire(uint8_t nCylinders)
{
  init_context_t context;
  context.page2.strokes = FOUR_STROKE;
  context.page2.engineType = ODD_FIRE;
  context.page2.injTiming = true;
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page2.nCylinders = nCylinders;
  context.page10.stagingEnabled = false;
  context.page2.divider = nCylinders;
  context.page2.oddfire2 = 13;
  context.page2.oddfire3 = 111;
  context.page2.oddfire4 = 217;
  return context;
}

static void cylinder_2_oddfire(void)
{
  auto context = setup_oddfire(2);

  context.initialise();

	const bool enabled[] = {true, true, false, false, false, false, false, false};
	const uint16_t angle[] = {0,13,0,0,0,0,0,0};
  assert_fuel_schedules(context.current, 720U, enabled, angle, __LINE__);
}

static void cylinder_1_oddfire(void)
{
  auto context = setup_oddfire(1);
  context.initialise();
  assert_1channel_0stage_over720(__LINE__, context);
}

static void cylinder_3_oddfire(void)
{
  auto context = setup_oddfire(3);
  context.initialise();
  assert_3cylinder_4stroke_seq_nostage(__LINE__, context);
}

static void cylinder_4_oddfire(void)
{
  auto context = setup_oddfire(4);
  context.initialise();
  assert_4cylinder_4stroke_seq_nostage(__LINE__, context.current);
}

static void cylinder_5_oddfire(void)
{
  auto context = setup_oddfire(5);
  context.initialise();
  assert_5cylinder_4stroke_seq_nostage(__LINE__, context.current);
}

static void cylinder_6_oddfire(void)
{
  auto context = setup_oddfire(6);
  context.initialise();
  assert_6cylinder_4stroke_seq_nostage(__LINE__, context);
}

static void cylinder_8_oddfire(void)
{
  auto context = setup_oddfire(8);
  context.initialise();
  assert_8cylinder_4stroke_seq_nostage(__LINE__, context.current);
}

static void run_oddfire_tests()
{
  RUN_TEST_P(cylinder_1_oddfire);
  RUN_TEST_P(cylinder_2_oddfire);
  RUN_TEST_P(cylinder_3_oddfire);
  RUN_TEST_P(cylinder_4_oddfire);
  RUN_TEST_P(cylinder_5_oddfire);
  RUN_TEST_P(cylinder_6_oddfire);
  RUN_TEST_P(cylinder_8_oddfire);
}

static init_context_t setupPartialSyncTest(uint8_t cylinders)
{
  init_context_t context;
  context.page2.nCylinders = cylinders;
  context.page2.engineType = EVEN_FIRE;
  context.page2.injTiming = true;
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page2.strokes = FOUR_STROKE;
  context.page2.divider = cylinders;
  context.page10.stagingEnabled = false;
  context.current.decoder = decoder_builder_t().setGetStatus(getFakeDecoderStatus).build();
    
  context.initialise();

  return context;
}

static void test_partial_sync_1_cylinder(void)
{
  auto context = setupPartialSyncTest(1);

  // Confirm initial state
  assert_1channel_0stage_over720(__LINE__, context);

  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  matchFuelSchedulersToSyncState(context.page2, context.page4, context.current);
  // Confirm no change
  assert_1channel_0stage_over720(__LINE__, context);

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  matchFuelSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_1channel_0stage_over720(__LINE__, context);
}

static void test_partial_sync_2_cylinder(void)
{
  auto context = setupPartialSyncTest(2);

  // Confirm initial state
  assert_2channel_0stage_over720(__LINE__, context);

  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  matchFuelSchedulersToSyncState(context.page2, context.page4, context.current);
  // Confirm no change
  assert_2channel_0stage_over720(__LINE__, context);

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  matchFuelSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_2channel_0stage_over720(__LINE__, context);
}

static void test_partial_sync_3_cylinder(void)
{
  auto context = setupPartialSyncTest(3);

  // Confirm initial state
  assert_3cylinder_4stroke_seq_nostage(__LINE__, context);

  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  matchFuelSchedulersToSyncState(context.page2, context.page4, context.current);
  // Confirm no change
  assert_3cylinder_4stroke_seq_nostage(__LINE__, context);

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  matchFuelSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_3cylinder_4stroke_seq_nostage(__LINE__, context);
}

static void test_partial_sync_4_cylinder(void)
{
  auto context = setupPartialSyncTest(4);

  // Confirm initial state
  assert_4cylinder_4stroke_seq_nostage(__LINE__, context.current);

  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  matchFuelSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_4cylinder_4stroke_semiseq_nostage(__LINE__, context.current);
  TEST_ASSERT_EQUAL(INJ_SEMISEQUENTIAL, context.current.injLayout);

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  matchFuelSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_4cylinder_4stroke_seq_nostage(__LINE__, context.current);
  TEST_ASSERT_EQUAL(INJ_SEQUENTIAL, context.current.injLayout);
}

static void test_partial_sync_5_cylinder(void)
{
  auto context = setupPartialSyncTest(5);

  // Confirm initial state
  assert_5cylinder_4stroke_seq_nostage(__LINE__, context.current);

  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  matchFuelSchedulersToSyncState(context.page2, context.page4, context.current);
  // Confirm no change
  assert_5cylinder_4stroke_seq_nostage(__LINE__, context.current);

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  matchFuelSchedulersToSyncState(context.page2, context.page4, context.current);
  // Confirm no change
  assert_5cylinder_4stroke_seq_nostage(__LINE__, context.current);
}

static void test_partial_sync_6_cylinder(void)
{
#if INJ_CHANNELS>=6
  auto context = setupPartialSyncTest(6);

  // Confirm initial state
  assert_6cylinder_4stroke_seq_nostage(__LINE__, context);

  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  matchFuelSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_3cylinder_semiseq_nostage(__LINE__, context);

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  matchFuelSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_6cylinder_4stroke_seq_nostage(__LINE__, context);
#else
  TEST_IGNORE_MESSAGE("Skipping - not enough injectors");
#endif
}

static void test_partial_sync_8_cylinder(void)
{
#if INJ_CHANNELS>=8
  auto context = setupPartialSyncTest(8);

  // Confirm initial state
  assert_8cylinder_4stroke_seq_nostage(__LINE__, context.current);

  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  matchFuelSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_8cylinder_4stroke_paired_nostage(__LINE__, context.current);
  TEST_ASSERT_EQUAL(INJ_SEMISEQUENTIAL, context.current.injLayout);

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  matchFuelSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_8cylinder_4stroke_seq_nostage(__LINE__, context.current);
  // Deliberate repeat
  matchFuelSchedulersToSyncState(context.page2, context.page4, context.current);
  assert_8cylinder_4stroke_seq_nostage(__LINE__, context.current);
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