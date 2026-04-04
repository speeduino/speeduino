#include <unity.h>
#include "src/PID/integerPID.h"
#include "../test_utils.h"

constexpr unsigned long NOW = 10000UL;

static void test_integerPID_manual_mode_compute_false(void)
{
    long input = 10;
    long output = 8;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(10, 0, 0);

    TEST_ASSERT_FALSE(pid.Compute(NOW));
    TEST_ASSERT_EQUAL(8, output);
}

static void test_integerPID_auto_mode_p_on_error(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(10, 0, 0);
    pid.SetSampleTime(1);
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    // Expected: kp scaled to 320, error=10, output=3200>>10=3
    TEST_ASSERT_EQUAL(3, output);
}

static void test_integerPID_output_limits_clamp(void)
{
    long input = 1;
    long output = 0;
    long setpoint = 1000;

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(255, 0, 0);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(0, 10);
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    TEST_ASSERT_EQUAL(10, output);
}

static void test_integerPID_output_limits_zero_range(void)
{
    long input = 1;
    long output = 0;
    long setpoint = 1000;

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(255, 0, 0);
    pid.SetSampleTime(1);
    pid.activate();

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

    integerPID pid(&input, &output, &setpoint);
    pid.SetControllerDirection(PidDirection::Direct);
    pid.SetTunings(10, 0, 0);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(-255, 255);
    pid.activate();

    unsigned long now = NOW;
    TEST_ASSERT_TRUE(pid.Compute(now));
    long directOutput = output;
    TEST_ASSERT_GREATER_THAN(0, directOutput);

    pid.SetControllerDirection(PidDirection::Reverse);
    now += 100;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_NOT_EQUAL(directOutput, output);
    TEST_ASSERT_LESS_THAN(0, output);

    pid.SetControllerDirection(PidDirection::Direct);
    now += 100;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_GREATER_THAN(0, output);
}

static void test_integerPID_controller_direction_maintains_output_limits(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 100;

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(255, 0, 0);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(-50, 50);
    pid.activate();

    unsigned long now = NOW;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_LESS_OR_EQUAL(50, output);

    pid.SetControllerDirection(PidDirection::Reverse);
    now += 100;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_GREATER_OR_EQUAL(-50, output);
}

static void test_integerPID_reverse_direction(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(10, 0, 0);
    pid.SetControllerDirection(PidDirection::Reverse);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(-255, 255);
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    TEST_ASSERT_LESS_THAN(0, output);  // negative output expected in reverse
}

static void test_integerPID_feedforward_term(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint);
    pid.SetSampleTime(1);
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute(NOW, 15));
    TEST_ASSERT_EQUAL(15, output);
}

static void test_integerPID_integral_with_feedforward(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 100;

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(0, 100, 0);
    pid.SetSampleTime(1);
    pid.activate();

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

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(255, 0, 0);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(0, 50); // Set max to 50
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    TEST_ASSERT_EQUAL(50, output); // Should be clamped to max
}

static void test_integerPID_output_limits_lower_clamp(void)
{
    long input = 1000;
    long output = 0;
    long setpoint = 0;

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(255, 0, 0);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(100, 255); // Set min to 100
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    TEST_ASSERT_EQUAL(100, output); // Should be clamped to min
}

static void test_integerPID_output_limits_negative_range(void)
{
    long input = 1;
    long output = 0;
    long setpoint = 1000;

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(255, 0, 0);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(-100, 10);
    pid.SetControllerDirection(PidDirection::Reverse);
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    TEST_ASSERT_LESS_OR_EQUAL(10, output);
    TEST_ASSERT_GREATER_OR_EQUAL(-100, output); // Output within negative range
}

static void test_integerPID_output_limits_no_clamp_needed(void)
{
    long input = 50;
    long output = 0;
    long setpoint = 100;

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(10, 0, 0);
    pid.SetControllerDirection(PidDirection::Direct);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(0, 255); // Wide limits
    pid.activate();

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

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(0, 50, 0);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(0, 20); // Tight limit
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    // Integral windup prevention: output should not exceed limit
    TEST_ASSERT_LESS_OR_EQUAL(20, output);
}

static void test_integerPID_auto_mode_output_limits(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 200;

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(0, 50, 0);
    pid.SetSampleTime(1);
    pid.activate();

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

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(0, 0, 1); // kp=0, ki=0, kd=1
    pid.SetSampleTime(1);
    pid.SetOutputLimits(-255, 255); // allow negative derivative output
    pid.SetTunings(0, 0, 1); // Re-set tunings after SetSampleTime
    pid.activate();

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

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(10, 0, 0);
    pid.SetSampleTime(100); // Set sample time to 100ms
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute(NOW)); // First compute should succeed

    // Call Compute again with the same time, should return false due to no time change
    TEST_ASSERT_FALSE(pid.Compute(NOW));
}

static void test_integerPID_set_sample_time_recalculates_tunings(void)
{
    long input = 1000;
    long output = 0;
    long setpoint = 2000;

    integerPID pid(&input, &output, &setpoint);
    pid.SetControllerDirection(PidDirection::Direct);
    pid.SetSampleTime(1); // Initial sample time
    pid.SetTunings(128, 64, 255);
    pid.SetOutputLimits(-5000, 5000);
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    long firstOutput = output;

    // Change sample time, which should recalculate tunings
    pid.SetSampleTime(500); // New sample time

    TEST_ASSERT_TRUE(pid.Compute(NOW+1000U));
    // Output should differ due to new tunings
    TEST_ASSERT_NOT_EQUAL_INT32(firstOutput, output);
}

