#include <unity.h>
#include "../test_utils.h"
#include "../channel_test_helpers.h"
#include "../fake_decoder_status.h"
#include "decoder_builder.h"
#include "scheduler_fuel_controller.h"
#include "src/pins/pinNumbers_t.h"

extern void matchFuelSchedulersToSyncState(const config2 &page2, const config4 &page4, const config10 &page10, statuses &current);

struct init_context_t
{
  statuses current = {};
  config2 page2 = {};
  config4 page4 = {};
  config6 page6 = {};
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
    initialiseFuelSchedules(current, page2, page4, page6, page10, pins);
  }

  void enableStaging(void)
  {
    page10.stagingEnabled = true;
    page10.stagedInjSizePri = 250;
    page10.stagedInjSizeSec = 500;
  }

  void setup_4stroke(uint8_t nCylinders)
  {
    setup_numcylinders(nCylinders);
    page2.strokes = FOUR_STROKE;
  }

  void setup_2stroke(uint8_t nCylinders)
  {
    setup_numcylinders(nCylinders);
    page2.strokes = TWO_STROKE;
  }


  void matchFuelSchedulersToSyncState(void)
  {
    ::matchFuelSchedulersToSyncState(page2, page4, page10, current);
  }

  const init_context_t& setAssertLine(int assertLineNum) const
  {
    _assertLine = assertLineNum;
    return *this;
  }
  const init_context_t& assertPrimaryChannels(uint8_t expected) const
  {
    UNITY_TEST_ASSERT_EQUAL_UINT8(expected, current.injOutputs.primary, _assertLine, "primary");
    return *this;
  }
  const init_context_t& assertSecondaryChannels(uint8_t expected) const
  {
    UNITY_TEST_ASSERT_EQUAL_UINT8(expected, current.injOutputs.secondary, _assertLine, "secondary");
    UNITY_TEST_ASSERT( (expected>0 && page10.stagingEnabled)
                    || (expected==0 && !page10.stagingEnabled), _assertLine, "stagingEnabled");
    return *this;
  }
  const init_context_t& assertMaxCrank(uint16_t expected) const
  {
    UNITY_TEST_ASSERT_EQUAL_UINT16(expected, CRANK_ANGLE_MAX_INJ, _assertLine, "CRANK_ANGLE_MAX_INJ");
    return *this;
  }

  const init_context_t& assertFuelChannels(const bool (&enabled)[8], const uint16_t (&angle)[8]) const
  {
    #define ASSERT_CHANNEL(channel) \
      CONCAT(RUNIF_INJCHANNEL, channel) \
      (assert_fuel_channel(enabled[channel-1], angle[channel-1], channel, fuelSchedule ## channel), {});

    ASSERT_CHANNEL(1);
    ASSERT_CHANNEL(2);
    ASSERT_CHANNEL(3);
    ASSERT_CHANNEL(4);
    ASSERT_CHANNEL(5);
    ASSERT_CHANNEL(6);
    ASSERT_CHANNEL(7);
    ASSERT_CHANNEL(8);

    return *this;
  }

private:
  void setup_numcylinders(uint8_t nCylinders)
  {
    page2.nCylinders = nCylinders;
    page2.divider = page2.nCylinders;
    page2.nInjectors = page2.nCylinders;
    page2.engineType = EVEN_FIRE;
    page2.injTiming = true;
  }

  void assert_fuel_channel(bool enabled, uint16_t angle, uint8_t channelIndex, const FuelSchedule &schedule) const
  {
    char msg[64];
    if (enabled)
    {
      sprintf_P(msg, PSTR("channel%" PRIu8 ".InjChannelIsEnabled"), channelIndex);
      UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT8(current.injOutputs.getTotalInjectors(), channelIndex, _assertLine, msg);
      sprintf_P(msg, PSTR("channel%" PRIu8 ".InjDegrees"), channelIndex);
      UNITY_TEST_ASSERT_EQUAL_UINT16(angle, schedule.channelDegrees, _assertLine, msg);
      sprintf_P(msg, PSTR("inj%" PRIu8 ".StartFunction"), channelIndex);
      UNITY_TEST_ASSERT(schedule._pStartCallback!=nullCallback, _assertLine, msg);
      sprintf_P(msg, PSTR("inj%" PRIu8 ".EndFunction"), channelIndex);
      UNITY_TEST_ASSERT(schedule._pEndCallback!=nullCallback, _assertLine, msg);
      sprintf_P(msg, PSTR("injAngle"));
      UNITY_TEST_ASSERT_SMALLER_THAN_UINT16(CRANK_ANGLE_MAX_INJ, angle, _assertLine, msg);
    }
    else 
    {
      sprintf_P(msg, PSTR("channel%" PRIu8 ".InjChannelIsEnabled"), channelIndex);
      UNITY_TEST_ASSERT_SMALLER_THAN_UINT8(channelIndex, current.injOutputs.getTotalInjectors(), _assertLine, msg);
    }
  }
  mutable int _assertLine = INT_MAX;
};

static init_context_t assert_injlayout(uint8_t layout, const init_context_t &context)
{
  TEST_ASSERT_EQUAL(layout, context.current.injLayout);
  TEST_ASSERT_EQUAL(layout, context.page2.injLayout);
  return context;
}

static void assert_1channel_0stage_over720(int assertLineNum, const init_context_t &context)
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(720)
    .assertPrimaryChannels(1)
    .assertSecondaryChannels(0)
    .assertFuelChannels({true, false, false, false, false, false, false, false},
                        {0,0,0,0,0,0,0,0});
}

