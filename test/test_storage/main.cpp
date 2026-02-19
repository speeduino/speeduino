#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllStorageTests(void)
{
    extern void test_layout(void);

    test_layout();
}

TEST_HARNESS(runAllStorageTests)
