//
// Created by Ognjen Galić on 23. 11. 2025..
//

#include <stdio.h>
#include <SPI.h>

SPIClass SPI;

void SPIClass::begin() {
    printf("SPIClass::begin()\n");
}

void SPIClass::usingInterrupt(uint8_t interruptNumber) {
    printf("SPIClass::usingInterrupt(%d)\n", interruptNumber);
}

void SPIClass::notUsingInterrupt(uint8_t interruptNumber) {
    printf("SPIClass::notUsingInterrupt(%d)\n", interruptNumber);
}

void SPIClass::beginTransaction(SPISettings settings) {
    printf("SPIClass::beginTransaction()\n");
}

uint8_t SPIClass::transfer(uint8_t data) {
    printf("SPIClass::transfer(%d)\n", data);
    return 0;
}

uint16_t SPIClass::transfer16(uint16_t data) {
    printf("SPIClass::transfer16(%d)\n", data);
    return 0;
}

void SPIClass::transfer(void *buf, size_t count) {
    printf("SPIClass::transfer(%ld)\n", count);
}

void SPIClass::endTransaction() {
    printf("SPIClass::endTransaction()\n");
}

void SPIClass::end() {
    printf("SPIClass::end()\n");
}

void SPIClass::setBitOrder(uint8_t bitOrder) {
    printf("SPIClass::setBitOrder(%d)\n", bitOrder);
}

void SPIClass::setDataMode(uint8_t dataMode) {
    printf("SPIClass::setDataMode(%d)\n", dataMode);
}

void SPIClass::setClockDivider(uint8_t clockDiv) {
    printf("SPIClass::setClockDivider(%d)\n", clockDiv);
}

void SPIClass::attachInterrupt() {
    printf("SPIClass::attachInterrupt()\n");
}

void SPIClass::detachInterrupt() {
    printf("SPIClass::detachInterrupt()\n");
}
