#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "units.h"
#include "src/pins/boardOutputPin.h"
#include "src/controllers/vvt/VvtOutputChannel.h"

// External declarations for testing VVT PWM interrupt handler
extern VvtOutputChannel vvtChannel1;
extern VvtOutputChannel vvtChannel2;
extern NextInterruptEvent nextVVT;

// ========================= Setup and Helpers =========================

static void setup_vvt_interrupt_base(void)
{
  // Initialize pins
  pinNumbers.pinVVT_1 = 19U;
  pinNumbers.pinVVT_2 = 20U;
  
  // Initialize pins through auxiliaries
  initialiseAuxPWM();
}

// ========================= Test: Both VVT outputs off (idle state) =========================

static void test_both_off_idle_state(void)
{
    setup_vvt_interrupt_base();

    // Both PWM values are zero (off), state is idle
    vvtChannel1.setTargetDutyFromDuty(0);
    vvtChannel2.setTargetDutyFromDuty(0);

    vvtInterrupt();

    // PWM states should remain false
    TEST_ASSERT_FALSE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_FALSE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::VVT1, nextVVT);
}

// ========================= Test: VVT1 only at 50% duty =========================

static void test_vvt1_at_50_percent_duty(void)
{
    setup_vvt_interrupt_base();

    // Set VVT1 to 50% duty from idle state
    vvtChannel1.setTargetDutyFromDuty(100);
    vvtChannel2.setTargetDutyFromDuty(0);

    vvtInterrupt();

    // VVT1 should be turned on and state set to true
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_FALSE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::VVT1, nextVVT);
}

// ========================= Test: VVT2 only at 50% duty =========================

static void test_vvt2_at_50_percent_duty(void)
{
    setup_vvt_interrupt_base();
    
    // Set VVT2 to 50% duty, VVT1 off
    vvtChannel1.setTargetDutyFromDuty(0);
    vvtChannel2.setTargetDutyFromDuty(100);
    
    vvtInterrupt();
    
    // VVT2 should be on, VVT1 should NOT activate (was at 0%)
    TEST_ASSERT_FALSE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::VVT2, nextVVT);
}

// ========================= Test: Both at different duty cycles =========================

static void test_both_on_different_duties(void)
{
    setup_vvt_interrupt_base();
    
    // VVT1 at 30%, VVT2 at 70%
    vvtChannel1.setTargetDutyFromDuty(60);
    vvtChannel2.setTargetDutyFromDuty(140);
    
    vvtInterrupt();
    
    // Both PWM states should be true
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::VVT1, nextVVT);
}

// ========================= Test: Both at same duty cycle =========================

static void test_both_same_duty_cycle(void)
{
    setup_vvt_interrupt_base();
    
    // Both at 50% duty
    vvtChannel1.setTargetDutyFromDuty(100);
    vvtChannel2.setTargetDutyFromDuty(100);
    
    vvtInterrupt();
    
    // Both should turn on
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::Both, nextVVT);
}

// ========================= Test: VVT at 100% duty (always on) =========================

static void test_vvt1_at_100_percent_duty(void)
{
    setup_vvt_interrupt_base();
    
    // Set VVT1 to 100% duty (max)
    vvtChannel1.setTargetDutyFromDuty(200);
    vvtChannel2.setTargetDutyFromDuty(0);
    
    vvtInterrupt();
    
    // At 100%, the PWM state still toggles (handled by max_pwm flag in practice)
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinLow());
    TEST_ASSERT_EQUAL(NextInterruptEvent::VVT1, nextVVT);
}

// ========================= Test: VVT at minimal duty (1%) =========================

static void test_vvt1_minimal_duty(void)
{
    setup_vvt_interrupt_base();
    
    // Set VVT1 to minimal duty (1%)
    vvtChannel1.setTargetDutyFromDuty(2);
    vvtChannel2.setTargetDutyFromDuty(0);

    vvtInterrupt();
    
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinLow());
    TEST_ASSERT_EQUAL(NextInterruptEvent::VVT1, nextVVT);
}

