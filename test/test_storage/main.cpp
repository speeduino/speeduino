#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllStorageTests(void)
{
    extern void test_layout(void);
    extern void testStorageApi(void);
    extern void test_storage(void);
    extern void test_update(void);

    test_layout();
    testStorageApi();
    test_storage();
    test_update();
}

TEST_HARNESS(runAllStorageTests)
