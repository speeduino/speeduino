//
// Created by Ognjen Galić on 23. 11. 2025..
//

#include <stdio.h>
#include <SPI.h>

SPIClass SPI;

void SPIClass::begin() {
    log(SPIDBG, "SPIClass::begin()\n");
}

void SPIClass::usingInterrupt(uint8_t interruptNumber) {
    log(SPIDBG, "SPIClass::usingInterrupt(%d)\n", interruptNumber);
}

void SPIClass::notUsingInterrupt(uint8_t interruptNumber) {
    log(SPIDBG, "SPIClass::notUsingInterrupt(%d)\n", interruptNumber);
}

void SPIClass::beginTransaction(SPISettings settings) {
    log(SPIDBG, "SPIClass::beginTransaction()\n");
}

uint8_t SPIClass::transfer(uint8_t data) {
    log(SPIDBG, "SPIClass::transfer(%d)\n", data);
    return 0;
}

uint16_t SPIClass::transfer16(uint16_t data) {
    log(SPIDBG, "SPIClass::transfer16(%d)\n", data);
    return 0;
}

void SPIClass::transfer(void *buf, size_t count) {
    log(SPIDBG, "SPIClass::transfer(%ld)\n", count);
}

void SPIClass::endTransaction() {
    log(SPIDBG, "SPIClass::endTransaction()\n");
}

void SPIClass::end() {
    log(SPIDBG, "SPIClass::end()\n");
}

void SPIClass::setBitOrder(uint8_t bitOrder) {
    log(SPIDBG, "SPIClass::setBitOrder(%d)\n", bitOrder);
}

void SPIClass::setDataMode(uint8_t dataMode) {
    log(SPIDBG, "SPIClass::setDataMode(%d)\n", dataMode);
}

void SPIClass::setClockDivider(uint8_t clockDiv) {
    log(SPIDBG, "SPIClass::setClockDivider(%d)\n", clockDiv);
}

void SPIClass::attachInterrupt() {
    log(SPIDBG, "SPIClass::attachInterrupt()\n");
}

void SPIClass::detachInterrupt() {
    log(SPIDBG, "SPIClass::detachInterrupt()\n");
}
