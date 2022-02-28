/**
 ******************************************************************************
 * @file    LPS25HBSensor.cpp
 * @author  AST
 * @version V1.0.0
 * @date    7 September 2017
 * @brief   Implementation of an LPS25HB Pressure sensor.
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


/* Includes ------------------------------------------------------------------*/

#include "Arduino.h"
#include "LPS25HBSensor.h"


/* Class Implementation ------------------------------------------------------*/
/** Constructor
 * @param i2c object of an helper class which handles the I2C peripheral
 * @param address the address of the component's instance
 */
LPS25HBSensor::LPS25HBSensor(TwoWire *i2c, uint8_t address) : dev_i2c(i2c), address(address)
{
  dev_spi = NULL;
}

/** Constructor
 * @param spi object of an helper class which handles the SPI peripheral
 * @param cs_pin the chip select pin
 * @param spi_speed the SPI speed
 */
LPS25HBSensor::LPS25HBSensor(SPIClass *spi, int cs_pin, uint32_t spi_speed) : dev_spi(spi), cs_pin(cs_pin), spi_speed(spi_speed)
{
  dev_i2c = NULL;
  address = 0;
}

/**
 * @brief  Configure the sensor in order to be used
 * @retval 0 in case of success, an error code otherwise
 */
LPS25HBStatusTypeDef LPS25HBSensor::begin(void)
{
  if(dev_spi)
  {
    // Configure CS pin
    pinMode(cs_pin, OUTPUT);
    digitalWrite(cs_pin, HIGH); 
  }

  /* Power down the device */
  if ( LPS25HB_DeActivate( (void *)this ) == LPS25HB_ERROR )
  {
    return LPS25HB_STATUS_ERROR;
  }

  if ( SetODR( 1.0f ) == LPS25HB_STATUS_ERROR )
  {
    return LPS25HB_STATUS_ERROR;
  }

  /* Enable interrupt circuit */
  if ( LPS25HB_Set_InterruptCircuitEnable( (void *)this, LPS25HB_ENABLE ) == LPS25HB_ERROR )
  {
    return LPS25HB_STATUS_ERROR;
  }

  /* Set block data update mode */
  if ( LPS25HB_Set_Bdu( (void *)this, LPS25HB_BDU_NO_UPDATE ) == LPS25HB_ERROR )
  {
    return LPS25HB_STATUS_ERROR;
  }

  /* Set SPI mode */
  if ( LPS25HB_Set_SpiInterface( (void *)this, LPS25HB_SPI_4_WIRE ) == LPS25HB_ERROR )
  {
    return LPS25HB_STATUS_ERROR;
  }

  /* Set internal averaging sample counts for pressure and temperature */
  if ( LPS25HB_Set_Avg( (void *)this, LPS25HB_AVGP_32, LPS25HB_AVGT_16 ) == LPS25HB_ERROR )
  {
    return LPS25HB_STATUS_ERROR;
  }

  return LPS25HB_STATUS_OK;
}

/**
 * @brief  Disable the sensor and relative resources
 * @retval 0 in case of success, an error code otherwise
 */
LPS25HBStatusTypeDef LPS25HBSensor::end(void)
{
  /* Disable pressure and temperature sensor */
  if (Disable() != LPS25HB_STATUS_OK)
  {
    return LPS25HB_STATUS_ERROR;
  }

  /* Reset CS configuration */
  if(dev_spi)
  {
    // Configure CS pin
    pinMode(cs_pin, INPUT); 
  }

  return LPS25HB_STATUS_OK;
}

/**
 * @brief  Enable LPS25HB
 * @retval LPS25HB_STATUS_OK in case of success, an error code otherwise
 */
LPS25HBStatusTypeDef LPS25HBSensor::Enable(void)
{
  /* Power up the device */
  if ( LPS25HB_Activate( (void *)this ) == LPS25HB_ERROR )
  {
    return LPS25HB_STATUS_ERROR;
  }

  return LPS25HB_STATUS_OK;
}

/**
 * @brief  Disable LPS25HB
 * @retval LPS25HB_STATUS_OK in case of success, an error code otherwise
 */
LPS25HBStatusTypeDef LPS25HBSensor::Disable(void)
{
  /* Power down the device */
  if ( LPS25HB_DeActivate( (void *)this ) == LPS25HB_ERROR )
  {
    return LPS25HB_STATUS_ERROR;
  }

  return LPS25HB_STATUS_OK;
}

/**
 * @brief  Read ID address of LPS25HB
 * @param  ht_id the pointer where the ID of the device is stored
 * @retval LPS25HB_STATUS_OK in case of success, an error code otherwise
 */
LPS25HBStatusTypeDef LPS25HBSensor::ReadID(uint8_t *p_id)
{
  if(!p_id)
  { 
    return LPS25HB_STATUS_ERROR;
  }
 
  /* Read WHO AM I register */
  if ( LPS25HB_Get_DeviceID( (void *)this, p_id ) == LPS25HB_ERROR )
  {
    return LPS25HB_STATUS_ERROR;
  }

  return LPS25HB_STATUS_OK;
}

/**
 * @brief  Reboot memory content of LPS25HB
 * @param  None
 * @retval LPS25HB_STATUS_OK in case of success, an error code otherwise
 */
