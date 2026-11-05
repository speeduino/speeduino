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
  extern void testWebber(void);
  extern void testFordST170(void);
  extern void testDRZ400(void);
  extern void testNGC(void);
  extern void testVMax(void);
  extern void testRenix(void);
  extern void testRoverMems(void);
  extern void testSuzukiK6A();
  extern void testFordTFI(void);

  testWebber();
  testFordST170();
  testDRZ400();
  testNGC();
  testVMax();
  testRenix();
  testRoverMems();
  testSuzukiK6A();
  testFordTFI();
}

TEST_HARNESS(runAllTests)
