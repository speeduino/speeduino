#include <unity.h>
#include "logger.h"
#include "../test_utils.h"

// Mirror of the static fsIntIndex[] table inside is2ByteEntry().
// MUST be kept in sync with logger.cpp.
static constexpr uint8_t expected_2byte_keys[] = {
  4, 14, 17, 22, 26, 28, 33, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62,
  64, 66, 68, 70, 72, 76, 78, 80, 82, 86, 88, 90, 93, 95, 99, 104, 111,
  121, 125, 130, 132, 134, 136
};

static bool isInExpected(uint8_t key)
{
  for (uint8_t i = 0U; i < (sizeof(expected_2byte_keys) / sizeof(expected_2byte_keys[0])); ++i)
  {
    if (expected_2byte_keys[i] == key) { return true; }
  }
  return false;
}

static void test_is2ByteEntry_known_2byte_keys(void)
{
  for (uint8_t i = 0U; i < (sizeof(expected_2byte_keys) / sizeof(expected_2byte_keys[0])); ++i)
  {
    TEST_ASSERT_TRUE_MESSAGE(is2ByteEntry(expected_2byte_keys[i]),
                             "Expected key to be a 2-byte entry");
  }
}

static void test_is2ByteEntry_exhaustive_complement(void)
{
  // Validate every byte 0..254 against the known-good list.
  // (Skip 255 because it's outside the table's documented range.)
  for (uint16_t k = 0U; k < 255U; ++k)
  {
    bool actual = is2ByteEntry((uint8_t)k);
    bool expected = isInExpected((uint8_t)k);
    TEST_ASSERT_EQUAL_MESSAGE(expected, actual,
                              "is2ByteEntry mismatch for key");
  }
}

static void test_is2ByteEntry_edges(void)
{
  // First entry in the table
  TEST_ASSERT_TRUE(is2ByteEntry(4U));
  // Last entry in the table
  TEST_ASSERT_TRUE(is2ByteEntry(136U));
  // Just below the lowest entry
  TEST_ASSERT_FALSE(is2ByteEntry(0U));
  TEST_ASSERT_FALSE(is2ByteEntry(3U));
  // Just above the highest entry
  TEST_ASSERT_FALSE(is2ByteEntry(137U));
  TEST_ASSERT_FALSE(is2ByteEntry(200U));
}

void testLogger(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_is2ByteEntry_known_2byte_keys);
    RUN_TEST(test_is2ByteEntry_exhaustive_complement);
    RUN_TEST(test_is2ByteEntry_edges);
  }
}
