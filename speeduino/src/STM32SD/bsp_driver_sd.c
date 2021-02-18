// /**
//  *******************************************************************************
//  * STM32L4 Bootloader
//  *******************************************************************************
//  * @author Akos Pasztor
//  * @file   bsp_driver_sd.c
//  * @brief  SD BSP driver
//  *	       This file contains the implementation of the SD BSP driver used by
//  *         the FatFs module. The driver uses the HAL library of ST.
//  *
//  *******************************************************************************
//  * Copyright (c) 2020 Akos Pasztor.                     https://akospasztor.com
//  *******************************************************************************
//  */

// #include "bsp_driver_sd.h"

// /* Definition for BSP SD */
// #if defined(SDMMC1) || defined(SDMMC2)
// #ifndef SD_INSTANCE
// #define SD_INSTANCE              SDMMC1
// #endif

// #define SD_CLK_ENABLE            __HAL_RCC_SDMMC1_CLK_ENABLE
// #define SD_CLK_DISABLE           __HAL_RCC_SDMMC1_CLK_DISABLE
// #ifdef SDMMC2
// #define SD_CLK2_ENABLE            __HAL_RCC_SDMMC2_CLK_ENABLE
// #define SD_CLK2_DISABLE           __HAL_RCC_SDMMC2_CLK_DISABLE
// #endif

// #define SD_CLK_EDGE              SDMMC_CLOCK_EDGE_RISING
// #define SD_CLK_BYPASS            SDMMC_CLOCK_BYPASS_DISABLE
// #define SD_CLK_PWR_SAVE          SDMMC_CLOCK_POWER_SAVE_DISABLE
// #define SD_BUS_WIDE_1B           SDMMC_BUS_WIDE_1B
// #define SD_BUS_WIDE_4B           SDMMC_BUS_WIDE_4B
// #define SD_BUS_WIDE_8B           SDMMC_BUS_WIDE_8B
// #define SD_HW_FLOW_CTRL_ENABLE   SDMMC_HARDWARE_FLOW_CONTROL_ENABLE
// #define SD_HW_FLOW_CTRL_DISABLE  SDMMC_HARDWARE_FLOW_CONTROL_DISABLE

// #ifdef STM32H7xx
// #define SD_CLK_DIV               1
// #else
// #define SD_CLK_DIV               SDMMC_TRANSFER_CLK_DIV
// #endif

// #ifdef SDMMC_TRANSCEIVER_ENABLE
// #define SD_TRANSCEIVER_ENABLE    SDMMC_TRANSCEIVER_ENABLE
// #define SD_TRANSCEIVER_DISABLE   SDMMC_TRANSCEIVER_DISABLE
// #endif

// #elif defined(SDIO)
// #define SD_INSTANCE              SDIO
// #define SD_CLK_ENABLE            __HAL_RCC_SDIO_CLK_ENABLE
// #define SD_CLK_DISABLE           __HAL_RCC_SDIO_CLK_DISABLE
// #define SD_CLK_EDGE              SDIO_CLOCK_EDGE_RISING
// #define SD_CLK_BYPASS            SDIO_CLOCK_BYPASS_DISABLE
// #define SD_CLK_PWR_SAVE          SDIO_CLOCK_POWER_SAVE_DISABLE
// #define SD_BUS_WIDE_1B           SDIO_BUS_WIDE_1B
// #define SD_BUS_WIDE_4B           SDIO_BUS_WIDE_4B
// #define SD_BUS_WIDE_8B           SDIO_BUS_WIDE_8B
// #define SD_HW_FLOW_CTRL_ENABLE   SDIO_HARDWARE_FLOW_CONTROL_ENABLE
// #define SD_HW_FLOW_CTRL_DISABLE  SDIO_HARDWARE_FLOW_CONTROL_DISABLE
// #define SD_CLK_DIV               SDIO_TRANSFER_CLK_DIV
// #else
// #error "Unknown SD_INSTANCE"
// #endif

