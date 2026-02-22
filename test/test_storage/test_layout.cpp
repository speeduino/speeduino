#include <unity.h>
#include "../test_utils.h"
#include "board_definition.h"
#include "storage.h"
#include "pages.h"
#include "config_pages.h"
#include "globals.h"

extern uint16_t getEntityStartAddress(page_iterator_t entity);
extern const uint16_t MAX_PAGE_ADDRESS;
extern uint16_t getSensorCalibrationCrcAddress(SensorCalibrationTable sensor);
extern const uint16_t STORAGE_SIZE;

static void test_getEntityStartAddress_invalid_entity(void) {
    config10 localPage10;
    page_iterator_t entity(NoEntity,entity_page_location_t(2, 1), entity_page_address_t(0, sizeof(localPage10)));
    entity.setRaw(&localPage10);
    TEST_ASSERT_EQUAL(0, getEntityStartAddress(entity));
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
    sprintf(msg, "EEPROM storage: entity %" PRIu16 " overlaps calibration CRC %" PRIu16, idxCurrBlock, (uint16_t)table);
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
        sprintf(msg, "EEPROM storage: entity %" PRIu8 " overlaps entity %" PRIu8, overlapBlock, idxCurrBlock);
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

const char* getEntityType(const page_iterator_t &entity) {
    switch (entity.type) {
        case Raw: return "Raw";
        case Table: return "Table";
        case NoEntity: return "NoEntity";
        case End: return "End";
        default: return "Unknown";
    }
}

const char *getEntityName(const page_iterator_t &it) {
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
    { &trim1Table, GET_VARIABLE_NAME(trim1Table) },
    { &trim2Table, GET_VARIABLE_NAME(trim2Table) },
    { &trim3Table, GET_VARIABLE_NAME(trim3Table) },
    { &trim4Table, GET_VARIABLE_NAME(trim4Table) },
    { &trim5Table, GET_VARIABLE_NAME(trim5Table) },
    { &trim6Table, GET_VARIABLE_NAME(trim6Table) },
    { &trim7Table, GET_VARIABLE_NAME(trim7Table) },
    { &trim8Table, GET_VARIABLE_NAME(trim8Table) },
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
  while ((pMapEntry!=entityMapEnd) && (it.pData!=pMapEntry->pEntity)) {
    ++pMapEntry;
  }
  if (pMapEntry!=entityMapEnd) {
    return pMapEntry->name.c_str();
  }
  static const char *unknown = "Unknown";
  return unknown;
}

static void print_entity(const page_iterator_t &entity)
{
    char msg[128];
    sprintf(msg, "%" PRIu8 ", %" PRIu8 ", %s, %s, %" PRIu16 ", %" PRIu16, entity.location.page, entity.location.index, getEntityName(entity), getEntityType(entity), getEntityStartAddress(entity), entity.address.size);
    UnityPrint(msg); UNITY_PRINT_EOL();
}

static void print_page_layout(uint8_t pageNum)
{
    page_iterator_t entity = page_begin(pageNum);
    while (entity.type!=End) {
        print_entity(entity);
        entity = advance(entity);
    }
}

// An informational function to print the layout of the EEPROM as CSV
// Requires "-v" flag on pio unit test runner 
static void print_eeprom_layout(void) {
    UnityPrint("Page, Index, Item, Type, Start Address, Length"); UNITY_PRINT_EOL();
    for (uint8_t page = 0; page < getPageCount(); page++) {
        print_page_layout(page);
    }

    #define GET_VARIABLE_NAME(Variable) (#Variable)
    char msg[128];
    sprintf(msg, "Calib. CRC, %d, %s, CRC, %" PRIu16 ", %zu", (int)SensorCalibrationTable::CoolantSensor, GET_VARIABLE_NAME(CoolantSensor), getSensorCalibrationCrcAddress(SensorCalibrationTable::CoolantSensor), sizeof(uint32_t));
    UnityPrint(msg); UNITY_PRINT_EOL();
    sprintf(msg, "Calib. CRC, %d, %s, CRC, %" PRIu16 ", %zu", (int)SensorCalibrationTable::IntakeAirTempSensor, GET_VARIABLE_NAME(IntakeAirTempSensor), getSensorCalibrationCrcAddress(SensorCalibrationTable::IntakeAirTempSensor), sizeof(uint32_t));
    UnityPrint(msg); UNITY_PRINT_EOL();
    sprintf(msg, "Calib. CRC, %d, %s, CRC, %" PRIu16 ", %zu", (int)SensorCalibrationTable::O2Sensor, GET_VARIABLE_NAME(O2Sensor), getSensorCalibrationCrcAddress(SensorCalibrationTable::O2Sensor), sizeof(uint32_t));
    UnityPrint(msg); UNITY_PRINT_EOL();
    sprintf(msg, "Calibrations, 0, Calib, Calib, %" PRIu16 ", %" PRIu16, MAX_PAGE_ADDRESS, (uint16_t)(STORAGE_SIZE-MAX_PAGE_ADDRESS));
    UnityPrint(msg); UNITY_PRINT_EOL();
}

void test_layout(void) {
    SET_UNITY_FILENAME() {     
        RUN_TEST_P(test_getEntityStartAddress_invalid_entity);
        RUN_TEST_P(test_no_entity_overlap);
        RUN_TEST_P(print_eeprom_layout);
    }
}