/**
 * IFX9201.cpp - Library for Arduino to control the IFX9201 H-Bridge.
 *
 * The IFX9201 is a general purpose 6A H-Bridge for industrial applications, home appliance and building
 * automation, power tools battery management and medical applications, designed for the control of small
 * DC motors and inductive loads.
 *
 * Have a look at the application note/datasheet for more information.
 */
/* Copyright (c) 2018 Infineon Technologies AG
 *
 * Redistribution and use in source and binary forms, with or without modification,are permitted provided that the
 * following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following
 * disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the copyright holders nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*!	\file IFX9201.cpp
 *	\brief This file defines functions and predefined instances from IFX9201.h
 */
 /* Paul Carpenter Sept-2019
    Add error traps and return codes to more functions
    Improve duty cycle conversion to work with any analogWriteResolution
       (requires Pull request #106 merged for getAnalogWriteMaximum)
    Try to make code more readable
    Correct Disable Pin polarity for correct operation */
#include <Arduino.h>
#include "IFX9201.h"

// Instantiate
IFX9201::IFX9201( )
{
m_Mode = IFX9201Mode_PWM;
m_PWMFreq = 0u;
m_SPIsettings = SPISettings( 1000000u, MSBFIRST, SPI_MODE1 );
}


// SPI Mode begin
void IFX9201::begin( SPIClass &bus, uint8_t pinSlaveSelect, uint8_t pinDirection, uint8_t pinPWM, uint8_t pinDisable )
{
m_Mode = IFX9201Mode_SPI;
m_bus = &bus;

m_bus->begin( );
m_bus->setBitOrder( MSBFIRST );
m_bus->setClockDivider( SPI_CLOCK_DIV16 );
m_bus->setDataMode( SPI_MODE1 );

m_SlaveSelect = pinSlaveSelect;
m_Disable = pinDisable;
m_Direction = pinDirection;
m_PWM = pinPWM;

begin( );

setCTRLReg( IFX9201__CTRL_SIN_MSK );
}


// PWM Mode begin
void IFX9201::begin( uint8_t pinDirection, uint8_t pinPWM, uint8_t pinDisable )
{
m_Mode = IFX9201Mode_PWM;

m_Disable = pinDisable;
m_Direction = pinDirection;
m_PWM = pinPWM;

begin( );
}


// Common begin
// Note for SPI mode DIR, PWM and DIS must all be LOW
// Otherwise IFX9201 reverts to PWM mode
void IFX9201::begin( )
{
pinMode( m_Disable, OUTPUT );
pinMode( m_Direction, OUTPUT );
pinMode( m_PWM, OUTPUT );

digitalWrite( m_PWM, LOW );
digitalWrite( m_Direction, LOW );

if( m_Mode == IFX9201Mode_SPI )
  {
  pinMode( m_SlaveSelect, OUTPUT );
  digitalWrite( m_SlaveSelect, HIGH );
  digitalWrite( m_Disable, LOW );        // To ensure we stay in SPI mode
  }
else        // PWM mode
  {
  digitalWrite( m_Disable, HIGH );        // Disable ON - MOSFET drivers OFF
  setPWMFreqency( IFX9201__DEFAULT_PWM_FREQUENCY );
  }
}


// Changed to return errors for invalid frequencies or PWM pin
uint8_t IFX9201::setPWMFreqency( uint16_t freq )
{
uint8_t ret = IFX9201__NO_ERROR;
int16_t val;

if( freq <= IFX9201__MAX_PWM_FREQ && freq > 0 )
  {
  m_PWMFreq = freq;
  // val = setAnalogWriteFrequency( m_PWM, m_PWMFreq );
  // if( val == -1 )
  //   ret = IFX9201__ILLEGAL_FREQUENCY;
  // else
  //   if( val == -2 )
  //     ret = IFX9201__ILLEGAL_PWM_PIN;
  }
else
  ret = IFX9201__ILLEGAL_FREQUENCY;
return ret;
}


// forwards SPI or default PWM mode
uint8_t IFX9201::forwards( )
{
uint8_t ret = IFX9201__NO_ERROR;

if( m_Mode == IFX9201Mode_SPI )
  ret = setCTRLReg( IFX9201__CTRL_SIN_MSK | IFX9201__CTRL_SEN_MSK | IFX9201__CTRL_SDIR_MSK | IFX9201__CTRL_SPWM_MSK );
else
  ret = forwards( IFX9201__DEFAULT_DUTY_CYCLE );
return ret;
}


// backwards SPI or default PWM mode
uint8_t IFX9201::backwards( )
{
uint8_t ret = IFX9201__NO_ERROR;

if( m_Mode == IFX9201Mode_SPI )
  ret = setCTRLReg( IFX9201__CTRL_SIN_MSK | IFX9201__CTRL_SEN_MSK | IFX9201__CTRL_SPWM_MSK );
else
    ret = backwards( IFX9201__DEFAULT_DUTY_CYCLE );
return ret;
}


