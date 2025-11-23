//
// Created by Ognjen Galić on 23. 11. 2025..
//

#ifndef FIRMWARE_PLATFORM_X86_H
#define FIRMWARE_PLATFORM_X86_H

/**
 * This houses the dumb boilerplate that is needed to compile
 * the firmware successfully.
 */
#define ARDUINO 300
#define PROGMEM
#define pgm_read_byte(arg) ((uint16_t) 123)
#define pgm_read_word(arg) ((uint16_t) 123)
#define pgm_read_dword(arg) ((uint16_t) 123)

#endif //FIRMWARE_PLATFORM_X86_H