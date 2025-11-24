//
// Created by Ognjen Galić on 23. 11. 2025..
//

#ifndef FIRMWARE_PLATFORM_X86_H
#define FIRMWARE_PLATFORM_X86_H

#include <stdint.h>
#include "log.h"

#include <chrono>

class X86Port {

    uint64_t opentime;
    uint8_t portId;
    uint64_t v = 0;

    static uint64_t micros() {
        return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    }

public:

    X86Port(uint8_t id, uint64_t initialValue) {
        this->portId = id;
        this->v = initialValue;
    }

    uint64_t operator&(const uint64_t& o) const {
        log(PORT, "PIN%d %lld & %lld\n", portId, v, o);
        return v & o;
    }

    uint64_t operator|(const uint64_t& o) const {
        log(PORT, "PIN %d %lld | %lld\n", portId, v, o);
        return v | o;
    }

    uint64_t operator&=(const uint64_t& o) {
        if (opentime != 0) {
            log(PORT, "PIN %d duration %lld uS\n", portId, micros() - opentime);
            opentime = 0;
        }
        log(PORT, "PIN %lld LOW\n", v);
        v &= o;
        return v;
    }

    uint64_t& operator|=(const uint64_t& o) {
        log(PORT, "PIN %lld HIGH\n", v);
        opentime = micros();
        v |= o;
        return v;
    }

    uint64_t operator~() const {
        log(PORT, "PIN %d %lld ~ \n", portId, v);
        return ~v;
    }

    uint64_t& operator=(const uint64_t& o) {
        log(PORT, "PIN %d %lld = \n", portId, o);
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