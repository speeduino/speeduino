#include <unity.h>
#include "src/PID/integerPID.h"
#include "../test_utils.h"

constexpr unsigned long NOW = 10000UL;

static void test_integerPID_manual_mode_compute_false(void)
{
    long input = 10;
    long output = 8;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(10, 0, 0), PidDirection::Direct, NOW, 250);
    pid.setTargetValue(20);

    TEST_ASSERT_FALSE(pid.Compute(NOW, input, &output));
    TEST_ASSERT_EQUAL(8, output);
}

static void test_integerPID_auto_mode_p_on_error(void)
{
    long input = 10;
    long output = 0;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(10, 0, 0), PidDirection::Direct, NOW, 250);
    pid.setTargetValue(20);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    // Expected: kp scaled to 320, error=10, output=3200>>10=3
    TEST_ASSERT_EQUAL(3, output);
}

static void test_integerPID_output_limits_clamp(void)
{
    long input = 1;
    long output = 0;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(255, 0, 0), PidDirection::Direct, NOW, 250);
    pid.setTargetValue(1000);
    pid.SetOutputLimits(0, 10);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    TEST_ASSERT_EQUAL(10, output);
}

static void test_integerPID_controller_direction_switches_effect(void)
{
    long input = 10;
    long output = 0;
    unsigned long now = NOW;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(10, 0, 0), PidDirection::Direct, now, 250);
    pid.SetOutputLimits(-255, 255);
    pid.setTargetValue(100);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.Compute(now, input, &output));
    long directOutput = output;
    TEST_ASSERT_GREATER_THAN(0, directOutput);

    pid.SetTunings(PidTuningParameters(10, 0, 0), PidDirection::Reverse, now, 1);
    now += 100;
    TEST_ASSERT_TRUE(pid.Compute(now, input, &output));
    TEST_ASSERT_NOT_EQUAL(directOutput, output);
    TEST_ASSERT_LESS_THAN(0, output);

    pid.SetTunings(PidTuningParameters(10, 0, 0), PidDirection::Direct, now, 250);
    now += 100;
    TEST_ASSERT_TRUE(pid.Compute(now, input, &output));
    TEST_ASSERT_GREATER_THAN(0, output);
}

static void test_integerPID_controller_direction_maintains_output_limits(void)
{
    long input = 10;
    long output = 0;
    unsigned long now = NOW;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(255, 0, 0), PidDirection::Direct, now, 250);
    pid.SetOutputLimits(-50, 50);
    pid.setTargetValue(100);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.Compute(now, input, &output));
    TEST_ASSERT_LESS_OR_EQUAL(50, output);

    pid.SetTunings(PidTuningParameters(10, 0, 0), PidDirection::Reverse, now, 1);
    now += 100;
    TEST_ASSERT_TRUE(pid.Compute(now, input, &output));
    TEST_ASSERT_GREATER_OR_EQUAL(-50, output);
}

static void test_integerPID_reverse_direction(void)
{
    long input = 10;
    long output = 0;
    
    integerPID pid;
    pid.SetTunings(PidTuningParameters(10, 0, 0), PidDirection::Reverse, NOW, 1);
    pid.SetOutputLimits(-255, 255);
    pid.setTargetValue(20);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    TEST_ASSERT_LESS_THAN(0, output);  // negative output expected in reverse
}

static void test_integerPID_feedforward_term(void)
{
    long input = 10;
    long output = 0;

    integerPID pid;
    pid.setTargetValue(20);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, 15, &output));
    TEST_ASSERT_EQUAL(15, output);
}

static void test_integerPID_integral_with_feedforward(void)
{
    long input = 10;
    long output = 0;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(0, 100, 0), PidDirection::Direct, NOW, 250);
    pid.setTargetValue(100);
    pid.activate(input);

    // With ki=100 (non-zero), error=90, kp=0, and feedforward=10
    // This tests that Compute with ki!=0 path is executed
    TEST_ASSERT_TRUE(pid.Compute(NOW, input, 10, &output));
    // Output should be non-zero (either integral term or feedforward)
    TEST_ASSERT_NOT_EQUAL(0, output);
}

static void test_integerPID_output_limits_upper_clamp(void)
{
    long input = 1;
    long output = 0;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(255, 0, 0), PidDirection::Direct, NOW, 250);
    pid.SetOutputLimits(0, 50); // Set max to 50
    pid.setTargetValue(1000);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    TEST_ASSERT_EQUAL(50, output); // Should be clamped to max
}

static void test_integerPID_output_limits_lower_clamp(void)
{
    long input = 1000;
    long output = 0;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(255, 0, 0), PidDirection::Direct, NOW, 250);
    pid.SetOutputLimits(100, 255); // Set min to 100
    pid.setTargetValue(0);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    TEST_ASSERT_EQUAL(100, output); // Should be clamped to min
}

