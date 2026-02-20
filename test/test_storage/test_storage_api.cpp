#include <unity.h>
#include <string.h>
#include "../test_utils.h"
#include "storage_api.h"

static uint16_t readCounter;
static byte readValue;
static byte countingReadMock(uint16_t) {
    ++readCounter;
    return readValue;
}

static uint16_t writeCounter;
static byte lastWriteValue;
static void countingWriteMock(uint16_t, byte value) {
    lastWriteValue = value;
    ++writeCounter;
}

static uint16_t mockLength = UINT16_MAX;
static uint16_t lengthMock(void) {
    return mockLength;
}

static void test_update_nowrite_ifnochange(void) {
    readCounter = writeCounter = 0U;
    storage_api_t api = { .read = countingReadMock, .write = countingWriteMock, .length = lengthMock, };

    readValue = 33;
    mockLength = UINT16_MAX;
    TEST_ASSERT_FALSE(update(api, 99, readValue));
    TEST_ASSERT_EQUAL(1, readCounter);
    TEST_ASSERT_EQUAL(0, writeCounter);
}

static void test_update_write_ifchange(void) {
    readCounter = writeCounter = 0U;
    storage_api_t api = { .read = countingReadMock, .write = countingWriteMock, .length = lengthMock, };

    readValue = 33;
    lastWriteValue = UINT8_MAX;
    mockLength = UINT16_MAX;
    TEST_ASSERT_TRUE(update(api, 99, 77));
    TEST_ASSERT_EQUAL(1, readCounter);
    TEST_ASSERT_EQUAL(1, writeCounter);
    TEST_ASSERT_EQUAL(77, lastWriteValue);
}

static void test_updateBlock_reads_all_addresses(void) {
    storage_api_t api = { .read = countingReadMock, .write = countingWriteMock, .length = lengthMock, };

    byte block[16];
    memset(block, 77, sizeof(block));

    mockLength = UINT16_MAX;
    readCounter = writeCounter = 0U;
    readValue = block[0];
    updateBlock(api, 99, block, block+sizeof(block));
    TEST_ASSERT_EQUAL(sizeof(block), readCounter);
    TEST_ASSERT_EQUAL(0, writeCounter);
}

static void test_updateBlock_writes_all_addresses(void) {
    storage_api_t api = { .read = countingReadMock, .write = countingWriteMock, .length = lengthMock, };

    mockLength = UINT16_MAX;
    readValue = 3;
    byte block[] = {
        readValue, (byte)(readValue*2), readValue, (byte)(readValue*3),
        readValue, (byte)(readValue*4), readValue, (byte)(readValue*5),
        readValue, (byte)(readValue*6), readValue, (byte)(readValue*7),
        readValue, (byte)(readValue*8), readValue, (byte)(readValue*9),
        readValue, // An non-power of 2 size is a good test
    };

    readCounter = writeCounter = 0U;
    lastWriteValue = UINT8_MAX;
    updateBlock(api, 99, block, block+sizeof(block));
    TEST_ASSERT_EQUAL(sizeof(block), readCounter);
    TEST_ASSERT_EQUAL(sizeof(block)/2, writeCounter);    
    TEST_ASSERT_EQUAL(block[sizeof(block)-2], lastWriteValue);
}

static void test_updateBlockLimitWriteOps_reads_all_addresses(void) {
    storage_api_t api = { .read = countingReadMock, .write = countingWriteMock, .length = lengthMock, };

    byte block[16];
    memset(block, 77, sizeof(block));

    mockLength = UINT16_MAX;
    readCounter = writeCounter = 0U;
    readValue = block[0];
    TEST_ASSERT_EQUAL(1, updateBlockLimitWriteOps(api, 99, block, block+sizeof(block), 1));
    TEST_ASSERT_EQUAL(sizeof(block), readCounter);
    TEST_ASSERT_EQUAL(0, writeCounter);
}

static void test_updateBlockLimitWriteOps_writes_all_addresses(void) {
    storage_api_t api = { .read = countingReadMock, .write = countingWriteMock, .length = lengthMock, };

    mockLength = UINT16_MAX;
    readValue = 3;
    byte block[] = {
        readValue, (byte)(readValue*2), readValue, (byte)(readValue*3),
        readValue, (byte)(readValue*4), readValue, (byte)(readValue*5),
        readValue, (byte)(readValue*6), readValue, (byte)(readValue*7),
        readValue, (byte)(readValue*8), readValue, (byte)(readValue*9),
        readValue, // An non-power of 2 size is a good test
    };

    readCounter = writeCounter = 0U;
    lastWriteValue = UINT8_MAX;
    TEST_ASSERT_EQUAL(sizeof(block)-8, updateBlockLimitWriteOps(api, 99, block, block+sizeof(block), sizeof(block)));
    TEST_ASSERT_EQUAL(sizeof(block), readCounter);
    TEST_ASSERT_EQUAL(8, writeCounter);    
    TEST_ASSERT_EQUAL(block[sizeof(block)-2], lastWriteValue);
}


