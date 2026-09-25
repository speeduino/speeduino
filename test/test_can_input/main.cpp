#include "../test_harness_device.h"
#include "../test_harness_native.h"
#include "../test_utils.h"
#include "can_input.h"

static void test_lengths_and_indices(void)
{
    const uint8_t data[8] = {0x12, 0x34, 2, 3, 4, 5, 0x56, 0x78};
    for (uint8_t length = 0; length <= 9; ++length)
    {
        for (uint16_t index = 0; index <= 255; ++index)
        {
            for (uint8_t width = 1; width <= 2; ++width)
            {
                uint16_t value = 0xABCD;
                const bool valid = length <= 8 && index + width <= length;
                TEST_ASSERT_EQUAL(valid, readCanInputValue(data, length, index, width == 2, true, value));
                if (!valid) { TEST_ASSERT_EQUAL_UINT16(0xABCD, value); }
            }
        }
    }
}

static void test_endianness_and_last_byte(void)
{
    const uint8_t data[8] = {0x12, 0x34, 2, 3, 4, 5, 0x56, 0x78};
    uint16_t value = 0;
    TEST_ASSERT_TRUE(readCanInputValue(data, 8, 6, true, true, value));
    TEST_ASSERT_EQUAL_UINT16(0x7856, value);
    TEST_ASSERT_TRUE(readCanInputValue(data, 8, 6, true, false, value));
    TEST_ASSERT_EQUAL_UINT16(0x5678, value);
    TEST_ASSERT_TRUE(readCanInputValue(data, 8, 7, false, false, value));
    TEST_ASSERT_EQUAL_UINT16(0x78, value);
}

void runAllTests(void)
{
    RUN_TEST_P(test_lengths_and_indices);
    RUN_TEST_P(test_endianness_and_last_byte);
}
TEST_HARNESS(runAllTests)