static void test_integerPID_output_limits_negative_range(void)
{
    long input = 1;
    long output = 0;
    
    integerPID pid;
    pid.SetTunings(PidTuningParameters(255, 0, 0), PidDirection::Reverse, NOW, 1);
    pid.SetOutputLimits(-100, 10);
    pid.setTargetValue(1000);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    TEST_ASSERT_LESS_OR_EQUAL(10, output);
    TEST_ASSERT_GREATER_OR_EQUAL(-100, output); // Output within negative range
}

static void test_integerPID_output_limits_no_clamp_needed(void)
{
    long input = 50;
    long output = 0;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(10, 0, 0), PidDirection::Direct, NOW, 250);
    pid.SetOutputLimits(0, 255); // Wide limits
    pid.setTargetValue(100);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    // Output should not be clamped; error=50, kp=10 yields ~15 internally
    TEST_ASSERT_GREATER_THAN(0, output);
    TEST_ASSERT_LESS_THAN(255, output);
}

static void test_integerPID_output_limits_affects_integral(void)
{
    long input = 10;
    long output = 0;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(0, 50, 0), PidDirection::Direct, NOW, 250);
    pid.SetOutputLimits(0, 20); // Tight limit
    pid.setTargetValue(200);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    // Integral windup prevention: output should not exceed limit
    TEST_ASSERT_LESS_OR_EQUAL(20, output);
}

static void test_integerPID_derivative_term(void)
{
    long input = 10;
    long output = 0;
    unsigned long now = NOW;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(0, 0, 1), PidDirection::Direct, now, 1); // kp=0, ki=0, kd=1
    pid.SetOutputLimits(-255, 255); // allow negative derivative output
    pid.setTargetValue(20);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.Compute(now, input, &output));
    long first_output = output;

    // Change input to introduce derivative effect
    input = 15; // dInput = 15 - 10 = 5
    now += 100;
    TEST_ASSERT_TRUE(pid.Compute(now, input, &output));

    // kd = 1*32*1000 = 32000, 32000*5=160000, >>2=40000, >>10 ~39 (platform-specific signed shift)
    // Accept -39 or -40 due implementation-defined signed shift rounding.
    TEST_ASSERT_INT32_WITHIN(1, first_output - 40, output);
}

static void test_Compute_NoTimeChange_ReturnsFalse(void)
{
    long input = 10;
    long output = 0;
    
    integerPID pid;
    pid.SetTunings(PidTuningParameters(10, 0, 0), PidDirection::Direct, NOW, 100);
    pid.setTargetValue(20);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output)); // First compute should succeed

    // Call Compute again with the same time, should return false due to no time change
    TEST_ASSERT_FALSE(pid.Compute(NOW, input, &output));
}

static void test_integerPID_set_sample_time_recalculates_tunings(void)
{
    long input = 1000;
    long output = 0;
    PidTuningParameters tuningParameters(128, 64, 255);

    integerPID pid;
    pid.SetOutputLimits(-5000, 5000);
    pid.SetTunings(tuningParameters, PidDirection::Direct, NOW, 1);
    pid.setTargetValue(2000);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    long firstOutput = output;

    // Change sample time, which should recalculate tunings
    pid.SetTunings(PidTuningParameters(128, 64, 255), PidDirection::Direct, NOW, 500); // New sample time

    TEST_ASSERT_TRUE(pid.Compute(NOW+1000U, input, &output));
    // Output should differ due to new tunings
    TEST_ASSERT_NOT_EQUAL_INT32(firstOutput, output);
}

static void test_integerPID_initialize_resets_state(void)
{
    long input = 10;
    long output = 50;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(0, 10, 0), PidDirection::Direct, NOW, 1);
    pid.setTargetValue(20);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output)); // Accumulate some integral

    pid.Initialize(input); // Reset state
    TEST_ASSERT_TRUE(pid.Compute(NOW+1000U, input, &output));
    // After reset, output should be based on new state (integral reset)
    TEST_ASSERT_NOT_EQUAL(50, output); // Should not be the initial output
}

static void test_integerPID_reset_integral_zeros_output_sum(void)
{
    long input = 1000;
    long output = 0;
    unsigned long now = NOW;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(5, 100, 60), PidDirection::Direct, now, 1);
    pid.setTargetValue(2000);
    pid.activate(input);
    pid.SetOutputLimits(-5000, 5000);
    pid.Initialize(input);

    TEST_ASSERT_TRUE(pid.Compute(now, input, &output)); // Accumulate integral
    long firstOutput = output;
    
    pid.ResetIntegeral(); // Zero integral
    now += 1000;
    TEST_ASSERT_TRUE(pid.Compute(now, input, &output));
    TEST_ASSERT_EQUAL_INT32(firstOutput, output);

    now += 1000;
    TEST_ASSERT_TRUE(pid.Compute(now, input, &output));
    TEST_ASSERT_NOT_EQUAL_INT32(firstOutput, output);
}

static void test_integerPID_set_tunings_runtime_changes(void)
{
    long input = 10;
    long output = 0;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(10, 0, 0), PidDirection::Direct, NOW, 1);
    pid.setTargetValue(20);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    long originalOutput = output;

    pid.SetTunings(PidTuningParameters(20, 0, 0), PidDirection::Direct, NOW, 1); // Change Kp
    TEST_ASSERT_TRUE(pid.Compute(NOW+1000U, input, &output));
    // Output should change due to new Kp
    TEST_ASSERT_NOT_EQUAL(originalOutput, output);
}