static void assert_1channel_1stage_over720(int assertLineNum, const init_context_t &context)
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(720)
    .assertPrimaryChannels(1)
    .assertSecondaryChannels(1)
    .assertFuelChannels({true, true, false, false, false, false, false, false},
	                      {0,0,0,0,0,0,0,0});
}

static init_context_t setup1_cylinder_4stroke(void)
{
  init_context_t context;
  context.setup_4stroke(1);
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
  assert_injlayout(INJ_PAIRED, context);
}

static void cylinder1_stroke4_seq_staged(void)
{
  auto context = setup1_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page2.injTiming = true;
  context.enableStaging();
  
  context.initialise();
  
  assert_1channel_1stage_over720(__LINE__, context);
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void cylinder1_stroke4_semiseq_staged(void)
{
  auto context = setup1_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page2.injTiming = true;
  context.enableStaging();

  context.initialise();
  
  assert_1channel_1stage_over720(__LINE__, context);
  assert_injlayout(INJ_PAIRED, context);
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
 context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(360)
    .assertPrimaryChannels(1)
    .assertSecondaryChannels(0)
    .assertFuelChannels({true, false, false, false, false, false, false, false},
	                      {0,0,0,0,0,0,0,0});
}

static void assert_1channel_1stage_over360(int assertLineNum, const init_context_t &context)
{
 context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(360)
    .assertPrimaryChannels(1)
    .assertSecondaryChannels(1)
    .assertFuelChannels({true, true, false, false, false, false, false, false},
	                      {0,0,0,0,0,0,0,0});
}

static init_context_t setup1_cylinder_2stroke(void)
{
  init_context_t context;
  context.setup_2stroke(1);
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
  assert_injlayout(INJ_PAIRED, context);
}

static void cylinder1_stroke2_seq_staged(void)
{
  auto context = setup1_cylinder_2stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.enableStaging();

  context.initialise();
	
  assert_1channel_1stage_over360(__LINE__, context);
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void cylinder1_stroke2_semiseq_staged(void)
{
  auto context = setup1_cylinder_2stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.enableStaging();

  context.initialise();

  assert_1channel_1stage_over360(__LINE__, context);
  assert_injlayout(INJ_PAIRED, context);
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
  context.setup_4stroke(2);
  context.page2.divider = 1;
  return context;
}

static void assert_2channel_0stage_over720(int assertLineNum, const init_context_t &context, const uint16_t (&angles)[8])
{
 context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(720)
    .assertPrimaryChannels(2)
    .assertSecondaryChannels(0)
    .assertFuelChannels({true, true, false, false, false, false, false, false}, angles);
}

static void cylinder2_stroke4_seq_nostage(void)
{
  auto context = setup2_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page10.stagingEnabled = false;

  context.initialise();

  assert_2channel_0stage_over720(__LINE__, context, {0,180,0,0,0,0,0,0});
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void assert_2chennel_0stage_over360(int assertLineNum, const init_context_t &context)
{
 context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(360)
    .assertPrimaryChannels(2)
    .assertSecondaryChannels(0)
    .assertFuelChannels({true, true, false, false, false, false, false, false},
	                      {0,180,0,0,0,0,0,0});
}

static void cylinder2_stroke4_semiseq_nostage(void)
{
  auto context = setup2_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page10.stagingEnabled = false;

  context.initialise();

  assert_2chennel_0stage_over360(__LINE__, context);
  assert_injlayout(INJ_PAIRED, context);
}

static void assert_2channel_2stage_over720(int assertLineNum, const init_context_t &context)
{
 context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(720)
    .assertPrimaryChannels(2)
    .assertSecondaryChannels(2)
    .assertFuelChannels({true, true, true, true, false, false, false, false},
	                      {0,180,0,360,0,0,0,0});
}

static void cylinder2_stroke4_seq_staged(void)
{
  auto context = setup2_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.enableStaging();

  context.initialise();

  assert_2channel_2stage_over720(__LINE__, context);
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void assert_2channel_2staged_over360(int assertLineNum, const init_context_t &context)
{
 context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(360)
    .assertPrimaryChannels(2)
    .assertSecondaryChannels(2)
    .assertFuelChannels({true, true, true, true, false, false, false, false},
	                      {0,180,0,180,0,0,0,0});
}

static void cylinder2_stroke4_semiseq_staged(void)
{
  auto context = setup2_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.enableStaging();

  context.initialise();
  
  assert_2channel_2staged_over360(__LINE__, context);
  assert_injlayout(INJ_PAIRED, context);
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
  context.setup_2stroke(2);
  context.page2.divider = 1;
  return context;
} 

static void assert_2channel_0stage_over180(int assertLineNum, const init_context_t &context)
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(180)
    .assertPrimaryChannels(2)
    .assertSecondaryChannels(0)
    .assertFuelChannels({true, true, false, false, false, false, false, false},
	                      {0, (uint16_t)(context.page2.nCylinders==2U ? 0U : 90U),0,0,0,0,0,0});
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
  assert_injlayout(INJ_PAIRED, context);
}

static void assert_2channel_2stage_over180(int assertLineNum, const init_context_t &context)
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(180)
    .assertPrimaryChannels(2)
    .assertSecondaryChannels(2)
    .assertFuelChannels({true, true, true, true, false, false, false, false},
	                      {0,(uint16_t)(context.page2.nCylinders==2U ? 0U : 90U),0,90,0,0,0,0});
}

static void cylinder2_stroke2_seq_staged(void)
{
  auto context = setup_2_cylinder_2stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.enableStaging();
  
  context.initialise();
	  
  assert_2channel_2stage_over180(__LINE__, context);
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void cylinder2_stroke2_semiseq_staged(void)
{
  auto context = setup_2_cylinder_2stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.enableStaging();
 
  context.initialise();
  
  assert_2channel_2stage_over180(__LINE__, context);
  assert_injlayout(INJ_PAIRED, context);
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
  context.setup_4stroke(3);
  context.page2.divider = 1;
  return context;
}

static void assert_3channel_0stage_over720(int assertLineNum, const init_context_t &context, const uint16_t (&angles)[8])
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(720)
    .assertPrimaryChannels(3)
    .assertSecondaryChannels(0)
    .assertFuelChannels({true, INJ_CHANNELS>=2, INJ_CHANNELS>=3, false, false, false, false, false}, angles);
}

static void cylinder3_stroke4_seq_nostage(void)
{
  auto context = setup_3_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page10.stagingEnabled = false;

  context.initialise();

  assert_3channel_0stage_over720(__LINE__, context, {0,240,480,0,0,0,0,0});
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void assert_3channel_0stage_over240(int assertLineNum, const init_context_t &context)
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(720U/3U)
    .assertPrimaryChannels(3)
    .assertSecondaryChannels(0)
    .assertFuelChannels({true, INJ_CHANNELS>=2, INJ_CHANNELS>=3, false, false, false, false, false},
	                      {0,80,160,0,0,0,0,0});
}

static void cylinder3_stroke4_semiseq_nostage_tb(void)
{
  auto context = setup_3_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page10.stagingEnabled = false;
  context.page2.injType = INJ_TYPE_TBODY;

  context.initialise();

  assert_3channel_0stage_over240(__LINE__, context);
  assert_injlayout(INJ_PAIRED, context);
}

static void assert_3channel_0stage_over360(int assertLineNum, const init_context_t &context, const uint16_t (&angles)[8])
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(360)
    .assertPrimaryChannels(3)
    .assertSecondaryChannels(0)
    .assertFuelChannels({true, INJ_CHANNELS>=2, INJ_CHANNELS>=3, false, false, false, false, false}, angles);
}

static void cylinder3_stroke4_semiseq_nostage_port(void)
{
  auto context = setup_3_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page10.stagingEnabled = false;
  context.page2.injType = INJ_TYPE_PORT;

  context.initialise();

  assert_3channel_0stage_over360(__LINE__, context, {0,120,240,0,0,0,0,0});
  assert_injlayout(INJ_PAIRED, context);
}

static void assert_3channel_3stage_over720(int assertLineNum, const init_context_t &context)
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(720)
    .assertPrimaryChannels(3)
    .assertSecondaryChannels(INJ_CHANNELS>=6 ? 3 : 1)
    .assertFuelChannels({true, INJ_CHANNELS>=2, INJ_CHANNELS>=3, INJ_CHANNELS>=4, INJ_CHANNELS>=6, INJ_CHANNELS>=6, false, false},
	                      {0,240,480,0,240,480,0,0});
}

