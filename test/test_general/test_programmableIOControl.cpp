#include "../test_utils.h"
#include "programmableIOControl.h"
#include "globals.h"

// Forward declare the TESTABLE_STATIC variables
extern uint8_t ioDelay[_countof(config13::outputPin)];
extern uint8_t ioOutDelay[_countof(config13::outputPin)];
extern uint8_t pinIsValid;
extern uint8_t currentRuleStatus;

// Forward declare the testable function
extern void checkProgrammableIO(statuses& current, const config13& page13, int16_t (*getData)(uint16_t index));

struct programmableIOTestContext_t {
    config13 page13 = {};
    statuses current = {};

    programmableIOTestContext_t() {
        // Reset state for each test
        pinIsValid = 0;
        currentRuleStatus = 0;
        for (uint8_t i = 0; i < _countof(ioDelay); i++) {
            ioDelay[i] = 0;
            ioOutDelay[i] = 0;
        }
    }
};

// Mock data values for testing
static int16_t mockDataValues[256];

// Helper to setup mock data
static void setupMockData(void)
{
    for (int i = 0; i < 256; i++) {
        mockDataValues[i] = i; // Simple pattern: index = value
    }
}

// Mock getData function for testing
static int16_t mockGetData(uint16_t index)
{
    return mockDataValues[index];
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
    TEST_ASSERT_EQUAL(0, pinIsValid);
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
    TEST_ASSERT_TRUE(BIT_CHECK(pinIsValid, 0));
    TEST_ASSERT_TRUE(BIT_CHECK(pinIsValid, 1));

    // Output status should match inverted config
    TEST_ASSERT_TRUE(BIT_CHECK(context.current.outputsStatus, 0));
    TEST_ASSERT_TRUE(BIT_CHECK(context.current.outputsStatus, 1));
}

