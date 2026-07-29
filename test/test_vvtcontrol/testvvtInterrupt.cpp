#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "units.h"
#include "src/pins/boardOutputPin.h"
#include "src/controllers/vvt/VvtOutputChannel.h"

// External declarations for testing VVT PWM interrupt handler
extern uint16_t vvt_pwm_max_count;
extern boardOutputPin_t vvt1_pin;
extern boardOutputPin_t vvt2_pin;
extern VvtOutputChannel vvtChannel1;
extern VvtOutputChannel vvtChannel2;

// ========================= Setup and Helpers =========================

static void setup_vvt_interrupt_base(void)
{
  // Initialize pins
  pinNumbers.pinVVT_1 = 19U;
  pinNumbers.pinVVT_2 = 20U;
  
  // Initialize all PWM state variables
  vvtChannel1.targetDuty = 0;
  vvtChannel2.targetDuty = 0;
  vvtChannel1.compareTicks = 0;
  vvtChannel2.compareTicks = 0;
  vvtChannel1.pinState = false;
  vvtChannel2.pinState = false;
  vvtChannel1.periodTicks = false;
  vvtChannel2.periodTicks = false;
  
  // Set max count (typical PWM period in ticks)
  vvt_pwm_max_count = 1000;
  
  // Initialize pins through auxiliaries
  initialiseAuxPWM();
}

static void setup_vvt_interrupt_active_state(void)
{
  // Initialize base
  setup_vvt_interrupt_base();
  
  // Set up for active state: PWM outputs already running
  vvtChannel1.pinState = true;
  vvtChannel2.pinState = true;
  vvtChannel1.periodTicks = false;
  vvtChannel2.periodTicks = false;
  
  // Initialize current values (set by previous idle entry)
  vvtChannel1.compareTicks = 0;
  vvtChannel2.compareTicks = 0;
}

static bool getVvt1PinState(void)
{
  return vvt1_pin._pin.isPinHigh();
}

static bool getVvt2PinState(void)
{
  return vvt2_pin._pin.isPinHigh();
}

// ========================= Test: Both VVT outputs off (idle state) =========================

static void test_both_off_idle_state(void)
{
setup_vvt_interrupt_base();

// Both PWM values are zero (off), state is idle
vvtChannel1.targetDuty = 0;
vvtChannel2.targetDuty = 0;
vvtChannel1.pinState = false;
vvtChannel2.pinState = false;
vvtChannel1.periodTicks = false;
vvtChannel2.periodTicks = false;

vvtInterrupt();

// PWM states should remain false
TEST_ASSERT_FALSE(vvtChannel1.pinState);
TEST_ASSERT_FALSE(vvtChannel2.pinState);
}

// ========================= Test: VVT1 only at 50% duty =========================

static void test_vvt1_at_50_percent_duty(void)
{
    setup_vvt_interrupt_base();

    // Set VVT1 to 50% duty from idle state
    vvtChannel1.targetDuty = 500;
    vvtChannel2.targetDuty = 0;
    vvtChannel1.pinState = false;  // Idle state
    vvtChannel2.pinState = false;
    vvtChannel1.periodTicks = false;
    vvtChannel2.periodTicks = false;

    vvtInterrupt();

    // VVT1 should be turned on and state set to true
    TEST_ASSERT_TRUE(vvtChannel1.pinState);
    TEST_ASSERT_FALSE(vvtChannel2.pinState);

    // Pin state depends on board type
    #if defined(CORE_TEENSY41)
    TEST_ASSERT_FALSE(getVvt1PinState());  // Teensy41: pin LOW = on
    #else
    TEST_ASSERT_TRUE(getVvt1PinState());   // Standard: pin HIGH = on
    #endif
}

// ========================= Test: VVT2 only at 50% duty =========================

static void test_vvt2_at_50_percent_duty(void)
{
    setup_vvt_interrupt_base();
    
    // Set VVT2 to 50% duty, VVT1 off
    vvtChannel1.targetDuty = 0;
    vvtChannel2.targetDuty = 500;
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    vvtChannel1.periodTicks = false;
    vvtChannel2.periodTicks = false;
    
    vvtInterrupt();
    
    // VVT2 should be on, VVT1 should NOT activate (was at 0%)
    TEST_ASSERT_FALSE(vvtChannel1.pinState);
    TEST_ASSERT_TRUE(vvtChannel2.pinState);
    
    #if defined(CORE_TEENSY41)
    TEST_ASSERT_FALSE(getVvt2PinState());
    #else
    TEST_ASSERT_TRUE(getVvt2PinState());
    #endif
}

