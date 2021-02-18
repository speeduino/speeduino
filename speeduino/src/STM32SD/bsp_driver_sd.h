// #ifndef __BSP_DRIVER_SD_H
// #define __BSP_DRIVER_SD_H

// /* Includes ------------------------------------------------------------------*/
// #include "stm32_def.h"

// /* Exported types ------------------------------------------------------------*/
// #define BSP_SD_CardInfo HAL_SD_CardInfoTypeDef

// /* Exported constants --------------------------------------------------------*/
// #define MSD_OK                   ((uint8_t)0x00)
// #define MSD_ERROR                ((uint8_t)0x01)
// #define MSD_ERROR_SD_NOT_PRESENT ((uint8_t)0x02)

// #define SD_TRANSFER_OK    ((uint8_t)0x00)
// #define SD_TRANSFER_BUSY  ((uint8_t)0x01)
// #define SD_TRANSFER_ERROR ((uint8_t)0x02)

// #define SD_PRESENT     ((uint8_t)0x01)
// #define SD_NOT_PRESENT ((uint8_t)0x00)

// #define SD_DATATIMEOUT (150U) /* ms */

// #define SDMMC_IRQ_PRIO  1
// #define SD_DMA_IRQ_PRIO 2

// /* Exported functions --------------------------------------------------------*/
// uint8_t BSP_SD_Init(void);
// uint8_t BSP_SD_DeInit(void);
// uint8_t BSP_SD_ReadBlocks(uint32_t* pData,
//                           uint32_t  ReadAddr,
//                           uint32_t  NumOfBlocks,
//                           uint32_t  Timeout);
// uint8_t BSP_SD_WriteBlocks(uint32_t* pData,
//                            uint32_t  WriteAddr,
//                            uint32_t  NumOfBlocks,
//                            uint32_t  Timeout);
// uint8_t
//         BSP_SD_ReadBlocks_DMA(uint32_t* pData, uint32_t ReadAddr, uint32_t NumOfBlocks);
// uint8_t BSP_SD_WriteBlocks_DMA(uint32_t* pData,
//                                uint32_t  WriteAddr,
//                                uint32_t  NumOfBlocks);
// uint8_t BSP_SD_Erase(uint32_t StartAddr, uint32_t EndAddr);
// uint8_t BSP_SD_GetCardState(void);
// void    BSP_SD_GetCardInfo(BSP_SD_CardInfo* CardInfo);
// uint8_t BSP_SD_IsDetected(void);

// __weak void BSP_SD_AbortCallback(void);
// __weak void BSP_SD_ErrorCallback(void);
// __weak void BSP_SD_WriteCpltCallback(void);
// __weak void BSP_SD_ReadCpltCallback(void);

// #ifdef __cplusplus
// extern "C" {
// #endif

// /* Includes ------------------------------------------------------------------*/
// #include "stm32_def.h"
// #if !defined(STM32_CORE_VERSION) || (STM32_CORE_VERSION  <= 0x01050000)
// #include "variant.h"
// #endif
// #if !defined(STM32_CORE_VERSION) || (STM32_CORE_VERSION  <= 0x01060100)
// #error "This library version required a STM32 core version > 1.6.1.\
// Please update the core or install previous libray version."
// #endif

// /*SD Card information structure */
// #ifndef STM32L1xx
// #define HAL_SD_CardInfoTypedef         HAL_SD_CardInfoTypeDef
// #define BSP_SD_CardInfo                HAL_SD_CardInfoTypeDef
// #define HAL_SD_WideBusOperation_Config HAL_SD_ConfigWideBusOperation
// #define HAL_SD_Get_CardInfo            HAL_SD_GetCardInfo
// #endif

// #define SD_CardInfo HAL_SD_CardInfoTypedef

// /*SD status structure definition */
// #define MSD_OK                   ((uint8_t)0x00)
// #define MSD_ERROR                ((uint8_t)0x01)
// #define MSD_ERROR_SD_NOT_PRESENT ((uint8_t)0x02)

// /* SD Exported Constants */
// #define SD_PRESENT               ((uint8_t)0x01)
// #define SD_NOT_PRESENT           ((uint8_t)0x00)
// #define SD_DETECT_NONE           NUM_DIGITAL_PINS
// #ifdef SDMMC_TRANSCEIVER_ENABLE
// #define SD_TRANSCEIVER_NONE      NUM_DIGITAL_PINS
// #endif

// /* Could be redefined in variant.h or using build_opt.h */
// #ifndef SD_DATATIMEOUT
// #define SD_DATATIMEOUT         100000000U
// #endif

// #ifdef SDMMC_TRANSCEIVER_ENABLE
// #ifndef SD_TRANSCEIVER_EN
// #define SD_TRANSCEIVER_EN      SD_TRANSCEIVER_NONE
// #endif

// #ifndef SD_TRANSCEIVER_SEL
// #define SD_TRANSCEIVER_SEL     SD_TRANSCEIVER_NONE
// #endif
// #endif

// /* SD Exported Functions */
// uint8_t BSP_SD_Init(void);
// uint8_t BSP_SD_DeInit(void);
// #ifdef SDMMC_TRANSCEIVER_ENABLE
// uint8_t BSP_SD_TransceiverPin(GPIO_TypeDef *enport, uint32_t enpin, GPIO_TypeDef *selport, uint32_t selpin);
// #endif
// uint8_t BSP_SD_DetectPin(GPIO_TypeDef *port, uint32_t pin);
// uint8_t BSP_SD_DetectITConfig(void (*callback)(void));
// #ifndef STM32L1xx
// uint8_t BSP_SD_ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout);
// uint8_t BSP_SD_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout);
// #else
// uint8_t BSP_SD_ReadBlocks(uint32_t *pData, uint64_t ReadAddr, uint32_t BlockSize, uint32_t NumOfBlocks);
// uint8_t BSP_SD_WriteBlocks(uint32_t *pData, uint64_t WriteAddr, uint32_t BlockSize, uint32_t NumOfBlocks);
// #endif
// uint8_t BSP_SD_Erase(uint64_t StartAddr, uint64_t EndAddr);
// #ifndef STM32L1xx
// uint8_t BSP_SD_GetCardState(void);
// #else /* STM32L1xx */
// HAL_SD_TransferStateTypedef BSP_SD_GetStatus(void);
// #endif
// void    BSP_SD_GetCardInfo(HAL_SD_CardInfoTypedef *CardInfo);
// uint8_t BSP_SD_IsDetected(void);

// /* These __weak function can be surcharged by application code in case the current settings (e.g. DMA stream)
//    need to be changed for specific needs */
// void    BSP_SD_MspInit(SD_HandleTypeDef *hsd, void *Params);
// void    BSP_SD_Detect_MspInit(SD_HandleTypeDef *hsd, void *Params);
// void    BSP_SD_MspDeInit(SD_HandleTypeDef *hsd, void *Params);
// #ifdef SDMMC_TRANSCEIVER_ENABLE
// void    BSP_SD_Transceiver_MspInit(SD_HandleTypeDef *hsd, void *Params);
// #endif

// #ifdef __cplusplus
// }
// #endif
// #endif /* __BSP_DRIVER_SD_H */
