/** \file logger.h
 * @brief File for generating log files and meta data
 * @author Josh Stewart
 * 
 * This file contains functions for creating a log file for use eith by TunerStudio directly or to be written to an SD card
 * 
 */

#ifndef LOGGER_H
#define LOGGER_H

#ifndef UNIT_TEST // Scope guard for unit testing
  #define LOG_ENTRY_SIZE      117 /**< The size of the live data packet. This MUST match ochBlockSize setting in the ini file */
  #define SD_LOG_ENTRY_SIZE   117 /**< The size of the live data packet used by the SD car.*/
#else
  #define LOG_ENTRY_SIZE      1 /**< The size of the live data packet. This MUST match ochBlockSize setting in the ini file */
  #define SD_LOG_ENTRY_SIZE   1 /**< The size of the live data packet used by the SD car.*/
#endif

void createLog(uint8_t *array);
void createSDLog(uint8_t *array);

#endif
