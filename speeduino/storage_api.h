#pragma once
/**
 * @file
 * @brief Defines the required external storage API plus some convenience functions built around that API.
 */

#include <stdint.h>
#include "statuses.h"

using byte = uint8_t;

/** @brief The external storage API. This must be supported by any storage system. E.g. EEPROM, SPI 
 * 
 * This abstracts our requirements for low level storage I/O from the underlying storage hardware.
 *
 * Modelled as a byte addressable address space 
*/
struct storage_api_t {
    /** @brief Function to read a single byte from storage */
    byte (*read)(uint16_t address);

    /** @brief Function to write a single byte to storage */
    void (*write)(uint16_t address, byte val);

    /** @brief Function to get the size of the address space
     * 
     * We assume the entire address space from 0 to this size is available
     * for use. 
     */
    uint16_t (*length)(void);

    /** @brief The maximum number of write operations that will be performed in one go. */
    uint16_t (*getMaxWriteBlockSize)(const statuses &current);
};

/** 
 * @brief Create a storage_api_t instance that wraps getEEPROM()
 * 
 * Applies the adapter pattern to provide storage_api_t from EEPROM_t 
 */
storage_api_t getEEPROMStorageApi(void);

/**
 * @brief Conditionally write a byte to storage if it differs from the one already saved.
 * 
 * Background: some storage hardware has a limited number of write cycles per cell.
 * E.g. flash memory can only withstand a certain number of write/erase cycles before it begins
 * to wear out and can no longer reliably store data. This is typically in the range of
 * 3000-5000 cycles for multi-level cell (MLC) NAND flash, or 10,000-100,000 cycles for 
 * single-level cell (SLC) NAND flash.
 * So we want to limit writes as much as possible. Hence this function.
 * 
 * @param api Raw storage API
 * @param address the location to write to (zero based)
 * @param value the value to write
 * @return true if the value was written, false otherwise
 */
bool update(const storage_api_t &api, uint16_t address, byte value);

/**
 * @brief Conditionally write bytes from block of memory to storage. Values are written only if they differ from the one already saved at the same address
 * 
 * @warning The code assumes <c>address+(pLast-pFirst) < api.length()</c>. I.e. that the block fits into the address space.
 * @see update
 * 
 * @param api Raw storage API
 * @param address the location to begin writing to (zero based)
 * @param pFirst Points to the start of the memory block
 * @param pLast Points to one byte past the end of the block 
 */
void updateBlock(const storage_api_t &api, uint16_t address, const byte* pFirst, const byte* pLast);

/**
 * @brief Conditionally write bytes from block of memory to storage, with a limited number of write operations.
 * 
 * Write operations are typically pretty slow (E.g. 3.3ms on AVR EEPROM). If a block contains many bytes that
 * need to be written, we can cause the system to stutter as the main loop stalls waiting for writes to complete.
 * So instead we limit the number of write operations per call to this function.
 * 
 * @note This means that a call to this function may not update the entire block. If the return value is zero, we 
 * ran out of writes, and repeated calls will be required.
 * 
 * @see updateBlock
 * 
 * @param api Raw storage API
 * @param address the location to begin writing to (zero based)
 * @param pFirst Points to the start of the memory block
 * @param pLast Points to one byte past the end of the block 
 * @param maxWrites Maximum number of write operations.
 * @return Number of writes *remaining*. E.g. if maxWrites==7 and 2 bytes are written, result is 5 
 */
uint16_t updateBlockLimitWriteOps(const storage_api_t &api, uint16_t address, const byte* pFirst, const byte* pLast, uint16_t maxWrites);

/**
 * @brief Conditionally write bytes from a POD object to storage. Values are written only if they differ from the one already saved at the same address
 *
 * This is purely a convenience function
 *  
 * @tparam T - must be a POD type 
 * @param api Raw storage API
 * @param t Object to write
 * @param address the location to begin writing to (zero based)
 */
template< typename T > 
static inline void updateObject(const storage_api_t &api, const T &t, uint16_t address) {
    updateBlock(api, address, (const byte*)&t, ((const byte*)&t)+sizeof(T));
}

/**
 * @brief Copy a block of data from storage to memory
 * 
 * This is essentially the opposite of updateBlock()
 * 
 * @param api Raw storage API
 * @param address the location to begin reading from (zero based)
 * @param pFirst Points to the start of the memory block
 * @param pLast Points to one byte past the end of the block 
 * @return uint16_t The storage address of pLast. Convenient for chaining calls.
 */
uint16_t loadBlock(const storage_api_t &api, int16_t address, byte *pFirst, const byte *pLast);

/**
 * @brief Copy a block of data from storage into a POD object
 * 
 * @tparam T - must be a POD type 
 * @param api Raw storage API
 * @param address the location to begin reading from (zero based)
 * @param t Object to write to
 * @return T& Returns t 
 */
template< typename T > 
static inline T &loadObject(const storage_api_t &api, uint16_t address, T &t ){
  byte *pFirst = (byte*) &t;
  const byte *pLast = pFirst + sizeof(T);
  (void)loadBlock(api, address, pFirst, pLast);
  return t;
}

/**
 * @brief Fills the given block with a constant
 * 
 * @param api Raw storage API
 * @param address the location to begin writing to (zero based)
 * @param length number of *bytes* to write
 * @param value fill value
 */
void fillBlock(const storage_api_t &api, uint16_t address, uint16_t length, byte value);

/**
 * @brief Move a block of bytes.
 * 
 * This will handle overlapping blocks. 
 * I.e. if [dest, size) overlaps [source, size)
 * 
 * @param api Raw storage API
 * @param dest Address to copy *to*
 * @param source Address to copy *from*
 * @param size number of bytes to copy 
 */
void moveBlock(const storage_api_t &api, uint16_t dest, uint16_t source, uint16_t size);