// ========================= Test: VVT1 transition from on to off =========================

static void test_vvt1_transition_off(void)
{
    // VVT1 was on from previous interrupt, now turning off

    setup_vvt_interrupt_base();
    
    vvtChannel1.setTargetDutyFromDuty(100);
    vvtChannel2.setTargetDutyFromDuty(0);
    
    vvtInterrupt();
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinLow());
    TEST_ASSERT_EQUAL(NextInterruptEvent::VVT1, nextVVT);
    
    // The interrupt will handle the off-transition internally
    // VVT1 PWM state should now be false
    vvtInterrupt();
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinLow());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinLow());
    TEST_ASSERT_EQUAL(NextInterruptEvent::Both, nextVVT);
}

// ========================= Test: VVT2 earlier than VVT1 =========================

static void test_vvt2_earlier_than_vvt1(void)
{
    setup_vvt_interrupt_base();
    
    // VVT2 has shorter pulse (earlier edge)
    vvtChannel1.setTargetDutyFromDuty(140);
    vvtChannel2.setTargetDutyFromDuty(60);

    vvtInterrupt();
    
    // Both should turn on regardless of order  
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::VVT2, nextVVT);
}

// ========================= Test: Only VVT2 enabled at max =========================

static void test_vvt2_max_vvt1_off(void)
{
    setup_vvt_interrupt_base();
    
    // VVT2 at max, VVT1 off
    vvtChannel1.setTargetDutyFromDuty(0);
    vvtChannel2.setTargetDutyFromDuty(200);
    
    vvtInterrupt();
    
    TEST_ASSERT_FALSE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::VVT2, nextVVT);
}

// ========================= Test: Both at max duty (always on) =========================

static void test_both_at_max_duty(void)
{
    setup_vvt_interrupt_base();
    
    // Both at 100% duty
    vvtChannel1.setTargetDutyFromDuty(200);
    vvtChannel2.setTargetDutyFromDuty(200);
    
    vvtInterrupt();
    
    // Both should be activated
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::Both, nextVVT);
}

// ========================= Test: nextVVT == 0 Branch (VVT1 edge deactivation) =========================

static void test_vvt_nextvvt0_vvt1_off_vvt2_on(void)
{
    setup_vvt_interrupt_base();
    
    // Set up: VVT1 at 300us, VVT2 at 700us - both active
    vvtChannel1.setTargetDutyFromDuty(60);
    vvtChannel2.setTargetDutyFromDuty(140);
    
    // Simulate idle entry first to set nextVVT
    vvtInterrupt();  // This enters idle, sets nextVVT based on duty values
    
    // Now both are on and ready
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::VVT1, nextVVT);
    
    // Simulate VVT1 edge deactivation (nextVVT == 0)
    vvtInterrupt();
    
    // VVT1 should be off, VVT2 should still be on
    TEST_ASSERT_FALSE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::VVT2, nextVVT);
}

// ========================= Test: nextVVT == 1 Branch (VVT2 edge deactivation) =========================

static void test_vvt_nextvvt1_vvt2_off_normal_duty(void)
{
    setup_vvt_interrupt_base();
    
    // Set up: VVT2 shorter than VVT1 (VVT2 edge occurs first)
    // This will set nextVVT = 1 during idle entry
    vvtChannel1.setTargetDutyFromDuty(100);
    vvtChannel2.setTargetDutyFromDuty(98);
    
    // First interrupt: Enter idle, activate both
    vvtInterrupt();
    
    // Both should be activated
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::VVT2, nextVVT);
}

// ========================= Test: nextVVT == 1 Branch (VVT2 at 100% duty) =========================

static void test_vvt_nextvvt1_vvt2_at_100percent(void)
{
    setup_vvt_interrupt_base();
    
    // VVT2 longer than VVT1 (100% means always on) at same time
    vvtChannel1.setTargetDutyFromDuty(100);
    vvtChannel2.setTargetDutyFromDuty(200);
    
    // Enter idle state and activate both
    vvtInterrupt();
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::VVT1, nextVVT);
}

