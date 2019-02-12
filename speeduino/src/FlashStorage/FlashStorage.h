/*
  Copyright (c) 2015 Arduino LLC.  All right reserved.
  Written by Cristian Maglie

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include <Arduino.h>

// Concatenate after macro expansion
#define PPCAT_NX(A, B) A ## B
#define PPCAT(A, B) PPCAT_NX(A, B)

#define Flash(name, size) \
  __attribute__((__aligned__(256))) \
  static const uint8_t PPCAT(_data,name)[(size+255)/256*256] = { }; \
  FlashClass name(PPCAT(_data,name), size);

#define FlashStorage(name, T) \
  __attribute__((__aligned__(256))) \
  static const uint8_t PPCAT(_data,name)[(sizeof(T)+255)/256*256] = { }; \
  FlashStorageClass<T> name(PPCAT(_data,name));

class FlashClass {
public:
  FlashClass(const void *flash_addr = NULL, uint32_t size = 0);

  void write(const void *data) { write(flash_address, data, flash_size); }
  void erase()                 { erase(flash_address, flash_size);       }
  void read(void *data)        { read(flash_address, data, flash_size);  }

  void write(const volatile void *flash_ptr, const void *data, uint32_t size);
  void erase(const volatile void *flash_ptr, uint32_t size);
  void read(const volatile void *flash_ptr, void *data, uint32_t size);

private:
  void erase(const volatile void *flash_ptr);

  const uint32_t PAGE_SIZE, PAGES, MAX_FLASH, ROW_SIZE;
  const volatile void *flash_address;
  const uint32_t flash_size;
};

template<class T>
class FlashStorageClass {
public:
  FlashStorageClass(const void *flash_addr) : flash(flash_addr, sizeof(T)) { };

  // Write data into flash memory.
  // Compiler is able to optimize parameter copy.
  inline void write(T data)  { flash.erase(); flash.write(&data); }

  // Read data from flash into variable.
  inline void read(T *data)  { flash.read(data); }

  // Overloaded version of read.
  // Compiler is able to optimize copy-on-return.
  inline T read() { T data; read(&data); return data; }

private:
  FlashClass flash;
};

