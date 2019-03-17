/*
  Speeduino - Simple engine management for the Arduino Mega 2560 platform
  Copyright (C) Josh Stewart
  A full copy of the license may be found in the projects root directory
*/


/*
  Returns how much free dynamic memory exists (between heap and stack)
  This function is one big MISRA violation. MISRA advisories forbid directly poking at memory addresses, however there is no other way of determining heap size on embedded systems.
*/
#include <avr/pgmspace.h>
#include "globals.h"
#include "utils.h"
#include "decoders.h"
#include "comms.h"
#include "src/FastCRC/FastCRC.h"

FastCRC32 CRC32;

//This function performs a translation between the pin list that appears in TS and the actual pin numbers
//For the digital IO, this will simply return the same number as the rawPin value as those are mapped directly.
//For analog pins, it will translate them into the currect internal pin number
byte pinTranslate(byte rawPin)
{
  byte outputPin = rawPin;
  if(rawPin > BOARD_DIGITAL_GPIO_PINS) { outputPin = A8 + (outputPin - BOARD_DIGITAL_GPIO_PINS - 1); }

  return outputPin;
}


void setResetControlPinState()
{
  BIT_CLEAR(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);

  /* Setup reset control initial state */
  switch (resetControl)
  {
    case RESET_CONTROL_PREVENT_WHEN_RUNNING:
      /* Set the reset control pin LOW and change it to HIGH later when we get sync. */
      digitalWrite(pinResetControl, LOW);
      BIT_CLEAR(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);
      break;
    case RESET_CONTROL_PREVENT_ALWAYS:
      /* Set the reset control pin HIGH and never touch it again. */
      digitalWrite(pinResetControl, HIGH);
      BIT_SET(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);
      break;
    case RESET_CONTROL_SERIAL_COMMAND:
      /* Set the reset control pin HIGH. There currently isn't any practical difference
         between this and PREVENT_ALWAYS but it doesn't hurt anything to have them separate. */
      digitalWrite(pinResetControl, HIGH);
      BIT_CLEAR(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);
      break;
  }
}

/*
Calculates and returns the CRC32 value of a given page of memory
*/
uint32_t calculateCRC32(byte pageNo)
{
  uint32_t CRC32_val;
  byte raw_value;
  void* pnt_configPage;

  //This sucks (again) for thall the 3D map pages that have to have a translation performed
  switch(pageNo)
  {
    case veMapPage:
      raw_value = getPageValue(veMapPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1);
      for(uint16_t x=1; x< sizeof(fuelTable); x++)
      {
        raw_value = getPageValue(veMapPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1);
      }
      break;

    case veSetPage:
      pnt_configPage = &configPage1; //Create a pointer to Page 1 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage1) );
      break;

    case ignMapPage:
      break;

    case ignSetPage:
      pnt_configPage = &configPage2; //Create a pointer to Page 4 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage2) );
      break;

    case afrMapPage:
      break;

    case afrSetPage:
      pnt_configPage = &configPage3; //Create a pointer to Page 4 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage3) );
      break;

    case boostvvtPage:
      break;

    case seqFuelPage:
      break;

    case canbusPage:
      pnt_configPage = &configPage9; //Create a pointer to Page 9 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage9) );
      break;

    case warmupPage:
      pnt_configPage = &configPage10; //Create a pointer to Page 10 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage10) );
      break;

    default:
      break;
  }
  
  return CRC32_val;
}
