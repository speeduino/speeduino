#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllStorageTests(void)
{
    extern void test_layout(void);
    extern void testStorageApi(void);

    test_layout();
    testStorageApi();
}

TEST_HARNESS(runAllStorageTests)