// forwards PWM mode ONLY
uint8_t IFX9201::forwards( uint8_t duty_cycle )
{
uint8_t ret = IFX9201__NO_ERROR;

if( m_Mode == IFX9201Mode_SPI )
  ret = IFX9201__ILLEGAL_OPERATION_ERROR;
else
  {
  digitalWrite( m_Direction, HIGH );
  ret = DutyCycleToanlogWrite( duty_cycle );
  digitalWrite( m_Disable, LOW );        // Disable OFF - MOSFET drivers ON
  }
return ret;
}


// backwards PWM mode ONLY
uint8_t IFX9201::backwards( uint8_t duty_cycle )
{
uint8_t ret = IFX9201__NO_ERROR;

if( m_Mode == IFX9201Mode_SPI )
  ret = IFX9201__ILLEGAL_OPERATION_ERROR;
else
  {
  digitalWrite( m_Direction, LOW );
  ret = DutyCycleToanlogWrite( duty_cycle );
  digitalWrite( m_Disable, LOW );        // Disable OFF - MOSFET drivers ON
  }
return ret;
}


// Stop both modes
uint8_t IFX9201::stop( )
{
uint8_t ret = IFX9201__NO_ERROR;

if( m_Mode == IFX9201Mode_SPI )
  ret = setCTRLReg( IFX9201__CTRL_SIN_MSK );
else
  {
  digitalWrite( m_Direction, LOW );
  ret = DutyCycleToanlogWrite( 0 );
  digitalWrite( m_Disable, HIGH );        // Disable ON - MOSFET drivers OFF
  }
return ret;
}


void IFX9201::end( )
{
pinMode( m_Disable, INPUT );
pinMode( m_SlaveSelect, INPUT );
pinMode( m_Direction, INPUT );
pinMode( m_PWM, INPUT );
}


uint8_t IFX9201::getDIAReg( )
{
SPItransfer( IFX9201__RD_DIA );
return SPItransfer( IFX9201__RD_DIA );
}


uint8_t IFX9201::getREVReg( )
{
SPItransfer( IFX9201__RD_REV );
return SPItransfer( IFX9201__RD_REV );
}


uint8_t IFX9201::getCTRLReg( )
{
SPItransfer( IFX9201__RD_CTRL );
return SPItransfer( IFX9201__RD_CTRL );
}


uint8_t IFX9201::setCTRLReg( uint8_t data_out )
{
uint8_t ret = IFX9201__NO_ERROR;

if( m_Mode == IFX9201Mode_SPI )
  {
  SPItransfer( IFX9201__WR_CTRL | ( IFX9201__CTRL_DATA_MSK & data_out ) );
  if( ( SPItransfer( IFX9201__RD_CTRL ) & IFX9201__CTRL_DATA_MSK) != ( data_out & IFX9201__CTRL_DATA_MSK ) )
    ret = IFX9201__SPI_WRITE_ERROR;
  }
else
  ret = IFX9201__MODE_ERROR;

return ret;
}


void IFX9201::resetDIAReg( )
{
if( m_Mode == IFX9201Mode_SPI )
  SPItransfer( IFX9201__RES_DIA );
}


uint8_t IFX9201::SPItransfer( uint8_t data_out )
{
uint8_t data_in = 0x00u;

m_bus->beginTransaction( m_SPIsettings );

digitalWrite( m_SlaveSelect, LOW );
data_in = m_bus->transfer( data_out );
digitalWrite( m_SlaveSelect, HIGH );

return data_in;
}


/* Improved Duty cycle conversion from integer 0 to 100 to analogWrite
   where 100% = Maximum Value for resolution ranges 8 bits to 16 bits
   Duty cycle scaled and sent to analogWrite and error code noted to pass up

   MUST ALWAYS get what current maximum value for CURRENT resolution as this
   can be changed between calls by other parts of application

   Changed to NOT use expensive and inaccurate floating point
*/
uint8_t IFX9201::DutyCycleToanlogWrite( uint8_t duty_cycle )
{
uint8_t ret = IFX9201__NO_ERROR;
int16_t val;

if( duty_cycle > IFX9201__MAX_DUTY_CYCLE )
  ret = IFX9201__ILLEGAL_DUTY_CYCLE;
else
  {
  // val = analogWrite( m_PWM, (uint16_t)( ( (uint32_t)duty_cycle * getAnalogWriteMaximum( ) ) / 100u ) );
  // if( val == -1 )
  //   ret = IFX9201__ILLEGAL_DUTY_VALUE;
  // else
  //   if( val == -2 )
  //     ret = IFX9201__ILLEGAL_PWM_PIN;
  }
return ret;
}
