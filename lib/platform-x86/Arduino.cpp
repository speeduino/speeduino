//
// Created by Ognjen Galić on 23. 11. 2025..
//

#include <Arduino.h>
#include "log.h"
#include <cstdio>
#include <chrono>
#include <thread>

void (*trigger_isr)();

void initVariant(void) {
    log(ARDUINO_CORE, "initiVariant\n");
}

int atexit(void (*func)()) __attribute__((weak)) {
    return 0;
}

void pinMode(uint8_t pin, uint8_t mode) {
    log(ARDUINO_CORE, "pinMode: %d, %d\n", pin, mode);
}

void digitalWrite(uint8_t pin, uint8_t val) {
    log(ARDUINO_CORE, "digitalWrite: %d, %d\n", pin, val);
}

int digitalRead(uint8_t pin) {
    log(ARDUINO_CORE, "digitalRead: %d\n", pin);
    return 1;
}

int analogRead(uint8_t pin) {
    log(ARDUINO_CORE, "analogRead: %d\n", pin);
    return 255;
}
void analogReference(uint8_t mode) {
    log(ARDUINO_CORE, "analogReference: %d\n", mode);
}
void analogWrite(uint8_t pin, int val) {
    log(ARDUINO_CORE, "analogWrite: %d, %d\n", pin, val);
}

uint32_t millis(void) {
    std::chrono::milliseconds ms = duration_cast< std::chrono::milliseconds >(
    std::chrono::system_clock::now().time_since_epoch());
    log(ARDUINO_CORE, "millis\n");
    return ms.count();
}
unsigned long micros(void) {
    log(ARDUINO_CORE, "micros()\n");
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}
void delay(unsigned long ms) {
    log(ARDUINO_CORE, "delay()\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
void delayMicroseconds(unsigned int us) {
    log(ARDUINO_CORE, "delayMicroseconds()\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(us / 1000));
}
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout) {
    log(ARDUINO_CORE, "pulseIn: %d, %d, %ld\n", pin, state, timeout);
    return 0;
}

unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout) {
    log(ARDUINO_CORE, "pulseInLong: %d, %d, %ld\n", pin, state, timeout);
    return 0;
}

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val) {
    log(ARDUINO_CORE, "shiftOut: %d, %d, %d, %d\n", dataPin, clockPin, bitOrder, val);
}
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) {
    log(ARDUINO_CORE, "shiftIn: %d, %d, %d\n", dataPin, clockPin, bitOrder);
    return 0;
}

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode) {
    log(ARDUINO_CORE, "attachInterrupt: %d, func, %d\n", interruptNum, mode);
    if (interruptNum == 19) {
        trigger_isr = userFunc;
    }
}

void detachInterrupt(uint8_t interruptNum) {
    log(ARDUINO_CORE, "detachInterrupt: %d\n", interruptNum);
}

void tone(uint8_t _pin, unsigned int frequency, unsigned long duration) {
    log(ARDUINO_CORE, "tone: %d, %d, %ld", _pin, frequency, duration);
}

void noTone(uint8_t _pin) {
    log(ARDUINO_CORE, "noTone: %d\n", _pin);
}

// WMath stuff

void randomSeed(unsigned long seed)
{
    if (seed != 0) {
        srandom(seed);
    }
}

long random(long howbig)
{
    if (howbig == 0) {
        return 0;
    }
    return random() % howbig;
}

long random(long howsmall, long howbig)
{
    if (howsmall >= howbig) {
        return howsmall;
    }
    long diff = howbig - howsmall;
    return random(diff) + howsmall;
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    long div = (in_max - in_min) + out_min;
    if (div == 0) {
        return x; // this can happen, *rare*, but it happens
    }
    return (x - in_min) * (out_max - out_min) / div;;
}

uint16_t makeWord(unsigned int w) {
    return w;
}
uint16_t makeWord(unsigned char h, unsigned char l) {
    return (h << 8) | l;
}
