#include "../test_utils.h"
#include "src/controllers/idle/idle.h"
#include "src/controllers/idle/idleController_state.h"
#include "context.h"

extern idleController::detail::state_t _idleState;

static void test_onOff(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_ONOFF);
  context.page6.iacFastTemp = 110; // 50C threshold
  context.init();

  context.current.coolant = 0;                            // Cold: below threshold -> idle ON

  idleControl();

  TEST_ASSERT_TRUE(_idleState.idleOn);
  TEST_ASSERT_TRUE(context.current.idleOn);
  TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinHigh());
  TEST_ASSERT_EQUAL_UINT8(100U, context.current.idleLoad);

  context.current.coolant = 80;                           // Warm: above threshold -> idle OFF
  idleControl();

  TEST_ASSERT_FALSE(_idleState.idleOn);
  TEST_ASSERT_FALSE(context.current.idleOn);
  TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinLow());
  TEST_ASSERT_EQUAL_UINT8(0U, context.current.idleLoad);
}

void testIdleControlOnOff(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_onOff);
}