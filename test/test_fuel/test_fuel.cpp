#include <Arduino.h>
#include <unity.h>
#include <avr/sleep.h>

#include "test_corrections.h"
#include "test_PW.h"
extern void testCalculateRequiredFuel(void);
extern void testCalculatePWLimit(void);
extern void testCalculateOpenTime(void);
extern void testPwApplyNitrous(void);
extern void testComputePrimaryPulseWidth(void);
extern void testApplyStagingToPw(void);
extern void testsetFuelChannelPulseWidths(void);

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

    testCorrections();
    testCalculateRequiredFuel();
    testCalculatePWLimit();
    testCalculateOpenTime();
    testPwApplyNitrous();
    testComputePrimaryPulseWidth();
    testApplyStagingToPw();
    testPW();
    testsetFuelChannelPulseWidths();

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