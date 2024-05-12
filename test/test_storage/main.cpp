#include "../device_test_harness.h"

void runAllStorageTests(void)
{
    extern void test_layout(void);

    test_layout();
}

DEVICE_TEST(runAllStorageTests)