static void test_integerPID_initialize_resets_state(void)
{
    long input = 10;
    long output = 50;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(0, 10, 0);
    pid.SetSampleTime(1);
    pid.activate();

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

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(5, 100, 60);
    pid.SetSampleTime(1);
    pid.activate();
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

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(10, 0, 0);
    pid.SetSampleTime(1);
    pid.activate();

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

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(10, 0, 0);

    TEST_ASSERT_FALSE(pid.Compute(NOW)); // Should return false in manual mode
    TEST_ASSERT_EQUAL(0, output); // Output unchanged
}

static void test_integerPID_set_output_limits_invalid_ignored(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(10, 0, 0);
    pid.SetOutputLimits(50, 20); // Invalid: Min >= Max
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute(NOW));
    // Output should still be computed normally (limits ignored)
    TEST_ASSERT_NOT_EQUAL(0, output);
}

static void test_integerPID_set_controller_direction_runtime_manual(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(10, 0, 0);
    pid.SetControllerDirection(PidDirection::Reverse);
    pid.SetOutputLimits(-25, 25);
    pid.activate();
    
    TEST_ASSERT_TRUE(pid.Compute(NOW));
    TEST_ASSERT_LESS_THAN(0, output);
}

static String createIterationMsg(int16_t iteration, long input, long output)
{
    char szMsg[64];
    snprintf(szMsg, _countof(szMsg)-1, "%" PRId16 ", %" PRId32 ", %" PRId32, iteration, (int32_t)input, (int32_t)output);
    return szMsg;
}

// Run the PID for 50 iterations and confirm it hits the setpoint
static inline void assert_pid_complete(integerPID &pid, long *pInput, long *pOutput, long setpoint, uint8_t sampleTime)
{
    char szMsg[64];
    snprintf(szMsg, _countof(szMsg)-1, "Start: %" PRId32 ", SetPoint: %" PRId32, (int32_t)*pInput, (int32_t)setpoint);
    UnityPrint(szMsg); UNITY_PRINT_EOL();

    UnityPrint("Iter,Input,Output"); UNITY_PRINT_EOL();
    UnityPrint(createIterationMsg(-1, *pInput, setpoint).c_str()); UNITY_PRINT_EOL();

    for (uint16_t iteration=0; iteration<50U; ++iteration)
    {
        TEST_ASSERT_TRUE(pid.Compute(NOW+(iteration*sampleTime)));
        *pInput += *pOutput;
        UnityPrint(createIterationMsg(iteration, *pInput, *pOutput).c_str()); UNITY_PRINT_EOL();
    }
    // Tolerance of 1%
    TEST_ASSERT_INT32_WITHIN(abs(DIV_ROUND_CLOSEST(setpoint, 100, int32_t)), setpoint, *pInput);
}

static void test_end_to_end_positive_positive_up(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    long output = 0;
    long input = 90;
    long setpoint = 155;

    integerPID pid(&input, &output, &setpoint);
    pid.SetSampleTime(SAMPLE_TIME);
    pid.SetTunings(3, 1, 2);
    pid.SetOutputLimits(-255, 255);
    pid.activate();

    assert_pid_complete(pid, &input, &output, setpoint, SAMPLE_TIME);
}

static void test_end_to_end_positive_positive_down(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    long output = 0;
    long input = 235;
    long setpoint = 155;

    integerPID pid(&input, &output, &setpoint);
    pid.SetSampleTime(SAMPLE_TIME);
    pid.SetTunings(5, 2, 4);
    pid.SetOutputLimits(-255, 255);
    pid.activate();

    assert_pid_complete(pid, &input, &output, setpoint, SAMPLE_TIME);
}


static void test_end_to_end_negative_negative_up(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    long output = 0;
    long input = -235;
    long setpoint = -155;

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(15, 3, 2);
    pid.SetSampleTime(SAMPLE_TIME);
    pid.SetOutputLimits(-255, 255);
    pid.activate();

    assert_pid_complete(pid, &input, &output, setpoint, SAMPLE_TIME);
}

static void test_end_to_end_negative_negative_down(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    long output = 0;
    long input = -155;
    long setpoint = -235;

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(15, 3, 2);
    pid.SetSampleTime(SAMPLE_TIME);
    pid.SetOutputLimits(-255, 255);
    pid.activate();

    assert_pid_complete(pid, &input, &output, setpoint, SAMPLE_TIME);
}

static void test_end_to_end_positive_negative(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    long output = 0;
    long input = 63;
    long setpoint = -55;

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(15, 1, 1);
    pid.SetSampleTime(SAMPLE_TIME);
    pid.SetOutputLimits(-255, 255);
    pid.activate();

    assert_pid_complete(pid, &input, &output, setpoint, SAMPLE_TIME);
}

static void test_end_to_end_negative_positive(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    long output = 0;
    long input = -55;
    long setpoint = 65;

    integerPID pid(&input, &output, &setpoint);
    pid.SetTunings(15, 3, 2);
    pid.SetSampleTime(SAMPLE_TIME);
    pid.SetOutputLimits(-255, 255);
    pid.activate();

    assert_pid_complete(pid, &input, &output, setpoint, SAMPLE_TIME);
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