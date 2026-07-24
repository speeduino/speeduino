#include <unity.h>
#include "../test_utils.h"
#include "board_definition.h"
#include "storage.h"
#include "storage_details.h"
#include "pages.h"
#include "config_pages.h"
#include "scheduler.h"

using namespace storage::details;

extern uint16_t getEntityStartAddress(page_iterator_t entity);
extern const uint16_t MAX_PAGE_ADDRESS;
extern const uint16_t STORAGE_SIZE;
extern uint16_t getSensorCalibrationAddress(SensorCalibrationTable sensor, SensorCalibrationTableElement element);

static void test_getEntityStartAddress_invalid_entity(void) {
    config10 localPage10;
    page_iterator_t iter(page_entity_t(entity_t(&localPage10, sizeof(localPage10)), 0U), entity_page_location_t(2, 1));
    TEST_ASSERT_EQUAL(0, getEntityStartAddress(iter));
}

static bool inline isInRangeExclusive(uint16_t rangeStart, uint16_t rangeEnd, uint16_t testValue) {
    return (testValue>=rangeStart) && (testValue<rangeEnd);
}

struct block {
    uint16_t start;
    uint16_t length;
};

static void assert_nocalibration_overlap(const block &newBlock, uint8_t idxCurrBlock, SensorCalibrationTable table) {
    uint16_t start = getSensorCalibrationAddress(table, SensorCalibrationTableElement::Crc);
    uint16_t end = start + sizeof(uint32_t);
    char msg[64];
    snprintf(msg, _countof(msg)-1, "EEPROM storage: entity %" PRIu16 " overlaps calibration CRC %" PRIu16, idxCurrBlock, (uint16_t)table);
    TEST_ASSERT_FALSE_MESSAGE(isInRangeExclusive(start, end, newBlock.start), msg);
    TEST_ASSERT_FALSE_MESSAGE(isInRangeExclusive(start, end, newBlock.start+newBlock.length), msg);
}

static void assert_nocalibration_overlap(const block &newBlock, uint8_t idxCurrBlock) {
    assert_nocalibration_overlap(newBlock, idxCurrBlock, SensorCalibrationTable::CoolantSensor);
    assert_nocalibration_overlap(newBlock, idxCurrBlock, SensorCalibrationTable::IntakeAirTempSensor);
    assert_nocalibration_overlap(newBlock, idxCurrBlock, SensorCalibrationTable::O2Sensor);
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
  page_iterator_t iter = page_begin(pageNum);
  while (iter.entity.type!=EntityType::End) {
    if (iter.entity.type!=EntityType::NoEntity) {
        TEST_ASSERT_LESS_THAN(length, idxCurrBlock);

        block newBlock = { getEntityStartAddress(iter), iter.entity.size };
        TEST_ASSERT_GREATER_THAN(0, newBlock.start);
        TEST_ASSERT_LESS_THAN(MAX_PAGE_ADDRESS, newBlock.start+newBlock.length);
        assert_nocalibration_overlap(newBlock, idxCurrBlock);
        uint8_t overlapBlock = find_overlap(blocks, idxCurrBlock, newBlock);
        if (overlapBlock!=idxCurrBlock) {
            char msg[64];
            snprintf(msg, _countof(msg)-1, "EEPROM storage: entity %" PRIu8 " overlaps entity %" PRIu8, overlapBlock, idxCurrBlock);
            TEST_FAIL_MESSAGE(msg);
        }
        TEST_ASSERT_TRUE(overlapBlock==idxCurrBlock);
        blocks[idxCurrBlock] = newBlock;

        ++idxCurrBlock;
    }
    iter = advance(iter);
  }

  return idxCurrBlock;
}

static void test_no_entity_overlap(void) {
    block blocks[28];
    uint8_t idxCurrBlock = 0U;

    for (uint8_t i = MIN_PAGE_NUM; i < MAX_PAGE_NUM; i++) {
        idxCurrBlock = test_no_overlap_page(i, blocks, _countof(blocks), idxCurrBlock);
    }
}

const char* getEntityTypeName(const page_iterator_t &iter) {
    switch (iter.entity.type) {
        case EntityType::Raw: return "Raw";
        case EntityType::Table: return "Table";
        case EntityType::NoEntity: return "NoEntity";
        case EntityType::End: return "End";
        default: return "Unknown";
    }
}

