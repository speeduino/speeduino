/* Arduino SPIMemory Library v.3.2.0
 * Copyright (C) 2017 by Prajwal Bhattaram
 * Created by Prajwal Bhattaram - 14/11/2016
 * Modified by Prajwal Bhattaram - 20/04/2018
 *
 * This file is part of the Arduino SPIMemory Library. This library is for
 * Winbond NOR flash memory modules. In its current form it enables reading
 * and writing individual data variables, structs and arrays from and to various locations;
 * reading and writing pages; continuous read functions; sector, block and chip erase;
 * suspending and resuming programming/erase and powering down for low power operation.
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License v3.0
 * along with the Arduino SPIMemory Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "diagnostics.h"

//Subfunctions for troubleshooting function
void Diagnostics::_printErrorCode(void) {
  Serial.print("Error code: 0x");
  if (errorcode < 0x10) {
    Serial.print("0");
  }
  Serial.println(errorcode, HEX);
}

void Diagnostics::_printSupportLink(void) {
  Serial.print("If this does not help resolve/clarify this issue, ");
  Serial.println("please raise an issue at http://www.github.com/Marzogh/SPIMemory/issues with the details of what your were doing when this error occurred");
}
//Troubleshooting function. Called when #ifdef RUNDIAGNOSTIC is uncommented at the top of SPIMemory.h.
void Diagnostics::troubleshoot(uint8_t _code, bool printoverride) {
  bool _printoverride;
  errorcode = _code;
#if defined (RUNDIAGNOSTIC) && !defined (ARDUINO_ARCH_AVR)
  _printoverride = true;
#elif defined (RUNDIAGNOSTIC) && defined (ARDUINO_ARCH_AVR)
  _printErrorCode();
#endif
#if !defined (RUNDIAGNOSTIC)
  _printoverride = printoverride;
#endif
  if (_printoverride) {
  #if defined (ARDUINO_ARCH_AVR)
    _printErrorCode();
  #else
    switch (_code) {
      case SUCCESS:
      Serial.println("Function executed successfully");
      break;

      case NORESPONSE:
      Serial.println("Check your wiring. Flash chip is non-responsive.");
      break;

      case CALLBEGIN:
      Serial.println("*constructor_of_choice*.begin() was not called in void setup()");
      break;

      case UNKNOWNCHIP:
      Serial.println("Unable to identify chip. Are you sure this chip is supported?");
      Serial.println("Chip details:");
      break;

      case UNKNOWNCAP:
      Serial.println("Unable to identify capacity. Is this chip officially supported? If not, please define a `CAPACITY` constant and include it in flash.begin(CAPACITY).");
      break;

      case CHIPBUSY:
      Serial.println("Chip is busy.");
      Serial.println("Make sure all pins have been connected properly");
      break;

      case OUTOFBOUNDS:
      Serial.println("Page overflow has been disabled and the address called exceeds the memory");
      break;

      case CANTENWRITE:
      Serial.println("Unable to Enable Writing to chip.");
      Serial.println("Please make sure the HOLD & WRITEPROTECT pins are pulled up to VCC");
      break;

      case PREVWRITTEN:
      Serial.println("This sector already contains data.");
      Serial.println("Please make sure the sectors being written to are erased.");
      break;

      case LOWRAM:
      Serial.println("You are running low on SRAM. Please optimise your program for better RAM usage");
      /*#if defined (ARDUINO_ARCH_SAM)
        Serial.print("Current Free SRAM: ");
        Serial.println(freeRAM());
      #endif*/
      break;

      case UNSUPPORTEDFUNC:
      Serial.println("This function is not supported by the flash memory hardware.");
      break;

      case SYSSUSPEND:
      Serial.println("Unable to suspend/resume operation.");
      break;

      case ERRORCHKFAIL:
      Serial.println("Write Function has failed errorcheck.");
      break;

      case UNABLETO4BYTE:
      Serial.println("Unable to enable 4-byte addressing.");
      break;

      case UNABLETO3BYTE:
      Serial.println("Unable to disable 4-byte addressing.");
      break;

      case CHIPISPOWEREDDOWN:
      Serial.println("The Flash chip is currently powered down.");
      break;

      case NOSFDP:
      Serial.println("The Flash chip does not support SFDP.");
      break;

      case NOSFDPERASEPARAM:
      Serial.println("Unable to read Erase Parameters from chip. Reverting to library defaults.");
      break;

      case NOSFDPERASETIME:
      Serial.println("Unable to read erase times from flash memory. Reverting to library defaults.");
      break;

      case NOSFDPPROGRAMTIMEPARAM:
      Serial.println("Unable to read program times from flash memory. Reverting to library defaults.");
      break;

      default:
      Serial.println("Unknown error");
      break;
    }
    if (_code == ERRORCHKFAIL || _code == CANTENWRITE || _code == UNKNOWNCHIP || _code == NORESPONSE) {
      _printSupportLink();
    }
  #endif
  }
}

Diagnostics diagnostics; // default instantiation of Diagnostics object