static void cylinder3_stroke4_seq_staged(void)
{
  auto context = setup_3_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.enableStaging();

  context.initialise();

	assert_3channel_3stage_over720(__LINE__, context);
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void assert_3channel_3stage_over240(int assertLineNum, const init_context_t &context)
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(240)
    .assertPrimaryChannels(3)
    .assertSecondaryChannels(INJ_CHANNELS>=6 ? 3 : 1)
    .assertFuelChannels({true, INJ_CHANNELS>=3, INJ_CHANNELS>=3, INJ_CHANNELS>=4, INJ_CHANNELS>=6, INJ_CHANNELS>=6, false, false},
	                      {0,80,160,0,80,160,0,0});
}

static void cylinder3_stroke4_semiseq_staged_tb(void)
{
  auto context = setup_3_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.enableStaging();
  context.page2.injType = INJ_TYPE_TBODY;
   context.initialise();

  assert_3channel_3stage_over240(__LINE__, context);
  assert_injlayout(INJ_PAIRED, context);
}

static void assert_3channel_3stage_over360(int assertLineNum, const init_context_t &context)
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(360)
    .assertPrimaryChannels(3)
    .assertSecondaryChannels(INJ_CHANNELS>=6 ? 3 : 1)
    .assertFuelChannels({true, INJ_CHANNELS>=3, INJ_CHANNELS>=3, INJ_CHANNELS>=4, INJ_CHANNELS>=6, INJ_CHANNELS>=6, false, false},
	                      {0,120,240,0,120,240,0,0});
}

