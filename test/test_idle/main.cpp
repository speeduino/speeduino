#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    extern void testInitialiseIdle(void);
    extern void testDisableIdle(void);
    extern void testIdleControl(void);
    extern void testIdleControlOnOff(void);
    extern void testIdleControlStepOlCl(void);
    extern void testIdleControlStepCl(void);
    extern void testIdleControlStepOl(void);
    extern void testIdleControlPwmOl(void);
    extern void testIdleInterrupt(void);

    testInitialiseIdle();
    testDisableIdle();
    testIdleControl();
    testIdleControlOnOff();
    testIdleControlStepOlCl();
    testIdleControlStepCl();
    testIdleControlStepOl();
    testIdleControlPwmOl();
    testIdleInterrupt();
}

TEST_HARNESS(runAllTests)
