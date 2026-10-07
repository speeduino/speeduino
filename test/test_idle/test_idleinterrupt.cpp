#include "../test_utils.h"
#include "src/controllers/idle/idle.h"
#include "src/controllers/idle/idleController_state.h"
#include "maths.h"
#include "units.h"
#include "context.h"

extern idleController::detail::state_t _idleState;

static void test_normalDirection(uint8_t numChannels)
{
    context_t context;

    context.page6.iacPWMdir = 0;
    context.page6.iacChannels = numChannels;

    _idleState.idle_pwm_state = true;
    _idleState.idle_pin.setPinHigh();
    _idleState.idle2_pin.setPinLow();

    idleInterrupt();
    TEST_ASSERT_FALSE(_idleState.idle_pin._pin.isPinHigh());
    TEST_ASSERT_FALSE(numChannels == 1 ? _idleState.idle2_pin._pin.isPinLow() : false);
    TEST_ASSERT_FALSE(_idleState.idle_pwm_state);

    idleInterrupt();
    TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinHigh());
    TEST_ASSERT_TRUE(numChannels==0 || _idleState.idle2_pin._pin.isPinLow());
    TEST_ASSERT_TRUE(_idleState.idle_pwm_state);
}

static void test_normalDirection(void)
{
    test_normalDirection(0);
    test_normalDirection(1);
}

static void test_reverseDirection(uint8_t numChannels)
{
    context_t context;

    context.page6.iacPWMdir = 1;
    context.page6.iacChannels = numChannels;

    _idleState.idle_pwm_state = true;
    _idleState.idle_pin.setPinLow();
    _idleState.idle2_pin.setPinHigh();

    idleInterrupt();
    TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinHigh());
    TEST_ASSERT_TRUE(numChannels==0 || _idleState.idle2_pin._pin.isPinLow());
    TEST_ASSERT_FALSE(_idleState.idle_pwm_state);

    idleInterrupt();
    TEST_ASSERT_FALSE(_idleState.idle_pin._pin.isPinHigh());
    TEST_ASSERT_FALSE(numChannels==1 ? _idleState.idle2_pin._pin.isPinLow() : false);
    TEST_ASSERT_TRUE(_idleState.idle_pwm_state);
}

static void test_reverseDirection(void)
{
    test_reverseDirection(0);
    test_reverseDirection(1);
}

void testIdleInterrupt(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_normalDirection);
  RUN_TEST_P(test_reverseDirection);
}
