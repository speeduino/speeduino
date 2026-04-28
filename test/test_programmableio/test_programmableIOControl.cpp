#include <stdlib.h>
#include "../test_utils.h"
#include "programmableIOControl.h"
#include "programmableIOControl_internals.h"
#include "units.h"
#include "logger.h"
#include "globals.h"

using namespace programmableIOControl_details;

// Forward declare the TESTABLE_STATIC variables
extern state_t state;

// Forward declare the testable functions
extern void checkProgrammableIO(const config13& page13, int16_t (*getData)(uint16_t index));
extern int16_t ProgrammableIOGetData(uint16_t index, byte (*pGetLogEntry)(uint16_t byteNum));
extern int16_t getComparisonData(uint8_t request, int16_t (*getData)(uint16_t index));
extern bool evaluateComparisonOp(uint8_t compType, int16_t lhs, int16_t rhs);
extern bool evaluateBitwiseOp(uint8_t compType, bool lhs, bool rhs);
extern bool applyOutputTimeLimit(const rule_t& rule, const channel_t& channel, bool ruleActive);
extern uint8_t nextOutDelay(const channel_t& channel, const rule_t& rule);

struct programmableIOTestContext_t {
    config13 page13 = {};
    statuses current = {};

    programmableIOTestContext_t() {
        // Reset state for each test
        for (auto &channel : state.channels) {
            channel = channel_t();
        }
    }
};

// Mock data values for testing
static int16_t mockDataValues[256];

// Helper to setup mock data
static void setupMockData(void)
{
    for (size_t i = 0; i < _countof(mockDataValues); i++) {
        mockDataValues[i] = (int16_t)i; // Simple pattern: index = value
    }
}

// Mock getData function for testing
static int16_t mockGetData(uint16_t index)
{
    return mockDataValues[index];
}

static const byte *mockLogEntryData = nullptr;
static byte mockGetLogEntry(uint16_t byteNum)
{
    return mockLogEntryData[byteNum];
}

static void test_ProgrammableIOGetData_single_byte_entry(void)
{
    static const byte logData[] = { 0x22, 0xAA, 0xBB, 0xCC };
    mockLogEntryData = logData;
    TEST_ASSERT_EQUAL_INT16(0x22, ProgrammableIOGetData(0, mockGetLogEntry));
}

static void test_ProgrammableIOGetData_two_byte_entry(void)
{
    static const byte logData[] = { 0, 0, 0, 0, 0x34, 0x12 }; // Little-endian representation of 0x1234
    mockLogEntryData = logData;
    TEST_ASSERT_EQUAL_INT16(0x1234, ProgrammableIOGetData(4, mockGetLogEntry));
}

static void test_ProgrammableIOGetData_special_indices(void)
{
    runSecsX10 = 1000U;
    TEST_ASSERT_EQUAL_INT16(32768, ProgrammableIOGetData(239U, mockGetLogEntry));

    runSecsX10 = 40000U;
    TEST_ASSERT_EQUAL_INT16(40000, ProgrammableIOGetData(239U, mockGetLogEntry));

    static const byte logData[] = { 0, 0, 0, 0, 0, 0, 12, 18 }; // Little-endian representation of 0x1234
    mockLogEntryData = logData;
    TEST_ASSERT_EQUAL_INT16(temperatureRemoveOffset(logData[6]), ProgrammableIOGetData(6, mockGetLogEntry));
    TEST_ASSERT_EQUAL_INT16(temperatureRemoveOffset(logData[7]), ProgrammableIOGetData(7, mockGetLogEntry));

    TEST_ASSERT_EQUAL_INT16(-1, ProgrammableIOGetData(LOG_ENTRY_SIZE, mockGetLogEntry));
}

static void test_initialiseProgrammableIO_disabled(void)
{
    programmableIOTestContext_t context;

    // All pins disabled (set to 0)
    for (uint8_t i = 0; i < _countof(context.page13.outputPin); i++) {
        context.page13.outputPin[i] = 0;
    }

    initialiseProgrammableIO(context.current, context.page13);

    // pinIsValid should be all zeros since no pins are configured
    for (auto &channel : state.channels) {
        TEST_ASSERT_FALSE(channel.isPinValid);
    }
}

static void test_initialiseProgrammableIO_cascade_rules(void)
{
    programmableIOTestContext_t context;

    // Set cascaded rule pins (>= 128)
    context.page13.outputPin[0] = 128; // Cascade rule
    context.page13.outputPin[1] = 129; // Cascade rule
    context.page13.outputInverted = 0x03; // Invert bits 0 and 1

    initialiseProgrammableIO(context.current, context.page13);

    // Cascade rules should set pinIsValid bits
    TEST_ASSERT_TRUE(state.channels[0].isPinValid);
    TEST_ASSERT_TRUE(state.channels[1].isPinValid);

    // Output status should match inverted config
    TEST_ASSERT_TRUE(state.channels[0].isOutputActive);
    TEST_ASSERT_TRUE(state.channels[1].isOutputActive);
}

static void test_initialiseProgrammableIO_delay_initialization(void)
{
    programmableIOTestContext_t context;

    // Enable a cascade rule pin to trigger initialization
    context.page13.outputPin[0] = 128;

    initialiseProgrammableIO(context.current, context.page13);

    // All delays should be initialized to 0
    for (auto &channel : state.channels) {
        TEST_ASSERT_EQUAL(0, channel.activationDelayCount);
        TEST_ASSERT_EQUAL(0, channel.outputDelayCount);
    }
}

static void test_initialiseProgrammableIO_mixed_configuration(void)
{
    programmableIOTestContext_t context;

    // Mix of disabled and cascade rule pins
    context.page13.outputPin[0] = 0;     // Disabled
    context.page13.outputPin[1] = 130;   // Cascade rule
    context.page13.outputPin[2] = 131;   // Cascade rule
    context.page13.outputPin[3] = 0;     // Disabled

    initialiseProgrammableIO(context.current, context.page13);

    // Check pinIsValid bits - only cascade rules should be valid
    TEST_ASSERT_FALSE(state.channels[0].isPinValid); // Disabled
    TEST_ASSERT_TRUE(state.channels[1].isPinValid);  // Cascade
    TEST_ASSERT_TRUE(state.channels[2].isPinValid);  // Cascade
    TEST_ASSERT_FALSE(state.channels[3].isPinValid); // Disabled

    // Remaining pins should not be valid
    TEST_ASSERT_FALSE(state.channels[4].isPinValid);
    TEST_ASSERT_FALSE(state.channels[5].isPinValid);
    TEST_ASSERT_FALSE(state.channels[6].isPinValid);
    TEST_ASSERT_FALSE(state.channels[7].isPinValid);
}

