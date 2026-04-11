#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllIgnitionTests(void)
{
    extern void testIgnCorrections(void);
    extern void testDwell(void);

    testIgnCorrections();
    testDwell();
}

TEST_HARNESS(runAllIgnitionTests)
