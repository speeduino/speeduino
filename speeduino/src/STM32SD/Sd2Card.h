#ifdef STM32F407xx
/**
  ******************************************************************************
  * @file    Sd2Card.h
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

#ifndef Sd2Card_h
#define Sd2Card_h

#include "bsp_sd.h"

// card types to match Arduino definition
#define SD_CARD_TYPE_UKN      0
/** Standard capacity V1 SD card */
#define SD_CARD_TYPE_SD1      1
/** Standard capacity V2 SD card */
#define SD_CARD_TYPE_SD2      2
/** High Capacity SD card */
#define SD_CARD_TYPE_SDHC     3
/** High Capacity SD card */
#define SD_CARD_TYPE_SECURED  4

class Sd2Card {
  public:

    bool init(uint32_t detectpin = SD_DETECT_NONE);

    /** Return the card type: SD V1, SD V2 or SDHC */
    uint8_t type(void) const;

  private:
    SD_CardInfo _SdCardInfo;

};
#endif  // sd2Card_h
#endif //STM32F407xx