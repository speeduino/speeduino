#include <unity.h>
#include "src/PID/integerPID.h"
#include "../test_utils.h"

constexpr unsigned long NOW = 10000UL;

static void test_integerPID_manual_mode_compute_false(void)
{
    long input = 10;
    long output = 8;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint, 10, 0, 0, DIRECT);
    pid.SetMode(MANUAL);

    TEST_ASSERT_FALSE(pid.Compute(NOW));
    TEST_ASSERT_EQUAL(8, output);
}

static void test_integerPID_auto_mode_p_on_error(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint, 10, 0, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    // Expected: kp scaled to 320, error=10, output=3200>>10=3
    TEST_ASSERT_EQUAL(3, output);
}

static void test_integerPID_output_limits_clamp(void)
{
    long input = 1;
    long output = 0;
    long setpoint = 1000;

    integerPID pid(&input, &output, &setpoint, 255, 0, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(0, 10);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    TEST_ASSERT_EQUAL(10, output);
}

static void test_integerPID_output_limits_zero_range(void)
{
    long input = 1;
    long output = 0;
    long setpoint = 1000;

    integerPID pid(&input, &output, &setpoint, 255, 0, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetMode(AUTOMATIC);

    output = 1000;
    pid.SetOutputLimits(0, 10);
    TEST_ASSERT_EQUAL(10, output);

    output = 1000;
    pid.SetOutputLimits(10, 10);
    TEST_ASSERT_EQUAL(1000, output);
}

static void test_integerPID_controller_direction_switches_effect(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 100;

    integerPID pid(&input, &output, &setpoint, 10, 0, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(-255, 255);
    pid.SetMode(AUTOMATIC);

    unsigned long now = NOW;
    TEST_ASSERT_TRUE(pid.Compute(now));
    long directOutput = output;
    TEST_ASSERT_GREATER_THAN(0, directOutput);

    pid.SetControllerDirection(REVERSE);
    now += 100;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_NOT_EQUAL(directOutput, output);
    TEST_ASSERT_LESS_THAN(0, output);

    pid.SetControllerDirection(DIRECT);
    now += 100;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_GREATER_THAN(0, output);
}

static void test_integerPID_controller_direction_maintains_output_limits(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 100;

    integerPID pid(&input, &output, &setpoint, 255, 0, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(-50, 50);
    pid.SetMode(AUTOMATIC);

    unsigned long now = NOW;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_LESS_OR_EQUAL(50, output);

    pid.SetControllerDirection(REVERSE);
    now += 100;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_GREATER_OR_EQUAL(-50, output);
}

static void test_integerPID_input_zero_failsafe(void)
{
    long input = 0;
    long output = 3;
    long setpoint = 100;

    integerPID pid(&input, &output, &setpoint, 10, 10, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_FALSE(pid.Compute(NOW, 0));
    TEST_ASSERT_EQUAL(3, output);
}

static void test_integerPID_reverse_direction(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint, 10, 0, 0, REVERSE);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(-255, 255);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    TEST_ASSERT_LESS_THAN(0, output);  // negative output expected in reverse
}

static void test_integerPID_feedforward_term(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint, 0, 0, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute(NOW, 15));
    TEST_ASSERT_EQUAL(15, output);
}

static void test_integerPID_integral_with_feedforward(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 100;

    integerPID pid(&input, &output, &setpoint, 0, 100, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetMode(AUTOMATIC);

    // With ki=100 (non-zero), error=90, kp=0, and feedforward=10
    // This tests that Compute with ki!=0 path is executed
    TEST_ASSERT_TRUE(pid.Compute(NOW, 10));
    // Output should be non-zero (either integral term or feedforward)
    TEST_ASSERT_NOT_EQUAL(0, output);
}

static void test_integerPID_output_limits_upper_clamp(void)
{
    long input = 1;
    long output = 0;
    long setpoint = 1000;

    integerPID pid(&input, &output, &setpoint, 255, 0, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(0, 50); // Set max to 50
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    TEST_ASSERT_EQUAL(50, output); // Should be clamped to max
}

static void test_integerPID_output_limits_lower_clamp(void)
{
    long input = 1000;
    long output = 0;
    long setpoint = 0;

    integerPID pid(&input, &output, &setpoint, 255, 0, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(100, 255); // Set min to 100
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    TEST_ASSERT_EQUAL(100, output); // Should be clamped to min
}

static void test_integerPID_output_limits_negative_range(void)
{
    long input = 1;
    long output = 0;
    long setpoint = 1000;

    integerPID pid(&input, &output, &setpoint, 255, 0, 0, REVERSE);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(-100, 10);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    TEST_ASSERT_LESS_OR_EQUAL(10, output);
    TEST_ASSERT_GREATER_OR_EQUAL(-100, output); // Output within negative range
}

static void test_integerPID_output_limits_no_clamp_needed(void)
{
    long input = 50;
    long output = 0;
    long setpoint = 100;

    integerPID pid(&input, &output, &setpoint, 10, 0, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(0, 255); // Wide limits
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    // Output should not be clamped; error=50, kp=10 yields ~15 internally
    TEST_ASSERT_GREATER_THAN(0, output);
    TEST_ASSERT_LESS_THAN(255, output);
}

static void test_integerPID_output_limits_affects_integral(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 200;

    integerPID pid(&input, &output, &setpoint, 0, 50, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(0, 20); // Tight limit
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    // Integral windup prevention: output should not exceed limit
    TEST_ASSERT_LESS_OR_EQUAL(20, output);
}

static void test_integerPID_auto_mode_output_limits(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 200;

    integerPID pid(&input, &output, &setpoint, 0, 50, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetMode(AUTOMATIC);

    output = 1000;
    pid.SetOutputLimits(-1001, 1001);
    TEST_ASSERT_EQUAL(1000, output);
    pid.Initialize();
    pid.SetOutputLimits(-20, 20);
    TEST_ASSERT_EQUAL(20, output);

    output = -1000;
    pid.SetOutputLimits(-1001, 1001);
    TEST_ASSERT_EQUAL(-1000, output);
    pid.Initialize();
    pid.SetOutputLimits(-20, 20);
    TEST_ASSERT_EQUAL(-20, output);
}

static void test_integerPID_derivative_term(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint, 0, 0, 1, DIRECT); // kp=0, ki=0, kd=1
    pid.SetSampleTime(1);
    pid.SetOutputLimits(-255, 255); // allow negative derivative output
    pid.SetTunings(0, 0, 1); // Re-set tunings after SetSampleTime
    pid.SetMode(AUTOMATIC);

    unsigned long now = NOW;
    TEST_ASSERT_TRUE(pid.Compute(now, 0));
    long first_output = output;

    // Change input to introduce derivative effect
    input = 15; // dInput = 15 - 10 = 5
    now += 100;
    TEST_ASSERT_TRUE(pid.Compute(now, 0));

    // kd = 1*32*1000 = 32000, 32000*5=160000, >>2=40000, >>10 ~39 (platform-specific signed shift)
    // Accept -39 or -40 due implementation-defined signed shift rounding.
    TEST_ASSERT_INT32_WITHIN(1, first_output - 40, output);
}

static void test_Compute_NoTimeChange_ReturnsFalse(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint, 10, 0, 0, DIRECT);
    pid.SetSampleTime(100); // Set sample time to 100ms
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute(NOW)); // First compute should succeed

    // Call Compute again with the same time, should return false due to no time change
    TEST_ASSERT_FALSE(pid.Compute(NOW));
}

static void test_integerPID_set_sample_time_recalculates_tunings(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint, 10, 10, 10, DIRECT);
    pid.SetSampleTime(1); // Initial sample time
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    long firstOutput = output;

    // Change sample time, which should recalculate tunings
    pid.SetSampleTime(500); // New sample time

    TEST_ASSERT_TRUE(pid.Compute(NOW+1000U));
    // Output should differ due to new tunings
    TEST_ASSERT_NOT_EQUAL(firstOutput, output);
}

static void test_integerPID_initialize_resets_state(void)
{
    long input = 10;
    long output = 50;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint, 0, 10, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute(NOW)); // Accumulate some integral

    pid.Initialize(); // Reset state
    TEST_ASSERT_TRUE(pid.Compute(NOW+1000U));
    // After reset, output should be based on new state (integral reset)
    TEST_ASSERT_NOT_EQUAL(50, output); // Should not be the initial output
}

static void test_integerPID_reset_integral_zeros_output_sum(void)
{
    long input = 1000;
    long output = 0;
    long setpoint = 2000;

    integerPID pid(&input, &output, &setpoint, 5, 100, 60, DIRECT);
    pid.SetSampleTime(1);
    pid.SetMode(AUTOMATIC);
    pid.SetOutputLimits(-5000, 5000);
    pid.Initialize();

    unsigned long now = NOW;
    TEST_ASSERT_TRUE(pid.Compute(now)); // Accumulate integral
    long firstOutput = output;
    
    pid.ResetIntegeral(); // Zero integral
    now += 1000;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_EQUAL_INT32(firstOutput, output);

    now += 1000;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_NOT_EQUAL_INT32(firstOutput, output);
}

static void test_integerPID_set_tunings_runtime_changes(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint, 10, 0, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    long originalOutput = output;

    pid.SetTunings(20, 0, 0); // Change Kp
    TEST_ASSERT_TRUE(pid.Compute(NOW+1000U));
    // Output should change due to new Kp
    TEST_ASSERT_NOT_EQUAL(originalOutput, output);
}

static void test_integerPID_compute_in_manual_mode_returns_false(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint, 10, 0, 0, DIRECT);
    pid.SetMode(MANUAL); // Manual mode

    TEST_ASSERT_FALSE(pid.Compute(NOW)); // Should return false in manual mode
    TEST_ASSERT_EQUAL(0, output); // Output unchanged
}

static void test_integerPID_set_output_limits_invalid_ignored(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint, 10, 0, 0, DIRECT);
    pid.SetOutputLimits(50, 20); // Invalid: Min >= Max
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    // Output should still be computed normally (limits ignored)
    TEST_ASSERT_NOT_EQUAL(0, output);
}

static void test_integerPID_set_controller_direction_runtime_manual(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint, 10, 0, 0, DIRECT);
    pid.SetMode(MANUAL); // Manual mode
    pid.SetControllerDirection(REVERSE); // Should not affect in manual mode
    pid.SetMode(AUTOMATIC);
    
    TEST_ASSERT_TRUE(pid.Compute(NOW));
    TEST_ASSERT_GREATER_THAN(0, output);
}

// Run the PID for 50 iterations and confirm it hits the setpoint
static inline void assert_pid_complete(integerPID &pid, long *pInput, long *pOutput, long setpoint, uint8_t sampleTime)
{
    UnityPrint("Iter,Input,Output"); UNITY_PRINT_EOL();

    char szMsg[64];
    for (uint16_t iteration=0; iteration<50U; ++iteration)
    {
        TEST_ASSERT_TRUE(pid.Compute(NOW+(iteration*sampleTime)));
        *pInput += *pOutput;

        snprintf(szMsg, _countof(szMsg)-1, "%" PRIu16 ", %" PRId32 ", %" PRId32, iteration, (int32_t)*pInput, (int32_t)*pOutput);
        UnityPrint(szMsg); UNITY_PRINT_EOL();
    }
    // Tolerance of 1%
    TEST_ASSERT_INT32_WITHIN(DIV_ROUND_CLOSEST(setpoint, 100, int32_t), setpoint, *pInput);
}

static void test_end_to_end_positive_positive_up(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    long output = 0;
    long input = 90;
    long setpoint = 155;

    integerPID pid(&input, &output, &setpoint, 3, 1, 2, DIRECT);
    pid.SetSampleTime(SAMPLE_TIME);
    pid.SetOutputLimits(-255, 255);
    pid.SetMode(AUTOMATIC);

    assert_pid_complete(pid, &input, &output, setpoint, SAMPLE_TIME);
}

static void test_end_to_end_positive_positive_down(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    long output = 0;
    long input = 235;
    long setpoint = 155;

    integerPID pid(&input, &output, &setpoint, 5, 2, 4, DIRECT);
    pid.SetSampleTime(SAMPLE_TIME);
    pid.SetOutputLimits(-255, 255);
    pid.SetMode(AUTOMATIC);

    assert_pid_complete(pid, &input, &output, setpoint, SAMPLE_TIME);
}

static void test_end_to_end_negative_negative_up(void) 
{
    TEST_IGNORE_MESSAGE("integerPID doesn't support negative inputs...yet");
}
static void test_end_to_end_negative_negative_down(void) 
{
    TEST_IGNORE_MESSAGE("integerPID doesn't support negative inputs...yet");
}
static void test_end_to_end_negative_positive(void) 
{
    TEST_IGNORE_MESSAGE("integerPID doesn't support negative inputs...yet");
}
static void test_end_to_end_positive_negative(void) 
{
    TEST_IGNORE_MESSAGE("integerPID doesn't support negative inputs...yet");
}

void testIntegerPID(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_integerPID_manual_mode_compute_false);
        RUN_TEST_P(test_integerPID_auto_mode_p_on_error);
        RUN_TEST_P(test_integerPID_output_limits_clamp);
        RUN_TEST_P(test_integerPID_output_limits_zero_range);
        RUN_TEST_P(test_integerPID_reverse_direction);
        RUN_TEST_P(test_integerPID_feedforward_term);
        RUN_TEST_P(test_integerPID_input_zero_failsafe);
        RUN_TEST_P(test_integerPID_integral_with_feedforward);
        RUN_TEST_P(test_integerPID_output_limits_upper_clamp);
        RUN_TEST_P(test_integerPID_output_limits_lower_clamp);
        RUN_TEST_P(test_integerPID_output_limits_negative_range);
        RUN_TEST_P(test_integerPID_output_limits_no_clamp_needed);
        RUN_TEST_P(test_integerPID_output_limits_affects_integral);
        RUN_TEST_P(test_integerPID_controller_direction_switches_effect);
        RUN_TEST_P(test_integerPID_controller_direction_maintains_output_limits);
        RUN_TEST_P(test_integerPID_auto_mode_output_limits);
        RUN_TEST_P(test_integerPID_derivative_term);
        RUN_TEST_P(test_Compute_NoTimeChange_ReturnsFalse);
        RUN_TEST_P(test_integerPID_set_sample_time_recalculates_tunings);
        RUN_TEST_P(test_integerPID_initialize_resets_state);
        RUN_TEST_P(test_integerPID_reset_integral_zeros_output_sum);
        RUN_TEST_P(test_integerPID_set_tunings_runtime_changes);
        RUN_TEST_P(test_integerPID_compute_in_manual_mode_returns_false);
        RUN_TEST_P(test_integerPID_set_output_limits_invalid_ignored);
        RUN_TEST_P(test_integerPID_set_controller_direction_runtime_manual);
        RUN_TEST_P(test_end_to_end_positive_positive_up);
        RUN_TEST_P(test_end_to_end_positive_positive_down);
        RUN_TEST_P(test_end_to_end_negative_negative_down);
        RUN_TEST_P(test_end_to_end_negative_negative_up);
        RUN_TEST_P(test_end_to_end_negative_negative_down);
        RUN_TEST_P(test_end_to_end_negative_positive);
        RUN_TEST_P(test_end_to_end_positive_negative);
    }
}