// #ifndef SD_HW_FLOW_CTRL
// #define SD_HW_FLOW_CTRL          SD_HW_FLOW_CTRL_DISABLE
// #endif

// #ifndef SD_BUS_WIDE
// #define SD_BUS_WIDE              SD_BUS_WIDE_4B
// #endif

// #if defined(SDMMC_TRANSCEIVER_ENABLE) && !defined(SD_TRANSCEIVER_MODE)
// #define SD_TRANSCEIVER_MODE      SD_TRANSCEIVER_DISABLE
// #endif

// /* BSP SD Private Variables */
// static SD_HandleTypeDef uSdHandle;
// static uint32_t SD_detect_ll_gpio_pin = LL_GPIO_PIN_ALL;
// static GPIO_TypeDef *SD_detect_gpio_port = GPIOA;
// #ifdef SDMMC_TRANSCEIVER_ENABLE
// static uint32_t SD_trans_en_ll_gpio_pin = LL_GPIO_PIN_ALL;
// static GPIO_TypeDef *SD_trans_en_gpio_port = GPIOA;
// static uint32_t SD_trans_sel_ll_gpio_pin = LL_GPIO_PIN_ALL;
// static GPIO_TypeDef *SD_trans_sel_gpio_port = GPIOA;
// #endif
// #ifndef STM32L1xx
// #define SD_OK                         HAL_OK
// #define SD_TRANSFER_OK                ((uint8_t)0x00)
// #define SD_TRANSFER_BUSY              ((uint8_t)0x01)
// #else /* STM32L1xx */
// static SD_CardInfo uSdCardInfo;
// #endif


// /* Private function prototypes -----------------------------------------------*/
// static void              BSP_SD_MspInit(void);
// static void              BSP_SD_MspDeInit(void);
// static HAL_StatusTypeDef SD_DMAConfigRx(SD_HandleTypeDef* hsd);
// static HAL_StatusTypeDef SD_DMAConfigTx(SD_HandleTypeDef* hsd);

// /**
//  * @brief  Initializes the SD card device.
//  * @retval SD status
//  */
// uint8_t BSP_SD_Init(void)
// {
//   uint8_t sd_state = MSD_OK;

//   /* uSD device interface configuration */
//   uSdHandle.Instance = SD_INSTANCE;

//   uSdHandle.Init.ClockEdge           = SD_CLK_EDGE;
// #if !defined(STM32L4xx) && !defined(STM32H7xx)
//   uSdHandle.Init.ClockBypass         = SD_CLK_BYPASS;
// #endif
//   uSdHandle.Init.ClockPowerSave      = SD_CLK_PWR_SAVE;
//   uSdHandle.Init.BusWide             = SD_BUS_WIDE_1B;
//   uSdHandle.Init.HardwareFlowControl = SD_HW_FLOW_CTRL;
//   uSdHandle.Init.ClockDiv            = SD_CLK_DIV;
// #ifdef SDMMC_TRANSCEIVER_ENABLE
//   uSdHandle.Init.Transceiver = SD_TRANSCEIVER_MODE;
//   if (SD_TRANSCEIVER_MODE == SD_TRANSCEIVER_ENABLE) {

//     BSP_SD_Transceiver_MspInit(&uSdHandle, NULL);
//   }
// #endif

//   if (SD_detect_ll_gpio_pin != LL_GPIO_PIN_ALL) {
//     /* Msp SD Detect pin initialization */
//     BSP_SD_Detect_MspInit(&uSdHandle, NULL);
//     if (BSP_SD_IsDetected() != SD_PRESENT) { /* Check if SD card is present */
//       return MSD_ERROR_SD_NOT_PRESENT;
//     }
//   }

//   /* Msp SD initialization */
//   BSP_SD_MspInit(&uSdHandle, NULL);

//   /* HAL SD initialization */
// #ifndef STM32L1xx
//   if (HAL_SD_Init(&uSdHandle) != SD_OK)
// #else /* STM32L1xx */
//   if (HAL_SD_Init(&uSdHandle, &uSdCardInfo) != SD_OK)
// #endif
//   {
//     sd_state = MSD_ERROR;
//   }