// ========================= Test: Both at different duty cycles =========================

static void test_both_on_different_duties(void)
{
    setup_vvt_interrupt_base();
    
    // VVT1 at 30%, VVT2 at 70%
    vvtChannel1.targetDuty = 300;
    vvtChannel2.targetDuty = 700;
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    vvtChannel1.periodTicks = false;
    vvtChannel2.periodTicks = false;
    
    vvtInterrupt();
    
    // Both should turn on
    #if defined(CORE_TEENSY41)
    TEST_ASSERT_FALSE(getVvt1PinState());
    TEST_ASSERT_FALSE(getVvt2PinState());
    #else
    TEST_ASSERT_TRUE(getVvt1PinState());
    TEST_ASSERT_TRUE(getVvt2PinState());
    #endif
    
    // Both PWM states should be true
    TEST_ASSERT_TRUE(vvtChannel1.pinState);
    TEST_ASSERT_TRUE(vvtChannel2.pinState);
}

// ========================= Test: Both at same duty cycle =========================

static void test_both_same_duty_cycle(void)
{
    setup_vvt_interrupt_base();
    
    // Both at 50% duty
    vvtChannel1.targetDuty = 500;
    vvtChannel2.targetDuty = 500;
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    vvtChannel1.periodTicks = false;
    vvtChannel2.periodTicks = false;
    
    vvtInterrupt();
    
    // Both should turn on
    #if defined(CORE_TEENSY41)
    TEST_ASSERT_FALSE(getVvt1PinState());
    TEST_ASSERT_FALSE(getVvt2PinState());
    #else
    TEST_ASSERT_TRUE(getVvt1PinState());
    TEST_ASSERT_TRUE(getVvt2PinState());
    #endif
    
    TEST_ASSERT_TRUE(vvtChannel1.pinState);
    TEST_ASSERT_TRUE(vvtChannel2.pinState);
}

// ========================= Test: VVT at 100% duty (always on) =========================

static void test_vvt1_at_100_percent_duty(void)
{
    setup_vvt_interrupt_base();
    
    // Set VVT1 to 100% duty (max)
    vvtChannel1.targetDuty = vvt_pwm_max_count;
    vvtChannel2.targetDuty = 0;
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    vvtChannel1.periodTicks = false;
    vvtChannel2.periodTicks = false;
    
    vvtInterrupt();
    
    // At 100%, the PWM state still toggles (handled by max_pwm flag in practice)
    TEST_ASSERT_TRUE(vvtChannel1.pinState);
}

// ========================= Test: VVT at minimal duty (1%) =========================

static void test_vvt1_minimal_duty(void)
{
    setup_vvt_interrupt_base();
    
    // Set VVT1 to minimal duty (1%)
    vvtChannel1.targetDuty = 10;
    vvtChannel2.targetDuty = 0;
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    vvtChannel1.periodTicks = false;
    vvtChannel2.periodTicks = false;
    
    vvtInterrupt();
    
    // Should still activate
    #if defined(CORE_TEENSY41)
    TEST_ASSERT_FALSE(getVvt1PinState());
    #else
    TEST_ASSERT_TRUE(getVvt1PinState());
    #endif
    
    TEST_ASSERT_TRUE(vvtChannel1.pinState);
}

// ========================= Test: VVT1 transition from on to off =========================

static void test_vvt1_transition_off(void)
{
    setup_vvt_interrupt_base();
    
    // VVT1 was on from previous interrupt, now turning off
    vvtChannel1.targetDuty = 500;
    vvtChannel2.targetDuty = 0;
    vvtChannel1.pinState = true;  // Already on
    vvtChannel2.pinState = false;
    vvtChannel1.periodTicks = false;
    vvtChannel2.periodTicks = false;
    
    vvtInterrupt();
    
    // The interrupt will handle the off-transition internally
    // VVT1 PWM state should now be false
    TEST_ASSERT_FALSE(vvtChannel1.pinState);
}

// ========================= Test: VVT2 earlier than VVT1 =========================

static void test_vvt2_earlier_than_vvt1(void)
{
    setup_vvt_interrupt_base();
    
    // VVT2 has shorter pulse (earlier edge)
    vvtChannel1.targetDuty = 700;
    vvtChannel2.targetDuty = 300;
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    vvtChannel1.periodTicks = false;
    vvtChannel2.periodTicks = false;
    
    vvtInterrupt();
    
    // Both should turn on regardless of order
    #if defined(CORE_TEENSY41)
    TEST_ASSERT_FALSE(getVvt1PinState());
    TEST_ASSERT_FALSE(getVvt2PinState());
    #else
    TEST_ASSERT_TRUE(getVvt1PinState());
    TEST_ASSERT_TRUE(getVvt2PinState());
    #endif
    
    TEST_ASSERT_TRUE(vvtChannel1.pinState);
    TEST_ASSERT_TRUE(vvtChannel2.pinState);
}

