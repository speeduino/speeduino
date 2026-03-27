#include <unity.h>
#include "../test_utils.h"
#include "storage.h"
#include "pages.h"
#include "sensors.h"
#include "fake_storage.h"

constexpr char WRITE_MARKER = 'X';
constexpr char BUFFER_MARKER = 'A';

static void test_saveAllPages(void)
{
    setStorageAPI(getOneByteStorageApi(8192, 8192, BUFFER_MARKER));
    saveAllPages();
    TEST_ASSERT_FALSE(isEepromWritePending());

    setStorageAPI(getOneByteStorageApi(8192, 16, BUFFER_MARKER));
    saveAllPages();
    TEST_ASSERT_TRUE(isEepromWritePending());
}

static void assert_entity(page_iterator_t iter, char expectedContent)
{
    for (uint16_t offset=0; offset<iter.entity.size; ++offset)
    {
        char szMsg[64];
        sprintf(szMsg, "Page %" PRIu8 ", Offset %" PRIu16, iter.location.page, offset);
        TEST_ASSERT_EQUAL_MESSAGE(expectedContent, getPageValue(iter.location.page, offset), szMsg);
    }
}
static void assert_page(uint8_t pageNum, char expectedContent)
{
    page_iterator_t pageIter = page_begin(pageNum);
    while (pageIter.entity.type!=EntityType::End)
    {
        assert_entity(pageIter, expectedContent);
        pageIter = advance(pageIter);
    }
}

static void test_loadAllPages(void)
{
    setStorageAPI(getOneByteStorageApi(8192, 8192, BUFFER_MARKER));
    loadAllPages();
    for (uint8_t page = MIN_PAGE_NUM; page<MAX_PAGE_NUM; ++page) {
        assert_page(page, BUFFER_MARKER);
    }
}


template <typename axis_t, typename value_t, uint8_t sizeT>
static void assert_2dtable(const table2D<axis_t, value_t, sizeT> &table, axis_t expectedAxis, value_t expectedValue)
{
    for (uint16_t index=0; index<sizeT; ++index)
    {
        TEST_ASSERT_EQUAL(expectedAxis, table.axis[index]);
        TEST_ASSERT_EQUAL(expectedValue, table.values[index]);
    }
}

static void test_loadAllCalibrationTables(void)
{
    setStorageAPI(getOneByteStorageApi(8192, 8192, BUFFER_MARKER));
    loadAllCalibrationTables();
    uint16_t U16_MARKER = word(BUFFER_MARKER, BUFFER_MARKER);
    constexpr uint8_t U8_MARKER = BUFFER_MARKER;

    assert_2dtable(o2CalibrationTable, U16_MARKER, U8_MARKER);
    assert_2dtable(iatCalibrationTable, U16_MARKER, U8_MARKER);
    assert_2dtable(cltCalibrationTable, U16_MARKER, U8_MARKER);
}

template <typename axis_t, typename value_t, uint8_t sizeT>
static void load_2dtable(table2D<axis_t, value_t, sizeT> &table, axis_t axis, value_t value)
{
    for (uint16_t index=0; index<sizeT; ++index)
    {
        table.axis[index] = axis;
        table.values[index] = value;
    }
}

static void test_saveAllCalibrationTables(void)
{
    uint16_t U16_MARKER = word(WRITE_MARKER, WRITE_MARKER);
    constexpr uint8_t U8_MARKER = WRITE_MARKER;

    load_2dtable(o2CalibrationTable, U16_MARKER, U8_MARKER);
    load_2dtable(iatCalibrationTable, U16_MARKER, U8_MARKER);
    load_2dtable(cltCalibrationTable, U16_MARKER, U8_MARKER);

    setStorageAPI(getOneByteStorageApi(8192, 8192, BUFFER_MARKER));
    saveAllCalibrationTables();
    TEST_ASSERT_GREATER_THAN(0, oneByteEeprom.readCount);
    TEST_ASSERT_GREATER_THAN(0, oneByteEeprom.writeCount);

    setStorageAPI(getOneByteStorageApi(8192, 8192, BUFFER_MARKER));
    saveCalibrationTable((SensorCalibrationTable)-1);
    TEST_ASSERT_EQUAL(0, oneByteEeprom.readCount);
    TEST_ASSERT_EQUAL(0, oneByteEeprom.writeCount);
}

void test_storage(void) {
    SET_UNITY_FILENAME() {     
        RUN_TEST_P(test_saveAllPages);
        RUN_TEST_P(test_loadAllPages);
        RUN_TEST_P(test_loadAllCalibrationTables);
        RUN_TEST_P(test_saveAllCalibrationTables);
    }
}