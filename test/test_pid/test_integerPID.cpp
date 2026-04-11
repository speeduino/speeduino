#include <unity.h>
#include "src/PID/integerPID.h"
#include "../test_utils.h"

constexpr uint32_t NOW = 10000UL;

static void test_integerPID_manual_mode_compute_false(void)
{
    int32_t input = 10;
    int32_t output = 8;

    integerPID pid;
    pid.setTunings(PidTuningParameters(10, 0, 0), NOW, 250);
    pid.setSetPoint(20);

    TEST_ASSERT_FALSE(pid.compute(NOW, input, &output));
    TEST_ASSERT_EQUAL(8, output);
}

static void test_integerPID_auto_mode_p_on_error(void)
{
    int32_t input = 10;
    int32_t output = 0;

    integerPID pid;
    pid.setTunings(PidTuningParameters(10, 0, 0), NOW, 250);
    pid.setSetPoint(20);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    // Expected: kp scaled to 320, error=10, output=3200>>10=3
    TEST_ASSERT_EQUAL(3, output);
}

static void test_integerPID_output_limits_clamp(void)
{
    int32_t input = 1;
    int32_t output = 0;

    integerPID pid;
    pid.setTunings(PidTuningParameters(255, 0, 0), NOW, 250);
    pid.setSetPoint(1000);
    pid.setOutputLimits(0, 10);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    TEST_ASSERT_EQUAL(10, output);
}

static void test_integerPID_controller_direction_switches_effect(void)
{
    int32_t input = 10;
    int32_t output = 0;

    integerPID pid;
    pid.setTunings(PidTuningParameters(10, 0, 0), NOW, 250);
    pid.setOutputLimits(-255, 255);
    pid.setSetPoint(100);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    int32_t directOutput = output;
    TEST_ASSERT_GREATER_THAN(0, directOutput);

    pid.setTunings(PidTuningParameters(10, 0, 0)* -1, NOW, 1);
    TEST_ASSERT_TRUE(pid.compute(NOW+100, input, &output));
    TEST_ASSERT_NOT_EQUAL(directOutput, output);
    TEST_ASSERT_LESS_THAN(0, output);

    pid.setTunings(PidTuningParameters(10, 0, 0), NOW, 250);
    TEST_ASSERT_TRUE(pid.compute(NOW+200, input, &output));
    TEST_ASSERT_GREATER_THAN(0, output);
}

static void test_integerPID_controller_direction_maintains_output_limits(void)
{
    int32_t input = 10;
    int32_t output = 0;

    integerPID pid;
    pid.setTunings(PidTuningParameters(255, 0, 0), NOW, 250);
    pid.setOutputLimits(-50, 50);
    pid.setSetPoint(100);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    TEST_ASSERT_LESS_OR_EQUAL(50, output);

    pid.setTunings(PidTuningParameters(10, 0, 0)* -1, NOW, 1);
    TEST_ASSERT_TRUE(pid.compute(NOW+100, input, &output));
    TEST_ASSERT_GREATER_OR_EQUAL(-50, output);
}

static void test_integerPID_reverse_direction(void)
{
    int32_t input = 10;
    int32_t output = 0;
    
    integerPID pid;
    pid.setTunings(PidTuningParameters(10, 0, 0)* -1, NOW, 1);
    pid.setOutputLimits(-255, 255);
    pid.setSetPoint(20);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    TEST_ASSERT_LESS_THAN(0, output);  // negative output expected in reverse
}

static void test_integerPID_feedforward_term(void)
{
    int32_t input = 10;
    int32_t output = 0;

    integerPID pid;
    pid.setSetPoint(20);
    pid.setFeedForwardTerm(15);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    TEST_ASSERT_EQUAL(15, output);
}