// ========================= Test: Only VVT1 enabled at max =========================

static void test_vvt1_max_vvt2_off(void)
{
    setup_vvt_interrupt_base();
    
    // VVT1 at max, VVT2 off
    vvtChannel1.targetDuty = vvt_pwm_max_count;
    vvtChannel2.targetDuty = 0;
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    vvtChannel1.periodTicks = false;
    vvtChannel2.periodTicks = false;
    
    vvtInterrupt();
    
    TEST_ASSERT_TRUE(vvtChannel1.pinState);
    TEST_ASSERT_FALSE(vvtChannel2.pinState);
}

// ========================= Test: Only VVT2 enabled at max =========================

static void test_vvt2_max_vvt1_off(void)
{
    setup_vvt_interrupt_base();
    
    // VVT2 at max, VVT1 off
    vvtChannel1.targetDuty = 0;
    vvtChannel2.targetDuty = vvt_pwm_max_count;
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    vvtChannel1.periodTicks = false;
    vvtChannel2.periodTicks = false;
    
    vvtInterrupt();
    
    TEST_ASSERT_FALSE(vvtChannel1.pinState);
    TEST_ASSERT_TRUE(vvtChannel2.pinState);
}

// ========================= Test: Both at max duty (always on) =========================

static void test_both_at_max_duty(void)
{
    setup_vvt_interrupt_base();
    
    // Both at 100% duty
    vvtChannel1.targetDuty = vvt_pwm_max_count;
    vvtChannel2.targetDuty = vvt_pwm_max_count;
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    vvtChannel1.periodTicks = false;
    vvtChannel2.periodTicks = false;
    
    vvtInterrupt();
    
    // Both should be activated
    TEST_ASSERT_TRUE(vvtChannel1.pinState);
    TEST_ASSERT_TRUE(vvtChannel2.pinState);
}

// ========================= Test: nextVVT == 0 Branch (VVT1 edge deactivation) =========================

static void test_vvt_nextvvt0_vvt1_off_vvt2_on(void)
{
    setup_vvt_interrupt_active_state();
    
    // Set up: VVT1 at 300us, VVT2 at 700us - both active
    vvtChannel1.targetDuty = 300;
    vvtChannel2.targetDuty = 700;
    vvtChannel1.pinState = true;
    vvtChannel2.pinState = true;
    vvtChannel1.compareTicks = 300;  // VVT1 edge just occurred
    vvtChannel2.compareTicks = 700;
    
    // Simulate idle entry first to set nextVVT
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    vvtInterrupt();  // This enters idle, sets nextVVT based on duty values
    
    // Now both are on and ready
    vvtChannel1.pinState = true;
    vvtChannel2.pinState = true;
    vvtChannel1.compareTicks = 300;
    vvtChannel2.compareTicks = 700;
    
    // Simulate VVT1 edge deactivation (nextVVT == 0)
    vvtInterrupt();
    
    // VVT1 should be off, VVT2 should still be on
    TEST_ASSERT_FALSE(vvtChannel1.pinState);
    TEST_ASSERT_TRUE(vvtChannel2.pinState);
}

// ========================= Test: nextVVT == 1 Branch (VVT2 edge deactivation) =========================

static void test_vvt_nextvvt1_vvt2_off_normal_duty(void)
{
    setup_vvt_interrupt_base();
    
    // Set up: VVT2 shorter than VVT1 (VVT2 edge occurs first)
    // This will set nextVVT = 1 during idle entry
    vvtChannel1.targetDuty = 700;
    vvtChannel2.targetDuty = 300;
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    
    // First interrupt: Enter idle, activate both
    vvtInterrupt();
    
    // Both should be activated
    TEST_ASSERT_TRUE(vvtChannel1.pinState);
    TEST_ASSERT_TRUE(vvtChannel2.pinState);
    
    // Verify both current values are set
    TEST_ASSERT_EQUAL(vvtChannel1.compareTicks, 700);
    TEST_ASSERT_EQUAL(vvtChannel2.compareTicks, 300);
}

// ========================= Test: nextVVT == 1 Branch (VVT2 at 100% duty) =========================

