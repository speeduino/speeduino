#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTests(void)
{
    extern void testStatusBuilders(void);
    extern void testGetEntry(void);

    testStatusBuilders();
    testGetEntry();
}

TEST_HARNESS(runAllTests)