//   /* Configure SD Bus width */
//   if (sd_state == MSD_OK) {
//     /* Enable wide operation */
//     if (HAL_SD_WideBusOperation_Config(&uSdHandle, SD_BUS_WIDE) != SD_OK) {
//       sd_state = MSD_ERROR;
//     } else {
//       sd_state = MSD_OK;
//     }
//   }
//   return  sd_state;
// }

// /**
//  * @brief  De-Initializes the SD card device
//  * @retval None
//  */
// uint8_t BSP_SD_DeInit(void)
// {
//     /* HAL SD de-initialization */
//     HAL_SD_DeInit(&uSdHandle);

//     /* Msp SD de-initialization */
//     BSP_SD_MspDeInit();

//     /* SDMMC Reset */
//     __HAL_RCC_SDMMC1_FORCE_RESET();
//     __HAL_RCC_SDMMC1_RELEASE_RESET();

//     /* Misc */
//     uSdHandle.State               = HAL_SD_STATE_RESET;
//     uSdHandle.Context             = 0;
//     uSdHandle.ErrorCode           = 0;
//     uSdHandle.SdCard.CardType     = 0;
//     uSdHandle.SdCard.CardVersion  = 0;
//     uSdHandle.SdCard.Class        = 0;
//     uSdHandle.SdCard.RelCardAdd   = 0;
//     uSdHandle.SdCard.BlockNbr     = 0;
//     uSdHandle.SdCard.BlockSize    = 0;
//     uSdHandle.SdCard.LogBlockNbr  = 0;
//     uSdHandle.SdCard.LogBlockSize = 0;
//     uSdHandle.CSD[0]              = 0;
//     uSdHandle.CSD[1]              = 0;
//     uSdHandle.CSD[2]              = 0;
//     uSdHandle.CSD[3]              = 0;
//     uSdHandle.CID[0]              = 0;
//     uSdHandle.CID[1]              = 0;
//     uSdHandle.CID[2]              = 0;
//     uSdHandle.CID[3]              = 0;

//     return MSD_OK;
// }

// /**
//  * @brief  Reads block(s) from a specified address in an SD card, in polling
//  * mode.
//  * @param  pData: Pointer to the buffer that will contain the data to transmit
//  * @param  ReadAddr: Address from where data is to be read
//  * @param  NumOfBlocks: Number of SD blocks to read
//  * @param  Timeout: Timeout for read operation
//  * @retval SD status
//  */
// uint8_t BSP_SD_ReadBlocks(uint32_t* pData,
//                           uint32_t  ReadAddr,
//                           uint32_t  NumOfBlocks,
//                           uint32_t  Timeout)
// {
//     uint8_t sd_state = MSD_OK;

//     if(HAL_SD_ReadBlocks(&uSdHandle, (uint8_t*)pData, ReadAddr, NumOfBlocks,
//                          Timeout) != HAL_OK)
//     {
//         sd_state = MSD_ERROR;
//     }

//     return sd_state;
// }

// /**
//  * @brief  Writes block(s) to a specified address in an SD card, in polling
//  * mode.
//  * @param  pData: Pointer to the buffer that will contain the data to transmit
//  * @param  WriteAddr: Address from where data is to be written
//  * @param  NumOfBlocks: Number of SD blocks to write
//  * @param  Timeout: Timeout for write operation
//  * @retval SD status
//  */
// uint8_t BSP_SD_WriteBlocks(uint32_t* pData,
//                            uint32_t  WriteAddr,
//                            uint32_t  NumOfBlocks,
//                            uint32_t  Timeout)
// {
//     uint8_t sd_state = MSD_OK;

//     if(HAL_SD_WriteBlocks(&uSdHandle, (uint8_t*)pData, WriteAddr, NumOfBlocks,
//                           Timeout) != HAL_OK)
//     {
//         sd_state = MSD_ERROR;
//     }

