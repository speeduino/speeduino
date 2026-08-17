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
    extern void testDecoder_General(void);
    extern void testToothLoggers(void);
    extern void testDecoderBuilder(void);
    extern void testDecoderInit(void);
    extern void testDecoderApiCoverage(void);
    extern void testinterrupt_t(void);
    extern void testSequentialToothTracker(void);

    testDecoder_General();
    testToothLoggers();
    testDecoderBuilder();
    testDecoderInit();
    testDecoderApiCoverage();
    testinterrupt_t();
    testSequentialToothTracker();
}

TEST_HARNESS(runAllTests)