static void cylinder3_stroke4_semiseq_staged_port(void)
{
  auto context = setup_3_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.enableStaging();
  context.page2.injType = INJ_TYPE_PORT;

  context.initialise();

  assert_3channel_3stage_over360(__LINE__, context);
  assert_injlayout(INJ_PAIRED, context);
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
  context.setup_2stroke(3);
  context.page2.divider = 1;
  return context;
}

static void cylinder3_stroke2_seq_nostage(void)
{
  auto context = setup_3_cylinder_2stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page10.stagingEnabled = false;
  context.initialise();

  assert_3channel_0stage_over360(__LINE__, context, {0,120,240,0,0,0,0,0});
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void assert_3channel_0stage_over120(int assertLineNum, const init_context_t &context)
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(120)
    .assertPrimaryChannels(3)
    .assertSecondaryChannels(0)
    .assertFuelChannels({true, true, true, false, false, false, false, false},
	                      {0,40,80,0,0,0,0,0});
}

static void cylinder3_stroke2_semiseq_nostage_tb(void)
{
  auto context = setup_3_cylinder_2stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page10.stagingEnabled = false;
  context.page2.injType = INJ_TYPE_TBODY;

  context.initialise();

  assert_3channel_0stage_over120(__LINE__, context);
  assert_injlayout(INJ_PAIRED, context);
}

static void assert_3channel_0stage_over180(int assertLineNum, const init_context_t &context)
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(180)
    .assertPrimaryChannels(3)
    .assertSecondaryChannels(0)
    .assertFuelChannels({true, true, true, false, false, false, false, false},
	                      {0,60,120,0,0,0,0,0});
}

static void cylinder3_stroke2_semiseq_nostage_port(void)
{
  auto context = setup_3_cylinder_2stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page10.stagingEnabled = false;
  context.page2.injType = INJ_TYPE_PORT;

  context.initialise();

  assert_3channel_0stage_over180(__LINE__, context);
  assert_injlayout(INJ_PAIRED, context);
}

static void cylinder3_stroke2_seq_staged(void)
{
  auto context = setup_3_cylinder_2stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.enableStaging();
  
  context.initialise();

	assert_3channel_3stage_over360(__LINE__, context);
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void assert_3channel_3stage_over120(int assertLineNum, const init_context_t &context)
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(120)
    .assertPrimaryChannels(3)
    .assertSecondaryChannels(INJ_CHANNELS>=6 ? 3 : 1)
    .assertFuelChannels({true, true, true, INJ_CHANNELS>=4, INJ_CHANNELS>=6, INJ_CHANNELS>=6, false, false},
	                      {0,40,80,0,40,80,0,0});
}

static void cylinder3_stroke2_semiseq_staged_tb(void)
{
  auto context = setup_3_cylinder_2stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.enableStaging();
  context.page2.injType = INJ_TYPE_TBODY;
  
  context.initialise();

	assert_3channel_3stage_over120(__LINE__, context);
  assert_injlayout(INJ_PAIRED, context);
}


static void assert_3channel_3stage_over180(int assertLineNum, const init_context_t &context)
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(180)
    .assertPrimaryChannels(3)
    .assertSecondaryChannels(INJ_CHANNELS>=6 ? 3 : 1)
    .assertFuelChannels({true, true, true, INJ_CHANNELS>=4, INJ_CHANNELS>=6, INJ_CHANNELS>=6, false, false},
	                      {0,60,120,0,60,120,0,0});
}

static void cylinder3_stroke2_semiseq_staged_port(void)
{
  auto context = setup_3_cylinder_2stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.enableStaging();
  context.page2.injType = INJ_TYPE_PORT;

  context.initialise();

	assert_3channel_3stage_over180(__LINE__, context);
  assert_injlayout(INJ_PAIRED, context);
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
  context.setup_4stroke(4);
  context.page2.divider = 2;
  return context;
}

static void assert_4channel_0stage_over720(int assertLineNum, const init_context_t &context, const uint16_t (&angles)[8])
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(720)
    .assertPrimaryChannels(4)
    .assertSecondaryChannels(0)
    .assertFuelChannels({true, true, true, true, false, false, false, false}, angles);
}

