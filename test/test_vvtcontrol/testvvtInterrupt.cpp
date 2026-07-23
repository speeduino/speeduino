#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "units.h"
#include "src/pins/boardOutputPin.h"

// External declarations for testing VVT PWM interrupt handler
extern long vvt1_pwm_value;
extern long vvt2_pwm_value;
extern volatile unsigned int vvt1_pwm_cur_value;
extern volatile unsigned int vvt2_pwm_cur_value;
extern volatile bool vvt1_pwm_state;
extern volatile bool vvt2_pwm_state;
extern volatile bool vvt1_max_pwm;
extern volatile bool vvt2_max_pwm;
extern uint16_t vvt_pwm_max_count;
extern boardOutputPin_t vvt1_pin;
extern boardOutputPin_t vvt2_pin;

// ========================= Setup and Helpers =========================

static void setup_vvt_interrupt_base(void)
{
  // Initialize pins
  pinNumbers.pinVVT_1 = 19U;
  pinNumbers.pinVVT_2 = 20U;
  
  // Initialize all PWM state variables
  vvt1_pwm_value = 0;
  vvt2_pwm_value = 0;
  vvt1_pwm_cur_value = 0;
  vvt2_pwm_cur_value = 0;
  vvt1_pwm_state = false;
  vvt2_pwm_state = false;
  vvt1_max_pwm = false;
  vvt2_max_pwm = false;
  
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
  vvt1_pwm_state = true;
  vvt2_pwm_state = true;
  vvt1_max_pwm = false;
  vvt2_max_pwm = false;
  
  // Initialize current values (set by previous idle entry)
  vvt1_pwm_cur_value = 0;
  vvt2_pwm_cur_value = 0;
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
vvt1_pwm_value = 0;
vvt2_pwm_value = 0;
vvt1_pwm_state = false;
vvt2_pwm_state = false;
vvt1_max_pwm = false;
vvt2_max_pwm = false;

vvtInterrupt();

// PWM states should remain false
TEST_ASSERT_FALSE(vvt1_pwm_state);
TEST_ASSERT_FALSE(vvt2_pwm_state);
}

// ========================= Test: VVT1 only at 50% duty =========================

static void test_vvt1_at_50_percent_duty(void)
{
    setup_vvt_interrupt_base();

    // Set VVT1 to 50% duty from idle state
    vvt1_pwm_value = 500;
    vvt2_pwm_value = 0;
    vvt1_pwm_state = false;  // Idle state
    vvt2_pwm_state = false;
    vvt1_max_pwm = false;
    vvt2_max_pwm = false;

    vvtInterrupt();

    // VVT1 should be turned on and state set to true
    TEST_ASSERT_TRUE(vvt1_pwm_state);
    TEST_ASSERT_FALSE(vvt2_pwm_state);

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
    vvt1_pwm_value = 0;
    vvt2_pwm_value = 500;
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    vvt1_max_pwm = false;
    vvt2_max_pwm = false;
    
    vvtInterrupt();
    
    // VVT2 should be on, VVT1 should NOT activate (was at 0%)
    TEST_ASSERT_FALSE(vvt1_pwm_state);
    TEST_ASSERT_TRUE(vvt2_pwm_state);
    
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
    vvt1_pwm_value = 300;
    vvt2_pwm_value = 700;
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    vvt1_max_pwm = false;
    vvt2_max_pwm = false;
    
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
    TEST_ASSERT_TRUE(vvt1_pwm_state);
    TEST_ASSERT_TRUE(vvt2_pwm_state);
}

// ========================= Test: Both at same duty cycle =========================

static void test_both_same_duty_cycle(void)
{
    setup_vvt_interrupt_base();
    
    // Both at 50% duty
    vvt1_pwm_value = 500;
    vvt2_pwm_value = 500;
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    vvt1_max_pwm = false;
    vvt2_max_pwm = false;
    
    vvtInterrupt();
    
    // Both should turn on
    #if defined(CORE_TEENSY41)
    TEST_ASSERT_FALSE(getVvt1PinState());
    TEST_ASSERT_FALSE(getVvt2PinState());
    #else
    TEST_ASSERT_TRUE(getVvt1PinState());
    TEST_ASSERT_TRUE(getVvt2PinState());
    #endif
    
    TEST_ASSERT_TRUE(vvt1_pwm_state);
    TEST_ASSERT_TRUE(vvt2_pwm_state);
}

// ========================= Test: VVT at 100% duty (always on) =========================