static void test_updateBlockLimitWriteOps_limited_writes(void) {
    storage_api_t api = { .read = countingReadMock, .write = countingWriteMock, .length = lengthMock, };

    mockLength = UINT16_MAX;
    readValue = 3;
    byte block[] = {
        readValue, (byte)(readValue*2), readValue, (byte)(readValue*3),
        readValue, (byte)(readValue*4), readValue, (byte)(readValue*5),
        readValue, (byte)(readValue*6), readValue, (byte)(readValue*7),
        readValue, (byte)(readValue*8), readValue, (byte)(readValue*9),
        readValue, // An non-power of 2 size is a good test
    };

    readCounter = writeCounter = 0U;
    lastWriteValue = UINT8_MAX;
    TEST_ASSERT_EQUAL(0, updateBlockLimitWriteOps(api, 99, block, block+sizeof(block), 7));
    TEST_ASSERT_EQUAL(14, readCounter);
    TEST_ASSERT_EQUAL(7, writeCounter);    
    TEST_ASSERT_EQUAL(readValue*8, lastWriteValue);
}

static void test_loadBlock(void) {
    storage_api_t api = { .read = countingReadMock, .write = countingWriteMock, .length = lengthMock, };

    byte block[16];

    mockLength = UINT16_MAX;
    readCounter = writeCounter = 0U;
    readValue = 33;
    TEST_ASSERT_EQUAL(99+sizeof(block), loadBlock(api, 99, block, block+sizeof(block)));
    TEST_ASSERT_EQUAL(sizeof(block), readCounter);
    TEST_ASSERT_EQUAL(0, writeCounter);
    byte expected[sizeof(block)];
    memset(expected, readValue, sizeof(expected));
    TEST_ASSERT_EQUAL_INT8_ARRAY(expected, block, sizeof(block));
}

static void test_fillBlock_nochange_nowrite(void) {
    storage_api_t api = { .read = countingReadMock, .write = countingWriteMock, .length = lengthMock, };

    mockLength = UINT16_MAX;
    readCounter = writeCounter = 0U;
    readValue = INT8_MAX;
    fillBlock(api, 0, 99, INT8_MAX);

    TEST_ASSERT_EQUAL(99, readCounter);
    TEST_ASSERT_EQUAL(0, writeCounter);
}

static void test_fillBlock_write_if_different(void) {
    storage_api_t api = { .read = countingReadMock, .write = countingWriteMock, .length = lengthMock, };

    mockLength = UINT16_MAX;
    readCounter = writeCounter = 0U;
    readValue = 33;
    lastWriteValue = UINT8_MAX;
    fillBlock(api, 0, 99, INT8_MAX);

    TEST_ASSERT_EQUAL(99, readCounter);
    TEST_ASSERT_EQUAL(99, writeCounter);
    TEST_ASSERT_EQUAL(99, writeCounter);
    TEST_ASSERT_EQUAL(INT8_MAX, lastWriteValue);
}

// static uint8_t clearMockCounter;
// static void countingClearMock(void) {
//     ++clearMockCounter;
// }

// static void test_clearStorage_custom(void) {
//     storage_api_t api = { .read = countingReadMock, .write = countingWriteMock, .length = lengthMock, .clear = countingClearMock, };

//     mockLength = 256;
//     readCounter = writeCounter = clearMockCounter = 0U;
//     readValue = 128U;

//     clearStorage(api);
//     TEST_ASSERT_EQUAL(0, readCounter);
//     TEST_ASSERT_EQUAL(0, writeCounter);
//     TEST_ASSERT_EQUAL(1, clearMockCounter);
// }

void testStorageApi(void) {
    Unity.TestFile = __FILE__;    

    RUN_TEST_P(test_update_nowrite_ifnochange);
    RUN_TEST_P(test_update_write_ifchange);
    RUN_TEST_P(test_updateBlock_reads_all_addresses);
    RUN_TEST_P(test_updateBlock_writes_all_addresses);
    RUN_TEST_P(test_updateBlockLimitWriteOps_reads_all_addresses);
    RUN_TEST_P(test_updateBlockLimitWriteOps_writes_all_addresses);
    RUN_TEST_P(test_updateBlockLimitWriteOps_limited_writes);
    RUN_TEST_P(test_loadBlock);
    RUN_TEST_P(test_fillBlock_nochange_nowrite);
    RUN_TEST_P(test_fillBlock_write_if_different);
}
 