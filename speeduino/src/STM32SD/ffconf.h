/*
 * @file    ffconf.h
 * @brief   Include header file to match Arduino library format
 */
#ifdef STM32F407xx
#ifndef _ARDUINO_FFCONF_H
#define _ARDUINO_FFCONF_H

#include "stm32_def.h"
#include "bsp_sd.h"

/* FatFs specific configuration options. */
#if __has_include("ffconf_custom.h")
#include "ffconf_custom.h"
#else
#if _FATFS == 68300
#include "ffconf_default_68300.h"
#else
#include "ffconf_default_32020.h"
#endif
#endif
#endif /* _ARDUINO_FFCONF_H */
#endif