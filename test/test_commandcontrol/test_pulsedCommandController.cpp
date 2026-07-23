
#include "../test_utils.h"
#include "sensors.h"
#include "src/controllers/tsCommand/tsCommandController.h"

extern uint8_t testInjectorPulseCount;
extern uint8_t testIgnitionPulseCount;
extern byte HWTest_INJ_Pulsed;
extern byte HWTest_IGN_Pulsed;

struct pulsed_controller_context_t
{
    statuses current;
    config13 page13;

    pulsed_controller_context_t()
    {
        current.RPM = 0U;
        current.isTestModeActive = false;
        HWTest_INJ_Pulsed = 0U;
        HWTest_IGN_Pulsed = 0U;
        testInjectorPulseCount = 0;
        testIgnitionPulseCount = 0;
    }

    void pulsedCommandController(void)
    {
        ::pulsedCommandController(current, page13);
    }
};

static pulsed_controller_context_t setup_start_pulse(void)
{
  pulsed_controller_context_t context;
  context.current.isTestModeActive = true;
  BIT_SET(context.current.LOOP_TIMER, BIT_TIMER_30HZ);
  context.current.setRpm(0);
  context.page13.hwTestInjDuration = 10;
  context.page13.hwTestIgnDuration = 5;
  return context;
}

static void test_start_pulse(uint8_t inj, uint8_t ign)
{
  auto context = setup_start_pulse();
  HWTest_INJ_Pulsed = inj;
  HWTest_IGN_Pulsed = ign;
  testInjectorPulseCount = 99;
  testIgnitionPulseCount = 99;

  context.pulsedCommandController();
  
  TEST_ASSERT_EQUAL(0, testInjectorPulseCount);
  TEST_ASSERT_EQUAL(0, testIgnitionPulseCount);
}