static void test_initialiseProgrammableIO_physical_pins(void)
{
    programmableIOTestContext_t context;

    // Set physical pins (assuming pins 10 and 11 are not used in test environment)
    context.page13.outputPin[0] = 10; // Physical pin
    context.page13.outputPin[1] = 11; // Physical pin
    context.page13.outputInverted = 0x02; // Invert bit 1

    initialiseProgrammableIO(context.current, context.page13);

    // Physical pins should set pinIsValid bits if not already used
    TEST_ASSERT_TRUE(state.channels[0].isPinValid); // Pin 10 should be valid
    TEST_ASSERT_TRUE(state.channels[1].isPinValid); // Pin 11 should be valid

    // Output status should match inverted config
    TEST_ASSERT_FALSE(state.channels[0].isOutputActive); // Not inverted
    TEST_ASSERT_TRUE(state.channels[1].isOutputActive); // Inverted
}

static void test_initialiseProgrammableIO_used_physical_pin(void)
{
    programmableIOTestContext_t context;

    pinCLT = 10;

    context.page13.outputPin[0] = pinCLT; // Physical pin (used)
    context.page13.outputPin[1] = 11; // Pin 11 - should be available

    initialiseProgrammableIO(context.current, context.page13);

    // Pin 0 should be invalid if used, pin 10 should be valid
    TEST_ASSERT_FALSE(state.channels[0].isPinValid); // Should be invalid if used
    TEST_ASSERT_TRUE(state.channels[1].isPinValid);  // Should be valid
}

static void test_checkProgrammableIO_disabled_pin(void)
{
    programmableIOTestContext_t context;
    setupMockData();

    // Call checkProgrammableIO
    checkProgrammableIO(context.page13, mockGetData);

    // No changes should occur since no pins are valid
    for (auto &channel : state.channels) {
        TEST_ASSERT_FALSE(channel.isRuleActive);
        TEST_ASSERT_FALSE(channel.isOutputActive);
    }
}

static void test_checkProgrammableIO_skips_invalid_pins(void)
{
    programmableIOTestContext_t context;
    setupMockData();

    // Set up cascade pins for indices 0 and 2
    context.page13.outputPin[0] = 128; // Cascade rule
    context.page13.outputPin[2] = 130; // Cascade rule

    // Set pinIsValid: only pin 0 and 2 valid
    state.channels[0].isPinValid = true;
    state.channels[2].isPinValid = true;

    // Configure comparison for pin 0: EQUAL, data index 5, target 5 (should match)
    context.page13.operation[0].firstCompType = COMPARATOR_EQUAL; // EQUAL
    context.page13.firstDataIn[0] = 5;
    context.page13.firstTarget[0] = 5;

    // Configure comparison for pin 2: EQUAL, data index 10, target 10 (should match)
    context.page13.operation[2].firstCompType = COMPARATOR_EQUAL; // EQUAL
    context.page13.firstDataIn[2] = 10;
    context.page13.firstTarget[2] = 10;

    // Call checkProgrammableIO
    checkProgrammableIO(context.page13, mockGetData);

    // Only valid pins should have outputs set
    TEST_ASSERT_TRUE(state.channels[0].isRuleActive); // Pin 0 should be set
    TEST_ASSERT_FALSE(state.channels[1].isRuleActive); // Pin 1 invalid, should not be set
    TEST_ASSERT_TRUE(state.channels[2].isRuleActive); // Pin 2 should be set
}

static void test_checkProgrammableIO_all_cascade_rules(void)
{
    programmableIOTestContext_t context;
    setupMockData();

    // Set all pins as cascade rules
    for (uint8_t i = 0; i < _countof(state.channels); i++) {
        context.page13.outputPin[i] = 128 + i; // Cascade rules 128-135
        state.channels[i].isPinValid = true;

        // Configure simple EQUAL comparison that will pass
        context.page13.operation[i].firstCompType = COMPARATOR_EQUAL; // EQUAL
        context.page13.firstDataIn[i] = i; // Use index i
        context.page13.firstTarget[i] = i; // Match the data
    }

    // Call checkProgrammableIO
    checkProgrammableIO(context.page13, mockGetData);

    // All cascade rules should be triggered
    for (auto &channel : state.channels) {
        TEST_ASSERT_TRUE(channel.isRuleActive);
    }
}

static void test_checkProgrammableIO_processes_all_eight_pins(void)
{
    programmableIOTestContext_t context;
    setupMockData();

    // Set all pins as cascade rules and valid
    for (uint8_t i = 0; i < _countof(state.channels); i++) {
        context.page13.outputPin[i] = 128 + i; // Cascade rules 128-135
        state.channels[i].isPinValid = true;

        // Configure comparison that will pass
        context.page13.operation[i].firstCompType = COMPARATOR_EQUAL; // EQUAL
        context.page13.firstDataIn[i] = i;
        context.page13.firstTarget[i] = i;
    }

    // Call checkProgrammableIO
    checkProgrammableIO(context.page13, mockGetData);

    // Verify all 8 pins were processed (all bits set in currentRuleStatus)
    for (auto &channel : state.channels) {
        TEST_ASSERT_TRUE(channel.isRuleActive);
    }
}
struct testOperation {
    uint8_t bitwiseCombiner;
    compOperation_t firstOperand;
    compOperation_t secondOperand;
};

static void setupTestOp(const testOperation &op, config13 &page13, uint8_t opIndex)
{
    page13.outputPin[opIndex] = 128; // Cascade rule
    state.channels[opIndex].isPinValid = true;
    page13.outputDelay[opIndex] = 0;
    page13.outputTimeLimit[opIndex] = 0;
    page13.kindOfLimiting = 0;

    page13.operation[opIndex].firstCompType = op.firstOperand.opType;
    page13.firstDataIn[opIndex] = op.firstOperand.dataIndex;
    page13.firstTarget[opIndex] = op.firstOperand.target;

    page13.operation[opIndex].bitwise = op.bitwiseCombiner;
    page13.operation[opIndex].secondCompType = op.secondOperand.opType;
    page13.secondDataIn[opIndex] = op.secondOperand.dataIndex;
    page13.secondTarget[opIndex] = op.secondOperand.target;
}