//     return sd_state;
// }

// /**
//  * @brief  Reads block(s) from a specified address in an SD card, in DMA mode.
//  * @param  pData: Pointer to the buffer that will contain the data to transmit
//  * @param  ReadAddr: Address from where data is to be read
//  * @param  NumOfBlocks: Number of SD blocks to read
//  * @retval SD status
//  */
// uint8_t
// BSP_SD_ReadBlocks_DMA(uint32_t* pData, uint32_t ReadAddr, uint32_t NumOfBlocks)
// {
//     HAL_StatusTypeDef sd_state = HAL_OK;

//     /* Invalidate the dma tx handle*/
//     uSdHandle.hdmatx = NULL;

//     /* Prepare the dma channel for a read operation */
//     sd_state = SD_DMAConfigRx(&uSdHandle);

//     if(sd_state == HAL_OK)
//     {
//         /* Read block(s) in DMA transfer mode */
//         sd_state = HAL_SD_ReadBlocks_DMA(&uSdHandle, (uint8_t*)pData, ReadAddr,
//                                          NumOfBlocks);
//     }

//     return (sd_state == HAL_OK) ? MSD_OK : MSD_ERROR;
// }

// /**
//  * @brief  Writes block(s) to a specified address in an SD card, in DMA mode.
//  * @param  pData: Pointer to the buffer that will contain the data to transmit
//  * @param  WriteAddr: Address from where data is to be written
//  * @param  NumOfBlocks: Number of SD blocks to write
//  * @retval SD status
//  */
// uint8_t BSP_SD_WriteBlocks_DMA(uint32_t* pData,
//                                uint32_t  WriteAddr,
//                                uint32_t  NumOfBlocks)
// {
//     HAL_StatusTypeDef sd_state = HAL_OK;

//     /* Invalidate the dma rx handle*/
//     uSdHandle.hdmarx = NULL;

//     /* Prepare the dma channel for a read operation */
//     sd_state = SD_DMAConfigTx(&uSdHandle);

//     if(sd_state == HAL_OK)
//     {
//         /* Write block(s) in DMA transfer mode */
//         sd_state = HAL_SD_WriteBlocks_DMA(&uSdHandle, (uint8_t*)pData, WriteAddr,
//                                           NumOfBlocks);
//     }

//     return (sd_state == HAL_OK) ? MSD_OK : MSD_ERROR;
// }

// /**
//  * @brief  Erases the specified memory area of the given SD card.
//  * @param  StartAddr: Start byte address
//  * @param  EndAddr: End byte address
//  * @retval SD status
//  */
// uint8_t BSP_SD_Erase(uint32_t StartAddr, uint32_t EndAddr)
// {
//     uint8_t sd_state = MSD_OK;

//     if(HAL_SD_Erase(&uSdHandle, StartAddr, EndAddr) != HAL_OK)
//     {
//         sd_state = MSD_ERROR;
//     }

//     return sd_state;
// }

// /**
//  * @brief  Gets the current SD card data status.
//  * @param  None
//  * @retval Data transfer state.
//  *          This value can be one of the following values:
//  *            @arg  SD_TRANSFER_OK: No data transfer is acting
//  *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
//  */
// uint8_t BSP_SD_GetCardState(void)
// {
//     HAL_SD_CardStateTypedef card_state;
//     card_state = HAL_SD_GetCardState(&uSdHandle);

//     if(card_state == HAL_SD_CARD_TRANSFER)
//     {
//         return SD_TRANSFER_OK;
//     }
//     else if((card_state == HAL_sd_logger_SENDING) ||
//             (card_state == HAL_sd_logger_RECEIVING) ||
//             (card_state == HAL_sd_logger_PROGRAMMING))
//     {
//         return SD_TRANSFER_BUSY;
//     }
//     else
//     {
//         return SD_TRANSFER_ERROR;
//     }
// }

