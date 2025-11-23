/*
  EEPROM.h - EEPROM library
  Original Copyright (c) 2006 David A. Mellis.  All right reserved.
  New version by Christopher Andrews 2015.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef EEPROM_h
#define EEPROM_h

#include <stdint.h>

#define EERef uint8_t
#define EEPtr uint8_t

class EEPROMClass{
public:
    //Basic user access methods.
    EERef operator[](int idx ) {
        printf("EEPROM::[%d]\n", idx);
    }

    static uint8_t read( int idx ) {
        printf("EEPROM::read(%d)\n", idx);
        return 0;
    }
    static void write( int idx, uint8_t val ) {
        printf("EEPROM::write(%d, %d)\n", idx, val);
    }

    static void update( int idx, uint8_t val ) {
        printf("EEPROM::update(%d, %d)\n", idx, val);
    }
    
    static EEPtr begin() {
        printf("EEPROM::begin()\n");
        return 0;
    }

    static EEPtr end() {
        printf("EEPROM::end()\n");
        return 0;
    }

    static uint16_t length() {
        printf("EEPROM::length()\n");
        return 0;
    }

    template<typename T>
    void put(uint16_t uint16, T const&) {
        printf("EEPROM::put(%d, ?)\n", uint16);
    }

    template<typename T>
    uint32_t get(uint16_t uint16, T const&) {
        printf("EEPROM::get(%d, ?)\n", uint16);
        return 0;
    }
};

static EEPROMClass EEPROM;
#endif
