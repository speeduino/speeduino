//
// Created by Ognjen Galić on 23. 11. 2025..
//

#ifndef FIRMWARE_PLATFORM_X86_H
#define FIRMWARE_PLATFORM_X86_H

struct X86Port {
    uint64_t v;

    uint64_t operator&(const uint64_t& o) const {
        return v & o;
    }

    uint64_t operator|(const uint64_t& o) const {
        return v | o;
    }

    uint64_t operator&=(const uint64_t& o) {
        v &= o;
        return v;
    }

    uint64_t& operator|=(const uint64_t& o) {
        v |= o;
        return v;
    }

    uint64_t operator~() const {
        return ~v;
    }

    uint64_t& operator=(const uint64_t& o) {
        v = o;
        return v;
    }

};

#define ARDUINO 300
#define PROGMEM
#define pgm_read_byte(arg) ((uint16_t) 123)
#define pgm_read_word(arg) ((uint16_t) 123)
#define pgm_read_dword(arg) ((uint16_t) 123)

#endif //FIRMWARE_PLATFORM_X86_H