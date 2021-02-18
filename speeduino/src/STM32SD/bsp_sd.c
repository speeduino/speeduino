/**
******************************************************************************
* @file    bsp_sd.c
* @brief   This file includes the uSD card driver mounted on stm32
*          board.
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*   1. Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright notice,
*      this list of conditions and the following disclaimer in the documentation
*      and/or other materials provided with the distribution.
*   3. Neither the name of STMicroelectronics nor the names of its contributors
*      may be used to endorse or promote products derived from this software
*      without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "bsp_sd.h"
#include "interrupt.h"
#include "PeripheralPins.h"
#include "stm32yyxx_ll_gpio.h"

/* Definition for BSP SD */
#if defined(SDMMC1) || defined(SDMMC2)
#ifndef SD_INSTANCE
#define SD_INSTANCE              SDMMC1
#endif

#define SD_CLK_ENABLE            __HAL_RCC_SDMMC1_CLK_ENABLE
#define SD_CLK_DISABLE           __HAL_RCC_SDMMC1_CLK_DISABLE
#ifdef SDMMC2
#define SD_CLK2_ENABLE            __HAL_RCC_SDMMC2_CLK_ENABLE
#define SD_CLK2_DISABLE           __HAL_RCC_SDMMC2_CLK_DISABLE
#endif

#define SD_CLK_EDGE              SDMMC_CLOCK_EDGE_RISING
#define SD_CLK_BYPASS            SDMMC_CLOCK_BYPASS_DISABLE
#define SD_CLK_PWR_SAVE          SDMMC_CLOCK_POWER_SAVE_DISABLE
#define SD_BUS_WIDE_1B           SDMMC_BUS_WIDE_1B
#define SD_BUS_WIDE_4B           SDMMC_BUS_WIDE_4B
#define SD_BUS_WIDE_8B           SDMMC_BUS_WIDE_8B
#define SD_HW_FLOW_CTRL_ENABLE   SDMMC_HARDWARE_FLOW_CONTROL_ENABLE
#define SD_HW_FLOW_CTRL_DISABLE  SDMMC_HARDWARE_FLOW_CONTROL_DISABLE

#ifdef STM32H7xx
#define SD_CLK_DIV               1
#else
#define SD_CLK_DIV               SDMMC_TRANSFER_CLK_DIV
#endif

#ifdef SDMMC_TRANSCEIVER_ENABLE
#define SD_TRANSCEIVER_ENABLE    SDMMC_TRANSCEIVER_ENABLE
#define SD_TRANSCEIVER_DISABLE   SDMMC_TRANSCEIVER_DISABLE
#endif

#elif defined(SDIO)
#define SD_INSTANCE              SDIO
#define SD_CLK_ENABLE            __HAL_RCC_SDIO_CLK_ENABLE
#define SD_CLK_DISABLE           __HAL_RCC_SDIO_CLK_DISABLE
#define SD_CLK_EDGE              SDIO_CLOCK_EDGE_RISING
#define SD_CLK_BYPASS            SDIO_CLOCK_BYPASS_DISABLE
#define SD_CLK_PWR_SAVE          SDIO_CLOCK_POWER_SAVE_DISABLE
#define SD_BUS_WIDE_1B           SDIO_BUS_WIDE_1B
#define SD_BUS_WIDE_4B           SDIO_BUS_WIDE_4B
#define SD_BUS_WIDE_8B           SDIO_BUS_WIDE_8B
#define SD_HW_FLOW_CTRL_ENABLE   SDIO_HARDWARE_FLOW_CONTROL_ENABLE
#define SD_HW_FLOW_CTRL_DISABLE  SDIO_HARDWARE_FLOW_CONTROL_DISABLE
#define SD_CLK_DIV               SDIO_TRANSFER_CLK_DIV
#else
#error "Unknown SD_INSTANCE"
#endif

#ifndef SD_HW_FLOW_CTRL
#define SD_HW_FLOW_CTRL          SD_HW_FLOW_CTRL_ENABLE
#endif

#ifndef SD_BUS_WIDE
#define SD_BUS_WIDE              SD_BUS_WIDE_4B
#endif

#if defined(SDMMC_TRANSCEIVER_ENABLE) && !defined(SD_TRANSCEIVER_MODE)
#define SD_TRANSCEIVER_MODE      SD_TRANSCEIVER_DISABLE
#endif

