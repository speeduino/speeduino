/*
  Speeduino - Simple engine management for the Arduino Mega 2560 platform
  Copyright (C) Josh Stewart
  A full copy of the license may be found in the projects root directory
*/

#include <avr/pgmspace.h>
#include "globals.h"
#include "utilities.h"
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
  if(rawPin > BOARD_MAX_DIGITAL_PINS) { outputPin = A8 + (outputPin - BOARD_MAX_DIGITAL_PINS - 1); }

  return outputPin;
}
//Translates an pin number (0 - 22) to the relevant Ax pin reference.
//This is required as some ARM chips do not have all analog pins in order (EG pin A15 != A14 + 1)
byte pinTranslateAnalog(byte rawPin)
{
  byte outputPin = rawPin;
  switch(rawPin)
  {
    case 0: outputPin = A0; break;
    case 1: outputPin = A1; break;
    case 2: outputPin = A2; break;
    case 3: outputPin = A3; break;
    case 4: outputPin = A4; break;
    case 5: outputPin = A5; break;
    case 6: outputPin = A6; break;
    case 7: outputPin = A7; break;
    case 8: outputPin = A8; break;
    case 9: outputPin = A9; break;
    case 10: outputPin = A10; break;
    case 11: outputPin = A11; break;
    case 12: outputPin = A12; break;
    case 13: outputPin = A13; break;
  #if BOARD_MAX_ADC_PINS >= 14
      case 14: outputPin = A14; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 15
      case 15: outputPin = A15; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 16
      case 16: outputPin = A16; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 17
      case 17: outputPin = A17; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 18
      case 18: outputPin = A18; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 19
      case 19: outputPin = A19; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 20
      case 20: outputPin = A20; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 21
      case 21: outputPin = A21; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 22
      case 22: outputPin = A22; break;
    #endif
  }

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

  //This sucks (again) for all the 3D map pages that have to have a translation performed
  switch(pageNo)
  {
    case veMapPage:
      //Confirmed working
      raw_value = getPageValue(veMapPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[veMapPage]; x++)
      //for(uint16_t x=1; x< 288; x++)
      {
        raw_value = getPageValue(veMapPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    case veSetPage:
      //Confirmed working
      pnt_configPage = &configPage2; //Create a pointer to Page 1 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage2) );
      break;

    case ignMapPage:
      //Confirmed working
      raw_value = getPageValue(ignMapPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[ignMapPage]; x++)
      {
        raw_value = getPageValue(ignMapPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    case ignSetPage:
      //Confirmed working
      pnt_configPage = &configPage4; //Create a pointer to Page 4 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage4) );
      break;

    case afrMapPage:
      //Confirmed working
      raw_value = getPageValue(afrMapPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[afrMapPage]; x++)
      {
        raw_value = getPageValue(afrMapPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    case afrSetPage:
      //Confirmed working
      pnt_configPage = &configPage6; //Create a pointer to Page 4 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage6) );
      break;

    case boostvvtPage:
      //Confirmed working
      raw_value = getPageValue(boostvvtPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[boostvvtPage]; x++)
      {
        raw_value = getPageValue(boostvvtPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    case seqFuelPage:
      //Confirmed working
      raw_value = getPageValue(seqFuelPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[seqFuelPage]; x++)
      {
        raw_value = getPageValue(seqFuelPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    case canbusPage:
      //Confirmed working
      pnt_configPage = &configPage9; //Create a pointer to Page 9 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage9) );
      break;

    case warmupPage:
      //Confirmed working
      pnt_configPage = &configPage10; //Create a pointer to Page 10 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage10) );
      break;

    case fuelMap2Page:
      //Confirmed working
      raw_value = getPageValue(fuelMap2Page, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[fuelMap2Page]; x++)
      //for(uint16_t x=1; x< 288; x++)
      {
        raw_value = getPageValue(fuelMap2Page, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    case wmiMapPage:
      //Confirmed working
      raw_value = getPageValue(wmiMapPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[wmiMapPage]; x++)
      {
        raw_value = getPageValue(wmiMapPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;
      
    case progOutsPage:
      //Confirmed working
      pnt_configPage = &configPage13; //Create a pointer to Page 10 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage13) );
      break;
    
    case ignMap2Page:
      //Confirmed working
      raw_value = getPageValue(ignMap2Page, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[ignMap2Page]; x++)
      {
        raw_value = getPageValue(ignMap2Page, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    default:
      CRC32_val = 0;
      break;
  }
  
  return CRC32_val;
}

//*********************************************************************************************************************************************************************************
void initialiseProgrammableIO()
{
  for (uint8_t y = 0; y < sizeof(configPage13.outputPin); y++)
  {
    if ( (configPage13.outputPin[y] > 0) && (configPage13.outputPin[y] < BOARD_MAX_DIGITAL_PINS) )
    {
      if ( !pinIsUsed(configPage13.outputPin[y]) )
      {
        pinMode(configPage13.outputPin[y], OUTPUT);
        digitalWrite(configPage13.outputPin[y], (configPage13.outputInverted & (1U << y)));
        BIT_SET(pinIsValid, y);
      }
    }
  }
}

void checkProgrammableIO()
{
  int16_t data, data2;
  bool firstCheck, secondCheck;

  for (uint8_t y = 0; y < sizeof(configPage13.outputPin); y++)
  {
    firstCheck = false;
    secondCheck = false;
    if ( BIT_CHECK(pinIsValid, y) ) //if outputPin == 0 it is disabled
    { 
      //byte theIndex = configPage13.firstDataIn[y];
      data = ProgrammableIOGetData(configPage13.firstDataIn[y]);
      data2 = configPage13.firstTarget[y];

      if ( (configPage13.operation[y].firstCompType == COMPARATOR_EQUAL) && (data == data2) ) { firstCheck = true; }
      else if ( (configPage13.operation[y].firstCompType == COMPARATOR_NOT_EQUAL) && (data != data2) ) { firstCheck = true; }
      else if ( (configPage13.operation[y].firstCompType == COMPARATOR_GREATER) && (data > data2) ) { firstCheck = true; }
      else if ( (configPage13.operation[y].firstCompType == COMPARATOR_GREATER_EQUAL) && (data >= data2) ) { firstCheck = true; }
      else if ( (configPage13.operation[y].firstCompType == COMPARATOR_LESS) && (data < data2) ) { firstCheck = true; }
      else if ( (configPage13.operation[y].firstCompType == COMPARATOR_LESS_EQUAL) && (data <= data2) ) { firstCheck = true; }

      if (configPage13.operation[y].bitwise != BITWISE_DISABLED)
      {
        if ( configPage13.secondDataIn[y] < LOG_ENTRY_SIZE ) //Failsafe check
        {
          data = ProgrammableIOGetData(configPage13.secondDataIn[y]);
          data2 = configPage13.secondTarget[y];
          
          if ( (configPage13.operation[y].secondCompType == COMPARATOR_EQUAL) && (data == data2) ) { secondCheck = true; }
          else if ( (configPage13.operation[y].secondCompType == COMPARATOR_NOT_EQUAL) && (data != data2) ) { secondCheck = true; }
          else if ( (configPage13.operation[y].secondCompType == COMPARATOR_GREATER) && (data > data2) ) { secondCheck = true; }
          else if ( (configPage13.operation[y].secondCompType == COMPARATOR_GREATER_EQUAL) && (data >= data2) ) { secondCheck = true; }
          else if ( (configPage13.operation[y].secondCompType == COMPARATOR_LESS) && (data < data2) ) { secondCheck = true; }
          else if ( (configPage13.operation[y].secondCompType == COMPARATOR_LESS_EQUAL) && (data <= data2) ) { secondCheck = true; }

          if (configPage13.operation[y].bitwise == BITWISE_AND) { firstCheck &= secondCheck; }
          if (configPage13.operation[y].bitwise == BITWISE_OR) { firstCheck |= secondCheck; }
          if (configPage13.operation[y].bitwise == BITWISE_XOR) { firstCheck ^= secondCheck; }
        }
      }
      

      if ( (firstCheck == true) && (configPage13.outputDelay[y] != 0) && (configPage13.outputDelay[y] < 255) )
      {
        if ( (ioDelay[y] >= configPage13.outputDelay[y]) )
        {
          if (configPage13.outputPin[y] <= 128) { digitalWrite(configPage13.outputPin[y], (configPage13.outputInverted & (1U << y)) ^ firstCheck); }
        }
        else { ioDelay[y]++; }
      }
      else
      {
        if ( configPage13.outputPin[y] <= 128 ) { digitalWrite(configPage13.outputPin[y], (configPage13.outputInverted & (1U << y)) ^ firstCheck); }
        if ( firstCheck == false ) { ioDelay[y] = 0; }
      }
      if ( firstCheck == true ) { BIT_SET(currentStatus.outputsStatus, y); }
      else { BIT_CLEAR(currentStatus.outputsStatus, y); }
    }
    else { BIT_CLEAR(currentStatus.outputsStatus, y); }
  }
}

int16_t ProgrammableIOGetData(uint16_t index)
{
  int16_t result;
  uint8_t x;
  if ( index < LOG_ENTRY_SIZE )
  {
    
    for(x = 0; x<sizeof(fsIntIndex); x++)
    {
      if (fsIntIndex[x] == index) { break; }
    }
    if (x >= sizeof(fsIntIndex)) { result = getStatusEntry(index); }
    else { result = word(getStatusEntry(index+1), getStatusEntry(index)); }
    

    //Special cases for temperatures
    if( (index == 6) || (index == 7) ) { result -= CALIBRATION_TEMPERATURE_OFFSET; }
  }
  else { result = -1; } //Index is bigger than fullStatus array
  return result;
}