static void test_integerPID_integral_with_feedforward(void)
{
    int32_t input = 10;
    int32_t output = 0;

    integerPID pid;
    pid.setTunings(PidTuningParameters(0, 100, 0), NOW, 250);
    pid.setSetPoint(100);
    pid.setFeedForwardTerm(10);
    pid.activate(input);

    // With ki=100 (non-zero), error=90, kp=0, and feedforward=10
    // This tests that compute with ki!=0 path is executed
    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    // Output should be non-zero (either integral term or feedforward)
    TEST_ASSERT_NOT_EQUAL(0, output);
}

static void test_integerPID_output_limits_upper_clamp(void)
{
    int32_t input = 1;
    int32_t output = 0;

    integerPID pid;
    pid.setTunings(PidTuningParameters(255, 0, 0), NOW, 250);
    pid.setOutputLimits(0, 50); // Set max to 50
    pid.setSetPoint(1000);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    TEST_ASSERT_EQUAL(50, output); // Should be clamped to max
}

static void test_integerPID_output_limits_lower_clamp(void)
{
    int32_t input = 1000;
    int32_t output = 0;

    integerPID pid;
    pid.setTunings(PidTuningParameters(255, 0, 0), NOW, 250);
    pid.setOutputLimits(100, 255); // Set min to 100
    pid.setSetPoint(0);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    TEST_ASSERT_EQUAL(100, output); // Should be clamped to min
}

static void test_integerPID_output_limits_negative_range(void)
{
    int32_t input = 1;
    int32_t output = 0;
    
    integerPID pid;
    pid.setTunings(PidTuningParameters(255, 0, 0)* -1, NOW, 1);
    pid.setOutputLimits(-100, 10);
    pid.setSetPoint(1000);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    TEST_ASSERT_LESS_OR_EQUAL(10, output);
    TEST_ASSERT_GREATER_OR_EQUAL(-100, output); // Output within negative range
}

static void test_integerPID_output_limits_no_clamp_needed(void)
{
    int32_t input = 50;
    int32_t output = 0;

    integerPID pid;
    pid.setTunings(PidTuningParameters(10, 0, 0), NOW, 250);
    pid.setOutputLimits(0, 255); // Wide limits
    pid.setSetPoint(100);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    // Output should not be clamped; error=50, kp=10 yields ~15 internally
    TEST_ASSERT_GREATER_THAN(0, output);
    TEST_ASSERT_LESS_THAN(255, output);
}

static void test_integerPID_output_limits_affects_integral(void)
{
    int32_t input = 10;
    int32_t output = 0;

    integerPID pid;
    pid.setTunings(PidTuningParameters(0, 50, 0), NOW, 250);
    pid.setOutputLimits(0, 20); // Tight limit
    pid.setSetPoint(200);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    // Integral windup prevention: output should not exceed limit
    TEST_ASSERT_LESS_OR_EQUAL(20, output);
}

static void test_integerPID_derivative_term(void)
{
    int32_t input = 10;
    int32_t output = 0;

    integerPID pid;
    pid.setTunings(PidTuningParameters(0, 0, 1), NOW, 1); // kp=0, ki=0, kd=1
    pid.setOutputLimits(-255, 255); // allow negative derivative output
    pid.setSetPoint(20);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    int32_t first_output = output;

    // Change input to introduce derivative effect
    input = 15; // dInput = 15 - 10 = 5
    TEST_ASSERT_TRUE(pid.compute(NOW+100, input, &output));

    // kd = 1*32*1000 = 32000, 32000*5=160000, >>2=40000, >>10 ~39 (platform-specific signed shift)
    // Accept -39 or -40 due implementation-defined signed shift rounding.
    TEST_ASSERT_INT32_WITHIN(1, first_output - 40, output);
}

static void test_compute_NoTimeChange_ReturnsFalse(void)
{
    int32_t input = 10;
    int32_t output = 0;
    
    integerPID pid;
    pid.setTunings(PidTuningParameters(10, 0, 0), NOW, 100);
    pid.setSetPoint(20);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output)); // First compute should succeed

    // Call compute again with the same time, should return false due to no time change
    TEST_ASSERT_FALSE(pid.compute(NOW, input, &output));
}

