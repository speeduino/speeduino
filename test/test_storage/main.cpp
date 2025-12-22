#include "../device_test_harness.h"

void runAllStorageTests(void)
{
    extern void test_layout(void);
    extern void testStorageApi(void);

    test_layout();
    testStorageApi();
}

DEVICE_TEST(runAllStorageTests)