static void test_checkProgrammableIO_all_comparators(void)
{
    programmableIOTestContext_t context;
    setupMockData();

    constexpr testOperation positiveTestOps[] = {
        // Positive conditions for all comparators without bitwise
        { BITWISE_DISABLED, { COMPARATOR_EQUAL, 5, 5 }, { COMPARATOR_EQUAL, 0, 0 } }, // EQUAL
        { BITWISE_DISABLED, { COMPARATOR_NOT_EQUAL, 5, 6 }, { COMPARATOR_NOT_EQUAL, 0, 0 } }, // NOT_EQUAL
        { BITWISE_DISABLED, { COMPARATOR_GREATER, 7, 6 }, { COMPARATOR_GREATER, 0, 0 } }, // GREATER
        { BITWISE_DISABLED, { COMPARATOR_GREATER_EQUAL, 7, 7 }, { COMPARATOR_GREATER_EQUAL, 0, 0 } }, // GREATER_EQUAL
        { BITWISE_DISABLED, { COMPARATOR_LESS, 1, 2 }, { COMPARATOR_LESS, 0, 0 } }, // LESS
        { BITWISE_DISABLED, { COMPARATOR_LESS_EQUAL, 3, 3 }, { COMPARATOR_LESS_EQUAL, 0, 0 } }, // LESS_EQUAL
        { BITWISE_DISABLED, { COMPARATOR_AND, 3, 1 }, { COMPARATOR_AND, 0, 0 } }, // AND
        { BITWISE_DISABLED, { COMPARATOR_XOR, 3, 1 }, { COMPARATOR_XOR, 0, 0 } }, // XOR
        // Same but bitwise AND
        { BITWISE_AND, { COMPARATOR_EQUAL, 5, 5 }, { COMPARATOR_EQUAL, 5, 5 } }, // EQUAL
        { BITWISE_AND, { COMPARATOR_NOT_EQUAL, 5, 6 }, { COMPARATOR_NOT_EQUAL, 5, 6 } }, // NOT_EQUAL
        { BITWISE_AND, { COMPARATOR_GREATER, 7, 6 }, { COMPARATOR_GREATER, 7, 6 } }, // GREATER
        { BITWISE_AND, { COMPARATOR_GREATER_EQUAL, 7, 7 }, { COMPARATOR_GREATER_EQUAL, 7, 7 } }, // GREATER_EQUAL
        { BITWISE_AND, { COMPARATOR_LESS, 1, 2 }, { COMPARATOR_LESS, 1, 2 } }, // LESS
        { BITWISE_AND, { COMPARATOR_LESS_EQUAL, 3, 3 }, { COMPARATOR_LESS_EQUAL, 3, 3 } }, // LESS_EQUAL
        { BITWISE_AND, { COMPARATOR_AND, 3, 1 }, { COMPARATOR_AND, 3, 1 } }, // AND
        { BITWISE_AND, { COMPARATOR_XOR, 3, 1 }, { COMPARATOR_XOR, 3, 1 } }, // XOR
        // Same but bitwise OR
        { BITWISE_OR, { COMPARATOR_EQUAL, 5, 5 }, { COMPARATOR_EQUAL, 5, 5 } }, // EQUAL
        { BITWISE_OR, { COMPARATOR_NOT_EQUAL, 5, 6 }, { COMPARATOR_NOT_EQUAL, 5, 6 } }, // NOT_EQUAL
        { BITWISE_OR, { COMPARATOR_GREATER, 7, 6 }, { COMPARATOR_GREATER, 7, 6 } }, // GREATER
        { BITWISE_OR, { COMPARATOR_GREATER_EQUAL, 7, 7 }, { COMPARATOR_GREATER_EQUAL, 7, 7 } }, // GREATER_EQUAL
        { BITWISE_OR, { COMPARATOR_LESS, 1, 2 }, { COMPARATOR_LESS, 1, 2 } }, // LESS
        { BITWISE_OR, { COMPARATOR_LESS_EQUAL, 3, 3 }, { COMPARATOR_LESS_EQUAL, 3, 3 } }, // LESS_EQUAL
        { BITWISE_OR, { COMPARATOR_AND, 3, 1 }, { COMPARATOR_AND, 3, 1 } }, // AND
        { BITWISE_OR, { COMPARATOR_XOR, 3, 1 }, { COMPARATOR_XOR, 3, 1 } }, // XOR
        // Same but bitwise XOR
        { BITWISE_XOR, { COMPARATOR_EQUAL, 5, 6 }, { COMPARATOR_EQUAL, 4, 4 } }, // EQUAL
        { BITWISE_XOR, { COMPARATOR_NOT_EQUAL, 5, 5 }, { COMPARATOR_NOT_EQUAL, 4, 6 } }, // NOT_EQUAL
        { BITWISE_XOR, { COMPARATOR_GREATER, 7, 6 }, { COMPARATOR_GREATER, 5, 5 } }, // GREATER
        { BITWISE_XOR, { COMPARATOR_GREATER_EQUAL, 7, 7 }, { COMPARATOR_GREATER_EQUAL, 7, 8 } }, // GREATER_EQUAL
        { BITWISE_XOR, { COMPARATOR_LESS, 1, 2 }, { COMPARATOR_LESS, 3, 3 } }, // LESS
        { BITWISE_XOR, { COMPARATOR_LESS_EQUAL, 3, 3 }, { COMPARATOR_LESS_EQUAL, 3, 2 } }, // LESS_EQUAL
        { BITWISE_XOR, { COMPARATOR_AND, 3, 3 }, { COMPARATOR_AND, 3, 0 } }, // AND
        { BITWISE_XOR, { COMPARATOR_XOR, 3, 1 }, { COMPARATOR_XOR, 7, 7 } }, // XOR
    };

    for (auto &operation : positiveTestOps) {
        context = programmableIOTestContext_t();
        setupMockData();

        setupTestOp(operation, context.page13, 0);

        checkProgrammableIO(context.page13, mockGetData);
        
        char szMsg[128];
        snprintf(szMsg, sizeof(szMsg), "Combiner %" PRIu8 ", Compare1 %" PRIu8 ", Compare2 %" PRIu8, 
        context.page13.operation[0].bitwise, 
        context.page13.operation[0].firstCompType, 
        context.page13.operation[0].secondCompType);
        TEST_ASSERT_TRUE_MESSAGE(state.channels[0].isRuleActive, szMsg);
    }

    constexpr testOperation negativeTestOps[] = {
        // Negative conditions for all comparators without bitwise
        { BITWISE_DISABLED, { COMPARATOR_EQUAL, 5, 4 }, { COMPARATOR_EQUAL, 0, 0 } }, // EQUAL
        { BITWISE_DISABLED, { COMPARATOR_NOT_EQUAL, 5, 5 }, { COMPARATOR_NOT_EQUAL, 0, 0 } }, // NOT_EQUAL
        { BITWISE_DISABLED, { COMPARATOR_GREATER, 7, 8 }, { COMPARATOR_GREATER, 0, 0 } }, // GREATER
        { BITWISE_DISABLED, { COMPARATOR_GREATER_EQUAL, 7, 8 }, { COMPARATOR_GREATER_EQUAL, 0, 0 } }, // GREATER_EQUAL
        { BITWISE_DISABLED, { COMPARATOR_LESS, 1, 1 }, { COMPARATOR_LESS, 0, 0 } }, // LESS
        { BITWISE_DISABLED, { COMPARATOR_LESS_EQUAL, 3, 2 }, { COMPARATOR_LESS_EQUAL, 0, 0 } }, // LESS_EQUAL
        { BITWISE_DISABLED, { COMPARATOR_AND, 3, 0 }, { COMPARATOR_AND, 0, 0 } }, // AND
        { BITWISE_DISABLED, { COMPARATOR_XOR, 3, 3 }, { COMPARATOR_XOR, 0, 0 } }, // XOR
        // Negative conditions for 2nd comparator with bitwise AND
        { BITWISE_AND, { COMPARATOR_EQUAL, 5, 5 }, { COMPARATOR_EQUAL, 5, 4 } }, // EQUAL
        { BITWISE_AND, { COMPARATOR_NOT_EQUAL, 5, 6 }, { COMPARATOR_NOT_EQUAL, 5, 5 } }, // NOT_EQUAL
        { BITWISE_AND, { COMPARATOR_GREATER, 7, 6 }, { COMPARATOR_GREATER, 7, 7 } }, // GREATER
        { BITWISE_AND, { COMPARATOR_GREATER_EQUAL, 7, 7 }, { COMPARATOR_GREATER_EQUAL, 7, 8 } }, // GREATER_EQUAL
        { BITWISE_AND, { COMPARATOR_LESS, 1, 2 }, { COMPARATOR_LESS, 1, 1 } }, // LESS
        { BITWISE_AND, { COMPARATOR_LESS_EQUAL, 3, 3 }, { COMPARATOR_LESS_EQUAL, 3, 2 } }, // LESS_EQUAL
        { BITWISE_AND, { COMPARATOR_AND, 3, 1 }, { COMPARATOR_AND, 3, 0 } }, // AND
         // Negative conditions for 2nd comparator with bitwise OR
        { BITWISE_OR, { COMPARATOR_EQUAL, 5, 6 }, { COMPARATOR_EQUAL, 5, 4 } }, // OR false
         // Negative conditions for 2nd comparator with bitwise XOR
        { BITWISE_XOR, { COMPARATOR_EQUAL, 5, 5 }, { COMPARATOR_EQUAL, 5, 5 } }, // XOR false
    };
    for (auto &operation : negativeTestOps) {
        context = programmableIOTestContext_t();
        setupMockData();
        state.channels[0].isRuleActive = true;

        setupTestOp(operation, context.page13, 0);

        checkProgrammableIO(context.page13, mockGetData);
        
        char szMsg[128];
        snprintf(szMsg, sizeof(szMsg), "Combiner %" PRIu8 ", Compare1 %" PRIu8 ", Compare2 %" PRIu8, 
        context.page13.operation[0].bitwise, 
        context.page13.operation[0].firstCompType, 
        context.page13.operation[0].secondCompType);
        TEST_ASSERT_FALSE_MESSAGE(state.channels[0].isRuleActive, szMsg);
    }
}