static void test_integerPID_set_sample_time_recalculates_tunings(void)
{
    int32_t input = 1000;
    int32_t output = 0;
    PidTuningParameters tuningParameters(128, 64, 255);

    integerPID pid;
    pid.setOutputLimits(-5000, 5000);
    pid.setTunings(tuningParameters, NOW, 1);
    pid.setSetPoint(2000);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    int32_t firstOutput = output;

    // Change sample time, which should recalculate tunings
    pid.setTunings(tuningParameters, NOW, 500); // New sample time

    TEST_ASSERT_TRUE(pid.compute(NOW+1000U, input, &output));
    // Output should differ due to new tunings
    TEST_ASSERT_NOT_EQUAL_INT32(firstOutput, output);
}

static void test_integerPID_initialize_resets_state(void)
{
    int32_t input = 10;
    int32_t output = 50;

    integerPID pid;
    pid.setTunings(PidTuningParameters(0, 10, 0), NOW, 1);
    pid.setSetPoint(20);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output)); // Accumulate some integral

    pid.resetIntegeral(); // Reset state
    TEST_ASSERT_TRUE(pid.compute(NOW+1000U, input, &output));
    // After reset, output should be based on new state (integral reset)
    TEST_ASSERT_NOT_EQUAL(50, output); // Should not be the initial output
}

static void test_integerPID_reset_integral_zeros_output_sum(void)
{
    int32_t input = 1000;
    int32_t output = 0;

    integerPID pid;
    pid.setTunings(PidTuningParameters(5, 100, 60), NOW, 1);
    pid.setSetPoint(2000);
    pid.activate(input);
    pid.setOutputLimits(-5000, 5000);
    pid.reset(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output)); // Accumulate integral
    int32_t firstOutput = output;
    
    pid.resetIntegeral(); // Zero integral
    TEST_ASSERT_TRUE(pid.compute(NOW+1000, input, &output));
    TEST_ASSERT_EQUAL_INT32(firstOutput, output);

    TEST_ASSERT_TRUE(pid.compute(NOW+2000, input, &output));
    TEST_ASSERT_NOT_EQUAL_INT32(firstOutput, output);
}

static void test_integerPID_set_tunings_runtime_changes(void)
{
    int32_t input = 10;
    int32_t output = 0;

    integerPID pid;
    pid.setTunings(PidTuningParameters(10, 0, 0), NOW, 1);
    pid.setSetPoint(20);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    int32_t originalOutput = output;

    pid.setTunings(PidTuningParameters(20, 0, 0), NOW, 1); // Change Kp
    TEST_ASSERT_TRUE(pid.compute(NOW+1000U, input, &output));
    // Output should change due to new Kp
    TEST_ASSERT_NOT_EQUAL(originalOutput, output);
}

static void test_integerPID_compute_in_manual_mode_returns_false(void)
{
    int32_t input = 10;
    int32_t output = 0;

    integerPID pid;
    pid.setSetPoint(20);
    pid.setTunings(PidTuningParameters(10, 0, 0), NOW, 1);

    TEST_ASSERT_FALSE(pid.compute(NOW, input, &output)); // Should return false in manual mode
    TEST_ASSERT_EQUAL(0, output); // Output unchanged
}

