#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    void testVvtControl(void);
    void testVvtInterrupt(void);

    testVvtControl();
    testVvtInterrupt();
}

TEST_HARNESS(runAllTests)
