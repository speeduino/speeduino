#pragma once

#if !defined(NATIVE_BOARD)

#include <unity.h>
#include <Arduino.h>
#include <avr/sleep.h>

void setup(void (*runAllTests)(void))
{
    pinMode(LED_BUILTIN, OUTPUT);

    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
#if !defined(SIMULATOR)
    delay(2000);
#endif

    UNITY_BEGIN();    // IMPORTANT LINE!

    runAllTests();

    UNITY_END(); // stop unit testing

#if defined(SIMULATOR)       // Tell SimAVR we are done
    cli();
    sleep_enable();
    sleep_cpu();
#endif     
}

#define DEVICE_TEST(testRunner) \
void setup() \
{ \
    setup(testRunner); \
}

void loop()
{
    // Blink to indicate end of test
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
}
#else
#define DEVICE_TEST(testRunner)
#endif