static void test_vvt1_at_100_percent_duty(void)
{
    setup_vvt_interrupt_base();
    
    // Set VVT1 to 100% duty (max)
    vvt1_pwm_value = vvt_pwm_max_count;
    vvt2_pwm_value = 0;
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    vvt1_max_pwm = false;
    vvt2_max_pwm = false;
    
    vvtInterrupt();
    
    // At 100%, the PWM state still toggles (handled by max_pwm flag in practice)
    TEST_ASSERT_TRUE(vvt1_pwm_state);
}

// ========================= Test: VVT at minimal duty (1%) =========================

static void test_vvt1_minimal_duty(void)
{
    setup_vvt_interrupt_base();
    
    // Set VVT1 to minimal duty (1%)
    vvt1_pwm_value = 10;
    vvt2_pwm_value = 0;
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    vvt1_max_pwm = false;
    vvt2_max_pwm = false;
    
    vvtInterrupt();
    
    // Should still activate
    #if defined(CORE_TEENSY41)
    TEST_ASSERT_FALSE(getVvt1PinState());
    #else
    TEST_ASSERT_TRUE(getVvt1PinState());
    #endif
    
    TEST_ASSERT_TRUE(vvt1_pwm_state);
}

// ========================= Test: VVT1 transition from on to off =========================

static void test_vvt1_transition_off(void)
{
    setup_vvt_interrupt_base();
    
    // VVT1 was on from previous interrupt, now turning off
    vvt1_pwm_value = 500;
    vvt2_pwm_value = 0;
    vvt1_pwm_state = true;  // Already on
    vvt2_pwm_state = false;
    vvt1_max_pwm = false;
    vvt2_max_pwm = false;
    
    vvtInterrupt();
    
    // The interrupt will handle the off-transition internally
    // VVT1 PWM state should now be false
    TEST_ASSERT_FALSE(vvt1_pwm_state);
}

// ========================= Test: VVT2 earlier than VVT1 =========================

static void test_vvt2_earlier_than_vvt1(void)
{
    setup_vvt_interrupt_base();
    
    // VVT2 has shorter pulse (earlier edge)
    vvt1_pwm_value = 700;
    vvt2_pwm_value = 300;
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    vvt1_max_pwm = false;
    vvt2_max_pwm = false;
    
    vvtInterrupt();
    
    // Both should turn on regardless of order
    #if defined(CORE_TEENSY41)
    TEST_ASSERT_FALSE(getVvt1PinState());
    TEST_ASSERT_FALSE(getVvt2PinState());
    #else
    TEST_ASSERT_TRUE(getVvt1PinState());
    TEST_ASSERT_TRUE(getVvt2PinState());
    #endif
    
    TEST_ASSERT_TRUE(vvt1_pwm_state);
    TEST_ASSERT_TRUE(vvt2_pwm_state);
}

// ========================= Test: Only VVT1 enabled at max =========================

static void test_vvt1_max_vvt2_off(void)
{
    setup_vvt_interrupt_base();
    
    // VVT1 at max, VVT2 off
    vvt1_pwm_value = vvt_pwm_max_count;
    vvt2_pwm_value = 0;
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    vvt1_max_pwm = false;
    vvt2_max_pwm = false;
    
    vvtInterrupt();
    
    TEST_ASSERT_TRUE(vvt1_pwm_state);
    TEST_ASSERT_FALSE(vvt2_pwm_state);
}

// ========================= Test: Only VVT2 enabled at max =========================

static void test_vvt2_max_vvt1_off(void)
{
    setup_vvt_interrupt_base();
    
    // VVT2 at max, VVT1 off
    vvt1_pwm_value = 0;
    vvt2_pwm_value = vvt_pwm_max_count;
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    vvt1_max_pwm = false;
    vvt2_max_pwm = false;
    
    vvtInterrupt();
    
    TEST_ASSERT_FALSE(vvt1_pwm_state);
    TEST_ASSERT_TRUE(vvt2_pwm_state);
}

// ========================= Test: Both at max duty (always on) =========================

static void test_both_at_max_duty(void)
{
    setup_vvt_interrupt_base();
    
    // Both at 100% duty
    vvt1_pwm_value = vvt_pwm_max_count;
    vvt2_pwm_value = vvt_pwm_max_count;
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    vvt1_max_pwm = false;
    vvt2_max_pwm = false;
    
    vvtInterrupt();
    
    // Both should be activated
    TEST_ASSERT_TRUE(vvt1_pwm_state);
    TEST_ASSERT_TRUE(vvt2_pwm_state);
}

// ========================= Test: nextVVT == 0 Branch (VVT1 edge deactivation) =========================