static void test_checkProgrammableIO_output_delay_time(void)
{
    programmableIOTestContext_t context;
    setupMockData();

    context.page13.outputPin[0] = 128; // Cascade rule
    state.channels[0].isPinValid = true;
    context.page13.operation[0].firstCompType = 0; // EQUAL
    context.page13.firstDataIn[0] = 5;
    context.page13.firstTarget[0] = 5;
    context.page13.outputDelay[0] = 2;
    context.page13.outputTimeLimit[0] = 0;
    context.page13.kindOfLimiting = 0;

    checkProgrammableIO(context.page13, mockGetData);
    TEST_ASSERT_FALSE(state.channels[0].isRuleActive);
    TEST_ASSERT_EQUAL(1, state.channels[0].activationDelayCount);

    checkProgrammableIO(context.page13, mockGetData);
    TEST_ASSERT_FALSE(state.channels[0].isRuleActive);
    TEST_ASSERT_EQUAL(2, state.channels[0].activationDelayCount);

    checkProgrammableIO(context.page13, mockGetData);
    TEST_ASSERT_TRUE(state.channels[0].isRuleActive);

    for (auto &channel : state.channels) {
        channel.isRuleActive = false;
    }
    context.page13.outputPin[0] = 1;
    constexpr testOperation equalityOp = { BITWISE_DISABLED, { COMPARATOR_EQUAL, 5, 5 }, { COMPARATOR_EQUAL, 5, 5 } };
    setupTestOp(equalityOp, context.page13, 0);
    context.page13.kindOfLimiting = 1; // Switch to time limit mode
    context.page13.outputTimeLimit[0] = 3;
    state.channels[0].activationDelayCount = context.page13.outputTimeLimit[0]+1; // Set delay above time limit
    state.channels[0].outputDelayCount = context.page13.outputTimeLimit[0]-1;
    checkProgrammableIO(context.page13, mockGetData);
    TEST_ASSERT_TRUE(state.channels[0].isRuleActive);
    TEST_ASSERT_TRUE(state.channels[0].isOutputActive);
    TEST_ASSERT_EQUAL(context.page13.outputTimeLimit[0]+2, state.channels[0].activationDelayCount);
    TEST_ASSERT_EQUAL(context.page13.outputTimeLimit[0], state.channels[0].outputDelayCount);

    for (auto &channel : state.channels) {
        channel.isRuleActive = false;
    }
    context.page13.outputPin[0] = 1;
    constexpr testOperation negativeEqualityOp = { BITWISE_DISABLED, { COMPARATOR_EQUAL, 5, 6 }, { COMPARATOR_EQUAL, 5, 5 } };
    setupTestOp(negativeEqualityOp, context.page13, 0);
    context.page13.kindOfLimiting = 1; // Switch to time limit mode
    context.page13.outputTimeLimit[0] = 3;
    state.channels[0].activationDelayCount = context.page13.outputTimeLimit[0]+1; // Set delay above time limit
    state.channels[0].outputDelayCount = context.page13.outputTimeLimit[0]-1;
    checkProgrammableIO(context.page13, mockGetData);
    TEST_ASSERT_FALSE(state.channels[0].isRuleActive);
    TEST_ASSERT_FALSE(state.channels[0].isOutputActive);
    TEST_ASSERT_EQUAL(0, state.channels[0].activationDelayCount);
    TEST_ASSERT_EQUAL(context.page13.outputTimeLimit[0]+1, state.channels[0].outputDelayCount);
}

