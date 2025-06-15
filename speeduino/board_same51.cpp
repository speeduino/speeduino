#if defined(CORE_SAME51)
#include "globals.h"
#include "auxiliaries.h"
#include "storage_api.h"

#ifdef USE_SPI_EEPROM
  #include "src/SPIAsEEPROM/SPIAsEEPROM.h"
  SPI_EEPROM_Class EEPROM({255UL, 4096UL, 31, 0x00100000UL}, //windbond W25Q16 SPI flash EEPROM emulation
                          SPIconfig);

  static void eeprom_clear(void) {
    for (uint16_t address=0; address<EEPROM.length(); ++address) {
      EEPROM.update(address, UINT8_MAX);
    }   
  }
#else
  #include "src/FlashStorage/FlashAsEEPROM.h"
  
  static void eeprom_clear(void) {    
    EEPROM.read(0); //needed for SPI eeprom emulation.
    EEPROM.clear(); 
  }
#endif

static byte eeprom_read(uint16_t address) {
  return EEPROM.read(address);
}
static void eeprom_write(uint16_t address, byte val) {
  EEPROM.write(address, val);
}
static uint16_t eeprom_length(void) {
  return EEPROM.length();
}

void initialiseStorage(void) {
  setStorageAPI(storage_api_t {
    .read = eeprom_read,
    .write = eeprom_write,
    .length = eeprom_length,
    .clear = eeprom_clear,
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