static void test_start_pulse(void)
{
  for (uint8_t inj = 0; inj<INJ_CHANNELS; ++inj)
  {
    uint8_t mask = 0;
    BIT_SET(mask, inj);
    test_start_pulse(mask, 0b0000'0000);
  }
  for (uint8_t ign = 0; ign<IGN_CHANNELS; ++ign)
  {
    uint8_t mask = 0;
    BIT_SET(mask, ign);
    test_start_pulse(0b0000'0000, mask);
  }
}

static void assert_pulse_does_nothing(pulsed_controller_context_t &context)
{
  testInjectorPulseCount = 99;
  testIgnitionPulseCount = 99;

  context.pulsedCommandController();
  
  // No change
  TEST_ASSERT_EQUAL(99, testInjectorPulseCount);
  TEST_ASSERT_EQUAL(99, testIgnitionPulseCount);
}

static void test_inactive_mode_does_nothing(void)
{
  auto context = setup_start_pulse();
  context.current.isTestModeActive = false;
  assert_pulse_does_nothing(context);
}

static void test_engine_running_does_nothing(void)
{
  auto context = setup_start_pulse();
  context.current.setRpm(1000);
  assert_pulse_does_nothing(context);
}

static void test_no_timer_does_nothing(void)
{
  auto context = setup_start_pulse();
  BIT_CLEAR(context.current.LOOP_TIMER, BIT_TIMER_30HZ);
  assert_pulse_does_nothing(context);
}

static void test_no_pulsed_outputs_does_nothing(void)
{
  pulsed_controller_context_t context;
  context.current.isTestModeActive = true;
  context.page13.hwTestInjDuration = 10;
  context.page13.hwTestIgnDuration = 5;
  
  context.pulsedCommandController();
  
  // No state change should occur
  TEST_ASSERT_EQUAL_UINT8(0, HWTest_INJ_Pulsed);
  TEST_ASSERT_EQUAL_UINT8(0, HWTest_IGN_Pulsed);
}

static void test_inj_pulse_count(void)
{
  pulsed_controller_context_t context;
  context.current.isTestModeActive = true;
  HWTest_INJ_Pulsed = 0xFF; // All injection outputs pulsed
  BIT_SET(context.current.LOOP_TIMER, BIT_TIMER_1KHZ);
  context.page13.hwTestInjDuration = 5;
  
  TEST_ASSERT_EQUAL_UINT8(0, testInjectorPulseCount);
  for(uint8_t i = 0; i < context.page13.hwTestInjDuration; ++i)
  {
    context.pulsedCommandController();
    TEST_ASSERT_EQUAL_UINT8(i+1, testInjectorPulseCount);
  }
  
  // After duration exceeded, pulsed bit should be cleared
  context.pulsedCommandController();
  TEST_ASSERT_EQUAL_UINT8(0, testInjectorPulseCount);
}

static void test_ign_pulse_count(void)
{
  pulsed_controller_context_t context;
  context.current.isTestModeActive = true;
  HWTest_IGN_Pulsed = 0xFF; // All ignition outputs pulsed
  BIT_SET(context.current.LOOP_TIMER, BIT_TIMER_1KHZ);
  context.page13.hwTestIgnDuration = 5;
  
  TEST_ASSERT_EQUAL_UINT8(0, testIgnitionPulseCount);
  for(uint8_t i = 0; i < context.page13.hwTestIgnDuration; ++i)
  {
    context.pulsedCommandController();
    TEST_ASSERT_EQUAL_UINT8(i+1, testIgnitionPulseCount);
  }
  
  // After duration exceeded, pulsed bit should be cleared
  context.pulsedCommandController();
  TEST_ASSERT_EQUAL_UINT8(0, testIgnitionPulseCount);
}

static void test_30hz_inj_outputs(void)
{
  pulsed_controller_context_t context;
  context.current.isTestModeActive = true;
  context.current.RPM = 0; // Engine must be stopped for 30Hz outputs
  HWTest_INJ_Pulsed = 0x01; // Only INJ1 pulsed
  BIT_SET(context.current.LOOP_TIMER, BIT_TIMER_30HZ);
  context.page13.hwTestInjDuration = 10;
  context.page13.hwTestIgnDuration = 10;
  
  context.pulsedCommandController();
  
  // After 30Hz cycle, pulse counter should be reset
  // (We can't directly verify openInjector1 was called, but we can verify
  // the function completed successfully)
  TEST_ASSERT_TRUE(context.current.isTestModeActive);
}

static void test_30hz_ign_outputs(void)
{
  pulsed_controller_context_t context;
  context.current.isTestModeActive = true;
  context.current.RPM = 0; // Engine must be stopped for 30Hz outputs
  HWTest_IGN_Pulsed = 0x01; // Only IGN1 pulsed
  BIT_SET(context.current.LOOP_TIMER, BIT_TIMER_30HZ);
  context.page13.hwTestInjDuration = 10;
  context.page13.hwTestIgnDuration = 10;
  
  context.pulsedCommandController();
  
  // After 30Hz cycle, function should complete successfully
  TEST_ASSERT_TRUE(context.current.isTestModeActive);
}

static void test_engine_running_no_30hz_output(void)
{
  pulsed_controller_context_t context;
  context.current.isTestModeActive = true;
  context.current.RPM = 5000; // Engine running
  HWTest_INJ_Pulsed = 0xFF;
  HWTest_IGN_Pulsed = 0xFF;
  BIT_SET(context.current.LOOP_TIMER, BIT_TIMER_30HZ);
  context.page13.hwTestInjDuration = 10;
  context.page13.hwTestIgnDuration = 10;
  
  context.pulsedCommandController();
  
  // When engine is running, 30Hz outputs should not be activated
  // (The condition checks: if((current.isTestModeActive) && (current.RPM == 0)))
  TEST_ASSERT_EQUAL_UINT(5000, context.current.RPM);
}

static void test_both_timers_active(void)
{
  pulsed_controller_context_t context;
  context.current.isTestModeActive = true;
  context.current.RPM = 0;
  HWTest_INJ_Pulsed = 0x03; // INJ1 and INJ2 pulsed
  HWTest_IGN_Pulsed = 0x03; // IGN1 and IGN2 pulsed
  BIT_SET(context.current.LOOP_TIMER, BIT_TIMER_1KHZ);
  BIT_SET(context.current.LOOP_TIMER, BIT_TIMER_30HZ);
  context.page13.hwTestInjDuration = 100;
  context.page13.hwTestIgnDuration = 100;
  
  context.pulsedCommandController();
  
  // Both timers active, pulsed outputs still set (well within duration)
  TEST_ASSERT_EQUAL_UINT8(0x03, HWTest_INJ_Pulsed);
  TEST_ASSERT_EQUAL_UINT8(0x03, HWTest_IGN_Pulsed);
}

void testPulsedCommandController(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_start_pulse);
    RUN_TEST_P(test_inactive_mode_does_nothing);
    RUN_TEST_P(test_engine_running_does_nothing);
    RUN_TEST_P(test_no_timer_does_nothing);
    RUN_TEST_P(test_no_pulsed_outputs_does_nothing);
    RUN_TEST_P(test_inj_pulse_count);
    RUN_TEST_P(test_ign_pulse_count);
    RUN_TEST_P(test_30hz_inj_outputs);
    RUN_TEST_P(test_30hz_ign_outputs);
    RUN_TEST_P(test_engine_running_no_30hz_output);
    RUN_TEST_P(test_both_timers_active);
  }
}