static void test_integerPID_compute_in_manual_mode_returns_false(void)
{
    long input = 10;
    long output = 0;

    integerPID pid;
    pid.setTargetValue(20);
    pid.SetTunings(PidTuningParameters(10, 0, 0), PidDirection::Direct, NOW, 1);

    TEST_ASSERT_FALSE(pid.Compute(NOW, input, &output)); // Should return false in manual mode
    TEST_ASSERT_EQUAL(0, output); // Output unchanged
}

static void test_integerPID_set_output_limits_invalid_ignored(void)
{
    long input = 10;
    long output = 0;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(10, 0, 0), PidDirection::Direct, NOW, 1);
    pid.SetOutputLimits(50, 20); // Invalid: Min >= Max
    pid.setTargetValue(20);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    // Output should still be computed normally (limits ignored)
    TEST_ASSERT_NOT_EQUAL(0, output);
}

static void test_integerPID_set_controller_direction_runtime_manual(void)
{
    long input = 10;
    long output = 0;

    integerPID pid;
    pid.setTargetValue(20);
    pid.SetTunings(PidTuningParameters(10, 0, 0), PidDirection::Reverse, NOW, 1);
    pid.SetOutputLimits(-25, 25);
    pid.activate(input);
    
    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    TEST_ASSERT_LESS_THAN(0, output);
}

static String createIterationMsg(int16_t iteration, long input, long output)
{
    char szMsg[64];
    snprintf(szMsg, _countof(szMsg)-1, "%" PRId16 ", %" PRId32 ", %" PRId32, iteration, (int32_t)input, (int32_t)output);
    return szMsg;
}

// Run the PID for 50 iterations and confirm it hits the setpoint
static inline void assert_pid_complete(integerPID &pid, long input, long setpoint, uint8_t sampleTime)
{
    UnityPrint("Iter,Input,Output"); UNITY_PRINT_EOL();
    UnityPrint(createIterationMsg(-1, input, setpoint).c_str()); UNITY_PRINT_EOL();

    pid.setTargetValue(setpoint);
    long output;
    for (uint16_t iteration=0; iteration<50U; ++iteration)
    {
        TEST_ASSERT_TRUE(pid.Compute(NOW+(iteration*sampleTime), input, &output));
        input += output;
        UnityPrint(createIterationMsg(iteration, input, output).c_str()); UNITY_PRINT_EOL();
    }
    // Tolerance of 1%
    TEST_ASSERT_INT32_WITHIN(DIV_ROUND_CLOSEST(setpoint, 100, int32_t), setpoint, input);
}

static void test_end_to_end_positive_positive_up(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    constexpr long START_POINT = 90;
    constexpr long SET_POINT = 155;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(3, 1, 2), PidDirection::Direct, NOW, SAMPLE_TIME);
    pid.SetOutputLimits(-255, 255);
    pid.activate(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, SAMPLE_TIME);
}

static void test_end_to_end_positive_positive_down(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    constexpr long START_POINT = 235;
    constexpr long SET_POINT = 155;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(5, 2, 4), PidDirection::Direct, NOW, SAMPLE_TIME);
    pid.SetOutputLimits(-255, 255);
    pid.activate(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, SAMPLE_TIME);
}


static void test_end_to_end_negative_negative_up(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    constexpr long START_POINT = -235;
    constexpr long SET_POINT = -155;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(15, 3, 2), PidDirection::Direct, NOW, SAMPLE_TIME);
    pid.SetOutputLimits(-255, 255);
    pid.activate(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, SAMPLE_TIME);
}

static void test_end_to_end_negative_negative_down(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    constexpr long START_POINT = -155;
    constexpr long SET_POINT = -235;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(15, 3, 2), PidDirection::Direct, NOW, SAMPLE_TIME);
    pid.SetOutputLimits(-255, 255);
    pid.activate(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, SAMPLE_TIME);
}

static void test_end_to_end_positive_negative(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    constexpr long START_POINT = 63;
    constexpr long SET_POINT = -55;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(15, 1, 1), PidDirection::Direct, NOW, SAMPLE_TIME);
    pid.SetOutputLimits(-255, 255);
    pid.activate(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, SAMPLE_TIME);
}

static void test_end_to_end_negative_positive(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    constexpr long START_POINT = -55;
    constexpr long SET_POINT = 65;

    integerPID pid;
    pid.SetTunings(PidTuningParameters(15, 3, 2), PidDirection::Direct, NOW, SAMPLE_TIME);
    pid.SetOutputLimits(-255, 255);
    pid.activate(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, SAMPLE_TIME);
}

void testIntegerPID(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_integerPID_manual_mode_compute_false);
        RUN_TEST_P(test_integerPID_auto_mode_p_on_error);
        RUN_TEST_P(test_integerPID_output_limits_clamp);
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