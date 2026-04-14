/**
 * @file
 * @brief Used internally by the board to create storage_api_t instances. 
 * 
 * This is not intended for use by user code. 
 * 
 * It's only purpose is to reduce code duplication between the different board files, as the 
 * implementation of the storage API is mostly the same across boards.
 * 
 * It is assumed that a global instance named `EEPROM` is available prior to including this file.
 */
#pragma once
#include "storage_api.h"

namespace EEPROMApi {

  static inline byte read(uint16_t address)
  {
    return EEPROM.read(address);
  }
  static inline void write(uint16_t address, byte val)
  {
    (void)EEPROM.write(address, val);
  }
  static inline uint16_t length(void)
  {
    return EEPROM.length();
  }
}

/** @brief Get the EEPROM storage API for the board */
storage_api_t getEEPROMStorageApi(uint16_t (*getMaxWriteBlockSize)(const statuses &))
{
  return {
    .read = EEPROMApi::read,
    .write = EEPROMApi::write,
    .length = EEPROMApi::length,
    .getMaxWriteBlockSize = getMaxWriteBlockSize,
  };
}
