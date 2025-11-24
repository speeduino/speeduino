//
// Created by Ognjen Galić on 23. 11. 2025..
//

#ifndef FIRMWARE_PLATFORM_X86_H
#define FIRMWARE_PLATFORM_X86_H

#include <stdint.h>
#include <stdio.h>
#include "log.h"

class X86Port {

    const char* portId;
    uint64_t v = 0;
public:

    X86Port(const char* id, uint64_t initialValue) {
        this->portId = id;
        this->v = initialValue;
    }

    uint64_t operator&(const uint64_t& o) const {
        log(PORT, "PORT %s %lld & %lld\n", portId, v, o);
        return v & o;
    }

    uint64_t operator|(const uint64_t& o) const {
        log(PORT, "PORT %s %lld | %lld\n", portId, v, o);
        return v | o;
    }

    uint64_t operator&=(const uint64_t& o) {
        log(PORT, "PORT %s %lld &= %lld\n", portId, v, o);
        v &= o;
        return v;
    }

    uint64_t& operator|=(const uint64_t& o) {
        log(PORT, "PORT %s %lld |= %lld\n", portId, v, o);
        v |= o;
        return v;
    }

    uint64_t operator~() const {
        log(PORT, "PORT %s %lld ~ \n", portId, v);
        return ~v;
    }

    uint64_t& operator=(const uint64_t& o) {
        log(PORT, "PORT %s %lld = \n", portId, o);
        v = o;
        return v;
    }

};

void fireInterrupts();

#define ARDUINO 300
#define PROGMEM
#define pgm_read_byte(addr)   (*(const uint8_t *)(addr))
#define pgm_read_word(addr)   (*(const uint16_t*)(addr))
#define pgm_read_dword(addr)  (*(const uint32_t*)(addr))
#define pgm_read_float(addr)  (*(const float*)(addr))
#define pgm_read_ptr(addr)    (*(const void **)(addr))

#endif //FIRMWARE_PLATFORM_X86_H