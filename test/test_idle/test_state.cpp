  #include "../test_utils.h"
#include "src/controllers/idle/idleController_state.h"
#include "context.h"

using namespace idleController::detail;

static void test_ctor_idle1_pin(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_ONOFF);
  {
    state_t subject(context.pins, context.page2, context.page6); 
    TEST_ASSERT_TRUE(subject.idle_pin.isValid());
    TEST_ASSERT_FALSE(subject.idle_pin._isInverted);
  }
  context.page6.iacPWMdir = true;
  {
    state_t subject(context.pins, context.page2, context.page6); 
    TEST_ASSERT_TRUE(subject.idle_pin.isValid());
    TEST_ASSERT_FALSE(subject.idle_pin._isInverted);
  }
  context.prepare_idle(IAC_ALGORITHM_PWM_CL);
  {
    state_t subject(context.pins, context.page2, context.page6); 
    TEST_ASSERT_TRUE(subject.idle_pin.isValid());
    TEST_ASSERT_TRUE(subject.idle_pin._isInverted);
  }
}

static void test_ctor_idle2_pin(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_ONOFF);
  {
    state_t subject(context.pins, context.page2, context.page6); 
    TEST_ASSERT_FALSE(subject.idle2_pin.isValid());
  }
  context.page6.iacChannels = 1;
  {
    state_t subject(context.pins, context.page2, context.page6); 
    TEST_ASSERT_TRUE(subject.idle2_pin.isValid());
    TEST_ASSERT_FALSE(subject.idle2_pin._isInverted);
  }
  context.page6.iacPWMdir = true;
  {
    state_t subject(context.pins, context.page2, context.page6); 
    TEST_ASSERT_TRUE(subject.idle2_pin.isValid());
    TEST_ASSERT_FALSE(subject.idle2_pin._isInverted);
  }
  context.prepare_idle(IAC_ALGORITHM_PWM_CL);
  {
    state_t subject(context.pins, context.page2, context.page6); 
    TEST_ASSERT_TRUE(subject.idle2_pin.isValid());
    TEST_ASSERT_TRUE(subject.idle2_pin._isInverted);
  }
}

static void test_ctor_idleupinput_pin(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_ONOFF);
  {
    state_t subject(context.pins, context.page2, context.page6); 
    TEST_ASSERT_FALSE(subject.pinIdleUpInput.isValid());
  }
  context.page2.idleUpEnabled = true;
  context.pins.pinFuelPump = context.pins.pinIdleUp;
  {
    state_t subject(context.pins, context.page2, context.page6); 
    TEST_ASSERT_FALSE(subject.pinIdleUpInput.isValid());
  }
  context.pins.pinFuelPump = NOT_A_PIN;
  {
    state_t subject(context.pins, context.page2, context.page6); 
    TEST_ASSERT_TRUE(subject.pinIdleUpInput.isValid());
  }
}

static void test_ctor_idleupoutput_pin(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_ONOFF);
  {
    state_t subject(context.pins, context.page2, context.page6); 
    TEST_ASSERT_FALSE(subject.pinIdleUpOutput.isValid());
  }
  context.page2.idleUpEnabled = true;
  {
    state_t subject(context.pins, context.page2, context.page6); 
    TEST_ASSERT_FALSE(subject.pinIdleUpOutput.isValid());
  }
  context.page2.idleUpOutputEnabled = true;  
  {
    state_t subject(context.pins, context.page2, context.page6); 
    TEST_ASSERT_TRUE(subject.pinIdleUpOutput.isValid());
    TEST_ASSERT_FALSE(subject.pinIdleUpOutput._isInverted);
  }
  context.page2.idleUpOutputInv = true;
  {
    state_t subject(context.pins, context.page2, context.page6); 
    TEST_ASSERT_TRUE(subject.pinIdleUpOutput.isValid());
    TEST_ASSERT_TRUE(subject.pinIdleUpOutput._isInverted);
  }
}

void testIdleControlState(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_ctor_idle1_pin);
  RUN_TEST_P(test_ctor_idle2_pin);
  RUN_TEST_P(test_ctor_idleupinput_pin);
  RUN_TEST_P(test_ctor_idleupoutput_pin);
}