#include "storage_api.h"
#include "preprocessor.h"

#if defined(CORE_AVR)
#pragma GCC push_options
// This minimizes RAM usage at no performance cost
#pragma GCC optimize ("Os") 
#endif

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

 __attribute__((noinline)) void moveBlock(const storage_api_t &api, uint16_t dest, uint16_t source, uint16_t size) {
  // Implementation is modelled after memmove.
  if (source<dest) {
    // Source is before dest - in other words we are moving the block *up* the address space
    // Must copy in reverse to handle overlapping blocks.
    dest += size;
    source += size;
    while(size!=0U) {
      --dest;
      --source;
      (void)update(api, dest, api.read(source));
      --size;
    }
  } else {
    // Source is after dest - in other words we are moving the block *down* the address space
    // Must use in-order copy to handle overlapping blocks.
    while(size!=0U) {
      (void)update(api, dest, api.read(source));
      ++dest;
      ++source;
      --size;
    }
  }
}

static byte default_read(uint16_t address) {
  UNUSED(address);
  return 0U;
}
static void default_write(uint16_t address, byte value) {
  UNUSED(address);
  UNUSED(value);
}
static uint16_t default_length(void) {
  return 0U;
}
static storage_api_t externalApi = {
    .read = default_read,
    .write = default_write,
    .length = default_length,
    .clear = nullptr,
  };

void setStorageAPI(const storage_api_t &api) {
  externalApi = api;
}

/** @brief Provide global access to the raw storage API */
const storage_api_t& getStorageAPI(void) {
  return externalApi;
}


#if defined(CORE_AVR)
#pragma GCC pop_options
#endif
