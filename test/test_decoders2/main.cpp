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

  testWebber();
  testFordST170();
  testDRZ400();
  testNGC();
  testVMax();
}

TEST_HARNESS(runAllTests)
