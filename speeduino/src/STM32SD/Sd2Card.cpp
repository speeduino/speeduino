#ifdef STM32F407xx
/**
  ******************************************************************************
  * @file    Sd2Card.cpp
  * @author  Frederic Pillon <frederic.pillon@st.com> for STMicroelectronics
  * @date    2017
  * @brief
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
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

#include <Arduino.h>
#include "Sd2Card.h"

bool Sd2Card::init(uint32_t detectpin)
{
  if (detectpin != SD_DETECT_NONE) {
    PinName p = digitalPinToPinName(detectpin);
    if ((p == NC) || \
        BSP_SD_DetectPin(set_GPIO_Port_Clock(STM_PORT(p)),
                         STM_LL_GPIO_PIN(p)) != MSD_OK) {
      return false;
    }
  }
#ifdef SDMMC_TRANSCEIVER_ENABLE
  PinName sd_en = digitalPinToPinName(SD_TRANSCEIVER_EN);
  PinName sd_sel = digitalPinToPinName(SD_TRANSCEIVER_SEL);
  BSP_SD_TransceiverPin(set_GPIO_Port_Clock(STM_PORT(sd_en)),
                        STM_LL_GPIO_PIN(sd_en),
                        set_GPIO_Port_Clock(STM_PORT(sd_sel)),
                        STM_LL_GPIO_PIN(sd_sel));
#endif
  if (BSP_SD_Init() == MSD_OK) {
    BSP_SD_GetCardInfo(&_SdCardInfo);
    return true;
  }
  return false;
}

uint8_t Sd2Card::type(void) const
{
  uint8_t cardType = SD_CARD_TYPE_UKN;
#ifndef STM32L1xx
  switch (_SdCardInfo.CardType) {
    case CARD_SDSC:
      switch (_SdCardInfo.CardVersion) {
        case CARD_V1_X:
          cardType = SD_CARD_TYPE_SD1;
          break;
        case CARD_V2_X:
          cardType = SD_CARD_TYPE_SD2;
          break;
        default:
          cardType = SD_CARD_TYPE_UKN;
      }
      break;
    case CARD_SDHC_SDXC:
      cardType = SD_CARD_TYPE_SDHC;
      break;
    case CARD_SECURED:
      cardType = SD_CARD_TYPE_SECURED;
      break;
    default:
      cardType = SD_CARD_TYPE_UKN;
  }
#else /* STM32L1xx */
  switch (_SdCardInfo.CardType) {
    case STD_CAPACITY_SD_CARD_V1_1:
      cardType = SD_CARD_TYPE_SD1;
      break;
    case STD_CAPACITY_SD_CARD_V2_0:
      cardType = SD_CARD_TYPE_SD2;
      break;
    case HIGH_CAPACITY_SD_CARD:
      cardType = SD_CARD_TYPE_SDHC;
      break;
    default:
      cardType = SD_CARD_TYPE_UKN;
  }
#endif
  return cardType;
}
#endif