static void test_vvt_nextvvt1_vvt2_at_100percent(void)
{
    setup_vvt_interrupt_base();
    
    // VVT2 longer than VVT1 (100% means always on) at same time
    vvtChannel1.targetDuty = 300;
    vvtChannel2.targetDuty = vvt_pwm_max_count;  // 100% duty
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    vvtChannel2.periodTicks = false;
    
    // Enter idle state and activate both
    vvtInterrupt();
    TEST_ASSERT_TRUE(vvtChannel1.pinState);
    TEST_ASSERT_TRUE(vvtChannel2.pinState);
    
    // Verify PWM values cached
    TEST_ASSERT_EQUAL(vvtChannel1.compareTicks, 300);
    TEST_ASSERT_EQUAL(vvtChannel2.compareTicks, vvt_pwm_max_count);
}

// ========================= Test: nextVVT == 2 Branch (Both edges simultaneously) =========================

static void test_vvt_nextvvt2_both_edges_same_duty(void)
{
    setup_vvt_interrupt_active_state();
    
    // Both at same duty (500us each)
    vvtChannel1.targetDuty = 500;
    vvtChannel2.targetDuty = 500;
    vvtChannel1.pinState = true;
    vvtChannel2.pinState = true;
    vvtChannel1.compareTicks = 500;
    vvtChannel2.compareTicks = 500;
    
    // Enter from idle
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    vvtInterrupt();  // Idle entry sets nextVVT = 2 (same duty)
    
    // Restore to active state
    vvtChannel1.pinState = true;
    vvtChannel2.pinState = true;
    vvtChannel1.compareTicks = 500;
    vvtChannel2.compareTicks = 500;
    
    vvtInterrupt();  // Should handle nextVVT == 2 (both edges simultaneously)
    
    // Both should be deactivated
    TEST_ASSERT_FALSE(vvtChannel1.pinState);
    TEST_ASSERT_FALSE(vvtChannel2.pinState);
}

// ========================= Test: nextVVT == 2 Branch (One at 100%, one below) =========================

static void test_vvt_nextvvt2_vvt1_at_100_vvt2_below(void)
{
    setup_vvt_interrupt_base();
    
    // Scenario: VVT1 at 100% (always on), VVT2 at a different value
    // When VVT1 is 100%, it doesn't actually turn on/off like normal PWM
    vvtChannel1.targetDuty = vvt_pwm_max_count;  // 100%
    vvtChannel2.targetDuty = 500;
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    vvtChannel1.periodTicks = false;
    vvtChannel2.periodTicks = false;
    
    // Enter idle state
    vvtInterrupt();
    
    // Both should be activated
    TEST_ASSERT_TRUE(vvtChannel1.pinState);
    TEST_ASSERT_TRUE(vvtChannel2.pinState);
    
    // Verify current values are set
    TEST_ASSERT_EQUAL(vvtChannel1.compareTicks, vvt_pwm_max_count);
    TEST_ASSERT_EQUAL(vvtChannel2.compareTicks, 500);
}

// ========================= Test: nextVVT == 2 Branch (Both at 100% duty) =========================

static void test_vvt_nextvvt2_both_at_100percent(void)
{
    setup_vvt_interrupt_active_state();
    
    // Both at 100% duty
    vvtChannel1.targetDuty = vvt_pwm_max_count;
    vvtChannel2.targetDuty = vvt_pwm_max_count;
    vvtChannel1.pinState = true;
    vvtChannel2.pinState = true;
    vvtChannel1.compareTicks = vvt_pwm_max_count;
    vvtChannel2.compareTicks = vvt_pwm_max_count;
    vvtChannel1.periodTicks = false;
    vvtChannel2.periodTicks = false;
    
    // Enter from idle
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    vvtInterrupt();
    
    // Restore to active state
    vvtChannel1.pinState = true;
    vvtChannel2.pinState = true;
    vvtChannel1.compareTicks = vvt_pwm_max_count;
    vvtChannel2.compareTicks = vvt_pwm_max_count;
    
    vvtInterrupt();  // Handle nextVVT == 2 with both at 100%
    
    // Both should have max_pwm flag set
    TEST_ASSERT_TRUE(vvtChannel1.periodTicks);
    TEST_ASSERT_TRUE(vvtChannel2.periodTicks);
}

// ========================= Test: State Machine Progression (VVT1 longer than VVT2) =========================

