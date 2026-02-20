#include <unity.h>
#include "../test_utils.h"
#include "storage.h"

struct infinite_eeprom_t
{
    char readValue;
    uint16_t _length;
    uint16_t _blockSize;
};

static infinite_eeprom_t infiniteBuffer;

static byte infiniteRead(uint16_t)
{
    return infiniteBuffer.readValue;
}

void infniteWrite(uint16_t, byte)
{
    // Do nothing
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

void test_storage(void) {
    SET_UNITY_FILENAME() {     
        RUN_TEST_P(test_saveAllPages);
    }
}