/* BSP SD Private Variables */
static SD_HandleTypeDef uSdHandle;
static uint32_t SD_detect_ll_gpio_pin = LL_GPIO_PIN_ALL;
static GPIO_TypeDef *SD_detect_gpio_port = GPIOA;
static DMA_HandleTypeDef hdma_tx;

#ifdef SDMMC_TRANSCEIVER_ENABLE
static uint32_t SD_trans_en_ll_gpio_pin = LL_GPIO_PIN_ALL;
static GPIO_TypeDef *SD_trans_en_gpio_port = GPIOA;
static uint32_t SD_trans_sel_ll_gpio_pin = LL_GPIO_PIN_ALL;
static GPIO_TypeDef *SD_trans_sel_gpio_port = GPIOA;
#endif
#ifndef STM32L1xx
#define SD_OK                         HAL_OK
#define SD_TRANSFER_OK                ((uint8_t)0x00)
#define SD_TRANSFER_BUSY              ((uint8_t)0x01)
#else /* STM32L1xx */
static SD_CardInfo uSdCardInfo;
#endif

/* Private function prototypes -----------------------------------------------*/
// static void              BSP_SD_MspInit(void);
// static void              BSP_SD_MspDeInit(void);
static HAL_StatusTypeDef SD_DMAConfigRx(SD_HandleTypeDef* hsd);
static HAL_StatusTypeDef SD_DMAConfigTx(SD_HandleTypeDef* hsd);

/**
  * @brief  Initializes the SD card device with CS check if any.
  * @retval SD status
  */
uint8_t BSP_SD_Init(void)
{
  uint8_t sd_state = MSD_OK;

  /* uSD device interface configuration */
  uSdHandle.Instance = SD_INSTANCE;

  uSdHandle.Init.ClockEdge           = SD_CLK_EDGE;
#if !defined(STM32L4xx) && !defined(STM32H7xx)
  uSdHandle.Init.ClockBypass         = SD_CLK_BYPASS;
#endif
  uSdHandle.Init.ClockPowerSave      = SD_CLK_PWR_SAVE;
  uSdHandle.Init.BusWide             = SD_BUS_WIDE_1B;
  uSdHandle.Init.HardwareFlowControl = SD_HW_FLOW_CTRL;
  uSdHandle.Init.ClockDiv            = SD_CLK_DIV;
#ifdef SDMMC_TRANSCEIVER_ENABLE
  uSdHandle.Init.Transceiver = SD_TRANSCEIVER_MODE;
  if (SD_TRANSCEIVER_MODE == SD_TRANSCEIVER_ENABLE) {

    BSP_SD_Transceiver_MspInit(&uSdHandle, NULL);
  }
#endif

  if (SD_detect_ll_gpio_pin != LL_GPIO_PIN_ALL) {
    /* Msp SD Detect pin initialization */
    BSP_SD_Detect_MspInit(&uSdHandle, NULL);
    if (BSP_SD_IsDetected() != SD_PRESENT) { /* Check if SD card is present */
      return MSD_ERROR_SD_NOT_PRESENT;
    }
  }

  /* Msp SD initialization */
  BSP_SD_MspInit(&uSdHandle, NULL);

  /* HAL SD initialization */
#ifndef STM32L1xx
  if (HAL_SD_Init(&uSdHandle) != SD_OK)
#else /* STM32L1xx */
  if (HAL_SD_Init(&uSdHandle, &uSdCardInfo) != SD_OK)
#endif
  {
    sd_state = MSD_ERROR;
  }

  /* Configure SD Bus width */
  if (sd_state == MSD_OK) {
    /* Enable wide operation */
    if (HAL_SD_WideBusOperation_Config(&uSdHandle, SD_BUS_WIDE) != SD_OK) {
      sd_state = MSD_ERROR;
    } else {
      sd_state = MSD_OK;
    }
  }
  return  sd_state;
}

/**
  * @brief  DeInitializes the SD card device.
  * @retval SD status
  */
uint8_t BSP_SD_DeInit(void)
{
  uint8_t sd_state = MSD_OK;

  uSdHandle.Instance = SD_INSTANCE;

  /* HAL SD deinitialization */
  if (HAL_SD_DeInit(&uSdHandle) != HAL_OK) {
    sd_state = MSD_ERROR;
  }

  /* Msp SD deinitialization */
  BSP_SD_MspDeInit(&uSdHandle, NULL);

  return  sd_state;
}

#ifdef SDMMC_TRANSCEIVER_ENABLE
/**
  * @brief  Set the SD card device detect pin and port.
  * @param  port one of the gpio port
  * @param  pin one of the gpio pin
  * @retval SD status
  */
