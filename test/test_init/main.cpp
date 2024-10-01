#include <Arduino.h>
#include <unity.h>
#include <avr/sleep.h>
#include "storage.h"
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
  resetConfigPages();
  // This is required to prevent initialiseAll() also
  // calling resetConfigPages & thus blatting any
  // configuration made in step 2.
  configPage2.pinMapping = boardId;
  currentStatus.initialisationComplete = false;
}


void testInitialisation(void);
void testFuelScheduleInit(void);
void testIgnitionScheduleInit(void);

#define UNITY_EXCLUDE_DETAILS

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
#if !defined(SIMULATOR)
    delay(2000);
#endif

    UNITY_BEGIN();    // IMPORTANT LINE!

    testFuelScheduleInit();
    testIgnitionScheduleInit();
    testInitialisation();

    UNITY_END(); // stop unit testing

#if defined(SIMULATOR)       // Tell SimAVR we are done
    cli();
    sleep_enable();
    sleep_cpu();
#endif     
}

void loop()
{
    // Blink to indicate end of test
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
}