#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllCommsTests(void)
{
    extern void testScatterOutputChannels(void);

    testScatterOutputChannels();
}

TEST_HARNESS(runAllCommsTests)
