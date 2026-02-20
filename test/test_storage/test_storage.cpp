#include <unity.h>
#include "../test_utils.h"
#include "storage.h"
#include "pages.h"
#include "sensors.h"

struct infinite_eeprom_t
{
    char readValue;
    uint16_t _length;
    uint16_t _blockSize;
    bool wasRead;
    bool wasWritten;
};

static infinite_eeprom_t infiniteBuffer;

static byte infiniteRead(uint16_t)
{
    infiniteBuffer.wasRead = true;
    return infiniteBuffer.readValue;
}

void infniteWrite(uint16_t, byte)
{
    infiniteBuffer.wasWritten = true;
}

uint16_t infniteLength(void)
{
    return infiniteBuffer._length;
}

uint16_t infiniteGetMaxWriteBlockSize(const statuses&)
{
    return infiniteBuffer._blockSize;
}

static void setupInfniteStorageApi(uint16_t length, uint16_t blockSize, char readValue)
{
    infiniteBuffer.readValue = readValue;
    infiniteBuffer._length = length;
    infiniteBuffer._blockSize = blockSize;
    infiniteBuffer.wasRead = false;
    infiniteBuffer.wasWritten = false;
    storage_api_t storage = { .read = infiniteRead, .write = infniteWrite, .length = infniteLength, .getMaxWriteBlockSize = infiniteGetMaxWriteBlockSize };
    setStorageAPI(storage);
}

constexpr char WRITE_MARKER = 'X';
constexpr char BUFFER_MARKER = 'A';

static void test_saveAllPages(void)
{
    setupInfniteStorageApi(8192, 8192, BUFFER_MARKER);
    saveAllPages();
    TEST_ASSERT_FALSE(isEepromWritePending());

    setupInfniteStorageApi(8192, 16, BUFFER_MARKER);
    saveAllPages();
    TEST_ASSERT_TRUE(isEepromWritePending());
}

static void assert_entity(page_iterator_t entity, char expectedContent)
{
    for (uint16_t offset=0; offset<entity.address.size; ++offset)
    {
        char szMsg[64];
        sprintf(szMsg, "Page %" PRIu8 ", Offset %" PRIu16, entity.location.page, offset);
        TEST_ASSERT_EQUAL_MESSAGE(expectedContent, getPageValue(entity.location.page, offset), szMsg);
    }
}
static void assert_page(uint8_t pageNum, char expectedContent)
{
    page_iterator_t pageIter = page_begin(pageNum);
    while (pageIter.type!=End)
    {
        assert_entity(pageIter, expectedContent);
        pageIter = advance(pageIter);
    }
}

static void test_loadAllPages(void)
{
    setupInfniteStorageApi(8192, 8192, BUFFER_MARKER);
    loadAllPages();
    uint8_t pageCount = getPageCount();
    for (uint8_t page = 0U; page<pageCount; ++page) {
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
    setupInfniteStorageApi(8192, 8192, BUFFER_MARKER);
    loadAllCalibrationTables();
    uint16_t U16_MARKER = word(BUFFER_MARKER, BUFFER_MARKER);
    constexpr uint8_t U8_MARKER = BUFFER_MARKER;

    assert_2dtable(o2CalibrationTable, U16_MARKER, U8_MARKER);
    assert_2dtable(iatCalibrationTable, U16_MARKER, U16_MARKER);
    assert_2dtable(cltCalibrationTable, U16_MARKER, U16_MARKER);
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
    load_2dtable(iatCalibrationTable, U16_MARKER, U16_MARKER);
    load_2dtable(cltCalibrationTable, U16_MARKER, U16_MARKER);

    setupInfniteStorageApi(8192, 8192, BUFFER_MARKER);
    saveAllCalibrationTables();
    TEST_ASSERT_TRUE(infiniteBuffer.wasRead);
    TEST_ASSERT_TRUE(infiniteBuffer.wasWritten);

    setupInfniteStorageApi(8192, 8192, BUFFER_MARKER);
    saveCalibrationTable((SensorCalibrationTable)-1);
    TEST_ASSERT_FALSE(infiniteBuffer.wasRead);
    TEST_ASSERT_FALSE(infiniteBuffer.wasWritten);
}

void test_storage(void) {
    SET_UNITY_FILENAME() {     
        RUN_TEST_P(test_saveAllPages);
        RUN_TEST_P(test_loadAllPages);
        RUN_TEST_P(test_loadAllCalibrationTables);
        RUN_TEST_P(test_saveAllCalibrationTables);
    }
}