static const char * getEntityNameNew(const page_iterator_t &it) {
   struct entity_name_map_t {
      entity_page_location_t location;
      String name;
  };

  // Store a map of page locations to human readable names
  static const entity_name_map_t entityMap[] = {
    { entity_page_location_t(O2_CALIBRATION_PAGE, PAGE_IDX_CALIBRATION_CRC), "O2 Crc" },
    { entity_page_location_t(O2_CALIBRATION_PAGE, PAGE_IDX_CALIBRATION_VALUES), "O2 Calib Values" },
    { entity_page_location_t(O2_CALIBRATION_PAGE, PAGE_IDX_CALIBRATION_BINS), "O2 Calib Bins" },
    { entity_page_location_t(IAT_CALIBRATION_PAGE, PAGE_IDX_CALIBRATION_CRC), "IAT Crc" },
    { entity_page_location_t(IAT_CALIBRATION_PAGE, PAGE_IDX_CALIBRATION_VALUES), "IAT Calib Values" },
    { entity_page_location_t(IAT_CALIBRATION_PAGE, PAGE_IDX_CALIBRATION_BINS), "IAT Calib Bins" },
    { entity_page_location_t(CLT_CALIBRATION_PAGE, PAGE_IDX_CALIBRATION_CRC), "Clt Crc" },
    { entity_page_location_t(CLT_CALIBRATION_PAGE, PAGE_IDX_CALIBRATION_VALUES), "Clt Calib Values" },
    { entity_page_location_t(CLT_CALIBRATION_PAGE, PAGE_IDX_CALIBRATION_BINS), "Clt Calib Bins" },
  };
  static const constexpr entity_name_map_t* entityMapEnd = entityMap + _countof(entityMap);  

  // Linear search of the name map.
  const entity_name_map_t *pMapEntry = entityMap;
  while ((pMapEntry!=entityMapEnd) && (it.location!=pMapEntry->location)) {
    ++pMapEntry;
  }
  if (pMapEntry!=entityMapEnd) {
    return pMapEntry->name.c_str();
  }

  return nullptr;
}

const char *getEntityName(const page_iterator_t &it) {
  auto name = getEntityNameNew(it);
  if (name!=nullptr) {
    return name;
  }

  #define GET_VARIABLE_NAME(Variable) (#Variable)

  struct entity_name_map_t {
      void *pEntity;
      String name;
  };

  // Store a map of entity to EEPROM address in FLASH memory.
  static const entity_name_map_t entityMap[] = {
    { &fuelTable, GET_VARIABLE_NAME(fuelTable) },
    { &configPage2, GET_VARIABLE_NAME(configPage2) },
    { &ignitionTable, GET_VARIABLE_NAME(ignitionTable) },
    { &configPage4, GET_VARIABLE_NAME(configPage4) },
    { &afrTable, GET_VARIABLE_NAME(afrTable) },
    { &configPage6, GET_VARIABLE_NAME(configPage6) },
    { &boostTable, GET_VARIABLE_NAME(boostTable) },
    { &vvtTable, GET_VARIABLE_NAME(vvtTable) },
    { &stagingTable, GET_VARIABLE_NAME(stagingTable) },
    { &trimTables[0], GET_VARIABLE_NAME(trimTables[0]) },
#if INJ_CHANNELS >= 2
    { &trimTables[1], GET_VARIABLE_NAME(trimTables[1]) },
#endif
#if INJ_CHANNELS >= 3
    { &trimTables[2], GET_VARIABLE_NAME(trimTables[2]) },
#endif
#if INJ_CHANNELS >= 4
    { &trimTables[3], GET_VARIABLE_NAME(trimTables[3]) },
#endif
#if INJ_CHANNELS >= 5
    { &trimTables[4], GET_VARIABLE_NAME(trimTables[4]) },
#endif
#if INJ_CHANNELS >= 6
    { &trimTables[5], GET_VARIABLE_NAME(trimTables[5]) },
#endif
#if INJ_CHANNELS >= 7
    { &trimTables[6], GET_VARIABLE_NAME(trimTables[6]) },
#endif
#if INJ_CHANNELS >= 8
    { &trimTables[7], GET_VARIABLE_NAME(trimTables[7]) },
#endif
    { &configPage9, GET_VARIABLE_NAME(configPage9) },
    { &configPage10, GET_VARIABLE_NAME(configPage10) },
    { &fuelTable2, GET_VARIABLE_NAME(fuelTable2) },
    { &wmiTable, GET_VARIABLE_NAME(wmiTable) },
    { &vvt2Table, GET_VARIABLE_NAME(vvt2Table) },
    { &dwellTable, GET_VARIABLE_NAME(dwellTable) },
    { &configPage13, GET_VARIABLE_NAME(configPage13) },
    { &ignitionTable2, GET_VARIABLE_NAME(ignitionTable2) },
    { &boostTableLookupDuty, GET_VARIABLE_NAME(boostTableLookupDuty) },
    { &configPage15, GET_VARIABLE_NAME(configPage15) },
  };
  static const constexpr entity_name_map_t* entityMapEnd = entityMap + _countof(entityMap);  

  // Linear search of the name map.
  const entity_name_map_t *pMapEntry = entityMap;
  while ((pMapEntry!=entityMapEnd) && (it.entity.pRaw!=pMapEntry->pEntity)) {
    ++pMapEntry;
  }
  if (pMapEntry!=entityMapEnd) {
    return pMapEntry->name.c_str();
  }
  static const char *unknown = "Unknown";
  return unknown;
}

