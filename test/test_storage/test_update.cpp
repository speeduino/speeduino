#include <unity.h>
#include "../test_utils.h"
#include "storage.h"
#include "fake_storage.h"
#include "table2d.h"

extern void updateTableU16toU8(table2D_u16_u8_32 &targetTable, uint16_t u16EEpromBinAddress);

static void test_updateTableU16toU8(void)
{
    uint8_t values[32];
    uint16_t axis[32];
    table2D_u16_u8_32 testSubject(&axis, &values);
    constexpr uint16_t OLD_AXIS = UINT16_MAX;
    constexpr uint8_t OLD_VALUE = 0U;
    populate_2dtable(&testSubject, OLD_VALUE, OLD_AXIS);

    constexpr uint8_t EEPROM_BYTE = 127U;
    setStorageAPI(getOneByteStorageApi(0xFFF, 0xFFF, EEPROM_BYTE));
    updateTableU16toU8(testSubject, 1024 /* Address doesn't matter */);

    // Reads but no writes
    TEST_ASSERT_NOT_EQUAL(0, oneByteEeprom.readCount);
    TEST_ASSERT_EQUAL(0, oneByteEeprom.writeCount);

    // Table has been updated
    constexpr uint16_t NEW_AXIS_VALUE = ((uint16_t)EEPROM_BYTE << 8U) | EEPROM_BYTE;
    static_assert(NEW_AXIS_VALUE!=OLD_AXIS, "Old & new need to be different for a valid test");
    static_assert(EEPROM_BYTE!=OLD_VALUE, "Old & new need to be different for a valid test");
    TEST_ASSERT_EACH_EQUAL_UINT16(NEW_AXIS_VALUE, axis, _countof(axis));
    TEST_ASSERT_EACH_EQUAL_UINT8(EEPROM_BYTE, values, _countof(values));
}

void test_update(void) {
    SET_UNITY_FILENAME() { 
        RUN_TEST(test_updateTableU16toU8);    
    }
}