uint8_t BSP_SD_TransceiverPin(GPIO_TypeDef *enport, uint32_t enpin, GPIO_TypeDef *selport, uint32_t selpin)
{
  if ((enport != 0) && (selport != 0)) {
    SD_trans_en_ll_gpio_pin = enpin;
    SD_trans_en_gpio_port = enport;
    SD_trans_sel_ll_gpio_pin = selpin;
    SD_trans_sel_gpio_port = selport;
    return MSD_OK;
  }
  return MSD_ERROR;
}
#endif

/**
  * @brief  Set the SD card device detect pin and port.
  * @param  port one of the gpio port
  * @param  pin one of the gpio pin
  * @retval SD status
  */
uint8_t BSP_SD_DetectPin(GPIO_TypeDef *port, uint32_t pin)
{
  if (port != 0) {
    SD_detect_ll_gpio_pin = pin;
    SD_detect_gpio_port = port;
    return MSD_OK;
  }
  return MSD_ERROR;
}

/**
  * @brief  Configures Interrupt mode for SD detection pin.
  * @retval Status
  */
uint8_t BSP_SD_DetectITConfig(void (*callback)(void))
{
  uint8_t sd_state = MSD_ERROR;
  if (SD_detect_ll_gpio_pin != LL_GPIO_PIN_ALL) {
    LL_GPIO_SetPinPull(SD_detect_gpio_port, SD_detect_ll_gpio_pin, LL_GPIO_PULL_UP);
    uint16_t SD_detect_gpio_pin = GPIO_PIN_All;
    switch (SD_detect_ll_gpio_pin) {
      case LL_GPIO_PIN_0:
        SD_detect_gpio_pin = GPIO_PIN_0;
        break;
      case LL_GPIO_PIN_1:
        SD_detect_gpio_pin = GPIO_PIN_1;
        break;
      case LL_GPIO_PIN_2:
        SD_detect_gpio_pin = GPIO_PIN_2;
        break;
      case LL_GPIO_PIN_3:
        SD_detect_gpio_pin = GPIO_PIN_3;
        break;
      case LL_GPIO_PIN_4:
        SD_detect_gpio_pin = GPIO_PIN_4;
        break;
      case LL_GPIO_PIN_5:
        SD_detect_gpio_pin = GPIO_PIN_5;
        break;
      case LL_GPIO_PIN_6:
        SD_detect_gpio_pin = GPIO_PIN_6;
        break;
      case LL_GPIO_PIN_7:
        SD_detect_gpio_pin = GPIO_PIN_7;
        break;
      case LL_GPIO_PIN_8:
        SD_detect_gpio_pin = GPIO_PIN_8;
        break;
      case LL_GPIO_PIN_9:
        SD_detect_gpio_pin = GPIO_PIN_9;
        break;
      case LL_GPIO_PIN_10:
        SD_detect_gpio_pin = GPIO_PIN_10;
        break;
      case LL_GPIO_PIN_11:
        SD_detect_gpio_pin = GPIO_PIN_11;
        break;
      case LL_GPIO_PIN_12:
        SD_detect_gpio_pin = GPIO_PIN_12;
        break;
      case LL_GPIO_PIN_13:
        SD_detect_gpio_pin = GPIO_PIN_13;
        break;
      case LL_GPIO_PIN_14:
        SD_detect_gpio_pin = GPIO_PIN_14;
        break;
      case LL_GPIO_PIN_15:
        SD_detect_gpio_pin = GPIO_PIN_15;
        break;
      default:
        Error_Handler();
        break;
    }
    stm32_interrupt_enable(SD_detect_gpio_port, SD_detect_gpio_pin, callback, GPIO_MODE_IT_RISING_FALLING);
    sd_state = MSD_OK;
  }
  return sd_state;
}

/**
 * @brief  Detects if SD card is correctly plugged in the memory slot or not.
 * @retval Returns if SD is detected or not
 */
uint8_t BSP_SD_IsDetected(void)
{
  uint8_t  status = SD_NOT_PRESENT;
  /* Check SD card detect pin */
  if (!LL_GPIO_IsInputPinSet(SD_detect_gpio_port, SD_detect_ll_gpio_pin)) {
    status = SD_PRESENT;
  }
  return status;
}

#ifndef STM32L1xx
/**
  * @brief  Reads block(s) from a specified address in an SD card, in polling mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  ReadAddr: Address from where data is to be read
  * @param  NumOfBlocks: Number of SD blocks to read
  * @param  Timeout: Timeout for read operation
  * @retval SD status
  */
