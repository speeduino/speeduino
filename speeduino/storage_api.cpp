#include "storage_api.h"

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
