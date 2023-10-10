/** \file logger.h
 * @brief File for generating log files and meta data
 * @author Josh Stewart
 * 
 * This file contains functions for creating a log file for use with by TunerStudio directly or to be written to an SD card
 * 
 */

#ifndef LOGGER_H
#define LOGGER_H

#include "globals.h" // Needed for FPU_MAX_SIZE

#ifndef UNIT_TEST // Scope guard for unit testing
  #define LOG_ENTRY_SIZE      127 /**< The size of the live data packet. This MUST match ochBlockSize setting in the ini file */
#else
  #define LOG_ENTRY_SIZE      1 /**< The size of the live data packet. This MUST match ochBlockSize setting in the ini file */
#endif

byte getTSLogEntry(uint16_t byteNum);
int16_t getReadableLogEntry(uint16_t logIndex);
#if defined(FPU_MAX_SIZE) && FPU_MAX_SIZE >= 32 //cppcheck-suppress misra-c2012-20.9
  float getReadableFloatLogEntry(uint16_t logIndex);
#endif
uint8_t getLegacySecondarySerialLogEntry(uint16_t byteNum);
bool is2ByteEntry(uint8_t key);

void startToothLogger(void);
void stopToothLogger(void);

void startCompositeLogger(void);
void stopCompositeLogger(void);

void startCompositeLoggerTertiary(void);
void stopCompositeLoggerTertiary(void);

void startCompositeLoggerCams(void);
void stopCompositeLoggerCams(void);

#endif
