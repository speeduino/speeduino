//
// Created by Ognjen Galić on 24. 11. 2025..
//

#ifndef FIRMWARE_EEPROM_MEMORY_H
#define FIRMWARE_EEPROM_MEMORY_H

#include <stdint.h>
#include <fstream>

#define E2END 0x0FFF          // 4095
#define EEPROM_SIZE (E2END+1) // 4096

static uint8_t MEMORY_EEPROM[EEPROM_SIZE];
static bool initialized = false;
static  std::fstream eeprom;

void eeprom_init();
uint8_t eeprom_read_byte(uint16_t* addr);
void eeprom_write_byte(uint16_t* addr, uint8_t val);

#endif //FIRMWARE_EEPROM_MEMORY_H