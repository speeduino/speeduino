#include <FastCRC.h>
#include "page_crc.h"
#include "pages.h"

uint32_t __attribute__((optimize("Os"))) calculatePageCRC32(byte pageNum)
{
    FastCRC32 crcCalc;
    
    byte buffer = getPageValue(pageNum, 0);
    uint32_t crc = crcCalc.crc32(&buffer, 1U);

    for (uint16_t offset=1; offset<getPageSize(pageNum); ++offset)
    {
        buffer = getPageValue(pageNum, offset);
        crc = crcCalc.crc32_upd(&buffer, 1U);
    }

    return crc;
}