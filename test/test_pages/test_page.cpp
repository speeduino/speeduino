#include <FastCRC.h>
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

static void test_getEntityValue_raw(void)
{
    constexpr char MARKER = 'X';
    char entity[48];
    memset(&entity, MARKER, sizeof(entity));

   page_iterator_t entityIter(  entity, 
                                entity_page_location_t(10, 0),
                                entity_page_address_t(0, sizeof(entity)));

    assert_entity(entityIter, MARKER);
}

static void test_getEntityValue_none(void)
{
    constexpr char MARKER = 'X';
    char entity[48];
    memset(&entity, MARKER, sizeof(entity));

   page_iterator_t entityIter(  NoEntity, 
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
    return page_iterator_t( &entity,
                            entity.type_key, 
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

void testPage(void) {
    SET_UNITY_FILENAME() {
        RUN_TEST(test_getEntityValue_raw);
        RUN_TEST(test_getEntityValue_none);
        RUN_TEST(test_getEntityValue_table);
    }
}