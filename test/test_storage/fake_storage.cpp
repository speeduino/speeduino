#include "fake_storage.h"

one_byte_eeprom_t oneByteEeprom;

static byte oneByteRead(uint16_t)
{
    ++oneByteEeprom.readCount;
    return oneByteEeprom.readValue;
}

void oneBytWrite(uint16_t, byte value)
{
    ++oneByteEeprom.writeCount;
    oneByteEeprom.lastWriteValue = value;
}

uint16_t oneByteLength(void)
{
    return oneByteEeprom._length;
}

uint16_t oneByteGetMaxWriteBlockSize(const statuses&)
{
    return oneByteEeprom._blockSize;
}

storage_api_t getOneByteStorageApi(uint16_t length, uint16_t blockSize, char readValue)
{
    oneByteEeprom.readValue = readValue;
    oneByteEeprom._length = length;
    oneByteEeprom._blockSize = blockSize;
    oneByteEeprom.readCount = 0U;
    oneByteEeprom.writeCount = 0U;
    return { .read = oneByteRead, .write = oneBytWrite, .length = oneByteLength, .getMaxWriteBlockSize = oneByteGetMaxWriteBlockSize };
}
