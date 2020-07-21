//Backup sram stores data in the battery backuped sram portion. 
//The backup battery is available on the ebay stm32F407VET6 black boards.  
#if defined(ARDUINO_ARCH_STM32)


#include <stdint.h>
#include "stm32f407xx.h"

class BackupSramAsEEPROM {
  private: 
    const uint16_t backup_size = 4096; //maximum of 4kb backuped sram available.
    int8_t write_byte( uint8_t *data, uint16_t bytes, uint16_t offset ); 
    int8_t read_byte( uint8_t *data, uint16_t bytes, uint16_t offset ); 
   
  public:
    BackupSramAsEEPROM();
    uint8_t read(uint16_t address);  
    int8_t write(uint16_t address, uint8_t val);
    int8_t update(uint16_t address, uint8_t val);
};

extern BackupSramAsEEPROM EEPROM;

#endif
