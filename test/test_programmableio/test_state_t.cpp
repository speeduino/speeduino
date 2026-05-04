#include "../test_utils.h"
#include "programmableIOControl_internals.h"

using namespace programmableIOControl_details;

static void test_compressedOutputStatus(void)
{
    state_t testSubject = {};

    TEST_ASSERT_EQUAL(0, testSubject.compressedOutputStatus());

    testSubject.channels[1].isOutputActive = true;
    testSubject.channels[3].isOutputActive = true;
    testSubject.channels[7].isOutputActive = true;
    TEST_ASSERT_EQUAL(0b10001010, testSubject.compressedOutputStatus());
}

void testProgrammableIOControlStateT(void) 
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_compressedOutputStatus);
    }
}