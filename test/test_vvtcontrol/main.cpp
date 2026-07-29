#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    void testVvtControl(void);
    void testVvtInterrupt(void);
    void testInit(void);

    testVvtControl();
    testVvtInterrupt();
    testInit();
}

TEST_HARNESS(runAllTests)
