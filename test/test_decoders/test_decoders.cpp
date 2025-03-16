
#include <Arduino.h>
#include <globals.h>
#include <unity.h>
#include <avr/sleep.h>

#include "missing_tooth/missing_tooth.h"
#include "dual_wheel/dual_wheel.h"
#include "renix/renix.h"
#include "Nissan360/Nissan360.h"
#include "FordST170/FordST170.h"
#include "NGC/test_ngc.h"
#include "ThirtySixMinus21/ThirtySixMinus21.h"
#include "SuzukiK6A/SuzukiK6A.h"

extern void testDecoder_General(void);

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
#if !defined(SIMULATOR)
    delay(2000);
#endif

    UNITY_BEGIN();    // IMPORTANT LINE!

    testMissingTooth();
    testDualWheel();
    testRenix();
    testNissan360();
    testFordST170();
    testNGC();
    testThirtySixMinus21();
    testSuzukiK6A_setEndTeeth();
    testSuzukiK6A_getCrankAngle();
    testDecoder_General();

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