static void assert_2channel_0stage_over360(int assertLineNum, const init_context_t &context)
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(360)
    .assertPrimaryChannels(2)
    .assertSecondaryChannels(0)
    .assertFuelChannels({true, true, false, false, false, false, false, false},
	                      {0,180,0,0,0,0,0,0});
}

static void cylinder4_stroke4_seq_nostage(void)
{
  auto context = setup_4_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page10.stagingEnabled = false;
 
  context.initialise();
 
  assert_4channel_0stage_over720(__LINE__, context, {0,180,360,540,0,0,0,0});
}

static void assert_4channel_4stage_over720(int assertLineNum, const init_context_t &context)
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(720)
    .assertPrimaryChannels(4)
    .assertSecondaryChannels(INJ_CHANNELS>=8 ? 4 : 0)
    .assertFuelChannels({true, true, true, true, INJ_CHANNELS>=5, INJ_CHANNELS>=8, INJ_CHANNELS>=8, INJ_CHANNELS>=8},
	                      {0,180,360,540,0,180,360,540});
}

static void cylinder4_stroke4_seq_staged(void)
{
  auto context = setup_4_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.enableStaging();

  context.initialise();
	
  assert_4channel_4stage_over720(__LINE__, context);
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void cylinder4_stroke4_paired_nostage(void)  
{
  auto context = setup_4_cylinder_4stroke();
  context.page2.injLayout = INJ_PAIRED;
  context.page10.stagingEnabled = false;

  context.initialise();

  assert_2channel_0stage_over360(__LINE__, context);
  assert_injlayout(INJ_PAIRED, context);
}

static void cylinder4_stroke4_paired_staged(void)  
{
  auto context = setup_4_cylinder_4stroke();
  context.page2.injLayout = INJ_PAIRED;
  context.enableStaging();

  context.initialise();
  
  assert_2channel_2staged_over360(__LINE__, context);
  assert_injlayout(INJ_PAIRED, context);
}

static void cylinder4_stroke4_semiseq_nostage(uint8_t pairMode)
{
  auto context = setup_4_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page4.inj4cylPairing = pairMode;
  context.page10.stagingEnabled = false;
  
  context.initialise();
  
  assert_2channel_0stage_over360(__LINE__, context);
  assert_injlayout(INJ_SEMISEQUENTIAL, context);
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
  context.enableStaging();
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
  context.setup_2stroke(4);
  context.page2.divider = 2;
  return context;
}

static void assert_4channel_0stage_over180(int assertLineNum, const init_context_t &context)
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(180)
    .assertPrimaryChannels(4)
    .assertSecondaryChannels(0)
    .assertFuelChannels({true, true, true, true, false, false, false, false},
	                      {0,45,90,135,0,0,0,0});
}

static void cylinder4_stroke2_seq_nostage(void)
{
  auto context = setup_4_cylinder_2stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page10.stagingEnabled = false;
  
  context.initialise();

  assert_4channel_0stage_over180(__LINE__, context);
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void cylinder4_stroke2_semiseq_nostage(void)
{
  auto context = setup_4_cylinder_2stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page10.stagingEnabled = false;

  context.initialise();

  assert_2channel_0stage_over180(__LINE__, context);
  assert_injlayout(INJ_SEMISEQUENTIAL, context);
}

static void assert_4channel_4stage_over180(int assertLineNum, const init_context_t &context)
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(180)
    .assertPrimaryChannels(4)
    .assertSecondaryChannels(INJ_CHANNELS>=5 ? 4 : 0)
    .assertFuelChannels({true, INJ_CHANNELS>=2, INJ_CHANNELS>=3, INJ_CHANNELS>=4, INJ_CHANNELS>=5, INJ_CHANNELS>=6, INJ_CHANNELS>=7, INJ_CHANNELS>=8},
	                      {0,45,90,135,0,45,90,135});
}

static void cylinder4_stroke2_seq_staged(void)
{
  auto context = setup_4_cylinder_2stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.enableStaging();
  
  context.initialise();
	
  assert_4channel_4stage_over180(__LINE__, context);
  assert_injlayout(INJ_SEQUENTIAL, context);
}

static void cylinder4_stroke2_semiseq_staged(void)
{
  auto context = setup_4_cylinder_2stroke();
  context.page2.injLayout = INJ_PAIRED;
  context.enableStaging();

  context.initialise();
	
  assert_2channel_2stage_over180(__LINE__, context);
  assert_injlayout(INJ_PAIRED, context);
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
  context.setup_4stroke(5);
  return context;
}

static void assert_5channel_0stage_over720(int assertLineNum, const init_context_t &context, const uint16_t (&angles)[8])
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(720)
    .assertPrimaryChannels(INJ_CHANNELS>=5 ? 5 : INJ_CHANNELS)
    .assertSecondaryChannels(0)
    .assertFuelChannels({true, true, true, true, INJ_CHANNELS>=5, false, false, false}, angles);
}