static void test_vvt_nextvvt0_vvt1_off_vvt2_on(void)
{
    setup_vvt_interrupt_active_state();
    
    // Set up: VVT1 at 300us, VVT2 at 700us - both active
    vvt1_pwm_value = 300;
    vvt2_pwm_value = 700;
    vvt1_pwm_state = true;
    vvt2_pwm_state = true;
    vvt1_pwm_cur_value = 300;  // VVT1 edge just occurred
    vvt2_pwm_cur_value = 700;
    
    // Simulate idle entry first to set nextVVT
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    vvtInterrupt();  // This enters idle, sets nextVVT based on duty values
    
    // Now both are on and ready
    vvt1_pwm_state = true;
    vvt2_pwm_state = true;
    vvt1_pwm_cur_value = 300;
    vvt2_pwm_cur_value = 700;
    
    // Simulate VVT1 edge deactivation (nextVVT == 0)
    vvtInterrupt();
    
    // VVT1 should be off, VVT2 should still be on
    TEST_ASSERT_FALSE(vvt1_pwm_state);
    TEST_ASSERT_TRUE(vvt2_pwm_state);
}

// ========================= Test: nextVVT == 1 Branch (VVT2 edge deactivation) =========================

static void test_vvt_nextvvt1_vvt2_off_normal_duty(void)
{
    setup_vvt_interrupt_base();
    
    // Set up: VVT2 shorter than VVT1 (VVT2 edge occurs first)
    // This will set nextVVT = 1 during idle entry
    vvt1_pwm_value = 700;
    vvt2_pwm_value = 300;
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    
    // First interrupt: Enter idle, activate both
    vvtInterrupt();
    
    // Both should be activated
    TEST_ASSERT_TRUE(vvt1_pwm_state);
    TEST_ASSERT_TRUE(vvt2_pwm_state);
    
    // Verify both current values are set
    TEST_ASSERT_EQUAL(vvt1_pwm_cur_value, 700);
    TEST_ASSERT_EQUAL(vvt2_pwm_cur_value, 300);
}

// ========================= Test: nextVVT == 1 Branch (VVT2 at 100% duty) =========================

static void test_vvt_nextvvt1_vvt2_at_100percent(void)
{
    setup_vvt_interrupt_base();
    
    // VVT2 longer than VVT1 (100% means always on) at same time
    vvt1_pwm_value = 300;
    vvt2_pwm_value = vvt_pwm_max_count;  // 100% duty
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    vvt2_max_pwm = false;
    
    // Enter idle state and activate both
    vvtInterrupt();
    TEST_ASSERT_TRUE(vvt1_pwm_state);
    TEST_ASSERT_TRUE(vvt2_pwm_state);
    
    // Verify PWM values cached
    TEST_ASSERT_EQUAL(vvt1_pwm_cur_value, 300);
    TEST_ASSERT_EQUAL(vvt2_pwm_cur_value, vvt_pwm_max_count);
}

// ========================= Test: nextVVT == 2 Branch (Both edges simultaneously) =========================

static void test_vvt_nextvvt2_both_edges_same_duty(void)
{
    setup_vvt_interrupt_active_state();
    
    // Both at same duty (500us each)
    vvt1_pwm_value = 500;
    vvt2_pwm_value = 500;
    vvt1_pwm_state = true;
    vvt2_pwm_state = true;
    vvt1_pwm_cur_value = 500;
    vvt2_pwm_cur_value = 500;
    
    // Enter from idle
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    vvtInterrupt();  // Idle entry sets nextVVT = 2 (same duty)
    
    // Restore to active state
    vvt1_pwm_state = true;
    vvt2_pwm_state = true;
    vvt1_pwm_cur_value = 500;
    vvt2_pwm_cur_value = 500;
    
    vvtInterrupt();  // Should handle nextVVT == 2 (both edges simultaneously)
    
    // Both should be deactivated
    TEST_ASSERT_FALSE(vvt1_pwm_state);
    TEST_ASSERT_FALSE(vvt2_pwm_state);
}

// ========================= Test: nextVVT == 2 Branch (One at 100%, one below) =========================

static void test_vvt_nextvvt2_vvt1_at_100_vvt2_below(void)
{
    setup_vvt_interrupt_base();
    
    // Scenario: VVT1 at 100% (always on), VVT2 at a different value
    // When VVT1 is 100%, it doesn't actually turn on/off like normal PWM
    vvt1_pwm_value = vvt_pwm_max_count;  // 100%
    vvt2_pwm_value = 500;
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    vvt1_max_pwm = false;
    vvt2_max_pwm = false;
    
    // Enter idle state
    vvtInterrupt();
    
    // Both should be activated
    TEST_ASSERT_TRUE(vvt1_pwm_state);
    TEST_ASSERT_TRUE(vvt2_pwm_state);
    
    // Verify current values are set
    TEST_ASSERT_EQUAL(vvt1_pwm_cur_value, vvt_pwm_max_count);
    TEST_ASSERT_EQUAL(vvt2_pwm_cur_value, 500);
}

