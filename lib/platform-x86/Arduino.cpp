//
// Created by Ognjen Galić on 23. 11. 2025..
//

#include <Arduino.h>
#include <cstdio>

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
    printf("millis\n");
    return 0;
}
unsigned long micros(void) {
    printf("micros\n");
    return 0;
}
void delay(unsigned long ms) {
    printf("delay\n");
}
void delayMicroseconds(unsigned int us) {
    printf("delayMicroseconds\n");
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
}

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode) {
    printf("attachInterrupt: %d, func, %d\n", interruptNum, mode);
}

void detachInterrupt(uint8_t interruptNum) {
    printf("detachInterrupt: %d\n", interruptNum);
}

uint16_t makeWord(uint16_t w) {
    printf("makeWord: %d\n", w);
    return 0;
}

uint16_t makeWord(byte h, byte l) {
    printf("makeWord: %d, %d\n", h, l);
    return 0;
}

void tone(uint8_t _pin, unsigned int frequency, unsigned long duration) {
    printf("tone: %d, %d, %ld", _pin, frequency, duration);
}

void noTone(uint8_t _pin) {
    printf("noTone: %d\n", _pin);
}

// WMath prototypes
long random(long) {
    printf("random\n");
    return 0;
}
long random(long, long) {
    printf("random\n");
    return 0;
}
void randomSeed(unsigned long) {
    printf("random\n");
}
long map(long, long, long, long, long) {
    printf("random\n");
    return 0;
}