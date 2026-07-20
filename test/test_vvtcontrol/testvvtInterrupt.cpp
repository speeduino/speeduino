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
  pinVVT_1 = 19U;
  pinVVT_2 = 20U;
  
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

// ========================= Main Test Runner =========================

void testVvtInterrupt(void)
{
  SET_UNITY_FILENAME()
  {
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
  }
}