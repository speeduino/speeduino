#include "../test_utils.h"
#include "storage.h"
#include "fake_storage.h"
#include "table2d.h"
#include "table3d.h"
#include "sensors.h"
#include "updates.h"
#include "pages.h"

extern void updateTableU16toU8(table2D_u16_u8_32 &targetTable, uint16_t u16EEpromBinAddress);
extern void upgradeV25toV26(void);

static void assert_2dTable(table2D_u16_u8_32 &testSubject, uint16_t newAxis, uint8_t newValue)
{
    TEST_ASSERT_EACH_EQUAL_UINT16(newAxis, testSubject.axis, testSubject.size());
    TEST_ASSERT_EACH_EQUAL_UINT8(newValue, testSubject.values, testSubject.size());
}

static void test_updateTableU16toU8(void)
{
    uint8_t values[32];
    uint16_t axis[32];
    table2D_u16_u8_32 testSubject(&axis, &values);
    constexpr uint16_t OLD_AXIS = UINT16_MAX;
    constexpr uint8_t OLD_VALUE = 0U;
    populate_2dtable(&testSubject, OLD_VALUE, OLD_AXIS);

    constexpr uint8_t EEPROM_BYTE = 127U;
    setStorageAPI(getOneByteStorageApi(0xFFF, 0xFFF, EEPROM_BYTE));
    updateTableU16toU8(testSubject, 1024 /* Address doesn't matter */);

    // Reads but no writes
    TEST_ASSERT_NOT_EQUAL(0, oneByteEeprom.readCount);
    TEST_ASSERT_EQUAL(0, oneByteEeprom.writeCount);

    // Table has been updated
    constexpr uint16_t NEW_AXIS_VALUE = ((uint16_t)EEPROM_BYTE << 8U) | EEPROM_BYTE;
    static_assert(NEW_AXIS_VALUE!=OLD_AXIS, "Old & new need to be different for a valid test");
    static_assert(EEPROM_BYTE!=OLD_VALUE, "Old & new need to be different for a valid test");
    assert_2dTable(testSubject, NEW_AXIS_VALUE, EEPROM_BYTE);
}

static storage_api_t innerApi;
static uint8_t eepromVersion = 0;

static byte eepromVersionReadWrapper(uint16_t address)
{
    byte read = innerApi.read(address);
    if (address==0U)
    {
        read = eepromVersion;
    }
    return read;
}
static storage_api_t setupEepromReadApi(uint8_t version, storage_api_t inner)
{
    eepromVersion = version;
    innerApi = inner;
    storage_api_t wrapper = inner;
    wrapper.read = eepromVersionReadWrapper;
    return wrapper;
}

static void test_upgradeV25toV26_positive(void)
{
    constexpr uint16_t OLD_AXIS = UINT16_MAX;
    constexpr uint8_t OLD_VALUE = 0U;
    populate_2dtable(&cltCalibrationTable, OLD_VALUE, OLD_AXIS);
    populate_2dtable(&iatCalibrationTable, OLD_VALUE, OLD_AXIS);
    populate_2dtable(&o2CalibrationTable, OLD_VALUE, OLD_AXIS);
   
    constexpr uint8_t EEPROM_BYTE = 127U;
    setStorageAPI(setupEepromReadApi(25U, getOneByteStorageApi(0xFFF, 0xFFF, EEPROM_BYTE)));
    upgradeV25toV26();

    TEST_ASSERT_EQUAL(644, oneByteEeprom.readCount);
    TEST_ASSERT_EQUAL(1, oneByteEeprom.writeCount);

    // Tables have been updated
    constexpr uint16_t NEW_AXIS_VALUE = ((uint16_t)EEPROM_BYTE << 8U) | EEPROM_BYTE;
    static_assert(NEW_AXIS_VALUE!=OLD_AXIS, "Old & new need to be different for a valid test");
    static_assert(EEPROM_BYTE!=OLD_VALUE, "Old & new need to be different for a valid test");
    assert_2dTable(o2CalibrationTable, NEW_AXIS_VALUE, EEPROM_BYTE);
    assert_2dTable(iatCalibrationTable, NEW_AXIS_VALUE, EEPROM_BYTE);
    assert_2dTable(cltCalibrationTable, NEW_AXIS_VALUE, EEPROM_BYTE);
}

static void test_upgradeV25toV26_negative(void)
{
    constexpr uint8_t EEPROM_BYTE = 127U;
    setStorageAPI(setupEepromReadApi(24U, getOneByteStorageApi(0xFFF, 0xFFF, EEPROM_BYTE)));
    upgradeV25toV26();

    TEST_ASSERT_EQUAL(1, oneByteEeprom.readCount);
    TEST_ASSERT_EQUAL(0, oneByteEeprom.writeCount);
}

// =========================== multiplyTableLoad ==============================

