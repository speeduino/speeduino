#ifdef STM32F407xx
/**
  ******************************************************************************
  * @file    SdFatFs.h
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

#ifndef SdFatFs_h
#define SdFatFs_h

#include "Sd2Card.h"

/* FatFs includes component */
#include "src/FatFs/FatFs.h"

/* To match Arduino definition*/
#define   FILE_WRITE  FA_WRITE
#define   FILE_READ   FA_READ

/** year part of FAT directory date field */
static inline uint16_t FAT_YEAR(uint16_t fatDate)
{
  return 1980 + (fatDate >> 9);
}
/** month part of FAT directory date field */
static inline uint8_t FAT_MONTH(uint16_t fatDate)
{
  return (fatDate >> 5) & 0XF;
}
/** day part of FAT directory date field */
static inline uint8_t FAT_DAY(uint16_t fatDate)
{
  return fatDate & 0X1F;
}

/** hour part of FAT directory time field */
static inline uint8_t FAT_HOUR(uint16_t fatTime)
{
  return fatTime >> 11;
}
/** minute part of FAT directory time field */
static inline uint8_t FAT_MINUTE(uint16_t fatTime)
{
  return (fatTime >> 5) & 0X3F;
}
/** second part of FAT directory time field */
static inline uint8_t FAT_SECOND(uint16_t fatTime)
{
  return 2 * (fatTime & 0X1F);
}

class SdFatFs {
  public:

    bool init(void);

    /** Return the FatFs type: 12, 16, 32 (0: unknown)*/
    uint8_t fatType(void);

    // inline functions that return volume info
    /** \return The volume's cluster size in blocks. */
    uint8_t blocksPerCluster(void) const
    {
      return _SDFatFs.csize;
    }
    /** \return The total number of clusters in the volume. */
    uint32_t clusterCount(void) const
    {
      return (_SDFatFs.n_fatent - 2);
    }

    char *getRoot(void)
    {
      return _SDPath;
    };
  private:
    FATFS _SDFatFs;  /* File system object for SD disk logical drive */
    char _SDPath[4]; /* SD disk logical drive path */
};
#endif  // sdFatFs_h
#endif