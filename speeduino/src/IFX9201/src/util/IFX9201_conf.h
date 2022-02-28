/**
 * IFX9201_conf.h - Library for Arduino to control the IFX9201 H-Bridge.
 *
 * The IFX9201 is a general purpose 6A H-Bridge for industrial applications, home appliance and building
 * automation, power tools battery management and medical applications, designed for the control of small
 * DC motors and inductive loads.
 *
 * Have a look at the application note/datasheet for more information.
 *
 * Copyright (c) 2018 Infineon Technologies AG
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

/*!	\file IFX9201_conf.h
 *	\brief This file is automatically included
 */
#ifndef IFX9201_CONF_H
#define IFX9201_CONF_H

/*! \brief IFX9201 default values for PWM mode
 */
#define IFX9201__MAX_PWM_FREQ   20000u
#define IFX9201__MAX_DUTY_CYCLE   100u

#define IFX9201__DEFAULT_DUTY_CYCLE 		50u
#define IFX9201__DEFAULT_PWM_FREQUENCY		10000u

/*! \brief IFX9201 Errors
 */
#define IFX9201__NO_ERROR 					0u
#define IFX9201__MODE_ERROR 				1u
#define IFX9201__SPI_WRITE_ERROR 			2u
#define IFX9201__ILLEGAL_OPERATION_ERROR 	3u
#define IFX9201__ILLEGAL_FREQUENCY       	4u
#define IFX9201__ILLEGAL_PWM_PIN 	        5u
#define IFX9201__ILLEGAL_DUTY_CYCLE         6u
#define IFX9201__ILLEGAL_DUTY_VALUE         7u

/*! \brief IFX9201 CTRL Reg masks
 */
#define IFX9201__CTRL_SPWM_MSK			0x01u
#define IFX9201__CTRL_SDIR_MSK			0x02u
#define IFX9201__CTRL_SEN_MSK			0x04u
#define IFX9201__CTRL_SIN_MSK 			0x08u
#define IFX9201__CTRL_OLDIS_MSK 		0x10u

/*! \brief IFX9201 CTRL data field mask
 */
#define IFX9201__CTRL_DATA_MSK 		0x1Fu

/*! \brief IFX9201 SPI command set
 */
#define IFX9201__RD_DIA  			0x00u
#define IFX9201__RES_DIA 			0x80u
#define IFX9201__RD_REV  			0x20u
#define IFX9201__RD_CTRL 			0x60u
#define IFX9201__WR_CTRL_RD_DIA  	0xD0u
#define IFX9201__WR_CTRL 			0xE0u

#endif