// /**
//  * @brief  Get SD information about specific SD card.
//  * @param  CardInfo: Pointer to HAL_SD_CardInfoTypedef structure
//  * @retval None
//  */
// void BSP_SD_GetCardInfo(BSP_SD_CardInfo* CardInfo)
// {
//     HAL_SD_GetCardInfo(&uSdHandle, CardInfo);
// }

// /**
//  * @brief  Detects if SD card is correctly plugged in the memory slot or not.
//  * @param  None
//  * @retval Returns if SD is detected or not
//  */
// uint8_t BSP_SD_IsDetected(void)
// {
//     return SD_PRESENT;
// }

// /**
//  * @brief SD Abort callbacks
//  * @param hsd: SD handle
//  * @retval None
//  */
// void HAL_SD_AbortCallback(SD_HandleTypeDef* hsd)
// {
//     BSP_SD_AbortCallback();
// }

// /**
//  * @brief SD Error callbacks
//  * @param hsd: SD handle
//  * @retval None
//  */
// void HAL_SD_ErrorCallback(SD_HandleTypeDef* hsd)
// {
//     BSP_SD_ErrorCallback();
// }

// /**
//  * @brief Rx Transfer completed callback
//  * @param hsd: SD handle
//  * @retval None
//  */
// void HAL_SD_RxCpltCallback(SD_HandleTypeDef* hsd)
// {
//     BSP_SD_ReadCpltCallback();
// }

// /**
//  * @brief Tx Transfer completed callback
//  * @param hsd: SD handle
//  * @retval None
//  */
// void HAL_SD_TxCpltCallback(SD_HandleTypeDef* hsd)
// {
//     BSP_SD_WriteCpltCallback();
// }

// /**
//  * @brief BSP SD Abort callback
//  * @retval None
//  */
// __weak void BSP_SD_AbortCallback(void)
// {
//     Error_Handler();
// }

// /**
//  * @brief BSP SD Error callback
//  * @retval None
//  */
// __weak void BSP_SD_ErrorCallback(void)
// {
//     Error_Handler();
// }

// /**
//  * @brief BSP Rx Transfer completed callback
//  * @retval None
//  */
// __weak void BSP_SD_ReadCpltCallback(void)
// {
//     // redefined in sd_diskio
// }

// /**
//  * @brief BSP Tx Transfer completed callback
//  * @retval None
//  */
// __weak void BSP_SD_WriteCpltCallback(void)
// {
//     // redefined in sd_diskio
// }

// /**
//  * @brief Initializes the SD MSP.
//  * @param None
//  * @retval None
//  */
// void BSP_SD_MspInit(void)
// {
//     GPIO_InitTypeDef GPIO_InitStruct = {0};

//     /* Enable SDMMC1 clock */
//     __HAL_RCC_SDMMC1_CLK_ENABLE();

//     /* Enable DMA2 clocks */
//     __HAL_RCC_DMA2_CLK_ENABLE();

//     /* Enable GPIOs clock */
//     __HAL_RCC_GPIOC_CLK_ENABLE();
//     __HAL_RCC_GPIOD_CLK_ENABLE();

//     /* Common GPIO configuration */
//     GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pull      = GPIO_PULLUP;
//     GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
//     GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;

//     /* GPIOC configuration */
//     GPIO_InitStruct.Pin =
//         GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
//     HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

//     /* GPIOD configuration */
//     GPIO_InitStruct.Pin = GPIO_PIN_2;
//     HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

//     /* NVIC configuration for SDMMC1 interrupts */
//     HAL_NVIC_SetPriority(SDMMC1_IRQn, SDMMC_IRQ_PRIO, 0);
//     HAL_NVIC_EnableIRQ(SDMMC1_IRQn);

//     /* DMA initialization should be done here but
//      * (as there is only one channel for RX and TX)
//      * it is configured and done directly when required.
//      */
// }

// /**
//  * @brief De-Initializes the SD MSP.
//  * @param None
//  * @retval None
//  */
// void BSP_SD_MspDeInit(void)
// {
//     GPIO_InitTypeDef GPIO_InitStruct = {0};

