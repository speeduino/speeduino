#include <unity.h>
#include "pages.h"
#include "../test_utils.h"

static void assert_entity(const entity_t &entity, byte expected)
{
    for (uint16_t offset=0; offset<entity.size; ++offset)
    {
        TEST_ASSERT_EQUAL(expected, getEntityValue(entity, offset));
    }
    TEST_ASSERT_EQUAL(0U, getEntityValue(entity, entity.size+1U));
}

struct fake_config_page_t : public config_page_t {
    byte data[48];
};

static void test_getEntityValue_raw(void)
{
    constexpr char MARKER = 'X';
    fake_config_page_t entity;
    memset(&entity, MARKER, sizeof(entity));
    assert_entity(entity_t(&entity, sizeof(entity)), MARKER);
}

static void test_getEntityValue_none(void)
{
    assert_entity(entity_t(EntityType::NoEntity, 32U), 0U);
}

template <typename TTable>
static TTable setup3dTable(byte valueMarker, byte xMarker, byte yMarker)
{
    TTable entity;

    fill_table_values(entity, valueMarker);
    populate_table_axis(entity.axisX.begin(), xMarker);
    populate_table_axis(entity.axisY.begin(), yMarker);

    return entity;
}

template <typename TTable>
static entity_t setupTableEntity(TTable &entity)
{
    constexpr uint16_t countTableValue = decltype(entity.axisX)::length*decltype(entity.axisY)::length;
    constexpr uint16_t size = countTableValue+decltype(entity.axisX)::length+decltype(entity.axisY)::length;
    return entity_t(&entity, entity.type_key, size);
}

template <typename TTable>
static void assert_3d_table(const entity_t &entity, const TTable &table, byte valueMarker, byte xMarker, byte yMarker)
{
    char szMsg[64];
    uint16_t offset=0; 
    uint16_t valueSize = table.values.num_rows*table.values.row_size; 
    for (; offset<valueSize; ++offset)
    {
        sprintf(szMsg, "Value %" PRIu16, offset);
        TEST_ASSERT_EQUAL_MESSAGE(valueMarker, getEntityValue(entity, offset), szMsg);
    }
    for (; offset<valueSize+table.axisX.length; ++offset)
    {
        sprintf(szMsg, "XAxis %" PRIu16, offset);
        TEST_ASSERT_EQUAL_MESSAGE(xMarker, getEntityValue(entity, offset), szMsg);
    }
    for (; offset<valueSize+table.axisX.length+table.axisY.length; ++offset)
    {
        sprintf(szMsg, "YAxis %" PRIu16, offset);
        TEST_ASSERT_EQUAL_MESSAGE(yMarker, getEntityValue(entity, offset), szMsg);
    }
    // Offset is too large
    TEST_ASSERT_EQUAL(0, getEntityValue(entity, entity.size+1));
}

template <typename TTable>
static void test_getEntityValue_tableT(void)
{
    auto table = setup3dTable<TTable>('X', 'Y', 'Z');
    assert_3d_table(setupTableEntity(table), table, 'X', 'Y', 'Z');
}

static void test_getEntityValue_table(void)
{
    test_getEntityValue_tableT<table3d4RpmLoad>();
    test_getEntityValue_tableT<table3d6RpmLoad>();
    test_getEntityValue_tableT<table3d8RpmLoad>();
    test_getEntityValue_tableT<table3d16RpmLoad>();
}

static void set_entity_values(entity_t &entity, uint16_t from, uint16_t to, char value)
{
    for (uint16_t offset=from; offset<to; ++offset)
    {
        bool result = setEntityValue(entity, offset, value);
        if (entity.type!=EntityType::NoEntity)
        {
            TEST_ASSERT_TRUE(result);
        }
        else
        {
            TEST_ASSERT_FALSE(result);
        }
    }
}

static void test_setEntityValue_raw(void)
{
    constexpr char PRE_MARKER = 'X';
    fake_config_page_t rawEntity;
    memset(&rawEntity, PRE_MARKER, sizeof(rawEntity));

    entity_t entity(&rawEntity, sizeof(rawEntity));
    constexpr char POST_MARKER = 'Y';
    set_entity_values(entity, 0, entity.size, POST_MARKER);
    TEST_ASSERT_EACH_EQUAL_CHAR(POST_MARKER, &rawEntity, sizeof(rawEntity));

    TEST_ASSERT_FALSE(setEntityValue(entity, entity.size+1U, POST_MARKER));
}

