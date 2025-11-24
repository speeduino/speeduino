//
// Created by Ognjen Galić on 23. 11. 2025..
//

#include <Arduino.h>
#include <cstdio>
#include <chrono>
#include <thread>

void initVariant(void) {
    printf("initiVariant\n");
}

int atexit(void (*func)()) __attribute__((weak)) {
    return 0;
}

void pinMode(uint8_t pin, uint8_t mode) {
    printf("pinMode: %d, %d\n", pin, mode);
}

void digitalWrite(uint8_t pin, uint8_t val) {
    printf("digitalWrite: %d, %d\n", pin, val);
}

int digitalRead(uint8_t pin) {
    printf("digitalRead: %d\n", pin);
    return 1;
}

int analogRead(uint8_t pin) {
    printf("analogRead: %d\n", pin);
    return 1;
}
void analogReference(uint8_t mode) {
    printf("analogReference: %d\n", mode);
}
void analogWrite(uint8_t pin, int val) {
    printf("analogWrite: %d, %d\n", pin, val);
}

uint32_t millis(void) {
    std::chrono::milliseconds ms = duration_cast< std::chrono::milliseconds >(
    std::chrono::system_clock::now().time_since_epoch());
    printf("millis\n");
    return ms.count();
}
unsigned long micros(void) {
    printf("micros()\n");
    return millis() * 1000UL;
}
void delay(unsigned long ms) {
    printf("delay()\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
void delayMicroseconds(unsigned int us) {
    printf("delayMicroseconds()\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(us / 1000));
}
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout) {
    printf("pulseIn: %d, %d, %ld\n", pin, state, timeout);
    return 0;
}

unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout) {
    printf("pulseInLong: %d, %d, %ld\n", pin, state, timeout);
    return 0;
}

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val) {
    printf("shiftOut: %d, %d, %d, %d\n", dataPin, clockPin, bitOrder, val);
}
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) {
    printf("shiftIn: %d, %d, %d\n", dataPin, clockPin, bitOrder);
    return 0;
}

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode) {
    printf("attachInterrupt: %d, func, %d\n", interruptNum, mode);
}

void detachInterrupt(uint8_t interruptNum) {
    printf("detachInterrupt: %d\n", interruptNum);
}

void tone(uint8_t _pin, unsigned int frequency, unsigned long duration) {
    printf("tone: %d, %d, %ld", _pin, frequency, duration);
}

void noTone(uint8_t _pin) {
    printf("noTone: %d\n", _pin);
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
