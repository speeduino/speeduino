//
// Created by Ognjen Galić on 23. 11. 2025..
//

#include <SPI.h>

SPIClass SPI;

void SPIClass::begin() {
}

void SPIClass::usingInterrupt(uint8_t interruptNumber) {
}

void SPIClass::notUsingInterrupt(uint8_t interruptNumber) {
}

void SPIClass::beginTransaction(SPISettings settings) {
}

uint8_t SPIClass::transfer(uint8_t data) {
    return 0;
}

uint16_t SPIClass::transfer16(uint16_t data) {
    return 0;
}

void SPIClass::transfer(void *buf, size_t count) {
}

void SPIClass::endTransaction() {
}

void SPIClass::end() {
}

void SPIClass::setBitOrder(uint8_t bitOrder) {
}

void SPIClass::setDataMode(uint8_t dataMode) {
}

void SPIClass::setClockDivider(uint8_t clockDiv) {
}

void SPIClass::attachInterrupt() {
}

void SPIClass::detachInterrupt() {
}
