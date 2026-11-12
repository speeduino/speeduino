#pragma once
#include <stdint.h>

/** @brief Decode an auxiliary value only when all its bytes were received.
 * @param data Classic CAN payload storage.
 * @param length Number of received data bytes.
 * @param start First byte of the configured value.
 * @param twoBytes Whether the configured value is two bytes wide.
 * @param littleEndian Whether a two-byte value is little-endian.
 * @param value Updated on success; unchanged on invalid input.
 * @return Whether a complete value was available.
 */
static inline bool readCanInputValue(const uint8_t (&data)[8], uint8_t length,
                                    uint8_t start, bool twoBytes, bool littleEndian,
                                    uint16_t &value)
{
    if ((length > sizeof(data)) || (start >= length) ||
        (twoBytes && (static_cast<uint16_t>(start) + 1U >= length)))
    {
        return false;
    }
    uint16_t decoded = data[start];
    if (twoBytes)
    {
        const uint16_t next = data[start + 1U];
        decoded = littleEndian ? decoded | (next << 8U) : (decoded << 8U) | next;
    }
    value = decoded;
    return true;
}
