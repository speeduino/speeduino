#include <unity.h>
#include "pages.h"
#include "page_crc.h"
#include "../test_utils.h"

static void setPageValues_Incremental(uint8_t pageNum, char seedValue)
{
    for (uint16_t offset=0; offset<getPageSize(pageNum); ++offset)
    {
        setPageValue(boostvvtPage2, offset, seedValue+(uint8_t)offset);
    }
}

static void test_calculatePageCRC32(void)
{
    // boostvvtPage2 (12) & wmiMapPage contain multiple types of entities
    // which makes it a good test subjects
    setPageValues_Incremental(boostvvtPage2, 'X');
    TEST_ASSERT_EQUAL_UINT32(0x3C15D3B7, calculatePageCRC32(boostvvtPage2));

    setPageValues_Incremental(wmiMapPage, 'X');
    TEST_ASSERT_EQUAL_UINT32(0x2ED1D425, calculatePageCRC32(wmiMapPage));
}

void testPageCrc(void) {
    SET_UNITY_FILENAME() {
        RUN_TEST(test_calculatePageCRC32);
    }
}