static void test_vvt_state_machine_vvt2_shorter(void)
{
    setup_vvt_interrupt_base();
    
    // VVT1 at 700us, VVT2 at 300us (VVT2 edge comes first)
    vvtChannel1.targetDuty = 700;
    vvtChannel2.targetDuty = 300;
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    vvtChannel1.periodTicks = false;
    vvtChannel2.periodTicks = false;
    
    // First interrupt: enter idle state, activate both
    vvtInterrupt();
    
    TEST_ASSERT_TRUE(vvtChannel1.pinState);
    TEST_ASSERT_TRUE(vvtChannel2.pinState);
    
    // Both PWM values should be cached
    TEST_ASSERT_EQUAL(vvtChannel1.compareTicks, 700);
    TEST_ASSERT_EQUAL(vvtChannel2.compareTicks, 300);
}

// ========================= Test: State Machine Progression (VVT1 shorter than VVT2) =========================

static void test_vvt_state_machine_vvt1_shorter(void)
{
    setup_vvt_interrupt_base();
    
    // VVT1 at 300us, VVT2 at 700us (VVT1 edge comes first)
    vvtChannel1.targetDuty = 300;
    vvtChannel2.targetDuty = 700;
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    vvtChannel1.periodTicks = false;
    vvtChannel2.periodTicks = false;
    
    // First interrupt: enter idle state
    vvtInterrupt();
    
    TEST_ASSERT_TRUE(vvtChannel1.pinState);
    TEST_ASSERT_TRUE(vvtChannel2.pinState);
    
    // Verify current values are cached
    TEST_ASSERT_EQUAL(vvtChannel1.compareTicks, 300);
    TEST_ASSERT_EQUAL(vvtChannel2.compareTicks, 700);
}

// ========================= Test: Transition from One Off to Next On =========================

static void test_vvt_vvt1_only_to_vvt2_only(void)
{
    setup_vvt_interrupt_base();
    
    // Start with only VVT1 active
    vvtChannel1.targetDuty = 500;
    vvtChannel2.targetDuty = 0;
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    vvtChannel1.periodTicks = false;
    vvtChannel2.periodTicks = false;
    
    vvtInterrupt();  // Activate VVT1, VVT2 stays off
    
    TEST_ASSERT_TRUE(vvtChannel1.pinState);
    TEST_ASSERT_FALSE(vvtChannel2.pinState);
    
    // Now change duty values: turn off VVT1, turn on VVT2
    vvtChannel1.targetDuty = 0;
    vvtChannel2.targetDuty = 500;
    
    // Simulate re-entry to idle (both off or at max)
    vvtChannel1.pinState = false;
    vvtChannel2.pinState = false;
    vvtChannel1.periodTicks = false;
    vvtChannel2.periodTicks = false;
    
    vvtInterrupt();  // Should activate only VVT2
    
    TEST_ASSERT_FALSE(vvtChannel1.pinState);
    TEST_ASSERT_TRUE(vvtChannel2.pinState);
}

// ========================= Main Test Runner =========================

void testVvtInterrupt(void)
{
  SET_UNITY_FILENAME()
  {
    // Idle state entry tests
    RUN_TEST_P(test_both_off_idle_state);
    RUN_TEST_P(test_vvt1_at_50_percent_duty);
    RUN_TEST_P(test_vvt2_at_50_percent_duty);
    RUN_TEST_P(test_both_on_different_duties);
    RUN_TEST_P(test_both_same_duty_cycle);
    RUN_TEST_P(test_vvt1_at_100_percent_duty);
    RUN_TEST_P(test_vvt1_minimal_duty);
    RUN_TEST_P(test_vvt1_transition_off);
    RUN_TEST_P(test_vvt2_earlier_than_vvt1);
    RUN_TEST_P(test_vvt1_max_vvt2_off);
    RUN_TEST_P(test_vvt2_max_vvt1_off);
    RUN_TEST_P(test_both_at_max_duty);
    
    // Active state and state machine progression tests
    RUN_TEST_P(test_vvt_nextvvt0_vvt1_off_vvt2_on);
    RUN_TEST_P(test_vvt_nextvvt1_vvt2_off_normal_duty);
    RUN_TEST_P(test_vvt_nextvvt1_vvt2_at_100percent);
    RUN_TEST_P(test_vvt_nextvvt2_both_edges_same_duty);
    RUN_TEST_P(test_vvt_nextvvt2_vvt1_at_100_vvt2_below);
    RUN_TEST_P(test_vvt_nextvvt2_both_at_100percent);
    RUN_TEST_P(test_vvt_state_machine_vvt2_shorter);
    RUN_TEST_P(test_vvt_state_machine_vvt1_shorter);
    RUN_TEST_P(test_vvt_vvt1_only_to_vvt2_only);
  }
}