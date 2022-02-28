/** IFX9201.h - Library for Arduino to control the IFX9201 H-Bridge.
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
/*!	\file IFX9201.h
 *	\brief This file has to be included in projects that use Infineon's DC Motor Control Shield with IFX9201
 */
#ifndef IFX9201_H
#define IFX9201_H

#include <SPI.h>
#include "util/IFX9201_conf.h"

//!	\brief	Class that represents an IFX9201 H-bridge
class IFX9201
{
	public:

		typedef enum IFX9201Mode{
			IFX9201Mode_PWM = 0x00u,
			IFX9201Mode_SPI = 0x01u
		}IFX9201Mode_t;

		//! \brief Standard constructor with default pin assignment
		IFX9201( );

		//! \brief Enables and initializes the IFX9201 for use over SPI with custom control pins
		void begin( SPIClass &bus, uint8_t pinSlaveSelect, uint8_t pinDirection, uint8_t pinPWM, uint8_t pinDisable );

		//! \brief Enables and initializes the IFX9201 with PWM control pins
		void begin( uint8_t pinDirection, uint8_t pinPWM, uint8_t pinDisable );

		//! \brief Function to set PWM frequency for the IFX9201
		uint8_t setPWMFreqency( uint16_t );

		//! \brief Functions to rotate the motor controlled by IFX9201 forwards
		uint8_t forwards( );
		uint8_t forwards( uint8_t );

		//! \brief Functions to rotate the motor controlled by IFX9201 backwards
		uint8_t backwards( );
		uint8_t backwards( uint8_t );

		//! \brief Function stop the motor controlled by IFX9201
		uint8_t stop( );

		//! \brief Deactivates all outputs and disables the IFX9201
		void end( );

		//! \brief Get content of DIA Reg (only in SPI Mode)
		uint8_t getDIAReg( );
		//! \brief Get content of REV Reg (only in SPI Mode)
		uint8_t getREVReg( );
		//! \brief Get content of CTRL Reg (only in SPI Mode)
		uint8_t getCTRLReg( );

		//! \brief Set value of CTRL Reg (only in SPI Mode)
		uint8_t setCTRLReg( uint8_t );

		//! \brief Reset value of DIA Reg (only in SPI Mode)
		void resetDIAReg( );

	private:
		IFX9201Mode_t m_Mode;
		uint16_t m_PWMFreq;
		SPIClass *m_bus;
		SPISettings m_SPIsettings;

		uint8_t m_SlaveSelect = 10;
		uint8_t m_Disable = 11;
		uint8_t m_Direction = 6;
		uint8_t m_PWM = 10;

		//! \brief Enables and initializes the IFX9201
		void begin( );

		//! \brief Transfer a byte using SPI
		uint8_t SPItransfer( uint8_t );

		//! \brief Converts a duty cycle to suitable value for the analogWrite function
		uint8_t DutyCycleToanlogWrite( uint8_t );
};
#endif	// IFX9201_H