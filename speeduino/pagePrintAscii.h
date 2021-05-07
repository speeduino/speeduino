#pragma once
#include <Arduino.h>

/** Send page as ASCII for debugging purposes.
 * Similar to sendPage(), however data is sent in human readable format. Sends page given in @ref pageNum.
 * 
 * This is used for testing only (Not used by TunerStudio) in order to see current map and config data without the need for TunerStudio. 
 */
void printPageAscii(byte pageNum, Print &target);