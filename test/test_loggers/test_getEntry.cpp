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

// ============================ getTSLogEntry sweep ===========================
//
// The TS log has one byte per index in [0, LOG_ENTRY_SIZE). Asserting every
// returned value would mean re-encoding the whole struct mapping, which is
// the implementation we're trying to test. So the sweep just checks every
// byte can be retrieved without crashing — that fires every case branch in
// the giant getTSLogEntry switch and pulls a lot of related helpers in too.

static void test_getTSLogEntry_sweep_all_indices(void)
{
  currentStatus = {};
  for (uint16_t i = 0U; i <= LOG_ENTRY_SIZE+1; ++i)
  {
    (void)getTSLogEntry(i);
  }
  TEST_PASS();
}

static void test_getTSLogEntry_secl_byte0(void)
{
  currentStatus = {};
  currentStatus.secl = 42U;
  TEST_ASSERT_EQUAL_UINT8(42U, getTSLogEntry(0));
}

static void test_getTSLogEntry_rpm_split_into_low_and_high(void)
{
  currentStatus = {};
  currentStatus.RPM = 0x1234U;
  TEST_ASSERT_EQUAL_UINT8(0x34U, getTSLogEntry(14));
  TEST_ASSERT_EQUAL_UINT8(0x12U, getTSLogEntry(15));
}

static void test_getTSLogEntry_engine_status_byte2_running(void)
{
  currentStatus = {};
  currentStatus.rotationStatus  = EngineRotationStatus::Running;
  TEST_ASSERT_EQUAL_UINT8(0x01U, getTSLogEntry(2));
}

// ============================ getReadableLogEntry sweep =====================

static void test_getReadableLogEntry_sweep_all_indices(void)
{
  currentStatus = {};
  for (uint16_t i = 0U; i < LOG_ENTRY_SIZE; ++i)
  {
    (void)getReadableLogEntry(i);
  }
  TEST_PASS();
}

static void test_getReadableLogEntry_rpm_returns_full_value(void)
{
  currentStatus = {};
  currentStatus.RPM = 4321U;
  TEST_ASSERT_EQUAL_INT16(4321, getReadableLogEntry(13));
}

// ============================ Float log sweep ===============================

#if defined(FPU_MAX_SIZE) && FPU_MAX_SIZE >= 32
static void test_getReadableFloatLogEntry_sweep(void)
{
  currentStatus = {};
  for (uint16_t i = 0U; i < LOG_ENTRY_SIZE; ++i)
  {
    (void)getReadableFloatLogEntry(i);
  }
  TEST_PASS();
}

static void test_getReadableFloatLogEntry_battery_voltage(void)
{
  currentStatus = {};
  currentStatus.battery10 = 138U;   // 13.8V
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 13.8f, getReadableFloatLogEntry(8));
}

static void test_getReadableFloatLogEntry_falls_back_to_int(void)
{
  // logIndex 13 (RPM) is not a float field so it should pass through to
  // getReadableLogEntry().
  currentStatus = {};
  currentStatus.RPM = 4321U;
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 4321.0f, getReadableFloatLogEntry(13));
}
#endif

// ============================ Legacy secondary log sweep ====================

static void test_getLegacySecondarySerialLogEntry_sweep(void)
{
  currentStatus = {};
  for (uint16_t i = 0U; i < LOG_ENTRY_SIZE; ++i)
  {
    (void)getLegacySecondarySerialLogEntry(i);
  }
  TEST_PASS();
}

static void test_getLegacySecondarySerialLogEntry_secl_byte0(void)
{
  currentStatus = {};
  currentStatus.secl = 99U;
  TEST_ASSERT_EQUAL_UINT8(99U, getLegacySecondarySerialLogEntry(0));
}

void testGetEntry(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_is2ByteEntry_known_2byte_keys);
    RUN_TEST(test_is2ByteEntry_exhaustive_complement);
    RUN_TEST(test_is2ByteEntry_edges);

    RUN_TEST(test_getTSLogEntry_sweep_all_indices);
    RUN_TEST(test_getTSLogEntry_secl_byte0);
    RUN_TEST(test_getTSLogEntry_rpm_split_into_low_and_high);
    RUN_TEST(test_getTSLogEntry_engine_status_byte2_running);

    RUN_TEST(test_getReadableLogEntry_sweep_all_indices);
    RUN_TEST(test_getReadableLogEntry_rpm_returns_full_value);

#if defined(FPU_MAX_SIZE) && FPU_MAX_SIZE >= 32
    RUN_TEST(test_getReadableFloatLogEntry_sweep);
    RUN_TEST(test_getReadableFloatLogEntry_battery_voltage);
    RUN_TEST(test_getReadableFloatLogEntry_falls_back_to_int);
#endif

    RUN_TEST(test_getLegacySecondarySerialLogEntry_sweep);
    RUN_TEST(test_getLegacySecondarySerialLogEntry_secl_byte0);
  }
}
