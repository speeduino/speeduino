#include <unity.h>
#include "globals.h"
#include "logger.h"
#include "../test_utils.h"

// Access internal comms state via TESTABLE_STATIC linkage
extern bool     buildScatterPayload(void);
extern uint8_t  serialPayload[];
extern uint16_t serialPayloadLength;

// ----- Entry construction helpers -----
// Scatter entry: bits[15:13] = size (1=byte, 2=word, 3=dword, 4=qword), bits[12:0] = log offset
static constexpr uint16_t makeByteEntry(uint16_t offset)  { return (1U << 13U) | offset; }
static constexpr uint16_t makeWordEntry(uint16_t offset)  { return (2U << 13U) | offset; }
static constexpr uint16_t makeDwordEntry(uint16_t offset) { return (3U << 13U) | offset; }

static void setupScatterTest(void)
{
    // Reset observable state before each test
    serialPayloadLength = 0U;
    memset(configPage16.entries, 0U, sizeof(configPage16.entries));
    memset(&currentStatus, 0U, sizeof(currentStatus));
}

// 1. Empty scatter array (first entry == 0) → returns false (reset)
static void test_scatter_empty_array_sends_reset(void)
{
    setupScatterTest();
    // configPage16.entries[0] is already 0 (sentinel)
    TEST_ASSERT_FALSE(buildScatterPayload());
}

// 2. Single BYTE entry: size=1, 1 data byte expected
static void test_scatter_single_byte_entry(void)
{
    setupScatterTest();
    currentStatus.secl           = 0xABU;
    configPage16.entries[0] = makeByteEntry(0U);  // offset 0 → secl
    // configPage16.entries[1] == 0: sentinel

    TEST_ASSERT_TRUE(buildScatterPayload());

    // serialPayload[0] = SERIAL_RC_OK (0x00), [1] = secl
    TEST_ASSERT_EQUAL_UINT16(2U, serialPayloadLength);
    TEST_ASSERT_EQUAL_HEX8(0x00U, serialPayload[0]); // SERIAL_RC_OK
    TEST_ASSERT_EQUAL_HEX8(0xABU, serialPayload[1]);
}

// 3. Single WORD entry: size=2, 2 data bytes expected
static void test_scatter_single_word_entry(void)
{
    setupScatterTest();
    currentStatus.MAP = 0x0564U; // lowByte=0x64, highByte=0x05
    configPage16.entries[0] = makeWordEntry(4U);  // offsets 4,5 → MAP low, MAP high

    TEST_ASSERT_TRUE(buildScatterPayload());

    TEST_ASSERT_EQUAL_UINT16(3U, serialPayloadLength); // 1 status + 2 data
    TEST_ASSERT_EQUAL_HEX8(0x00U, serialPayload[0]); // SERIAL_RC_OK
    TEST_ASSERT_EQUAL_HEX8(0x64U, serialPayload[1]); // lowByte(MAP)
    TEST_ASSERT_EQUAL_HEX8(0x05U, serialPayload[2]); // highByte(MAP)
}

// 4. Single DWORD entry: size=3, 4 data bytes expected
static void test_scatter_single_dword_entry(void)
{
    setupScatterTest();
    currentStatus.RPM         = 0x1234U; // offset 14=0x34, offset 15=0x12
    currentStatus.AEamount    = 2U;      // offset 16 = lowByte(AEamount>>1) = 0x01
    currentStatus.corrections = 0xBCU;  // offset 17 = lowByte(corrections) = 0xBC
    configPage16.entries[0] = makeDwordEntry(14U); // offsets 14-17

    TEST_ASSERT_TRUE(buildScatterPayload());

    TEST_ASSERT_EQUAL_UINT16(5U, serialPayloadLength); // 1 status + 4 data
    TEST_ASSERT_EQUAL_HEX8(0x00U, serialPayload[0]); // SERIAL_RC_OK
    TEST_ASSERT_EQUAL_HEX8(0x34U, serialPayload[1]); // lowByte(RPM)
    TEST_ASSERT_EQUAL_HEX8(0x12U, serialPayload[2]); // highByte(RPM)
    TEST_ASSERT_EQUAL_HEX8(0x01U, serialPayload[3]); // lowByte(AEamount>>1)
    TEST_ASSERT_EQUAL_HEX8(0xBCU, serialPayload[4]); // lowByte(corrections)
}

// 5. Multiple mixed-size entries are concatenated in declaration order
static void test_scatter_multiple_mixed_entries(void)
{
    setupScatterTest();
    currentStatus.secl = 0x11U;           // offset 0 → 1 byte
    currentStatus.MAP  = 0x0022U;        // offsets 4,5 → 2 bytes  (low=0x22, high=0x00)
    configPage16.entries[0] = makeByteEntry(0U);  // secl
    configPage16.entries[1] = makeWordEntry(4U);  // MAP low+high
   
    TEST_ASSERT_TRUE(buildScatterPayload());

    TEST_ASSERT_EQUAL_UINT16(4U, serialPayloadLength); // 1 + 1 + 2
    TEST_ASSERT_EQUAL_HEX8(0x00U, serialPayload[0]); // SERIAL_RC_OK
    TEST_ASSERT_EQUAL_HEX8(0x11U, serialPayload[1]); // secl
    TEST_ASSERT_EQUAL_HEX8(0x22U, serialPayload[2]); // lowByte(MAP)
    TEST_ASSERT_EQUAL_HEX8(0x00U, serialPayload[3]); // highByte(MAP)
}

// 6. Zero entry mid-array terminates iteration; trailing entries are ignored
static void test_scatter_zero_mid_array_terminates_early(void)
{
    setupScatterTest();
    currentStatus.secl            = 0x55U; // offset 0
    currentStatus.syncLossCounter = 0x66U; // offset 3
    configPage16.entries[0] = makeByteEntry(0U); // secl → included
    configPage16.entries[1] = 0U;               // sentinel → stops here
    configPage16.entries[2] = makeByteEntry(3U);// syncLossCounter → must be ignored

    TEST_ASSERT_TRUE(buildScatterPayload());

    TEST_ASSERT_EQUAL_UINT16(2U, serialPayloadLength); // only entry[0]
    TEST_ASSERT_EQUAL_HEX8(0x00U, serialPayload[0]);
    TEST_ASSERT_EQUAL_HEX8(0x55U, serialPayload[1]);
}

void testScatterOutputChannels(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_scatter_empty_array_sends_reset);
        RUN_TEST_P(test_scatter_single_byte_entry);
        RUN_TEST_P(test_scatter_single_word_entry);
        RUN_TEST_P(test_scatter_single_dword_entry);
        RUN_TEST_P(test_scatter_multiple_mixed_entries);
        RUN_TEST_P(test_scatter_zero_mid_array_terminates_early);
    }
}