static void test_checkProgrammableIO_time_limit_disables_output(void)
{
    programmableIOTestContext_t context;
    setupMockData();

    context.page13.outputPin[0] = 128; // Cascade rule
    state.channels[0].isPinValid = true;
    context.page13.operation[0].firstCompType = 0; // EQUAL
    context.page13.firstDataIn[0] = 5;
    context.page13.firstTarget[0] = 5;
    context.page13.outputDelay[0] = 0;
    context.page13.outputTimeLimit[0] = 1;
    context.page13.kindOfLimiting = 1;

    state.channels[0].isRuleActive = false;
    state.channels[0].outputDelayCount = context.page13.outputTimeLimit[0] + 1;

    checkProgrammableIO(context.page13, mockGetData);

    TEST_ASSERT_FALSE(state.channels[0].isRuleActive);
    TEST_ASSERT_FALSE(state.channels[0].isOutputActive);
}

static void test_checkProgrammableIO_cascade_REUSE_RULES(void)
{
    programmableIOTestContext_t context;
    setupMockData();

    // Set up two cascade rule pins: pin 0 and pin 1
    context.page13.outputPin[0] = 128; // Cascade rule (will be output)
    context.page13.outputPin[1] = 129; // Cascade rule (will use pin 0's output)
    state.channels[0].isPinValid = true;
    state.channels[1].isPinValid = true;

    // Pin 0: Simple EQUAL comparison that will pass (data=0, target=0)
    context.page13.operation[0].firstCompType = COMPARATOR_EQUAL; // EQUAL
    context.page13.firstDataIn[0] = 0; // Use getData(0) = 0
    context.page13.firstTarget[0] = 0; // Target = 0, will match

    // Pin 1: Use cascade rule reuse (firstDataIn = 240 + rule_index 0 = 240)
    // This means: dataRequested = 240, after subtracting REUSE_RULES(240) = 0
    // So it checks BIT_CHECK(currentRuleStatus, 0)
    context.page13.operation[1].firstCompType = COMPARATOR_EQUAL; // EQUAL (compare with target)
    context.page13.firstDataIn[1] = REUSE_RULES; // REUSE_RULES + rule_index 0
    context.page13.firstTarget[1] = 1;   // Target = 1 (true)

    // First call to process pin 0
    checkProgrammableIO(context.page13, mockGetData);

    // After processing pin 0, currentRuleStatus bit 0 should be set (because comparison passed)
    TEST_ASSERT_TRUE(state.channels[0].isRuleActive);

    // Now test pin 1 which references bit 0 of currentRuleStatus
    // We need to call again or manually set the bit for testing
    // Let's manually set currentRuleStatus bit 0 to 1 (true) and verify pin 1 sees it
    state.channels[0].isRuleActive = true; // Reset to false to test the cascade rule reuse properly

    // Process pin 1 with cascade rule reference
    checkProgrammableIO(context.page13, mockGetData);

    // Pin 1 should now be set because it compared cascaded rule 0 (which is 1) == target 1
    TEST_ASSERT_TRUE(state.channels[1].isRuleActive);
}

static void test_checkProgrammableIO_cascade_REUSE_RULES_out_of_bounds(void)
{
    programmableIOTestContext_t context;
    setupMockData();

    // Set up a cascade rule pin that references an out-of-bounds rule
    context.page13.outputPin[0] = 128; // Cascade rule
    state.channels[0].isPinValid = true;

    // Configure with firstDataIn > 239 that results in out-of-bounds after subtraction
    // firstDataIn = 240 + 10 = 250
    // After subtraction: 250 - 240 = 10, which is > sizeof(page13.outputPin) (8)
    context.page13.operation[0].firstCompType = COMPARATOR_EQUAL; // EQUAL
    context.page13.firstDataIn[0] = REUSE_RULES + 10; // REUSE_RULES + 10 (out of bounds)
    context.page13.firstTarget[0] = 0;   // Target = 0

    // Call checkProgrammableIO
    checkProgrammableIO(context.page13, mockGetData);

    // When out of bounds, data should be 0, so comparison 0 == 0 should pass
    // Pin should be set
    TEST_ASSERT_TRUE(state.channels[0].isRuleActive);
}

static void test_checkProgrammableIO_cascade_rule_second_comparison(void)
{
    programmableIOTestContext_t context;
    setupMockData();

    // Set up a cascade rule pin with second comparison using cascade rule reuse
    context.page13.outputPin[0] = 128; // Cascade rule
    state.channels[0].isPinValid = true;

    // First comparison: simple EQUAL that passes
    context.page13.operation[0].firstCompType = COMPARATOR_EQUAL; // EQUAL
    context.page13.firstDataIn[0] = 5;
    context.page13.firstTarget[0] = 5; // Will match

    // Second comparison: use cascade rule reuse
    context.page13.operation[0].bitwise = BITWISE_AND; // BITWISE_AND
    context.page13.operation[0].secondCompType = COMPARATOR_EQUAL; // EQUAL
    context.page13.secondDataIn[0] = REUSE_RULES; // REUSE_RULES + rule 0
    context.page13.secondTarget[0] = 1;

    // Set currentRuleStatus bit 0 to 1 so second comparison passes
    state.channels[0].isRuleActive = true;

    // Call checkProgrammableIO
    checkProgrammableIO(context.page13, mockGetData);

    // Both comparisons pass and are AND'd together, so result should be true
    TEST_ASSERT_TRUE(state.channels[0].isRuleActive);
}