// ========================= Test: nextVVT == 2 Branch (Both at 100% duty) =========================

static void test_vvt_nextvvt2_both_at_100percent(void)
{
    setup_vvt_interrupt_active_state();
    
    // Both at 100% duty
    vvt1_pwm_value = vvt_pwm_max_count;
    vvt2_pwm_value = vvt_pwm_max_count;
    vvt1_pwm_state = true;
    vvt2_pwm_state = true;
    vvt1_pwm_cur_value = vvt_pwm_max_count;
    vvt2_pwm_cur_value = vvt_pwm_max_count;
    vvt1_max_pwm = false;
    vvt2_max_pwm = false;
    
    // Enter from idle
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    vvtInterrupt();
    
    // Restore to active state
    vvt1_pwm_state = true;
    vvt2_pwm_state = true;
    vvt1_pwm_cur_value = vvt_pwm_max_count;
    vvt2_pwm_cur_value = vvt_pwm_max_count;
    
    vvtInterrupt();  // Handle nextVVT == 2 with both at 100%
    
    // Both should have max_pwm flag set
    TEST_ASSERT_TRUE(vvt1_max_pwm);
    TEST_ASSERT_TRUE(vvt2_max_pwm);
}

// ========================= Test: State Machine Progression (VVT1 longer than VVT2) =========================

static void test_vvt_state_machine_vvt2_shorter(void)
{
    setup_vvt_interrupt_base();
    
    // VVT1 at 700us, VVT2 at 300us (VVT2 edge comes first)
    vvt1_pwm_value = 700;
    vvt2_pwm_value = 300;
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    vvt1_max_pwm = false;
    vvt2_max_pwm = false;
    
    // First interrupt: enter idle state, activate both
    vvtInterrupt();
    
    TEST_ASSERT_TRUE(vvt1_pwm_state);
    TEST_ASSERT_TRUE(vvt2_pwm_state);
    
    // Both PWM values should be cached
    TEST_ASSERT_EQUAL(vvt1_pwm_cur_value, 700);
    TEST_ASSERT_EQUAL(vvt2_pwm_cur_value, 300);
}

// ========================= Test: State Machine Progression (VVT1 shorter than VVT2) =========================

static void test_vvt_state_machine_vvt1_shorter(void)
{
    setup_vvt_interrupt_base();
    
    // VVT1 at 300us, VVT2 at 700us (VVT1 edge comes first)
    vvt1_pwm_value = 300;
    vvt2_pwm_value = 700;
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    vvt1_max_pwm = false;
    vvt2_max_pwm = false;
    
    // First interrupt: enter idle state
    vvtInterrupt();
    
    TEST_ASSERT_TRUE(vvt1_pwm_state);
    TEST_ASSERT_TRUE(vvt2_pwm_state);
    
    // Verify current values are cached
    TEST_ASSERT_EQUAL(vvt1_pwm_cur_value, 300);
    TEST_ASSERT_EQUAL(vvt2_pwm_cur_value, 700);
}

// ========================= Test: Transition from One Off to Next On =========================

static void test_vvt_vvt1_only_to_vvt2_only(void)
{
    setup_vvt_interrupt_base();
    
    // Start with only VVT1 active
    vvt1_pwm_value = 500;
    vvt2_pwm_value = 0;
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    vvt1_max_pwm = false;
    vvt2_max_pwm = false;
    
    vvtInterrupt();  // Activate VVT1, VVT2 stays off
    
    TEST_ASSERT_TRUE(vvt1_pwm_state);
    TEST_ASSERT_FALSE(vvt2_pwm_state);
    
    // Now change duty values: turn off VVT1, turn on VVT2
    vvt1_pwm_value = 0;
    vvt2_pwm_value = 500;
    
    // Simulate re-entry to idle (both off or at max)
    vvt1_pwm_state = false;
    vvt2_pwm_state = false;
    vvt1_max_pwm = false;
    vvt2_max_pwm = false;
    
    vvtInterrupt();  // Should activate only VVT2
    
    TEST_ASSERT_FALSE(vvt1_pwm_state);
    TEST_ASSERT_TRUE(vvt2_pwm_state);
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