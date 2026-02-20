#include "storage_api.h"
#include "board_definition.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#include EEPROM_LIB_H //This is defined in the board .h files
#pragma GCC diagnostic pop

namespace EEPROMApi {

  static inline byte read(uint16_t address) {
    return getEEPROM().read(address);
  }
  static inline void write(uint16_t address, byte val) {
    getEEPROM().write(address, val);
  }
  static inline uint16_t length(void) {
    return getEEPROM().length();
  }
  
}

storage_api_t getEEPROMStorageApi(void)
{
  return storage_api_t {
    .read = EEPROMApi::read,
    .write = EEPROMApi::write,
    .length = EEPROMApi::length,
  };  
}

bool update(const storage_api_t &api, uint16_t address, byte value) {
  if (api.read(address)!=value) {
    api.write(address, value); 
    return true;
  }
  return false;    
}

__attribute__((noinline)) void updateBlock(const storage_api_t &api, uint16_t address, const byte* pFirst, const byte* pLast) {
  for (; pFirst != pLast; ++address, (void)++pFirst) {
    (void)update(api, address, *pFirst);
  }
}

__attribute__((noinline)) uint16_t updateBlockLimitWriteOps(const storage_api_t &api, uint16_t address, const byte* pFirst, const byte* pLast, uint16_t maxWrites) {
  while (pFirst!=pLast && maxWrites>0U) {
    if (update(api, address, *pFirst)) {
      --maxWrites;
    }
    ++address;
    ++pFirst;
  }

  return maxWrites;
}

__attribute__((noinline)) uint16_t loadBlock(const storage_api_t &api, int16_t address, byte *pFirst, const byte *pLast)
{
  for (; pFirst != pLast; ++address, (void)++pFirst) {
    *pFirst = api.read(address);
  }
  return address;
}

__attribute__((noinline)) void fillBlock(const storage_api_t &api, uint16_t address, uint16_t length, byte value) {
  for (uint16_t end=address+length; address<end; ++address) {
    (void)update(api, address, value);
  }
}