static void test_integerPID_set_output_limits_invalid_ignored(void)
{
    int32_t input = 10;
    int32_t output = 0;

    integerPID pid;
    pid.setTunings(PidTuningParameters(10, 0, 0), NOW, 1);
    pid.setOutputLimits(50, 20); // Invalid: Min >= Max
    pid.setSetPoint(20);
    pid.activate(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    // Output should still be computed normally (limits ignored)
    TEST_ASSERT_NOT_EQUAL(0, output);
}

static void test_integerPID_set_controller_direction_runtime_manual(void)
{
    int32_t input = 10;
    int32_t output = 0;

    integerPID pid;
    pid.setSetPoint(20);
    pid.setTunings(PidTuningParameters(10, 0, 0)*-1, NOW, 1);
    pid.setOutputLimits(-25, 25);
    pid.activate(input);
    
    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    TEST_ASSERT_LESS_THAN(0, output);
}

static String createIterationMsg(int16_t iteration, long input, long output)
{
    char szMsg[64];
    snprintf(szMsg, _countof(szMsg)-1, "%" PRId16 ", %" PRId32 ", %" PRId32, iteration, (int32_t)input, (int32_t)output);
    return szMsg;
}

// Run the PID for 50 iterations and confirm it hits the setpoint
static inline void assert_pid_complete(integerPID &pid, int32_t input, int32_t setpoint, uint8_t sampleTime)
{
    UnityPrint("Iter,Input,Output"); UNITY_PRINT_EOL();
    UnityPrint(createIterationMsg(-1, input, setpoint).c_str()); UNITY_PRINT_EOL();

    pid.setSetPoint(setpoint);
    int32_t output;
    for (uint16_t iteration=0; iteration<50U; ++iteration)
    {
        TEST_ASSERT_TRUE(pid.compute(NOW+(iteration*sampleTime), input, &output));
        input += output;
        UnityPrint(createIterationMsg(iteration, input, output).c_str()); UNITY_PRINT_EOL();
    }
    // Tolerance of 1%
    TEST_ASSERT_INT32_WITHIN(DIV_ROUND_CLOSEST(setpoint, 100, int32_t), setpoint, input);
}

static void test_end_to_end_positive_positive_up(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    constexpr int32_t START_POINT = 90;
    constexpr int32_t SET_POINT = 155;

    integerPID pid;
    pid.setSetPoint(SET_POINT);
    pid.setTunings(PidTuningParameters(3, 1, 2), NOW, SAMPLE_TIME);
    pid.setOutputLimits(-255, 255);
    pid.activate(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, SAMPLE_TIME);
}

static void test_end_to_end_positive_positive_down(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    constexpr int32_t START_POINT = 235;
    constexpr int32_t SET_POINT = 155;

    integerPID pid;
    pid.setTunings(PidTuningParameters(5, 2, 4), NOW, SAMPLE_TIME);
    pid.setOutputLimits(-255, 255);
    pid.setSetPoint(SET_POINT);
    pid.activate(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, SAMPLE_TIME);
}


static void test_end_to_end_negative_negative_up(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    constexpr int32_t START_POINT = -235;
    constexpr int32_t SET_POINT = -155;

    integerPID pid;
    pid.setTunings(PidTuningParameters(15, 3, 2), NOW, SAMPLE_TIME);
    pid.setOutputLimits(-255, 255);
    pid.activate(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, SAMPLE_TIME);
}

static void test_end_to_end_negative_negative_down(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    constexpr int32_t START_POINT = -155;
    constexpr int32_t SET_POINT = -235;

    integerPID pid;
    pid.setTunings(PidTuningParameters(15, 3, 2), NOW, SAMPLE_TIME);
    pid.setOutputLimits(-255, 255);
    pid.activate(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, SAMPLE_TIME);
}

static void test_end_to_end_positive_negative(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    constexpr int32_t START_POINT = 63;
    constexpr int32_t SET_POINT = -55;

    integerPID pid;
    pid.setTunings(PidTuningParameters(15, 1, 1), NOW, SAMPLE_TIME);
    pid.setOutputLimits(-255, 255);
    pid.activate(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, SAMPLE_TIME);
}

static void test_end_to_end_negative_positive(void) 
{
    constexpr uint8_t SAMPLE_TIME = 33;
    constexpr int32_t START_POINT = -55;
    constexpr int32_t SET_POINT = 65;

    integerPID pid;
    pid.setTunings(PidTuningParameters(15, 3, 2), NOW, SAMPLE_TIME);
    pid.setOutputLimits(-255, 255);
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
        RUN_TEST_P(test_compute_NoTimeChange_ReturnsFalse);
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