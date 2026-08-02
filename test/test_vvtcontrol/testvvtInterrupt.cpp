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
extern uint16_t lastVvtComparatorOffset;

constexpr uint8_t LOOP_COUNT = 6; // Number of iterations for each test loop

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

static void test_vvt1_off_vvt2_off(void)
{
    setup_vvt_interrupt_base();

    // Both PWM values are zero (off), state is idle
    vvtChannel1.setTargetDutyFromDuty(0);
    vvtChannel2.setTargetDutyFromDuty(0);

    for (uint8_t i=0; i<LOOP_COUNT; ++i)
    {
        vvtInterrupt();
        // PWM states should remain false
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinLow());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinLow());
        TEST_ASSERT_EQUAL(NextInterruptEvent::BothOn, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel1.maxDuty, lastVvtComparatorOffset);
    }
}

// ========================= Test: VVT1 only at 50% duty =========================

static void test_vvt1_partial_vvt2_off(void)
{
    setup_vvt_interrupt_base();

    // Set VVT1 to 50% duty from idle state
    vvtChannel1.setTargetDutyFromDuty(100);
    vvtChannel2.setTargetDutyFromDuty(0);

    for (uint8_t i=0; i<LOOP_COUNT; ++i)
    {
        vvtInterrupt();
        // VVT1 should be turned on
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinLow());
        TEST_ASSERT_EQUAL(NextInterruptEvent::VVT1, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel1.targetDuty, lastVvtComparatorOffset);

        vvtInterrupt();
        // VVT1 should be turned off
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinLow());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinLow());
        TEST_ASSERT_EQUAL(NextInterruptEvent::BothOff, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel1.targetDuty, lastVvtComparatorOffset);
    }
}

// ========================= Test: VVT2 only at 50% duty =========================

static void test_vvt1_off_vvt2_partial(void)
{
    setup_vvt_interrupt_base();
    
    // Set VVT2 to 50% duty, VVT1 off
    vvtChannel1.setTargetDutyFromDuty(0);
    vvtChannel2.setTargetDutyFromDuty(100);
    
    for (uint8_t i=0; i<LOOP_COUNT; ++i)
    {
        vvtInterrupt();
        // VVT2 should be on, VVT1 should NOT activate (was at 0%)
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinLow());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
        TEST_ASSERT_EQUAL(NextInterruptEvent::VVT2, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel2.targetDuty, lastVvtComparatorOffset);

        vvtInterrupt();
        // VVT2 should be off, VVT1 should NOT activate (was at 0%)
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinLow());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinLow());
        TEST_ASSERT_EQUAL(NextInterruptEvent::BothOff, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel2.targetDuty, lastVvtComparatorOffset);
    }
}

// ========================= Test: Both at same duty cycle =========================

static void test_both_same_duty_cycle(void)
{
    setup_vvt_interrupt_base();
    
    // Both at 50% duty
    vvtChannel1.setTargetDutyFromDuty(100);
    vvtChannel2.setTargetDutyFromDuty(100);
    
    for (uint8_t i=0; i<LOOP_COUNT; ++i)
    {
        vvtInterrupt();   
        // Both should turn on
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
        TEST_ASSERT_EQUAL(NextInterruptEvent::BothOff, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel1.targetDuty, lastVvtComparatorOffset);

        vvtInterrupt();   
        // Both should turn off
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinLow());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinLow());
        TEST_ASSERT_EQUAL(NextInterruptEvent::BothOn, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel1.targetDuty, lastVvtComparatorOffset);
    }
}

// ========================= Test: VVT at 100% duty (always on) =========================

static void test_vvt1_fullon_vvt2_off(void)
{
    setup_vvt_interrupt_base();
    
    // Set VVT1 to 100% duty (max)
    vvtChannel1.setTargetDutyFromDuty(200);
    vvtChannel2.setTargetDutyFromDuty(0);
    
    for (uint8_t i=0; i<LOOP_COUNT; ++i)
    {
        vvtInterrupt();
        
        // At 100%, the PWM state still toggles (handled by max_pwm flag in practice)
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinLow());
        TEST_ASSERT_EQUAL(NextInterruptEvent::VVT1, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel1.targetDuty, lastVvtComparatorOffset);
    }
}

// ========================= Test: VVT2 earlier than VVT1 =========================

static void test_vvt1_later_than_vvt2(void)
{
    setup_vvt_interrupt_base();
    
    // VVT2 has shorter pulse (earlier edge)
    vvtChannel1.setTargetDutyFromDuty(140);
    vvtChannel2.setTargetDutyFromDuty(60);

   for (uint8_t i=0; i<LOOP_COUNT; ++i)
    {
        vvtInterrupt();   
        // Both should turn on regardless of order  
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
        TEST_ASSERT_EQUAL(NextInterruptEvent::VVT2, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel2.targetDuty, lastVvtComparatorOffset);

        vvtInterrupt();   
        // Both should turn on regardless of order  
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinLow());
        TEST_ASSERT_EQUAL(NextInterruptEvent::VVT1, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel1.targetDuty-vvtChannel2.targetDuty, lastVvtComparatorOffset);

        vvtInterrupt();   
        // Both should turn on regardless of order  
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinLow());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinLow());
        TEST_ASSERT_EQUAL(NextInterruptEvent::BothOff, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel2.targetDuty, lastVvtComparatorOffset);
    }
}