static void test_checkProgrammableIO_second_comparator_failsafe_skip(void)
{
    programmableIOTestContext_t context;
    setupMockData();

    context.page13.outputPin[0] = 128;
    state.channels[0].isPinValid = true;
    context.page13.operation[0].firstCompType = 0; // EQUAL
    context.page13.firstDataIn[0] = 5;
    context.page13.firstTarget[0] = 5;

    context.page13.operation[0].bitwise = BITWISE_AND; // BITWISE_AND
    context.page13.operation[0].secondCompType = COMPARATOR_EQUAL; // EQUAL
    context.page13.secondDataIn[0] = REUSE_RULES + 9; // Out-of-range reuse index, skip second comparator
    context.page13.secondTarget[0] = 1;

    checkProgrammableIO(context.page13, mockGetData);
    TEST_ASSERT_TRUE(state.channels[0].isRuleActive);
}

static void test_checkProgrammableIO_kindOfLimiting_false_resets_ioOutDelay(void)
{
    programmableIOTestContext_t context;
    setupMockData();

    context.page13.outputPin[0] = 128;
    state.channels[0].isPinValid = true;
    context.page13.operation[0].firstCompType = 0; // EQUAL
    context.page13.firstDataIn[0] = 5;
    context.page13.firstTarget[0] = 6; // Will be false
    context.page13.outputDelay[0] = 0;
    context.page13.outputTimeLimit[0] = 1;
    context.page13.kindOfLimiting = 0;

    state.channels[0].outputDelayCount = 1;

    checkProgrammableIO(context.page13, mockGetData);
    TEST_ASSERT_FALSE(state.channels[0].isRuleActive);
    TEST_ASSERT_FALSE(state.channels[0].isOutputActive);
    TEST_ASSERT_EQUAL(0, state.channels[0].outputDelayCount);
}

static void test_checkProgrammableIO_physical_pin_outputTimeLimit_expiry(void)
{
    programmableIOTestContext_t context;
    setupMockData();

    context.page13.outputPin[0] = 1;
    state.channels[0].isPinValid = true;
    context.page13.operation[0].firstCompType = 0; // EQUAL
    context.page13.firstDataIn[0] = 5;
    context.page13.firstTarget[0] = 6; // Will be false
    context.page13.outputDelay[0] = 0;
    context.page13.outputTimeLimit[0] = 1;
    context.page13.kindOfLimiting = 0;

    state.channels[0].outputDelayCount = 1;

    checkProgrammableIO(context.page13, mockGetData);
    TEST_ASSERT_FALSE(state.channels[0].isRuleActive);
    TEST_ASSERT_FALSE(state.channels[0].isOutputActive);
    TEST_ASSERT_EQUAL(0, state.channels[0].outputDelayCount);
}

static void assert_checkProgrammableIO(programmableIOTestContext_t &context, uint8_t iterations, uint8_t expectedRuleStatus, uint8_t expectedOutputsStatus)
{
    for (uint8_t i = 0; i < iterations; i++) {
        checkProgrammableIO(context.page13, mockGetData);
        for (uint8_t channelIndex = 0; channelIndex < _countof(state.channels); channelIndex++) {
            TEST_ASSERT_EQUAL(BIT_CHECK(expectedRuleStatus, channelIndex), state.channels[channelIndex].isRuleActive);
            TEST_ASSERT_EQUAL(BIT_CHECK(expectedOutputsStatus, channelIndex), state.channels[channelIndex].isOutputActive);
        }
    }
}