static void assert_5channel_0stage_over720_default_angles(int assertLineNum, const init_context_t &context)
{
  assert_5channel_0stage_over720(assertLineNum, context,
#if INJ_CHANNELS>=5
    {0,144U,288U,432U,576U,0,0,0});
#else
    {0,72U,144U,216U,288U,0,0,0});
#endif
}

static void cylinder5_stroke4_seq_nostage(void)
{
  auto context = setup_5_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page10.stagingEnabled = false;

  context.initialise();
  
  assert_5channel_0stage_over720_default_angles(__LINE__, context);
  assert_injlayout(INJ_CHANNELS>=5 ? INJ_SEQUENTIAL : INJ_PAIRED, context);
}

static void cylinder5_stroke4_semiseq_nostage(void)
{
  auto context = setup_5_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page10.stagingEnabled = false;

  context.initialise();
  
  assert_5channel_0stage_over720(__LINE__, context, {0,72U,144U,216U,288U,0,0,0});
  assert_injlayout(INJ_CHANNELS>=5 ? INJ_SEMISEQUENTIAL : INJ_PAIRED, context);
}

static void assert_5channel_1stage_over720(int assertLineNum, const init_context_t &context, const uint16_t (&angles)[8])
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(720)
    .assertPrimaryChannels(INJ_CHANNELS>=5 ? 5 : 4)
    .assertSecondaryChannels(INJ_CHANNELS>=6 ? 1 : 0)
    .assertFuelChannels({true, true, true, true, INJ_CHANNELS>=5, INJ_CHANNELS>=6, false, false}, angles);
}

static void cylinder5_stroke4_seq_staged(void)
{
  auto context = setup_5_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.enableStaging();

  context.initialise();
 
  assert_5channel_1stage_over720(__LINE__, context, 
#if INJ_CHANNELS>=5
    {0,144,288,432,576,0,0,0}
#else
    {0,72,144,216,0,0,0,0}
#endif
);
  assert_injlayout(INJ_CHANNELS>=5 ? INJ_SEQUENTIAL : INJ_PAIRED, context);
}

static void cylinder5_stroke4_semiseq_staged(void)
{
  auto context = setup_5_cylinder_4stroke();
  context.page2.injLayout = INJ_PAIRED;
  context.enableStaging();

  context.initialise();

  assert_5channel_1stage_over720(__LINE__, context, {0,72,144,216,288,360,0,0});
  assert_injlayout(INJ_PAIRED, context);
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
  context.setup_4stroke(6);
  return context;
}

static void assert_6channel_0stage_over720(int assertLineNum, const init_context_t &context)
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(720)
    .assertPrimaryChannels(INJ_CHANNELS>=6 ? 6 : 3)
    .assertSecondaryChannels(0)
    .assertFuelChannels({true, true, true, INJ_CHANNELS>=6, INJ_CHANNELS>=6, INJ_CHANNELS>=6, false, false},
	                      {0,120,240,360,480,600,0,0});
}

static void cylinder6_stroke4_seq_nostage(void)
{
  auto context = setup_6_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page10.stagingEnabled = false;

  context.initialise();

  assert_6channel_0stage_over720(__LINE__, context);
  assert_injlayout(INJ_CHANNELS>=6 ? INJ_SEQUENTIAL : INJ_PAIRED, context);
}

static void cylinder6_stroke4_semiseq_nostage(void)
{
  auto context = setup_6_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.page10.stagingEnabled = false;

  context.initialise();
	
  assert_3channel_0stage_over720(__LINE__, context, {0,120,240,0,0,0,0,0});
  assert_injlayout(INJ_CHANNELS>=5 ? INJ_SEMISEQUENTIAL : INJ_PAIRED, context);
}

static void cylinder6_stroke4_seq_staged(void)
{
  auto context = setup_6_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.enableStaging();

  context.initialise();

  assert_6channel_0stage_over720(__LINE__, context);
  assert_injlayout(INJ_CHANNELS>=6 ? INJ_SEQUENTIAL : INJ_PAIRED, context);
}

static void cylinder6_stroke4_semiseq_staged(void)
{
  auto context = setup_6_cylinder_4stroke();
  context.page2.injLayout = INJ_SEMISEQUENTIAL;
  context.enableStaging();
  
  context.initialise();

#if INJ_CHANNELS >= 6
  TEST_IGNORE_MESSAGE("Fix code so test passes :-()");
#else
  context
    .setAssertLine(__LINE__)
    .assertMaxCrank(720)
    .assertPrimaryChannels(3)
    .assertSecondaryChannels(INJ_CHANNELS >= 6 ? 3 : 0)
    .assertFuelChannels({true, true, true, false, false, false, false, false},
	                      {0,120,240,360,0,120,240,0});
#endif
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
  context.setup_4stroke(8);
  return context;
}