// ========================= Test: VVT2 later than VVT1 =========================

static void test_vvt1_earlier_than_vvt2(void)
{
    setup_vvt_interrupt_base();
    
    // VVT2 has shorter pulse (earlier edge)
    vvtChannel1.setTargetDutyFromDuty(60);
    vvtChannel2.setTargetDutyFromDuty(140);

   for (uint8_t i=0; i<LOOP_COUNT; ++i)
    {
        vvtInterrupt();   
        // Both should turn on regardless of order  
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
        TEST_ASSERT_EQUAL(NextInterruptEvent::VVT1, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel1.targetDuty, lastVvtComparatorOffset);

        vvtInterrupt();   
        // Both should turn on regardless of order  
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinLow());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
        TEST_ASSERT_EQUAL(NextInterruptEvent::VVT2, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel2.targetDuty-vvtChannel1.targetDuty, lastVvtComparatorOffset);

        vvtInterrupt();   
        // Both should turn on regardless of order  
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinLow());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinLow());
        TEST_ASSERT_EQUAL(NextInterruptEvent::BothOff, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel1.targetDuty, lastVvtComparatorOffset);
    }
}

// ========================= Test: Only VVT2 enabled at max =========================

static void test_vvt1_off_vvt2_fullon(void)
{
    setup_vvt_interrupt_base();
    
    // VVT2 at max, VVT1 off
    vvtChannel1.setTargetDutyFromDuty(0);
    vvtChannel2.setTargetDutyFromDuty(200);
    
    for (uint8_t i=0; i<LOOP_COUNT; ++i)
    {
        vvtInterrupt();
        
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinLow());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
        TEST_ASSERT_EQUAL(NextInterruptEvent::VVT2, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel2.targetDuty, lastVvtComparatorOffset);
    }
}

// ========================= Test: Both at max duty (always on) =========================

static void test_vvt1_fullon_vvt2_fullon(void)
{
    setup_vvt_interrupt_base();
    
    // Both at 100% duty
    vvtChannel1.setTargetDutyFromDuty(200);
    vvtChannel2.setTargetDutyFromDuty(200);

    for (uint8_t i=0; i<LOOP_COUNT; ++i)
    {
        vvtInterrupt();
        
        // Both should be activated
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
        TEST_ASSERT_EQUAL(NextInterruptEvent::BothOff, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel1.targetDuty, lastVvtComparatorOffset);
    }
}

// ========================= Test: nextVVT == 1 Branch (VVT2 at 100% duty) =========================

static void test_vvt1_partial_vvt2_max(void)
{
    setup_vvt_interrupt_base();
    
    // VVT2 longer than VVT1 (100% means always on) at same time
    vvtChannel1.setTargetDutyFromDuty(100);
    vvtChannel2.setTargetDutyFromDuty(200);
    
    // Enter idle state and activate both
    for (uint8_t i=0; i<LOOP_COUNT; ++i)
    {
        vvtInterrupt();
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
        TEST_ASSERT_EQUAL(NextInterruptEvent::VVT1, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel1.targetDuty, lastVvtComparatorOffset);

        vvtInterrupt();
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinLow());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
        TEST_ASSERT_EQUAL(NextInterruptEvent::VVT2, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel1.targetDuty, lastVvtComparatorOffset);
    }
}

// ========================= Test: nextVVT == 2 Branch (One at 100%, one below) =========================

static void test_vvt1_max_vvt2_partial(void)
{
    setup_vvt_interrupt_base();
    
    // Scenario: VVT1 at 100% (always on), VVT2 at a different value
    // When VVT1 is 100%, it doesn't actually turn on/off like normal PWM
    vvtChannel1.setTargetDutyFromDuty(200);
    vvtChannel2.setTargetDutyFromDuty(100);
  
    for (uint8_t i=0; i<LOOP_COUNT; ++i)
    {
        vvtInterrupt();    
        // Both should be activated
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinHigh());
        TEST_ASSERT_EQUAL(NextInterruptEvent::VVT2, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel2.targetDuty, lastVvtComparatorOffset);

        vvtInterrupt();
        TEST_ASSERT_TRUE(vvtChannel1.pin.isPinHigh());
        TEST_ASSERT_TRUE(vvtChannel2.pin.isPinLow());
        TEST_ASSERT_EQUAL(NextInterruptEvent::VVT1, nextVVT);
        TEST_ASSERT_EQUAL(vvtChannel2.targetDuty, lastVvtComparatorOffset);
    }
}

// ========================= Main Test Runner =========================

void testVvtInterrupt(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_vvt1_off_vvt2_off);
    RUN_TEST_P(test_vvt1_off_vvt2_fullon);
    RUN_TEST_P(test_vvt1_fullon_vvt2_fullon);
    RUN_TEST_P(test_both_same_duty_cycle);
    RUN_TEST_P(test_vvt1_later_than_vvt2);
    RUN_TEST_P(test_vvt1_earlier_than_vvt2);
    RUN_TEST_P(test_vvt1_fullon_vvt2_off);
    RUN_TEST_P(test_vvt1_partial_vvt2_off);
    RUN_TEST_P(test_vvt1_off_vvt2_partial);
    RUN_TEST_P(test_vvt1_max_vvt2_partial);
    RUN_TEST_P(test_vvt1_partial_vvt2_max);
  }
}