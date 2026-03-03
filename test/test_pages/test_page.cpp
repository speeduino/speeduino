#include <unity.h>
#include "pages.h"
#include "../test_utils.h"

static void assert_entity(const page_iterator_t &entity, byte expected)
{
    for (uint16_t offset=0; offset<entity.address.size; ++offset)
    {
        TEST_ASSERT_EQUAL(expected, getEntityValue(entity, offset));
    }
}

struct fake_config_page_t : public config_page_t {
    byte data[48];
};

static void test_getEntityValue_raw(void)
{
    constexpr char MARKER = 'X';
    fake_config_page_t entity;
    memset(&entity, MARKER, sizeof(entity));

    page_iterator_t entityIter(&entity,
                            entity_page_location_t(10, 0),
                            entity_page_address_t(0, sizeof(entity)));

    assert_entity(entityIter, MARKER);
}

static void test_getEntityValue_none(void)
{
    constexpr char MARKER = 'X';
    char entity[48];
    memset(&entity, MARKER, sizeof(entity));

   page_iterator_t entityIter(  EntityType::NoEntity, 
                                entity_page_location_t(10, 0),
                                entity_page_address_t(0, sizeof(entity)));

    assert_entity(entityIter, 0U);
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
static page_iterator_t setupTableIterator(TTable &entity)
{
    constexpr uint16_t countTableValue = decltype(entity.axisX)::length*decltype(entity.axisY)::length;
    constexpr uint16_t size = countTableValue+decltype(entity.axisX)::length+decltype(entity.axisY)::length;
    return page_iterator_t( &entity, entity.type_key,
                            entity_page_location_t(10, 0),
                            entity_page_address_t(0, size));
}

template <typename TTable>
static void assert_3d_table(const page_iterator_t &entity, const TTable &table, byte valueMarker, byte xMarker, byte yMarker)
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
    TEST_ASSERT_EQUAL(0, getEntityValue(entity, entity.address.size+1));
}

template <typename TTable>
static void test_getEntityValue_tableT(void)
{
    auto entity = setup3dTable<TTable>('X', 'Y', 'Z');
    page_iterator_t entityIter = setupTableIterator(entity);
    assert_3d_table(entityIter, entity, 'X', 'Y', 'Z');
}

static void test_getEntityValue_table(void)
{
    test_getEntityValue_tableT<table3d4RpmLoad>();
    test_getEntityValue_tableT<table3d6RpmLoad>();
    test_getEntityValue_tableT<table3d8RpmLoad>();
    test_getEntityValue_tableT<table3d16RpmLoad>();
}

static void set_entity_values(page_iterator_t &entity, uint16_t from, uint16_t to, char value)
{
    for (uint16_t offset=from; offset<to; ++offset)
    {
        setEntityValue(entity, offset, value);
    }
}

static void test_setEntityValue_raw(void)
{
    constexpr char PRE_MARKER = 'X';
    fake_config_page_t entity;
    memset(&entity, PRE_MARKER, sizeof(entity));

    page_iterator_t entityIter( &entity,
                                entity_page_location_t(10, 0),
                                entity_page_address_t(0, sizeof(entity)));

    constexpr char POST_MARKER = 'Y';
    set_entity_values(entityIter, 0, entityIter.address.size, POST_MARKER);
    TEST_ASSERT_EACH_EQUAL_CHAR(POST_MARKER, &entity, sizeof(entity));
}

static void test_setEntityValue_none(void)
{
    constexpr char PRE_MARKER = 'X';
    char entity[48];
    memset(&entity, PRE_MARKER, sizeof(entity));

    page_iterator_t entityIter( EntityType::NoEntity, 
                                entity_page_location_t(10, 0),
                                entity_page_address_t(0, sizeof(entity)));

    constexpr char POST_MARKER = 'Y';
    set_entity_values(entityIter, 0, entityIter.address.size, POST_MARKER);
    // setEntityValue should have no effect
    TEST_ASSERT_EACH_EQUAL_CHAR(PRE_MARKER, entity, sizeof(entity));
}

template <typename TTable>
static void assert_set_3d_table(page_iterator_t entity, const TTable &table, byte valuePre, byte xAxisPre, byte yAxisPre)
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
}


template <typename TTable>
static void test_setEntityValue_tableT(void)
{
    auto entity = setup3dTable<TTable>('X', 'Y', 'Z');
    assert_set_3d_table(setupTableIterator(entity), entity, 'X', 'Y', 'Z');
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
    while (EntityType::End!=it.type)
    {
        sum += it.address.size;
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
        entity.type==EntityType::Raw ? "Raw" : (entity.type==EntityType::Table ? "Table" : (entity.type==EntityType::NoEntity ? "NoEntity" : "End")),
        entity.address.start, 
        entity.address.size);
    UnityPrint(szMsg); UNITY_PRINT_EOL();
}

static void print_page_entity_layout(uint8_t pageNum)
{
    page_iterator_t entity = page_begin(pageNum);
    while (EntityType::End!=entity.type)
    {
        print_entity_layout(entity);
        entity = advance(entity);
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
        TEST_ASSERT_FALSE(previousEntities[index].pTable==testSubject.pTable);
        TEST_ASSERT_FALSE(previousEntities[index].pRaw==testSubject.pRaw);
        // Page iterator locations are unique
        TEST_ASSERT_FALSE(previousEntities[index].location==testSubject.location);
    }
    previousEntities[nextSlot] = testSubject;
    return nextSlot+1U;
}

static uint16_t test_unique_entities(uint8_t pageNum, page_iterator_t* previousEntities, uint16_t nextSlot)
{
    uint8_t lastPageIndex = -1;
    page_iterator_t entity = page_begin(pageNum);
    while (EntityType::End!=entity.type)
    {
        TEST_ASSERT_EQUAL(pageNum, entity.location.page);
        // Indexes should be monotonic
        TEST_ASSERT_EQUAL(++lastPageIndex, entity.location.index);
        // Entities should be next to each other with zero overlap
        if (entity.location.index!=0U)
        {
            TEST_ASSERT_EQUAL(previousEntities[nextSlot-1U].address.start+previousEntities[nextSlot-1U].address.size, entity.address.start);
        }
        if (EntityType::NoEntity!=entity.type)
        {
            nextSlot = assert_unique(entity, previousEntities, nextSlot);
        }
        entity = advance(entity);
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