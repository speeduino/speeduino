//
// Created by Ognjen Galić on 24. 11. 2025..
//
#include "EEPROM_local.h"

#include <iostream>

void eeprom_init() {

    if (initialized) {
        return;
    }

    std::string path = std::string(getenv("HOME")) + "/eeprom.bin";
    std::ifstream f(path, std::ios::binary);

    if (f.good()) {
        f.read((char*)MEMORY_EEPROM, EEPROM_SIZE);
        f.close();
        eeprom.open(path, std::ios::in | std::ios::out | std::ios::binary);
    } else {
        f.close();
        std::ofstream create(path, std::ios::binary | std::ios::trunc);
        create.seekp(0);
        create.write((char*)MEMORY_EEPROM, EEPROM_SIZE);
        eeprom.open(path, std::ios::in | std::ios::out | std::ios::binary);
    }

    initialized = true;
}

uint8_t eeprom_read_byte(uint16_t* addr) {
    eeprom_init();
    uint64_t real_addr = (uint64_t) addr;
    return MEMORY_EEPROM[real_addr];
}

void eeprom_write_byte(uint16_t* addr, uint8_t val) {
    eeprom_init();
    uint64_t real_addr = (uint64_t) addr;
    MEMORY_EEPROM[real_addr] = val;
    if (eeprom.good()) {
        eeprom.seekp(0);
        eeprom.write((char*) MEMORY_EEPROM, EEPROM_SIZE);
        eeprom.flush();
    }
}