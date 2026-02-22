#include "../test_harness_device.h"
#include "../test_harness_native.h"
#include "storage.h"
#include "pages.h"
#include "globals.h"

// Since it's almost impossible for the tests to clean up
// after themselves, we need to reset the global context
// prior to each test running.
//
// Since each test is (usually) testing the results of
// initialiseAll(), the flow is:
// 1. prepareForInitialise()
// 2. Set any config page values.
// 3. initialiseAll()
// 4. ASSERT on the results.
void prepareForInitialiseAll(uint8_t boardId) {
  setTuneToEmpty();
  // This is required to prevent initialiseAll() also
  // calling setTuneToEmpty & thus blatting any
  // configuration made in step 2.
  configPage2.pinMapping = boardId;
  currentStatus.initialisationComplete = false;
}

void runAllInitTests(void)
{
    extern void testInitialisation(void);
    extern void testFuelScheduleInit(void);
    extern void testIgnitionScheduleInit(void);

    testInitialisation();
    testFuelScheduleInit();
    testIgnitionScheduleInit();
}

TEST_HARNESS(runAllInitTests)
