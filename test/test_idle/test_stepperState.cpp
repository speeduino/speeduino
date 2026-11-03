  #include "../test_utils.h"
#include "src/controllers/idle/stepper.h"
#include "context.h"

using namespace idleController::detail;

static void test_ctor_stepperdir_pin(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_ONOFF);
  {
    stepperState_t subject(context.pins, context.page9); 
    TEST_ASSERT_TRUE(subject.pinStepperDir.isValid());
    TEST_ASSERT_FALSE(subject.pinStepperDir._isInverted);
  }

  context.page9.iacStepperInv = true;
  {
    stepperState_t subject(context.pins, context.page9); 
    TEST_ASSERT_TRUE(subject.pinStepperDir.isValid());
    TEST_ASSERT_TRUE(subject.pinStepperDir._isInverted);
  }
}

void testStepperState(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_ctor_stepperdir_pin);
}