static void test_initialiseProgrammableIO_delay_initialization(void)
{
    programmableIOTestContext_t context;

    // Enable a cascade rule pin to trigger initialization
    context.page13.outputPin[0] = 128;

    initialiseProgrammableIO(context.current, context.page13);

    // All delays should be initialized to 0
    for (uint8_t i = 0; i < _countof(ioDelay); i++) {
        TEST_ASSERT_EQUAL(0, ioDelay[i]);
        TEST_ASSERT_EQUAL(0, ioOutDelay[i]);
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
    TEST_ASSERT_FALSE(BIT_CHECK(pinIsValid, 0)); // Disabled
    TEST_ASSERT_TRUE(BIT_CHECK(pinIsValid, 1));  // Cascade
    TEST_ASSERT_TRUE(BIT_CHECK(pinIsValid, 2));  // Cascade
    TEST_ASSERT_FALSE(BIT_CHECK(pinIsValid, 3)); // Disabled

    // Remaining pins should not be valid
    TEST_ASSERT_FALSE(BIT_CHECK(pinIsValid, 4));
    TEST_ASSERT_FALSE(BIT_CHECK(pinIsValid, 5));
    TEST_ASSERT_FALSE(BIT_CHECK(pinIsValid, 6));
    TEST_ASSERT_FALSE(BIT_CHECK(pinIsValid, 7));
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
    TEST_ASSERT_TRUE(BIT_CHECK(pinIsValid, 0)); // Pin 10 should be valid
    TEST_ASSERT_TRUE(BIT_CHECK(pinIsValid, 1)); // Pin 11 should be valid

    // Output status should match inverted config
    TEST_ASSERT_FALSE(BIT_CHECK(context.current.outputsStatus, 0)); // Not inverted
    TEST_ASSERT_TRUE(BIT_CHECK(context.current.outputsStatus, 1));  // Inverted
}

static void test_initialiseProgrammableIO_used_physical_pin(void)
{
    programmableIOTestContext_t context;

    pinCLT = 10;

    context.page13.outputPin[0] = pinCLT; // Physical pin (used)
    context.page13.outputPin[1] = 11; // Pin 11 - should be available

    initialiseProgrammableIO(context.current, context.page13);

    // Pin 0 should be invalid if used, pin 10 should be valid
    TEST_ASSERT_FALSE(BIT_CHECK(pinIsValid, 0)); // Should be invalid if used
    TEST_ASSERT_TRUE(BIT_CHECK(pinIsValid, 1));  // Should be valid
}

static void test_checkProgrammableIO_disabled_pin(void)
{
    programmableIOTestContext_t context;
    setupMockData();

    // No pins valid
    pinIsValid = 0;

    // Call checkProgrammableIO
    checkProgrammableIO(context.current, context.page13, mockGetData);

    // No changes should occur since no pins are valid
    TEST_ASSERT_EQUAL(0, context.current.outputsStatus);
    TEST_ASSERT_EQUAL(0, currentRuleStatus);
}

static void test_checkProgrammableIO_skips_invalid_pins(void)
{
    programmableIOTestContext_t context;
    setupMockData();

    // Set up cascade pins for indices 0 and 2
    context.page13.outputPin[0] = 128; // Cascade rule
    context.page13.outputPin[2] = 130; // Cascade rule

    // Set pinIsValid: only pin 0 and 2 valid
    BIT_SET(pinIsValid, 0);
    BIT_SET(pinIsValid, 2);

    // Configure comparison for pin 0: EQUAL, data index 5, target 5 (should match)
    context.page13.operation[0].firstCompType = 0; // EQUAL
    context.page13.firstDataIn[0] = 5;
    context.page13.firstTarget[0] = 5;

    // Configure comparison for pin 2: EQUAL, data index 10, target 10 (should match)
    context.page13.operation[2].firstCompType = 0; // EQUAL
    context.page13.firstDataIn[2] = 10;
    context.page13.firstTarget[2] = 10;

    // Call checkProgrammableIO
    checkProgrammableIO(context.current, context.page13, mockGetData);

    // Only valid pins should have outputs set
    TEST_ASSERT_TRUE(BIT_CHECK(currentRuleStatus, 0)); // Pin 0 should be set
    TEST_ASSERT_FALSE(BIT_CHECK(currentRuleStatus, 1)); // Pin 1 invalid, should not be set
    TEST_ASSERT_TRUE(BIT_CHECK(currentRuleStatus, 2)); // Pin 2 should be set
}

static void test_checkProgrammableIO_all_cascade_rules(void)
{
    programmableIOTestContext_t context;
    setupMockData();

    // Set all pins as cascade rules
    for (uint8_t i = 0; i < 8; i++) {
        context.page13.outputPin[i] = 128 + i; // Cascade rules 128-135
        BIT_SET(pinIsValid, i);

        // Configure simple EQUAL comparison that will pass
        context.page13.operation[i].firstCompType = 0; // EQUAL
        context.page13.firstDataIn[i] = i; // Use index i
        context.page13.firstTarget[i] = i; // Match the data
    }

    // Call checkProgrammableIO
    checkProgrammableIO(context.current, context.page13, mockGetData);

    // All cascade rules should be triggered
    for (uint8_t i = 0; i < 8; i++) {
        TEST_ASSERT_TRUE(BIT_CHECK(currentRuleStatus, i));
    }
}

static void test_checkProgrammableIO_processes_all_eight_pins(void)
{
    programmableIOTestContext_t context;
    setupMockData();

    // Set all pins as cascade rules and valid
    for (uint8_t i = 0; i < 8; i++) {
        context.page13.outputPin[i] = 128 + i;
        BIT_SET(pinIsValid, i);

        // Configure comparison that will pass
        context.page13.operation[i].firstCompType = 0; // EQUAL
        context.page13.firstDataIn[i] = i;
        context.page13.firstTarget[i] = i;
    }

    // Call checkProgrammableIO
    checkProgrammableIO(context.current, context.page13, mockGetData);

    // Verify all 8 pins were processed (all bits set in currentRuleStatus)
    TEST_ASSERT_EQUAL(0xFF, currentRuleStatus);
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
    }
}
