#include <unity.h>
#include "../test_utils.h"
#include "board_definition.h"
#include "storage.h"
#include "pages.h"
#include "config_pages.h"

extern uint16_t getEntityStartAddress(page_iterator_t entity);
extern const eeprom_address_t MAX_PAGE_ADDRESS;
extern eeprom_address_t getSensorCalibrationCrcAddress(SensorCalibrationTable sensor);
extern const uint16_t STORAGE_SIZE;

static void test_getEntityStartAddress_invalid_entity(void) {
    config10 localPage10;
    TEST_ASSERT_EQUAL(0, getEntityStartAddress(page_iterator_t(&localPage10,
                                                                entity_page_location_t(2, 1),
                                                                entity_page_address_t(0, sizeof(localPage10)))));
}

static bool inline isInRangeExclusive(uint16_t rangeStart, uint16_t rangeEnd, uint16_t testValue) {
    return (testValue>=rangeStart) && (testValue<rangeEnd);
}

struct block {
    uint16_t start;
    uint16_t length;
};

static void assert_nocalibration_overlap(const block &newBlock, uint8_t idxCurrBlock, SensorCalibrationTable table) {
    uint16_t start = getSensorCalibrationCrcAddress(table);
    uint16_t end = start + sizeof(uint32_t);
    char msg[64];
    sprintf(msg, "EEPROM storage: entity %d overlaps calibration CRC %d", idxCurrBlock, table);
    TEST_ASSERT_FALSE_MESSAGE(isInRangeExclusive(start, end, newBlock.start), msg);
    TEST_ASSERT_FALSE_MESSAGE(isInRangeExclusive(start, end, newBlock.start+newBlock.length), msg);
}

static void assert_nocalibration_overlap(const block &newBlock, uint8_t idxCurrBlock) {
    assert_nocalibration_overlap(newBlock, idxCurrBlock, CoolantSensor);
    assert_nocalibration_overlap(newBlock, idxCurrBlock, IntakeAirTempSensor);
    assert_nocalibration_overlap(newBlock, idxCurrBlock, O2Sensor);
}

static bool inline overlaps(const block &a, const block &b) {
    return  isInRangeExclusive(a.start, a.start+a.length-1, b.start);
            isInRangeExclusive(a.start, a.start+a.length, b.start+b.length);
}

static uint8_t find_overlap(const block blocks[], uint8_t idxCurrBlock, const block &newBlock) {
    uint8_t i = 0;
    while ((i < idxCurrBlock) && !overlaps(blocks[i], newBlock)) {
        ++i;
    }
    return i;
}

static uint8_t test_no_overlap_page(uint8_t pageNum, block blocks[], size_t length, uint8_t idxCurrBlock) {
  page_iterator_t entity = page_begin(pageNum);
  while (entity.type!=End && entity.type!=NoEntity) {
    TEST_ASSERT_LESS_THAN(length, idxCurrBlock);

    block newBlock = { getEntityStartAddress(entity), entity.address.size };
    TEST_ASSERT_GREATER_THAN(0, newBlock.start);
    TEST_ASSERT_LESS_THAN(MAX_PAGE_ADDRESS, newBlock.start+newBlock.length);
    assert_nocalibration_overlap(newBlock, idxCurrBlock);
    uint8_t overlapBlock = find_overlap(blocks, idxCurrBlock, newBlock);
    if (overlapBlock!=idxCurrBlock) {
        char msg[64];
        sprintf(msg, "EEPROM storage: entity %d overlaps entity %d", overlapBlock, idxCurrBlock);
        TEST_FAIL_MESSAGE(msg);
    }
    TEST_ASSERT_TRUE(overlapBlock==idxCurrBlock);
    blocks[idxCurrBlock] = newBlock;

    ++idxCurrBlock;
    entity = advance(entity);
  }

  return idxCurrBlock;
}

static void test_no_entity_overlap(void) {
    block blocks[28];
    uint8_t idxCurrBlock = 0U;

    for (uint8_t i = 0; i < getPageCount(); i++) {
        idxCurrBlock = test_no_overlap_page(i, blocks, _countof(blocks), idxCurrBlock);
    }
}

void test_layout(void) {
    Unity.TestFile = __FILE__;    
    
    RUN_TEST(test_getEntityStartAddress_invalid_entity);
    RUN_TEST(test_no_entity_overlap);
}