static void test_FlatShiftBlink_EveryHalfSecond(void)
{
    // Based the wiki example: https://wiki.speeduino.com/en/configuration/Programmable_Outputs
    programmableIOTestContext_t context;

    context.page13.outputPin[0] = 0;
    context.page13.outputPin[1] = 0;
    context.page13.outputPin[3] = 0;
    context.page13.outputPin[4] = 0;
    context.page13.outputPin[5] = 0;
    context.page13.outputPin[6] = 0;
    context.page13.outputPin[7] = 0;

    constexpr uint8_t VIRTUAL_PIN = 129;
    constexpr uint8_t RPM_INDEX = 5;
    constexpr int16_t RPM_THRESHOLD = 5600;
    constexpr uint8_t MAP_INDEX = 1;
    constexpr int16_t MAP_THRESHOLD = 80;

    // Rule 2: Flat Shift
    context.page13.outputPin[2] = VIRTUAL_PIN;
    BIT_CLEAR(context.page13.outputInverted, 2);    // Activate on high
    context.page13.outputDelay[2] = 2;
    // Flat shift above 5600 RPM and 80 kPa, with bitwise AND to combine conditions
    context.page13.operation[2].bitwise = BITWISE_AND;
    context.page13.operation[2].firstCompType = COMPARATOR_GREATER_EQUAL;
    context.page13.firstDataIn[2] = RPM_INDEX;
    context.page13.firstTarget[2] = RPM_THRESHOLD; 
    context.page13.operation[2].secondCompType = COMPARATOR_GREATER_EQUAL;
    context.page13.secondDataIn[2] = MAP_INDEX;
    context.page13.secondTarget[2] = MAP_THRESHOLD;
    // No time limit, output should stay active as long as conditions are met
    BIT_SET(context.page13.kindOfLimiting, 2);
    context.page13.outputTimeLimit[2] = 0;

    // Rule 3: Flat shift timer
    context.page13.outputPin[3] = VIRTUAL_PIN;
    BIT_CLEAR(context.page13.outputInverted, 3);    // Activate on high
    context.page13.outputDelay[3] = 5;
    // Flat shift active if rule 2 is active and rule 4 is NOT active
    context.page13.operation[3].bitwise = BITWISE_AND;
    context.page13.operation[3].firstCompType = COMPARATOR_EQUAL;
    context.page13.firstDataIn[3] = REUSE_RULES + 2; // Reuse 2nd rule
    context.page13.firstTarget[3] = 1; 
    context.page13.operation[3].secondCompType = COMPARATOR_EQUAL;
    context.page13.secondDataIn[3] = REUSE_RULES + 4; // Reuse 4th rule
    context.page13.secondTarget[3] = 0;
    // Once activated, rule 3 should keep the output active for 0.5 seconds 
    BIT_CLEAR(context.page13.kindOfLimiting, 3);
    context.page13.outputTimeLimit[3] = 5;

    // Rule 4: Shift light
    context.page13.outputPin[4] = 5; // Physical pin 5
    BIT_CLEAR(context.page13.outputInverted, 4);    // Activate on high
    context.page13.outputDelay[4] = 0;  // No activation delay
    context.page13.operation[4].bitwise = BITWISE_AND;
    context.page13.operation[4].firstCompType = COMPARATOR_EQUAL;
    context.page13.firstDataIn[4] = REUSE_RULES + 2; // Reuse 2nd rule
    context.page13.firstTarget[4] = 1; 
    context.page13.operation[4].secondCompType = COMPARATOR_EQUAL;
    context.page13.secondDataIn[4] = REUSE_RULES + 3; // Reuse 3rd rule
    context.page13.secondTarget[4] = 0;

    initialiseProgrammableIO(context.current, context.page13);
    // Rules 2, 3, and 4 are active
    TEST_ASSERT_FALSE(state.channels[0].isPinValid);
    TEST_ASSERT_FALSE(state.channels[1].isPinValid);
    TEST_ASSERT_TRUE(state.channels[2].isPinValid);
    TEST_ASSERT_TRUE(state.channels[3].isPinValid);
    TEST_ASSERT_TRUE(state.channels[4].isPinValid);
    TEST_ASSERT_FALSE(state.channels[5].isPinValid);
    TEST_ASSERT_FALSE(state.channels[6].isPinValid);
    TEST_ASSERT_FALSE(state.channels[7].isPinValid);

    mockDataValues[RPM_INDEX] = RPM_THRESHOLD - 100;
    mockDataValues[MAP_INDEX] = MAP_THRESHOLD - 10;
    assert_checkProgrammableIO(context, 13 /* Arbitrary number */, 0, 0);

    // Activate rule 2 by meeting conditions...
    mockDataValues[RPM_INDEX] = RPM_THRESHOLD + 100;
    mockDataValues[MAP_INDEX] = MAP_THRESHOLD + 10;
    // ...after 2 iterations (0.2 seconds, the rule 2 delay)...
    assert_checkProgrammableIO(context, context.page13.outputDelay[2], 0, 0);
     // ..until the rule 3 output delay activates...
    assert_checkProgrammableIO(context, context.page13.outputDelay[3], 0b00000100, 0b00010100);
    // ...which turns the shift light off after it's output delay expires...
    assert_checkProgrammableIO(context, context.page13.outputTimeLimit[3], 0b00001100, 0b00001100);
    // ...this continues as long as rule 2 conditions are met    
    for (uint8_t i = 0; i < 7 /* Arbitrary */; i++) {
        assert_checkProgrammableIO(context, context.page13.outputDelay[3], 0b00001100, 0b00001100);
        assert_checkProgrammableIO(context, context.page13.outputTimeLimit[3], 0b00001100, 0b00001100);
    }
    mockDataValues[RPM_INDEX] = RPM_THRESHOLD - 100;
    mockDataValues[MAP_INDEX] = MAP_THRESHOLD - 10;
    assert_checkProgrammableIO(context, 13 /* Arbitrary number */, 0, 0);
}

static void test_getData(void)
{
    setupMockData();

    // Test valid index
    TEST_ASSERT_EQUAL_INT16(5, getComparisonData(5, mockGetData)); // Should return mockDataValues[0] = 0

    // Test another valid index
    TEST_ASSERT_EQUAL_INT16(10, getComparisonData(10, mockGetData)); // Should return mockDataValues[1] = 1

    // Rule result reuse tests
    state.channels[2].isRuleActive = 0;
    TEST_ASSERT_EQUAL_INT16(0, getComparisonData(242, mockGetData));
    state.channels[3].isRuleActive = 1;
    TEST_ASSERT_EQUAL_INT16(1, getComparisonData(243, mockGetData));

    // Out of bounds index should return 0
    TEST_ASSERT_EQUAL_INT16(0, getComparisonData(254, mockGetData));
}

static void test_evaluateComparisonOp(void)
{
    // Test EQUAL
    TEST_ASSERT_TRUE(evaluateComparisonOp(COMPARATOR_EQUAL, 5, 5));
    TEST_ASSERT_FALSE(evaluateComparisonOp(COMPARATOR_EQUAL, 5, 6));

    // Test NOT_EQUAL
    TEST_ASSERT_TRUE(evaluateComparisonOp(COMPARATOR_NOT_EQUAL, 5, 6));
    TEST_ASSERT_FALSE(evaluateComparisonOp(COMPARATOR_NOT_EQUAL, 5, 5));

    // Test GREATER
    TEST_ASSERT_TRUE(evaluateComparisonOp(COMPARATOR_GREATER, 6, 5));
    TEST_ASSERT_FALSE(evaluateComparisonOp(COMPARATOR_GREATER, 5, 6));

    // Test GREATER_EQUAL
    TEST_ASSERT_TRUE(evaluateComparisonOp(COMPARATOR_GREATER_EQUAL, 6, 5));
    TEST_ASSERT_TRUE(evaluateComparisonOp(COMPARATOR_GREATER_EQUAL, 5, 5));
    TEST_ASSERT_FALSE(evaluateComparisonOp(COMPARATOR_GREATER_EQUAL, 4, 5));

    // Test LESS
    TEST_ASSERT_TRUE(evaluateComparisonOp(COMPARATOR_LESS, 4, 5));
    TEST_ASSERT_FALSE(evaluateComparisonOp(COMPARATOR_LESS, 5, 4));

    // Test LESS_EQUAL
    TEST_ASSERT_TRUE(evaluateComparisonOp(COMPARATOR_LESS_EQUAL, 4, 5));
    TEST_ASSERT_TRUE(evaluateComparisonOp(COMPARATOR_LESS_EQUAL, 5, 5));
    TEST_ASSERT_FALSE(evaluateComparisonOp(COMPARATOR_LESS_EQUAL, 6, 5));

    // Test AND
    TEST_ASSERT_TRUE(evaluateComparisonOp(COMPARATOR_AND, 5, 3));
    TEST_ASSERT_FALSE(evaluateComparisonOp(COMPARATOR_AND, 5, 2));

    // Test XOR
    TEST_ASSERT_TRUE(evaluateComparisonOp(COMPARATOR_XOR, 5, 3));
    TEST_ASSERT_FALSE(evaluateComparisonOp(COMPARATOR_XOR, 5, 5));

    // Test invalid comparator type should return false
    TEST_ASSERT_FALSE(evaluateComparisonOp(211, 5, 3));
    TEST_ASSERT_FALSE(evaluateComparisonOp(123, 5, 5));
}

