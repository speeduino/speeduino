#include "../test_utils.h"
#include "src/pins/pinMapping.h"
#include "board_definition.h"

static void test_pinTranslate(void)
{
    TEST_ASSERT_EQUAL(0, pinTranslate(0));
    TEST_ASSERT_EQUAL(BOARD_MAX_DIGITAL_PINS, pinTranslate(BOARD_MAX_DIGITAL_PINS));
    TEST_ASSERT_EQUAL(A8 + 1, pinTranslate(BOARD_MAX_DIGITAL_PINS+2));
}

static void test_pinTranslateAnalog(void)
{
    for (uint8_t loop=0; loop<_countof(ANALOG_PINS); ++loop)
    {
        TEST_ASSERT_EQUAL(ANALOG_PINS[loop], pinTranslateAnalog(loop));
    }

    TEST_ASSERT_EQUAL(_countof(ANALOG_PINS)+1, pinTranslateAnalog(_countof(ANALOG_PINS)+1));
}

void testPinMapping(void) 
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_pinTranslate);
        RUN_TEST_P(test_pinTranslateAnalog);
    }
}