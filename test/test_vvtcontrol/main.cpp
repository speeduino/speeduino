#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    void testVvtControl(void);

    testVvtControl();
}

TEST_HARNESS(runAllTests)