static void test_setEntityValue_none(void)
{
    constexpr char PRE_MARKER = 'X';
    fake_config_page_t rawEntity;
    memset(&rawEntity, PRE_MARKER, sizeof(rawEntity));

    entity_t entity(&rawEntity, sizeof(rawEntity));
    entity.type = EntityType::NoEntity;

    constexpr char POST_MARKER = 'Y';
    set_entity_values(entity, 0, entity.size, POST_MARKER);
    // setEntityValue should have no effect
    TEST_ASSERT_EACH_EQUAL_CHAR(PRE_MARKER, &rawEntity, sizeof(rawEntity));

    TEST_ASSERT_FALSE(setEntityValue(entity, entity.size+1U, POST_MARKER));
}

template <typename TTable>
static void assert_set_3d_table(entity_t entity, const TTable &table, byte valuePre, byte xAxisPre, byte yAxisPre)
{
    const uint16_t valueSize = table.values.num_rows*table.values.row_size; 
    const char valuePost = valuePre+1;
    set_entity_values(entity, 0, valueSize, valuePost);
    TEST_ASSERT_EACH_EQUAL_CHAR(valuePost, table.values.values, valueSize);

    const char xAxisPost = xAxisPre+1;
    set_entity_values(entity, valueSize, valueSize+table.axisX.length, xAxisPost);
    TEST_ASSERT_EACH_EQUAL_CHAR(xAxisPost, table.axisX.axis, table.axisX.length);

    const char yAxisPost = yAxisPre+1;
    set_entity_values(entity, valueSize+table.axisX.length, valueSize+table.axisX.length+table.axisY.length, yAxisPost);
    TEST_ASSERT_EACH_EQUAL_CHAR(yAxisPost, table.axisY.axis, table.axisY.length);

    TEST_ASSERT_FALSE(setEntityValue(entity, entity.size+1U, valuePost));
}


template <typename TTable>
static void test_setEntityValue_tableT(void)
{
    auto entity = setup3dTable<TTable>('X', 'Y', 'Z');
    assert_set_3d_table(setupTableEntity(entity), entity, 'X', 'Y', 'Z');
}

static void test_setEntityValue_table(void)
{
    test_setEntityValue_tableT<table3d4RpmLoad>();
    test_setEntityValue_tableT<table3d6RpmLoad>();
    test_setEntityValue_tableT<table3d8RpmLoad>();
    test_setEntityValue_tableT<table3d16RpmLoad>();
}

static void assert_getPageValue(uint8_t page, uint16_t offset)
{
    constexpr char MARKER = 'X';
    if (setPageValue(page, offset, MARKER))
    {
        char szMsg[32];
        sprintf(szMsg, "Offset %" PRIu16, offset);
        TEST_ASSERT_EQUAL_MESSAGE(MARKER, getPageValue(page, offset), szMsg);
    }
}

static uint8_t testPageNum;

static void test_get_set_PageValueN(void)
{
    for (uint16_t offset=0; offset<getPageSize(testPageNum); ++offset)
    {
        assert_getPageValue(testPageNum, offset);
    }
}

static void test_get_set_PageValue_InvalidOffset_N(void)
{
    uint16_t invalidOffset = getPageSize(testPageNum)*2U;
    TEST_ASSERT_FALSE(setPageValue(testPageNum, invalidOffset, 'X'));
    TEST_ASSERT_EQUAL(0U, getPageValue(testPageNum, invalidOffset));
}

static void testGetSetPageValues(void)
{
    char szTestName[64];
    for (testPageNum = 0U; testPageNum<MAX_PAGE_NUM+1U; ++testPageNum)
    {
        snprintf(szTestName, sizeof(szTestName)-1U, "test_get_set_PageValue_%" PRIu8, testPageNum);
        UnityDefaultTestRun(test_get_set_PageValueN, szTestName, __LINE__);
        snprintf(szTestName, sizeof(szTestName)-1U, "test_get_set_PageValue_InvalidOffset_%" PRIu8, testPageNum);
        UnityDefaultTestRun(test_get_set_PageValue_InvalidOffset_N, szTestName, __LINE__);
    }
}

static void test_getPageSize(void)
{
    TEST_ASSERT_EQUAL(0, getPageSize(0U));
    TEST_ASSERT_EQUAL(0, getPageSize(MAX_PAGE_NUM+1U));
}

static uint16_t sumEntitySizes(uint8_t pageNum)
{
    uint16_t sum = 0;
    page_iterator_t it = page_begin(pageNum);
    while (EntityType::End!=it.entity.type)
    {
        sum += it.entity.size;
        it = advance(it);
    }
    return sum;
}

static void test_sumEntity_matches_pageSize(void)
{
    // Page sizes as defined in the .ini file
    constexpr uint16_t ini_page_sizes[] = { 0, 128, 288, 288, 128, 288, 128, 240, 384, 192, 192, 288, 192, 128, 288, 256 };

    for (uint8_t pageNum=MIN_PAGE_NUM; pageNum<MAX_PAGE_NUM; ++pageNum)
    {
        char szMsg[32];
        sprintf(szMsg, "Page %" PRIu8, pageNum);
        TEST_ASSERT_EQUAL_MESSAGE(getPageSize(pageNum), sumEntitySizes(pageNum), szMsg);
        TEST_ASSERT_EQUAL_MESSAGE(ini_page_sizes[pageNum], getPageSize(pageNum), szMsg);
    }
}