static void assert_8channel_0stage_over720(int assertLineNum, const init_context_t &context)
{
#if INJ_CHANNELS==4
  assert_4channel_0stage_over720(assertLineNum, context, {0,180,360,540,0,0,0,0});
#elif INJ_CHANNELS==8
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(720)
    .assertPrimaryChannels(8)
    .assertSecondaryChannels(0)
    .assertFuelChannels({true, true, true, true, INJ_CHANNELS>=8, INJ_CHANNELS>=8, INJ_CHANNELS>=8, INJ_CHANNELS>=8},
	                      {0,90,180,270,360,450,540,630});
#endif
}

static void cylinder8_stroke4_seq_nostage(void)
{
  auto context = setup_8_cylinder_4stroke();
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page10.stagingEnabled = false;

  context.initialise();

  assert_8channel_0stage_over720(__LINE__, context);
  assert_injlayout(INJ_CHANNELS>=8 ? INJ_SEQUENTIAL : INJ_PAIRED, context);
}

static void assert_4channel_0stage_over360(int assertLineNum, const init_context_t &context)
{
  context
    .setAssertLine(assertLineNum)
    .assertMaxCrank(360)
    .assertPrimaryChannels(4)
    .assertSecondaryChannels(0)
    .assertFuelChannels({true, true, true, true, false, false, false, false},
	                      {0,90,180,270,360,450,540,630});
}

static void cylinder8_stroke4_paired_nostage(void)
{
  auto context = setup_8_cylinder_4stroke();
  context.page2.divider = context.page2.nCylinders/2U;
  context.page2.injLayout = INJ_PAIRED;
  context.page10.stagingEnabled = false;

  context.initialise();
  
  assert_4channel_0stage_over360(__LINE__, context);
  assert_injlayout(INJ_PAIRED, context);
}

static void run_8_cylinder_4stroke_tests(void)
{
  // Staging not supported on 8 cylinders

  RUN_TEST_P(cylinder8_stroke4_seq_nostage);
  RUN_TEST_P(cylinder8_stroke4_paired_nostage);
}

static init_context_t setup_no_inj_timing(uint8_t nCylinders)
{
  init_context_t context;
  context.setup_4stroke(nCylinders);
  context.page2.injTiming = false;
  context.page2.injLayout = INJ_PAIRED;
  return context;
}

static constexpr uint16_t zeroAngles[] = {0,0,0,0,0,0,0,0};

static void cylinder_1_NoinjTiming_paired(void) {
  auto context = setup_no_inj_timing(1);

  context.initialise();

  assert_1channel_0stage_over720(__LINE__, context);
  assert_injlayout(INJ_PAIRED, context);
}

static void cylinder_2_NoinjTiming_paired(void) {
  auto context = setup_no_inj_timing(2);

  context.initialise();

  assert_2channel_0stage_over720(__LINE__, context, zeroAngles);
  assert_injlayout(INJ_PAIRED, context);
}

static void cylinder_3_NoinjTiming_paired(void) {
  auto context = setup_no_inj_timing(3);
  context.page2.injType = INJ_TYPE_PORT;

  context.initialise();

  assert_3channel_0stage_over360(__LINE__, context, zeroAngles);
  assert_injlayout(INJ_PAIRED, context);
}

static void cylinder_4_NoinjTiming_paired(void) {
  auto context = setup_no_inj_timing(4);

  context.initialise();

  assert_2channel_0stage_over720(__LINE__, context, zeroAngles);
  assert_injlayout(INJ_PAIRED, context);
}

static void cylinder_5_NoinjTiming_paired(void) {
  auto context = setup_no_inj_timing(5);

  context.initialise();

  assert_5channel_0stage_over720(__LINE__, context, zeroAngles);
  assert_injlayout(INJ_PAIRED, context);
 }

static void cylinder_6_NoinjTiming_paired(void) {
  auto context = setup_no_inj_timing(6);

  context.initialise();

  assert_3channel_0stage_over720(__LINE__, context, zeroAngles);
  assert_injlayout(INJ_PAIRED, context);
}

static void cylinder_8_NoinjTiming_paired(void) {
  auto context = setup_no_inj_timing(8);

  context.initialise();

  assert_4channel_0stage_over720(__LINE__, context, zeroAngles);
  assert_injlayout(INJ_PAIRED, context);
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
  context.setup_4stroke(nCylinders);
  context.page2.engineType = ODD_FIRE;
  context.page2.injLayout = INJ_SEQUENTIAL;
  context.page2.oddfire2 = 13;
  context.page2.oddfire3 = 111;
  context.page2.oddfire4 = 217;
  return context;
}

static void cylinder_2_oddfire(void)
{
  auto context = setup_oddfire(2);

  context.initialise();

  assert_2channel_0stage_over720(__LINE__, context, {0,13,0,0,0,0,0,0});
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
  assert_3channel_0stage_over720(__LINE__, context, {0,240,480,0,0,0,0,0});
}

static void cylinder_4_oddfire(void)
{
  auto context = setup_oddfire(4);
  context.initialise();
  assert_4channel_0stage_over720(__LINE__, context, {0,180,360,540,0,0,0,0});
}