LPS25HBStatusTypeDef LPS25HBSensor::Reset(void)
{
  if ( LPS25HB_MemoryBoot((void *)this) == LPS25HB_ERROR )
  {
    return LPS25HB_STATUS_ERROR;
  }

  return LPS25HB_STATUS_OK;
}

/**
 * @brief  Read LPS25HB output register, and calculate the pressure in mbar
 * @param  pfData the pressure value in mbar
 * @retval LPS25HB_STATUS_OK in case of success, an error code otherwise
 */
LPS25HBStatusTypeDef LPS25HBSensor::GetPressure(float* pfData)
{
  int32_t int32data = 0;

  /* Read data from LPS25HB. */
  if ( LPS25HB_Get_Pressure( (void *)this, &int32data ) == LPS25HB_ERROR )
  {
    return LPS25HB_STATUS_ERROR;
  }

  *pfData = ( float )int32data / 100.0f;

  return LPS25HB_STATUS_OK;
}

/**
 * @brief  Read LPS25HB output register, and calculate the temperature
 * @param  pfData the temperature value
 * @retval LPS25HB_STATUS_OK in case of success, an error code otherwise
 */
LPS25HBStatusTypeDef LPS25HBSensor::GetTemperature(float *pfData)
{
  int16_t int16data = 0;

  /* Read data from LPS25HB. */
  if ( LPS25HB_Get_Temperature( (void *)this, &int16data ) == LPS25HB_ERROR )
  {
    return LPS25HB_STATUS_ERROR;
  }

  *pfData = ( float )int16data / 10.0f;

  return LPS25HB_STATUS_OK;
}

/**
 * @brief  Read LPS25HB output data rate
 * @param  odr the pointer to the output data rate
 * @retval LPS25HB_STATUS_OK in case of success, an error code otherwise
 */
LPS25HBStatusTypeDef LPS25HBSensor::GetODR(float* odr)
{
  LPS25HB_Odr_et odr_low_level;

  if ( LPS25HB_Get_Odr( (void *)this, &odr_low_level ) == LPS25HB_ERROR )
  {
    return LPS25HB_STATUS_ERROR;
  }

  switch( odr_low_level )
  {
    case LPS25HB_ODR_ONE_SHOT:
      *odr =  0.0f;
      break;
    case LPS25HB_ODR_1HZ:
      *odr =  1.0f;
      break;
    case LPS25HB_ODR_7HZ:
      *odr =  7.0f;
      break;
    case LPS25HB_ODR_12_5HZ:
      *odr = 12.5f;
      break;
    case LPS25HB_ODR_25HZ:
      *odr = 25.0f;
      break;
    default:
      *odr = -1.0f;
      return LPS25HB_STATUS_ERROR;
  }

  return LPS25HB_STATUS_OK;
}

/**
 * @brief  Set ODR
 * @param  odr the output data rate to be set
 * @retval LPS25HB_STATUS_OK in case of success, an error code otherwise
 */
LPS25HBStatusTypeDef LPS25HBSensor::SetODR(float odr)
{
  LPS25HB_Odr_et new_odr;

  new_odr = ( odr <=  1.0f ) ? LPS25HB_ODR_1HZ
          : ( odr <=  7.0f ) ? LPS25HB_ODR_7HZ
          : ( odr <= 12.5f ) ? LPS25HB_ODR_12_5HZ
          :                    LPS25HB_ODR_25HZ;

  if ( LPS25HB_Set_Odr( (void *)this, new_odr ) == LPS25HB_ERROR )
  {
    return LPS25HB_STATUS_ERROR;
  }

  return LPS25HB_STATUS_OK;
}

/**
 * @brief Read the data from register
 * @param reg register address
 * @param data register data
 * @retval LPS25HB_STATUS_OK in case of success
 * @retval LPS25HB_STATUS_ERROR in case of failure
 */
LPS25HBStatusTypeDef LPS25HBSensor::ReadReg( uint8_t reg, uint8_t *data )
{

  if ( LPS25HB_ReadReg( (void *)this, reg, 1, data ) == LPS25HB_ERROR )
  {
    return LPS25HB_STATUS_ERROR;
  }

  return LPS25HB_STATUS_OK;
}

/**
 * @brief Write the data to register
 * @param reg register address
 * @param data register data
 * @retval LPS25HB_STATUS_OK in case of success
 * @retval LPS25HB_STATUS_ERROR in case of failure
 */
LPS25HBStatusTypeDef LPS25HBSensor::WriteReg( uint8_t reg, uint8_t data )
{

  if ( LPS25HB_WriteReg( (void *)this, reg, 1, &data ) == LPS25HB_ERROR )
  {
    return LPS25HB_STATUS_ERROR;
  }

  return LPS25HB_STATUS_OK;
}


uint8_t LPS25HB_IO_Write( void *handle, uint8_t WriteAddr, uint8_t *pBuffer, uint16_t nBytesToWrite )
{
  return ((LPS25HBSensor *)handle)->IO_Write(pBuffer, WriteAddr, nBytesToWrite);
}

uint8_t LPS25HB_IO_Read( void *handle, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t nBytesToRead )
{
  return ((LPS25HBSensor *)handle)->IO_Read(pBuffer, ReadAddr, nBytesToRead);
}
