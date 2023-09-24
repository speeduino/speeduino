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
#if FPU_MAX_SIZE >= 32
  float getReadableFloatLogEntry(uint16_t logIndex);
#endif
bool is2ByteEntry(uint8_t key);

void startToothLogger(void);
void stopToothLogger(void);

void startCompositeLogger(void);
void stopCompositeLogger(void);

void startCompositeLoggerTertiary(void);
void stopCompositeLoggerTertiary(void);

void startCompositeLoggerCams(void);
void stopCompositeLoggerCams(void);

// This array indicates which index values from the log are 2 byte values
// This array MUST remain in ascending order
// !!!! WARNING: If any value above 255 is required in this array, changes MUST be made to is2ByteEntry() function !!!!
const byte PROGMEM fsIntIndex[] = {4, 14, 17, 22, 26, 28, 33, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 76, 78, 80, 82, 86, 88, 90, 93, 95, 99, 104, 111, 121, 125 };

#endif