static void test_evaluateBitwiseOp(void)
{
    // Test no bitwise operation
    TEST_ASSERT_FALSE(evaluateBitwiseOp(0, true, true));

    // Test bitwise AND
    TEST_ASSERT_FALSE(evaluateBitwiseOp(BITWISE_AND, false, true));
    TEST_ASSERT_TRUE(evaluateBitwiseOp(BITWISE_AND, true, true));

    // Test bitwise OR
    TEST_ASSERT_TRUE(evaluateBitwiseOp(BITWISE_OR, false, true));
    TEST_ASSERT_FALSE(evaluateBitwiseOp(BITWISE_OR, false, false));

    // Test bitwise XOR
    TEST_ASSERT_FALSE(evaluateBitwiseOp(BITWISE_XOR, true, true));
    TEST_ASSERT_TRUE(evaluateBitwiseOp(BITWISE_XOR, true, false));
}

static void assert_applyOutputTimeLimit_nochange(uint8_t limit, uint8_t outDelay) {
    rule_t rule = {};
    channel_t channel = {};
    rule.outputTimeLimit = limit;
    channel.outputDelayCount = outDelay;
    TEST_ASSERT_FALSE(applyOutputTimeLimit(rule, channel, false)); 
    TEST_ASSERT_TRUE(applyOutputTimeLimit(rule, channel, true)); 
    rule.limitType = LimitingType::Max;
    TEST_ASSERT_FALSE(applyOutputTimeLimit(rule, channel, false)); 
    TEST_ASSERT_TRUE(applyOutputTimeLimit(rule, channel, true)); 
}

static void test_applyOutputTimeLimit(void) {
    assert_applyOutputTimeLimit_nochange(0, 0);
    assert_applyOutputTimeLimit_nochange(0, 5);
    assert_applyOutputTimeLimit_nochange(5, 5);
    assert_applyOutputTimeLimit_nochange(6, 5);

    rule_t rule = {};
    channel_t channel = {};

    rule.outputTimeLimit = 5;
    channel.outputDelayCount = rule.outputTimeLimit + 1;
    rule.limitType = LimitingType::Max;
    TEST_ASSERT_FALSE(applyOutputTimeLimit(rule, channel, false)); 
    TEST_ASSERT_FALSE(applyOutputTimeLimit(rule, channel, true));     
}

static void test_nextOutDelay(void)
{
    channel_t channel;
    rule_t rule;

    rule.limitType = LimitingType::Min;
    rule._index = 0;

    TEST_ASSERT_EQUAL_UINT8(1, nextOutDelay(channel, rule));
    channel.outputDelayCount = 5;
    TEST_ASSERT_EQUAL_UINT8(6, nextOutDelay(channel, rule));

    rule.limitType = LimitingType::Max;
    rule.outputTimeLimit = 6;
    channel.isOutputActive = true;
    TEST_ASSERT_EQUAL_UINT8(7, nextOutDelay(channel, rule));
    channel.isOutputActive = false;
    TEST_ASSERT_EQUAL_UINT8(1, nextOutDelay(channel, rule));
}

void testProgrammableIOControl(void) 
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_initialiseProgrammableIO_disabled);
        RUN_TEST_P(test_initialiseProgrammableIO_cascade_rules);
        RUN_TEST_P(test_initialiseProgrammableIO_delay_initialization);
        RUN_TEST_P(test_initialiseProgrammableIO_mixed_configuration);
        RUN_TEST_P(test_initialiseProgrammableIO_physical_pins);
        RUN_TEST_P(test_initialiseProgrammableIO_used_physical_pin);
        RUN_TEST_P(test_checkProgrammableIO_disabled_pin);
        RUN_TEST_P(test_checkProgrammableIO_skips_invalid_pins);
        RUN_TEST_P(test_checkProgrammableIO_all_cascade_rules);
        RUN_TEST_P(test_checkProgrammableIO_processes_all_eight_pins);
        RUN_TEST_P(test_checkProgrammableIO_all_comparators);
        RUN_TEST_P(test_checkProgrammableIO_output_delay_time);
        RUN_TEST_P(test_checkProgrammableIO_time_limit_disables_output);
        RUN_TEST_P(test_checkProgrammableIO_cascade_REUSE_RULES);
        RUN_TEST_P(test_checkProgrammableIO_cascade_REUSE_RULES_out_of_bounds);
        RUN_TEST_P(test_checkProgrammableIO_cascade_rule_second_comparison);
        RUN_TEST_P(test_checkProgrammableIO_second_comparator_failsafe_skip);
        RUN_TEST_P(test_checkProgrammableIO_kindOfLimiting_false_resets_ioOutDelay);
        RUN_TEST_P(test_checkProgrammableIO_physical_pin_outputTimeLimit_expiry);
        RUN_TEST_P(test_ProgrammableIOGetData_single_byte_entry);
        RUN_TEST_P(test_ProgrammableIOGetData_two_byte_entry);
        RUN_TEST_P(test_ProgrammableIOGetData_special_indices);
        RUN_TEST_P(test_FlatShiftBlink_EveryHalfSecond);
        RUN_TEST_P(test_getData);
        RUN_TEST_P(test_evaluateComparisonOp);
        RUN_TEST_P(test_evaluateBitwiseOp);
        RUN_TEST_P(test_applyOutputTimeLimit);
        RUN_TEST_P(test_nextOutDelay);
    }
}