static void cylinder_5_oddfire(void)
{
  auto context = setup_oddfire(5);
  context.initialise();
  assert_5channel_0stage_over720_default_angles(__LINE__, context);
}

static void cylinder_6_oddfire(void)
{
  auto context = setup_oddfire(6);
  context.initialise();
  assert_6channel_0stage_over720(__LINE__, context);
}

static void cylinder_8_oddfire(void)
{
  auto context = setup_oddfire(8);
  context.initialise();
  assert_8channel_0stage_over720(__LINE__, context);
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
  context.setup_4stroke(cylinders);
  context.page2.injLayout = INJ_SEQUENTIAL;
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
  context.matchFuelSchedulersToSyncState();
  // Confirm no change
  assert_1channel_0stage_over720(__LINE__, context);

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  context.matchFuelSchedulersToSyncState();
  assert_1channel_0stage_over720(__LINE__, context);
}

static void test_partial_sync_2_cylinder(void)
{
  auto context = setupPartialSyncTest(2);

  // Confirm initial state
  assert_2channel_0stage_over720(__LINE__, context, {0,180,0,0,0,0,0,0});

  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  context.matchFuelSchedulersToSyncState();
  // Confirm no change
  assert_2channel_0stage_over720(__LINE__, context, {0,180,0,0,0,0,0,0});

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  context.matchFuelSchedulersToSyncState();
  assert_2channel_0stage_over720(__LINE__, context, {0,180,0,0,0,0,0,0});
}

static void test_partial_sync_3_cylinder(void)
{
  auto context = setupPartialSyncTest(3);

  // Confirm initial state
  assert_3channel_0stage_over720(__LINE__, context, {0,240,480,0,0,0,0,0});

  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  context.matchFuelSchedulersToSyncState();
  // Confirm no change
  assert_3channel_0stage_over720(__LINE__, context, {0,240,480,0,0,0,0,0});

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  context.matchFuelSchedulersToSyncState();
  assert_3channel_0stage_over720(__LINE__, context, {0,240,480,0,0,0,0,0});
}

static void test_partial_sync_4_cylinder(void)
{
  auto context = setupPartialSyncTest(4);
  context.page2.divider = context.page2.nCylinders / 2U;

  // Confirm initial state
  assert_4channel_0stage_over720(__LINE__, context, {0,180,360,540,0,0,0,0});

  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  context.matchFuelSchedulersToSyncState();
  assert_2channel_0stage_over360(__LINE__, context);
  TEST_ASSERT_EQUAL(INJ_SEMISEQUENTIAL, context.current.injLayout);

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  context.matchFuelSchedulersToSyncState();
  assert_4channel_0stage_over720(__LINE__, context, {0,180,360,540,0,0,0,0});
  TEST_ASSERT_EQUAL(INJ_SEQUENTIAL, context.current.injLayout);
}

static void test_partial_sync_5_cylinder(void)
{
  auto context = setupPartialSyncTest(5);

  // Confirm initial state
  assert_5channel_0stage_over720_default_angles(__LINE__, context);

  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  context.matchFuelSchedulersToSyncState();
  // Confirm no change
  assert_5channel_0stage_over720_default_angles(__LINE__, context);

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  context.matchFuelSchedulersToSyncState();
  // Confirm no change
  assert_5channel_0stage_over720_default_angles(__LINE__, context);
}

static void test_partial_sync_6_cylinder(void)
{
#if INJ_CHANNELS>=6
  auto context = setupPartialSyncTest(6);
  context.page2.divider = context.page2.nCylinders / 2U;

  // Confirm initial state
  assert_6channel_0stage_over720(__LINE__, context);

  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  context.matchFuelSchedulersToSyncState();
  assert_3channel_0stage_over360(__LINE__, context, {0,120,240,0,0,0,0,0});

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  context.matchFuelSchedulersToSyncState();
  assert_6channel_0stage_over720(__LINE__, context);
#else
  TEST_IGNORE_MESSAGE("Skipping - not enough injectors");
#endif
}

static void test_partial_sync_8_cylinder(void)
{
#if INJ_CHANNELS>=8
  auto context = setupPartialSyncTest(8);
  context.page2.divider = context.page2.nCylinders / 2U;

  // Confirm initial state
  assert_8channel_0stage_over720(__LINE__, context);

  fakeDecoderStatus.syncStatus = SyncStatus::Partial;
  context.matchFuelSchedulersToSyncState();
  assert_4channel_0stage_over360(__LINE__, context);
  TEST_ASSERT_EQUAL(INJ_SEMISEQUENTIAL, context.current.injLayout);

  fakeDecoderStatus.syncStatus = SyncStatus::Full;
  context.matchFuelSchedulersToSyncState();
  assert_8channel_0stage_over720(__LINE__, context);
  // Deliberate repeat
  context.matchFuelSchedulersToSyncState();
  assert_8channel_0stage_over720(__LINE__, context);
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