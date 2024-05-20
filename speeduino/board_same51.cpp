#if defined(CORE_SAME51)
#include "globals.h"
#include "auxiliaries.h"
#include "storage_api.h"
#include "storage.h"
#ifdef USE_SPI_EEPROM
  #include "src/SPIAsEEPROM/SPIAsEEPROM.h"
  SPI_EEPROM_Class EEPROM({255UL, 4096UL, 31, 0x00100000UL}, //windbond W25Q16 SPI flash EEPROM emulation
                          SPIconfig);
#else
  #include "src/FlashStorage/FlashAsEEPROM.h"
#if !defined(STORAGE_API_CUSTOM_CLEAR)
#error Flash requires custom clear
#endif
#endif

namespace EEPROMApi {
  static inline byte read(uint16_t address) {
    return EEPROM.read(address);
  }
  static inline void write(uint16_t address, byte val) {
    EEPROM.write(address, val);
  }
  static inline uint16_t length(void) {
    return EEPROM.length();
  }
#if defined(STORAGE_API_CUSTOM_CLEAR)
  static inline void clear(void) {    
    EEPROM.read(0); //needed for SPI eeprom emulation.
    EEPROM.clear(); 
  }
#endif
}

void initialiseStorage(void) {
  setStorageAPI(storage_api_t {
    .read = EEPROMApi::read,
    .write = EEPROMApi::write,
    .length = EEPROMApi::length,
#if defined(STORAGE_API_CUSTOM_CLEAR)
    .clear = EEPROMApi::clear,
#else
    .clear = nullptr,
#endif    
  });
}

void initBoard()
{
    /*
    ***********************************************************************************************************
    * General
    */

    /*
    ***********************************************************************************************************
    * Timers
    */

    /*
    ***********************************************************************************************************
    * Auxiliaries
    */

    /*
    ***********************************************************************************************************
    * Idle
    */

    /*
    ***********************************************************************************************************
    * Schedules
    */
}

uint16_t freeRam()
{
  return 0;
}

void doSystemReset() { return; }
void jumpToBootloader() { return; }

#endif
