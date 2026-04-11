#pragma once

#include "storage_api.h"

struct one_byte_eeprom_t
{
    char readValue;
    char lastWriteValue;
    uint16_t _length;
    uint16_t _blockSize;
    uint16_t readCount;
    uint16_t writeCount;
};
extern one_byte_eeprom_t oneByteEeprom;
storage_api_t getOneByteStorageApi(uint16_t length, uint16_t blockSize, char readValue);