//     /* Disable SDMMC1 clock */
//     __HAL_RCC_SDMMC1_CLK_DISABLE();

//     /* Do NOT disable DMA2 clock */
//     /*__HAL_RCC_DMA2_CLK_DISABLE();*/

//     /* GPIOs to analog */
//     __HAL_RCC_GPIOC_CLK_ENABLE();
//     __HAL_RCC_GPIOD_CLK_ENABLE();

//     GPIO_InitStruct.Mode      = GPIO_MODE_ANALOG;
//     GPIO_InitStruct.Pull      = GPIO_NOPULL;
//     GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
//     GPIO_InitStruct.Alternate = 0;

//     /* GPIOC configuration */
//     GPIO_InitStruct.Pin =
//         GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
//     HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

//     /* GPIOD configuration */
//     GPIO_InitStruct.Pin = GPIO_PIN_2;
//     HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

//     /* NVIC configuration for SDMMC1 interrupts */
//     HAL_NVIC_DisableIRQ(SDMMC1_IRQn);
// }

// /**
//  * @brief Configure the DMA to receive data from the SD card
//  * @retval
//  *  HAL_ERROR or HAL_OK
//  */
// static HAL_StatusTypeDef SD_DMAConfigRx(SD_HandleTypeDef* hsd)
// {
//     static DMA_HandleTypeDef hdma_rx;
//     HAL_StatusTypeDef        status = HAL_ERROR;

//     /* Configure DMA Rx parameters */
//     hdma_rx.Instance                 = DMA2_Channel5;
//     hdma_rx.Init.Request             = DMA_REQUEST_7;
//     hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
//     hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
//     hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
//     hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
//     hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
//     hdma_rx.Init.Priority            = DMA_PRIORITY_VERY_HIGH;

//     /* Associate the DMA handle */
//     __HAL_LINKDMA(hsd, hdmarx, hdma_rx);

//     /* Stop any ongoing transfer and reset the state */
//     HAL_DMA_Abort(&hdma_rx);

//     /* Deinitialize the Channel for new transfer */
//     HAL_DMA_DeInit(&hdma_rx);

//     /* Configure the DMA Channel */
//     status = HAL_DMA_Init(&hdma_rx);

//     /* NVIC configuration for DMA transfer complete interrupt */
//     HAL_NVIC_SetPriority(DMA2_Channel5_IRQn, SD_DMA_IRQ_PRIO, 0);
//     HAL_NVIC_EnableIRQ(DMA2_Channel5_IRQn);

//     return status;
// }

// /**
//  * @brief Configure the DMA to transmit data to the SD card
//  * @retval
//  *  HAL_ERROR or HAL_OK
//  */
// static HAL_StatusTypeDef SD_DMAConfigTx(SD_HandleTypeDef* hsd)
// {
//     static DMA_HandleTypeDef hdma_tx;
//     HAL_StatusTypeDef        status;

//     /* Configure DMA Tx parameters */
//     hdma_tx.Instance                 = DMA2_Channel5;
//     hdma_tx.Init.Request             = DMA_REQUEST_7;
//     hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
//     hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
//     hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
//     hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
//     hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
//     hdma_tx.Init.Priority            = DMA_PRIORITY_VERY_HIGH;

//     /* Associate the DMA handle */
//     __HAL_LINKDMA(hsd, hdmatx, hdma_tx);

//     /* Stop any ongoing transfer and reset the state */
//     HAL_DMA_Abort(&hdma_tx);

//     /* Deinitialize the Channel for new transfer */
//     HAL_DMA_DeInit(&hdma_tx);

//     /* Configure the DMA Channel */
//     status = HAL_DMA_Init(&hdma_tx);

//     /* NVIC configuration for DMA transfer complete interrupt */
//     HAL_NVIC_SetPriority(DMA2_Channel5_IRQn, SD_DMA_IRQ_PRIO, 0);
//     HAL_NVIC_EnableIRQ(DMA2_Channel5_IRQn);

//     return status;
// }