static void print_entity(const page_iterator_t &iter)
{
    if (EntityType::NoEntity!=iter.entity.type)
    {
        char msg[128];
        snprintf(msg, _countof(msg)-1, "%" PRIu8 ", %" PRIu8 ", %s, %s, %" PRIu16 ", %" PRIu16, iter.location.page, iter.location.index, getEntityName(iter), getEntityTypeName(iter), getEntityStartAddress(iter), iter.entity.size);
        UnityPrint(msg); UNITY_PRINT_EOL();
    }
}

static void print_page_layout(uint8_t pageNum)
{
    page_iterator_t iter = page_begin(pageNum);
    while (iter.entity.type!=EntityType::End) {
        print_entity(iter);
        iter = advance(iter);
    }
}

struct dummyPage : config_page_t
{
};
static dummyPage dummyPage;

static page_iterator_t getCalibrationIterator(SensorCalibrationTable table, SensorCalibrationTableElement element)
{
    uint8_t pageNum = SensorCalibrationTable::CoolantSensor==table ? CLT_CALIBRATION_PAGE
                        : SensorCalibrationTable::IntakeAirTempSensor==table ? IAT_CALIBRATION_PAGE
                            : O2_CALIBRATION_PAGE;

    entity_t entity(&dummyPage, getCalibrationElementSize(table, element));
    page_entity_t pageEntity(entity, getSensorCalibrationAddress(table, element));
    entity_page_location_t location(pageNum, (uint8_t)element);
    return page_iterator_t(pageEntity, location);
}

static void print_calib_table(SensorCalibrationTable table)
{
    print_entity(getCalibrationIterator(table, SensorCalibrationTableElement::Crc));
    print_entity(getCalibrationIterator(table, SensorCalibrationTableElement::Values));
    print_entity(getCalibrationIterator(table, SensorCalibrationTableElement::Bins));
}

// An informational function to print the layout of the EEPROM as CSV
// Requires "-v" flag on pio unit test runner 
static void print_eeprom_layout(void) {
    UnityPrint("Page, Index, Item, Type, Start Address, Length"); UNITY_PRINT_EOL();
    for (uint8_t page = MIN_PAGE_NUM; page < MAX_PAGE_NUM; page++) {
        print_page_layout(page);
    }

    print_calib_table(SensorCalibrationTable::IntakeAirTempSensor);
    print_calib_table(SensorCalibrationTable::CoolantSensor);
    print_calib_table(SensorCalibrationTable::O2Sensor);
}

void test_layout(void) {
    SET_UNITY_FILENAME() {     
        RUN_TEST_P(test_getEntityStartAddress_invalid_entity);
        RUN_TEST_P(test_no_entity_overlap);
        RUN_TEST_P(print_eeprom_layout);
    }
}