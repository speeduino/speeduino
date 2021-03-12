#if defined(__MK64FX512__) || defined(__MK66FX1M0__) 
#ifndef _NXP_SDHC_H
#define _NXP_SDHC_H

#define SDHC_STATUS_NOINIT              0x01    /* Drive not initialized */
#define SDHC_STATUS_NODISK              0x02    /* No medium in the drive */
#define SDHC_STATUS_PROTECT             0x04    /* Write protected */

uint8_t SDHC_CardInit(void);
uint8_t SDHC_CardGetType(void);
//uint8_t SDHC_CardGetStatus(void);
//uint32_t SDHC_CardGetBlockCnt(void);

int SDHC_CardReadBlock(void * buff, uint32_t sector);
int SDHC_CardWriteBlock(const void * buff, uint32_t sector);

#endif
#endif