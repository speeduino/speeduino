#define CUSTOM_TEARDOWN
#include "../test_harness_device.h"
#include "../test_harness_native.h"
#include "globals.h"

void tearDown(void) 
{ 
  detachInterrupt( digitalPinToInterrupt(pinNumbers.pinTrigger) );
  detachInterrupt( digitalPinToInterrupt(pinNumbers.pinTrigger2) );
  detachInterrupt( digitalPinToInterrupt(pinNumbers.pinTrigger3) );
}

void runAllTests(void)
{
    extern void testMissingTooth(void);
    extern void testDualWheel(void);
    extern void testRenix(void);
    extern void testNissan360(void);
    extern void testFordST170(void);
    extern void testNGC(void);
    extern void testSuzukiK6A_setEndTeeth(void);
    extern void testSuzukiK6A_getCrankAngle(void);
    extern void testGM7X(void);
    extern void testHondaJ32(void);
    extern void test4G63(void);
    extern void test24X(void);
    extern void testJeep2000(void);
    extern void testMiata9905(void);
    extern void testMazdaAU(void);
    extern void testSubaru67(void);
    extern void test420a(void);
    extern void testVMax(void);
    extern void testBasicDistributor(void);
    extern void testAudi135(void);
    extern void testDaihatsu(void);
    extern void testFordTFI(void);
    extern void testHarley(void);
    extern void testHondaD17(void);
    extern void testNon360(void);
    extern void testRoverMems(void);
    extern void testThirtySixMinus21(void);
    extern void testThirtySixMinus22(void);

    testMissingTooth();
    testDualWheel();
    testRenix();
    testNissan360();
    testFordST170();
    testNGC();
    testSuzukiK6A_setEndTeeth();
    testSuzukiK6A_getCrankAngle();
    testGM7X();
    testHondaJ32();
    test4G63();
    test24X();
    testJeep2000();
    testMiata9905();
    testMazdaAU();
    testSubaru67();
    test420a();
    testVMax();
    testBasicDistributor();
    testAudi135();
    testDaihatsu();
    testFordTFI();
    testHarley();
    testHondaD17();
    testNon360();
    testRoverMems();
    testThirtySixMinus21();
    testThirtySixMinus22();
}

TEST_HARNESS(runAllTests)
