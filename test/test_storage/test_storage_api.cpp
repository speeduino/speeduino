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

static constexpr uint16_t MOVE_BLOCK_SIZE = 37; // An non-power of 2 size is a good test

static void test_moveBlock_up_nooverlap_noochanges(void) {
    storage_api_t api = { .read = countingReadMock, .write = countingWriteMock, .length = lengthMock };

    readCounter = writeCounter = 0U;
    readValue = 33;
    moveBlock(api, MOVE_BLOCK_SIZE*3, 0, MOVE_BLOCK_SIZE);
    TEST_ASSERT_EQUAL(MOVE_BLOCK_SIZE*2, readCounter);
    TEST_ASSERT_EQUAL(0, writeCounter);    
}

static void test_moveBlock_down_nooverlap_noochanges(void) {
    storage_api_t api = { .read = countingReadMock, .write = countingWriteMock, .length = lengthMock };

    readCounter = writeCounter = 0U;
    readValue = 33;
    moveBlock(api, 0, MOVE_BLOCK_SIZE*3, MOVE_BLOCK_SIZE);
    TEST_ASSERT_EQUAL(MOVE_BLOCK_SIZE*2, readCounter);
    TEST_ASSERT_EQUAL(0, writeCounter);    
}

static uint16_t moveBlockReadAddresses[MOVE_BLOCK_SIZE*2];
static uint8_t sourceValue;
static uint8_t destValue;
static bool readingSource;

static byte moveBlockReadMock(uint16_t address) {
    TEST_ASSERT_LESS_THAN(_countof(moveBlockReadAddresses), readCounter);
    moveBlockReadAddresses[readCounter] = address;
    if (readingSource) {
        readValue = sourceValue;
    } else {
        readValue = destValue;
    }
    readingSource = !readingSource;
    return countingReadMock(address);
}

static uint16_t moveBlockWriteAddresses[MOVE_BLOCK_SIZE];
static void moveBlockWriteMock(uint16_t address, byte value) {
    TEST_ASSERT_LESS_THAN(_countof(moveBlockWriteAddresses), writeCounter);
    moveBlockWriteAddresses[writeCounter] = address;
    countingWriteMock(address, value);
}

static uint16_t findWriteAddressIndex(uint16_t address) {
    for (uint8_t index=0; index<_countof(moveBlockWriteAddresses); ++index) {
        if (moveBlockWriteAddresses[index]==address) {
            return index;
        }
    }
    return _countof(moveBlockWriteAddresses);
}

static void assert_moveBlock_read_before_write(int16_t distanceMoved) {
    for (uint8_t index=0; index<_countof(moveBlockReadAddresses); index+=2) {
        char szMsg[64];
        sprintf_P(szMsg, PSTR("Index %" PRIu8 ", Distance %" PRIi16), index, distanceMoved);
        // Confirm the source was read, then the destination
        TEST_ASSERT_EQUAL_MESSAGE((int16_t)moveBlockReadAddresses[index]+distanceMoved, moveBlockReadAddresses[index+1], szMsg);
        // Confirm the 2nd read address matches the write address 
        TEST_ASSERT_EQUAL_MESSAGE((int16_t)moveBlockReadAddresses[index]+distanceMoved, moveBlockWriteAddresses[index/2], szMsg);

        // Confirm there was no earlier write to the source read address
        sprintf_P(szMsg, PSTR("Address written to before read. Index %" PRIu8 ", Distance %" PRIi16), index, distanceMoved);
        TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(index/2, findWriteAddressIndex(moveBlockReadAddresses[index]), szMsg);
    }
}

static void test_moveBlock_up_overlap(void) {
    storage_api_t api = { .read = moveBlockReadMock, .write = moveBlockWriteMock, .length = lengthMock, };

    readCounter = writeCounter = 0U;
    readValue = 33;
    sourceValue = 7;
    destValue = 19;
    readingSource = true;

    moveBlock(api, MOVE_BLOCK_SIZE/2, 0, MOVE_BLOCK_SIZE);
    TEST_ASSERT_EQUAL(MOVE_BLOCK_SIZE*2, readCounter);
    TEST_ASSERT_EQUAL(MOVE_BLOCK_SIZE, writeCounter);

    assert_moveBlock_read_before_write(MOVE_BLOCK_SIZE/2);
}

static void test_moveBlock_down_overlap(void) {
    storage_api_t api = { .read = moveBlockReadMock, .write = moveBlockWriteMock, .length = lengthMock, };

    readCounter = writeCounter = 0U;
    readValue = 33;
    sourceValue = 7;
    destValue = 19;
    readingSource = true;

    moveBlock(api, 0, MOVE_BLOCK_SIZE/2, MOVE_BLOCK_SIZE);
    TEST_ASSERT_EQUAL(MOVE_BLOCK_SIZE*2, readCounter);
    TEST_ASSERT_EQUAL(MOVE_BLOCK_SIZE, writeCounter);

    assert_moveBlock_read_before_write(-1*(int16_t)MOVE_BLOCK_SIZE/2);
}

static void test_moveBlock_up_adjacent(void) {
    storage_api_t api = { .read = moveBlockReadMock, .write = moveBlockWriteMock, .length = lengthMock, };

    readCounter = writeCounter = 0U;
    readValue = 33;
    sourceValue = 7;
    destValue = 19;
    readingSource = true;

    moveBlock(api, MOVE_BLOCK_SIZE, 0, MOVE_BLOCK_SIZE);
    TEST_ASSERT_EQUAL(MOVE_BLOCK_SIZE*2, readCounter);
    TEST_ASSERT_EQUAL(MOVE_BLOCK_SIZE, writeCounter);

    assert_moveBlock_read_before_write(MOVE_BLOCK_SIZE);
}

static void test_moveBlock_down_adjacent(void) {
    storage_api_t api = { .read = moveBlockReadMock, .write = moveBlockWriteMock, .length = lengthMock, };

    readCounter = writeCounter = 0U;
    readValue = 33;
    sourceValue = 7;
    destValue = 19;
    readingSource = true;

    moveBlock(api, 0, MOVE_BLOCK_SIZE, MOVE_BLOCK_SIZE);
    TEST_ASSERT_EQUAL(MOVE_BLOCK_SIZE*2, readCounter);
    TEST_ASSERT_EQUAL(MOVE_BLOCK_SIZE, writeCounter);

    assert_moveBlock_read_before_write(-1*(int16_t)MOVE_BLOCK_SIZE);
}

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
    RUN_TEST_P(test_moveBlock_up_nooverlap_noochanges);
    RUN_TEST_P(test_moveBlock_down_nooverlap_noochanges);
    RUN_TEST_P(test_moveBlock_up_overlap);
    RUN_TEST_P(test_moveBlock_down_overlap);
    RUN_TEST_P(test_moveBlock_up_adjacent);
    RUN_TEST_P(test_moveBlock_down_adjacent);
}