static void test_multiplyTableLoad_doubles_y_axis(void)
{
  table3d8RpmLoad testSubject;

  // Seed the testSubject Y-axis with known values, then verify each was doubled.
  populate_table_axis(y_begin(&testSubject, testSubject.type_key), (table3d_axis_t)10);
  // Force a couple of distinct values too so we don't trivially pass with all zeros.
  table_axis_iterator it = y_begin(&testSubject, testSubject.type_key);
  uint8_t i = 0;
  while (!it.at_end())
  {
    *it = (table3d_axis_t)(5 + i);
    ++it; ++i;
  }

  multiplyTableLoad(&testSubject, testSubject.type_key, 2U);

  it = y_begin(&testSubject, testSubject.type_key);
  uint8_t j = 0;
  while (!it.at_end())
  {
    TEST_ASSERT_EQUAL((table3d_axis_t)((5 + j) * 2), *it);
    ++it; ++j;
  }
}

static void test_multiplyTableLoad_by_one_is_identity(void)
{
  table3d8RpmLoad testSubject;

  table_axis_iterator it = y_begin(&testSubject, testSubject.type_key);
  uint8_t i = 0;
  while (!it.at_end()) { *it = (table3d_axis_t)(20 + i); ++it; ++i; }

  multiplyTableLoad(&testSubject, testSubject.type_key, 1U);

  it = y_begin(&testSubject, testSubject.type_key);
  uint8_t j = 0;
  while (!it.at_end())
  {
    TEST_ASSERT_EQUAL((table3d_axis_t)(20 + j), *it);
    ++it; ++j;
  }
}

// =========================== divideTableLoad ================================

static void test_divideTableLoad_halves_y_axis(void)
{
  table3d8RpmLoad testSubject;

  table_axis_iterator it = y_begin(&testSubject, testSubject.type_key);
  uint8_t i = 0;
  while (!it.at_end()) { *it = (table3d_axis_t)((10 + i) * 2); ++it; ++i; }

  divideTableLoad(&testSubject, testSubject.type_key, 2U);

  it = y_begin(&testSubject, testSubject.type_key);
  uint8_t j = 0;
  while (!it.at_end())
  {
    TEST_ASSERT_EQUAL((table3d_axis_t)(10 + j), *it);
    ++it; ++j;
  }
}

static void test_multiplyDivideTableLoad_round_trip(void)
{
  table3d8RpmLoad testSubject;

  table_axis_iterator it = y_begin(&testSubject, testSubject.type_key);
  uint8_t i = 0;
  while (!it.at_end()) { *it = (table3d_axis_t)(7 + i); ++it; ++i; }

  multiplyTableLoad(&testSubject, testSubject.type_key, 4U);
  divideTableLoad(&testSubject, testSubject.type_key, 4U);

  it = y_begin(&testSubject, testSubject.type_key);
  uint8_t j = 0;
  while (!it.at_end())
  {
    TEST_ASSERT_EQUAL((table3d_axis_t)(7 + j), *it);
    ++it; ++j;
  }
}

// =========================== multiplyTableValue / divideTableValue ==========
//
// These walk the entire page-as-bytes (including non-table fields). Pages 1
// and 3 hold scalar config (no tables) so we work on a small table-free page
// to avoid dragging the whole 3D map into the assertion. Page 1 is used
// because getPageSize/setPageValue are page-aware.

static void test_multiplyTableValue_scales_each_byte(void)
{
  // Pick a small enough page and stamp it with known values.
  constexpr uint8_t pageNum = 1U;
  uint16_t count = getPageSize(pageNum);
  TEST_ASSERT_GREATER_THAN_UINT16(0U, count);

  for (uint16_t i = 0U; i < count; ++i) { setPageValue(pageNum, i, 2U); }

  multiplyTableValue(pageNum, 3U);

  for (uint16_t i = 0U; i < count; ++i)
  {
    TEST_ASSERT_EQUAL_UINT8(6U, getPageValue(pageNum, i));
  }
}

static void test_divideTableValue_scales_each_byte(void)
{
  constexpr uint8_t pageNum = 1U;
  uint16_t count = getPageSize(pageNum);

  for (uint16_t i = 0U; i < count; ++i) { setPageValue(pageNum, i, 12U); }

  divideTableValue(pageNum, 4U);

  for (uint16_t i = 0U; i < count; ++i)
  {
    TEST_ASSERT_EQUAL_UINT8(3U, getPageValue(pageNum, i));
  }
}

void test_update(void) {
    SET_UNITY_FILENAME() { 
        RUN_TEST(test_updateTableU16toU8); 
        RUN_TEST(test_upgradeV25toV26_positive);  
        RUN_TEST(test_upgradeV25toV26_negative); 
        RUN_TEST(test_multiplyTableLoad_doubles_y_axis);
        RUN_TEST(test_multiplyTableLoad_by_one_is_identity);
        RUN_TEST(test_multiplyDivideTableLoad_round_trip);
        RUN_TEST(test_multiplyTableValue_scales_each_byte);
        RUN_TEST(test_divideTableValue_scales_each_byte);
        RUN_TEST(test_divideTableLoad_halves_y_axis);
  }
}