// ========================= Test: nextVVT == 2 Branch (Both edges simultaneously) =========================

static void test_vvt_nextvvt2_both_edges_same_duty(void)
{
    setup_vvt_interrupt_base();
    
    // Both at same duty (50% each)
    vvtChannel1.setTargetDutyFromDuty(100);
    vvtChannel2.setTargetDutyFromDuty(100);
    
    // Enter from idle
    vvtInterrupt();  // Idle entry sets nextVVT = 2 (same duty)
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::Both, nextVVT);
      
    vvtInterrupt();  // Should handle nextVVT == 2 (both edges simultaneously)
    // Both should be deactivated
    TEST_ASSERT_FALSE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_FALSE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::Both, nextVVT);
}

// ========================= Test: nextVVT == 2 Branch (One at 100%, one below) =========================

static void test_vvt_nextvvt2_vvt1_at_100_vvt2_below(void)
{
    setup_vvt_interrupt_base();
    
    // Scenario: VVT1 at 100% (always on), VVT2 at a different value
    // When VVT1 is 100%, it doesn't actually turn on/off like normal PWM
    vvtChannel1.setTargetDutyFromDuty(200);
    vvtChannel2.setTargetDutyFromDuty(100);
  
    // Enter idle state
    vvtInterrupt();
    
    // Both should be activated
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::VVT2, nextVVT);
}

// ========================= Test: nextVVT == 2 Branch (Both at 100% duty) =========================

static void test_vvt_nextvvt2_both_at_100percent(void)
{
    setup_vvt_interrupt_base();
    
    // Both at 100% duty
    vvtChannel1.setTargetDutyFromDuty(200);
    vvtChannel2.setTargetDutyFromDuty(200);
    
    // Enter from idle
    vvtInterrupt();
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::Both, nextVVT);
    
    // Restore to active state
    vvtInterrupt();  // Handle nextVVT == 2 with both at 100%
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::Both, nextVVT);
}

// ========================= Test: State Machine Progression (VVT1 longer than VVT2) =========================

static void test_vvt_state_machine_vvt2_shorter(void)
{
    setup_vvt_interrupt_base();
    
    // VVT1 at 70%s, VVT2 at 30%s (VVT2 edge comes first)
    vvtChannel1.setTargetDutyFromDuty(140);
    vvtChannel2.setTargetDutyFromDuty(60);
    
    // First interrupt: enter idle state, activate both
    vvtInterrupt();  
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::VVT2, nextVVT);
}

// ========================= Test: State Machine Progression (VVT1 shorter than VVT2) =========================

static void test_vvt_state_machine_vvt1_shorter(void)
{
    setup_vvt_interrupt_base();
    
    // VVT1 at 30%s, VVT2 at 70%s (VVT1 edge comes first)
    vvtChannel1.setTargetDutyFromDuty(60);
    vvtChannel2.setTargetDutyFromDuty(140);
    
    // First interrupt: enter idle state, activate both
    vvtInterrupt();  
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::VVT1, nextVVT);
}

// ========================= Test: Transition from One Off to Next On =========================

static void test_vvt_vvt1_only_to_vvt2_only(void)
{
    setup_vvt_interrupt_base();
    
    // Start with only VVT1 active
    vvtChannel1.setTargetDutyFromDuty(100);
    vvtChannel2.setTargetDutyFromDuty(0);
    
    vvtInterrupt();  // Activate VVT1, VVT2 stays off  
    TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_FALSE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::VVT1, nextVVT);
   
    // Now change duty values: turn off VVT1, turn on VVT2
    vvtChannel1.setTargetDutyFromDuty(0);
    vvtChannel2.setTargetDutyFromDuty(100);
   
    vvtInterrupt();  // Should activate only VVT2  
    TEST_ASSERT_FALSE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_EQUAL(NextInterruptEvent::VVT2, nextVVT);
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