uint8_t BSP_SD_ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
  if (HAL_SD_ReadBlocks(&uSdHandle, (uint8_t *)pData, ReadAddr, NumOfBlocks, Timeout) != HAL_OK) {
    return MSD_ERROR;
  } else {
    return MSD_OK;
  }
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in polling mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  WriteAddr: Address from where data is to be written
  * @param  NumOfBlocks: Number of SD blocks to write
  * @param  Timeout: Timeout for write operation
  * @retval SD status
  */
uint8_t BSP_SD_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
  if (HAL_SD_WriteBlocks(&uSdHandle, (uint8_t *)pData, WriteAddr, NumOfBlocks, Timeout) != HAL_OK) {
    return MSD_ERROR;
  } else {
    return MSD_OK;
  }
}
#else /* STM32L1xx */
/**
  * @brief  Reads block(s) from a specified address in an SD card, in polling mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  ReadAddr: Address from where data is to be read
  * @param  BlockSize: SD card data block size, that should be 512
  * @param  NumOfBlocks: Number of SD blocks to read
  * @retval SD status
  */
uint8_t BSP_SD_ReadBlocks(uint32_t *pData, uint64_t ReadAddr, uint32_t BlockSize, uint32_t NumOfBlocks)
{
  if (HAL_SD_ReadBlocks(&uSdHandle, (uint8_t *)pData, ReadAddr, BlockSize, NumOfBlocks) != SD_OK) {
    return MSD_ERROR;
  } else {
    return MSD_OK;
  }
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in polling mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  WriteAddr: Address from where data is to be written
  * @param  BlockSize: SD card data block size, that should be 512
  * @param  NumOfBlocks: Number of SD blocks to write
  * @retval SD status
  */
uint8_t BSP_SD_WriteBlocks(uint32_t *pData, uint64_t WriteAddr, uint32_t BlockSize, uint32_t NumOfBlocks)
{
  if (HAL_SD_WriteBlocks(&uSdHandle, (uint8_t *)pData, WriteAddr, BlockSize, NumOfBlocks) != SD_OK) {
    return MSD_ERROR;
  } else {
    return MSD_OK;
  }
}
#endif /* !STM32L1xx */

/**
  * @brief  Erases the specified memory area of the given SD card.
  * @param  StartAddr: Start byte address
  * @param  EndAddr: End byte address
  * @retval SD status
  */
uint8_t BSP_SD_Erase(uint64_t StartAddr, uint64_t EndAddr)
{
  if (HAL_SD_Erase(&uSdHandle, StartAddr, EndAddr) != SD_OK) {
    return MSD_ERROR;
  } else {
    return MSD_OK;
  }
}

/**
  * @brief  Initializes the SD MSP.
  * @param  hsd: SD handle
  * @param  Params : pointer on additional configuration parameters, can be NULL.
  */
__weak void BSP_SD_MspInit(SD_HandleTypeDef *hsd, void *Params)
{
  UNUSED(Params);

  /* Configure SD GPIOs */
  const PinMap *map = PinMap_SD;
  while (map->pin != NC) {
    pin_function(map->pin, map->function);
    map++;
  }

  /* Enable SD clock */
#if defined(SDMMC1) && defined(SDMMC2)
  if (hsd->Instance == SDMMC1) {
    SD_CLK_ENABLE();
  } else {
    SD_CLK2_ENABLE();
  }
#else
  UNUSED(hsd);
  SD_CLK_ENABLE();
#endif
}

/**
  * @brief  Initializes the SD Detect pin MSP.
  * @param  hsd: SD handle
  * @param  Params : pointer on additional configuration parameters, can be NULL.
  */
__weak void BSP_SD_Detect_MspInit(SD_HandleTypeDef *hsd, void *Params)
{
  UNUSED(hsd);
  UNUSED(Params);

  /* GPIO configuration in input for uSD_Detect signal */
#ifdef LL_GPIO_SPEED_FREQ_VERY_HIGH
  LL_GPIO_SetPinSpeed(SD_detect_gpio_port, SD_detect_ll_gpio_pin, LL_GPIO_SPEED_FREQ_VERY_HIGH);
#else
  LL_GPIO_SetPinSpeed(SD_detect_gpio_port, SD_detect_ll_gpio_pin, LL_GPIO_SPEED_FREQ_HIGH);
#endif
  LL_GPIO_SetPinMode(SD_detect_gpio_port, SD_detect_ll_gpio_pin, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinPull(SD_detect_gpio_port, SD_detect_ll_gpio_pin, LL_GPIO_PULL_UP);
}

/**
  * @brief  DeInitializes the SD MSP.
  * @param  hsd: SD handle
  * @param  Params : pointer on additional configuration parameters, can be NULL.
  */
__weak void BSP_SD_MspDeInit(SD_HandleTypeDef *hsd, void *Params)
{
  UNUSED(Params);

  /* DeInit GPIO pins can be done in the application
     (by surcharging this __weak function) */

  /* Disable SD clock */
#if defined(SDMMC1) && defined(SDMMC2)
  if (hsd->Instance == SDMMC1) {
    SD_CLK_DISABLE();
  } else {
    SD_CLK2_DISABLE();
  }
#else
  UNUSED(hsd);
  SD_CLK_DISABLE();
#endif
}

#ifdef SDMMC_TRANSCEIVER_ENABLE
/**
  * @brief  Initializes the SD Transceiver pin MSP.
  * @param  hsd: SD handle
  * @param  Params : pointer on additional configuration parameters, can be NULL.
  */
__weak void BSP_SD_Transceiver_MspInit(SD_HandleTypeDef *hsd, void *Params)
{
  UNUSED(hsd);
  UNUSED(Params);

  LL_GPIO_SetPinSpeed(SD_trans_en_gpio_port, SD_trans_en_ll_gpio_pin, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinMode(SD_trans_en_gpio_port, SD_trans_en_ll_gpio_pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinPull(SD_trans_en_gpio_port, SD_trans_en_ll_gpio_pin, LL_GPIO_PULL_NO);

  LL_GPIO_SetPinSpeed(SD_trans_sel_gpio_port, SD_trans_sel_ll_gpio_pin, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinMode(SD_trans_sel_gpio_port, SD_trans_sel_ll_gpio_pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinPull(SD_trans_sel_gpio_port, SD_trans_sel_ll_gpio_pin, LL_GPIO_PULL_NO);

  /* Enable the level shifter */
  LL_GPIO_SetOutputPin(SD_trans_en_gpio_port, SD_trans_en_ll_gpio_pin);

  /* By default start with the default voltage */
  LL_GPIO_ResetOutputPin(SD_trans_sel_gpio_port, SD_trans_sel_ll_gpio_pin);
}

/**
  * @brief  Enable/Disable the SD Transceiver 1.8V Mode Callback.
  * @param  status: Voltage Switch State
  * @retval None
  */
void HAL_SDEx_DriveTransceiver_1_8V_Callback(FlagStatus status)
{
  if (status == SET) {
    LL_GPIO_SetOutputPin(SD_trans_sel_gpio_port, SD_trans_sel_ll_gpio_pin);
  } else {
    LL_GPIO_ResetOutputPin(SD_trans_sel_gpio_port, SD_trans_sel_ll_gpio_pin);
  }
}
#endif /* SDMMC_TRANSCEIVER_ENABLE */

#ifndef STM32L1xx
/**
  * @brief  Gets the current SD card data status.
  * @retval Data transfer state.
  *          This value can be one of the following values:
  *            @arg  SD_TRANSFER_OK: No data transfer is acting
  *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
  */
uint8_t BSP_SD_GetCardState(void)
{
  return ((HAL_SD_GetCardState(&uSdHandle) == HAL_SD_CARD_TRANSFER) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}
#else /* STM32L1xx */
/**
  * @brief  Gets the current SD card data status.
  * @retval Data transfer state.
  *          This value can be one of the following values:
  *            @arg  SD_TRANSFER_OK: No data transfer is acting
  *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
  *            @arg  SD_TRANSFER_ERROR: Data transfer error
  */
HAL_SD_TransferStateTypedef BSP_SD_GetStatus(void)
{
  return (HAL_SD_GetStatus(&uSdHandle));
}
#endif

/**
  * @brief  Get SD information about specific SD card.
  * @param  CardInfo: Pointer to HAL_SD_CardInfoTypedef structure
  */
void BSP_SD_GetCardInfo(HAL_SD_CardInfoTypeDef *CardInfo)
{
  /* Get SD card Information */
  HAL_SD_Get_CardInfo(&uSdHandle, CardInfo);
}


/**
 * @brief  Writes block(s) to a specified address in an SD card, in DMA mode.
 * @param  pData: Pointer to the buffer that will contain the data to transmit
 * @param  WriteAddr: Address from where data is to be written
 * @param  NumOfBlocks: Number of SD blocks to write
 * @retval SD status
 */
uint8_t BSP_SD_WriteBlocks_DMA(uint32_t* pData,
                               uint32_t  WriteAddr,
                               uint32_t  NumOfBlocks,
                               uint32_t Timeout)
{
    HAL_StatusTypeDef sd_state = HAL_OK;
    HAL_StatusTypeDef sd_state_while_loop = HAL_OK;
    HAL_SD_CardStateTypeDef card_state;
    uint32_t loopnr = 0;

    /* Invalidate the dma rx handle*/
    uSdHandle.hdmarx = NULL;

    /* Prepare the dma channel for a read operation */
    sd_state = SD_DMAConfigTx(&uSdHandle);

    if(sd_state == HAL_OK)
    {
        /* Write block(s) in DMA transfer mode */
        sd_state = HAL_SD_WriteBlocks_DMA(&uSdHandle, (uint8_t*)pData, WriteAddr,
                                          NumOfBlocks);
    }
  // busy waiting
  // do
  // {
  //   sd_state_while_loop = HAL_SD_GetState(&uSdHandle);
  // } while ((HAL_SD_STATE_BUSY == sd_state_while_loop));
  // if (HAL_SD_STATE_READY != sd_state_while_loop) { return MSD_ERROR; }

  do
  {
    loopnr++;
  } while ((loopnr <1000000));
  // busy waiting
  do
  {
    card_state = HAL_SD_GetCardState(&uSdHandle);
    loopnr--;
  } while ((HAL_SD_CARD_TRANSFER != card_state));
  return MSD_OK;
  //  return (sd_state_while_loop == HAL_OK) ? MSD_OK : MSD_ERROR;
 }

//  /**
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

/**
 * @brief Configure the DMA to transmit data to the SD card
 * @retval
 *  HAL_ERROR or HAL_OK
 */
static HAL_StatusTypeDef SD_DMAConfigTx(SD_HandleTypeDef* hsd)
{
    HAL_StatusTypeDef        status;

    /* Configure DMA Tx parameters */
    hdma_tx.Instance                 = DMA2_Stream3;
    hdma_tx.Init.Channel             = DMA_CHANNEL_4;
    hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    hdma_tx.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    hdma_tx.Init.Mode                = DMA_PFCTRL;
    hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_tx.Init.MemBurst            = DMA_MBURST_INC4;
    hdma_tx.Init.PeriphBurst         = DMA_MBURST_INC4;


    __HAL_RCC_DMA2_CLK_ENABLE();

    /* Associate the DMA handle */
    __HAL_LINKDMA(hsd, hdmatx, hdma_tx);

    /* Stop any ongoing transfer and reset the state */
    HAL_DMA_Abort(&hdma_tx);

    /* Deinitialize the Channel for new transfer */
    HAL_DMA_DeInit(&hdma_tx);

    /* Configure the DMA Channel */
    status = HAL_DMA_Init(&hdma_tx);

    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

    return status;
}

/******************************************************************************/
/* STM32F7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f7xx.s).                    */
/******************************************************************************/

/**
* @brief This function handles DMA2 stream0 global interrupt.
*/
void DMA2_Stream3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream0_IRQn 0 */

  /* USER CODE END DMA2_Stream0_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_tx);
  /* USER CODE BEGIN DMA2_Stream0_IRQn 1 */

  /* USER CODE END DMA2_Stream0_IRQn 1 */
}

// /**
// * @brief This function handles DMA2 stream5 global interrupt.
// */
// void DMA2_Stream5_IRQHandler(void)
// {
//   /* USER CODE BEGIN DMA2_Stream5_IRQn 0 */

//   /* USER CODE END DMA2_Stream5_IRQn 0 */
//   HAL_DMA_IRQHandler(&hdma_rx);
//   /* USER CODE BEGIN DMA2_Stream5_IRQn 1 */

//   /* USER CODE END DMA2_Stream5_IRQn 1 */
// }

/**
* @brief This function handles SDMMC2 global interrupt.
*/
void SDIO_IRQHandler(void)
{
  /* USER CODE BEGIN SDMMC2_IRQn 0 */

  /* USER CODE END SDMMC2_IRQn 0 */
  HAL_SD_IRQHandler(&uSdHandle);
  /* USER CODE BEGIN SDMMC2_IRQn 1 */

  /* USER CODE END SDMMC2_IRQn 1 */
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