static void print_entity_layout(const page_iterator_t &entity)
{
    char szMsg[64];
    sprintf(szMsg, "%" PRIu8 ", %" PRIu8 ", %s, %" PRIu16 ", %" PRIu16, 
        entity.location.page, 
        entity.location.index, 
        entity.entity.type==EntityType::Raw ? "Raw" : (entity.entity.type==EntityType::Table ? "Table" : (entity.entity.type==EntityType::NoEntity ? "NoEntity" : "End")),
        entity.entity.start, 
        entity.entity.size);
    UnityPrint(szMsg); UNITY_PRINT_EOL();
}

static void print_page_entity_layout(uint8_t pageNum)
{
    page_iterator_t iter = page_begin(pageNum);
    while (EntityType::End!=iter.entity.type)
    {
        print_entity_layout(iter);
        iter = advance(iter);
    }
}

static void print_all_page_entity_layout(void)
{
    UnityPrint("Page, Index, Type, Start, Size"); UNITY_PRINT_EOL();
    for (uint8_t pageNum=MIN_PAGE_NUM; pageNum<MAX_PAGE_NUM; ++pageNum)
    {
        print_page_entity_layout(pageNum);
    }
}

static void print_page_layout(void)
{
    UnityPrint("Page, Size"); UNITY_PRINT_EOL();
    for (uint8_t pageNum=MIN_PAGE_NUM; pageNum<MAX_PAGE_NUM; ++pageNum)
    {
        char szMsg[32];
        sprintf(szMsg, "%" PRIu8 ", %" PRIu16, pageNum, getPageSize(pageNum));
        UnityPrint(szMsg); UNITY_PRINT_EOL();
    }
}

static uint16_t assert_unique(page_iterator_t testSubject, page_iterator_t* previousEntities, uint16_t nextSlot)
{
    for (uint16_t index=0; index<nextSlot; ++index)
    {
        // Every page iterator points to a different object
        TEST_ASSERT_FALSE(previousEntities[index].entity.pTable==testSubject.entity.pTable);
        TEST_ASSERT_FALSE(previousEntities[index].entity.pRaw==testSubject.entity.pRaw);
        // Page iterator locations are unique
        TEST_ASSERT_FALSE(previousEntities[index].location==testSubject.location);
    }
    previousEntities[nextSlot] = testSubject;
    return nextSlot+1U;
}

static uint16_t test_unique_entities(uint8_t pageNum, page_iterator_t* previousEntities, uint16_t nextSlot)
{
    uint8_t lastPageIndex = -1;
    page_iterator_t iter = page_begin(pageNum);
    while (EntityType::End!=iter.entity.type)
    {
        TEST_ASSERT_EQUAL(pageNum, iter.location.page);
        // Indexes should be monotonic
        TEST_ASSERT_EQUAL(++lastPageIndex, iter.location.index);
        // Entities should be next to each other with zero overlap
        if (iter.location.index!=0U)
        {
            TEST_ASSERT_EQUAL(previousEntities[nextSlot-1U].entity.start+previousEntities[nextSlot-1U].entity.size, iter.entity.start);
        }
        if (EntityType::NoEntity!=iter.entity.type)
        {
            nextSlot = assert_unique(iter, previousEntities, nextSlot);
        }
        iter = advance(iter);
    }
    return nextSlot;
}

static void test_unique_entities(void)
{
    page_iterator_t globalEntityIterators[256];
    uint16_t index = 0;

    for (uint8_t pageNum=MIN_PAGE_NUM; pageNum<MAX_PAGE_NUM; ++pageNum)
    {
        index = test_unique_entities(pageNum, globalEntityIterators, index);
    }
}

void testPage(void) {
    SET_UNITY_FILENAME() {
        RUN_TEST(test_getEntityValue_raw);
        RUN_TEST(test_getEntityValue_none);
        RUN_TEST(test_getEntityValue_table);
        RUN_TEST(test_setEntityValue_raw);
        RUN_TEST(test_setEntityValue_none);
        RUN_TEST(test_setEntityValue_table);
        RUN_TEST(test_getPageSize);
        // Not a unit test, as it runs multiple tests in a loop
        // DO NOT PLACE INSIDE a RUN_TEST().
        testGetSetPageValues(); 
        RUN_TEST(print_all_page_entity_layout);
        RUN_TEST(print_page_layout);
        RUN_TEST(test_sumEntity_matches_pageSize);
        RUN_TEST(test_unique_entities);
    }
}