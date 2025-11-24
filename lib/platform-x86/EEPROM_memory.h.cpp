//
// Created by Ognjen Galić on 24. 11. 2025..
//
#include "EEPROM_memory.h"

uint8_t eeprom_read_byte(uint8_t* addr) {
    uint64_t real_addr = (uint64_t) addr;
    return MEMORY_EEPROM[real_addr];
}

void eeprom_write_byte(uint8_t* addr, uint8_t val) {
    uint64_t real_addr = (uint64_t) addr;
    MEMORY_EEPROM[real_addr] = val;
}