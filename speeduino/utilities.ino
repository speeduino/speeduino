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

uint16_t ioDelay[sizeof(configPage13.outputPin)];
uint8_t pinIsValid = 0;


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


//*********************************************************************************************************************************************************************************
void initialiseProgrammableIO()
{
  for (uint8_t y = 0; y < sizeof(configPage13.outputPin); y++)  {
    if ( (configPage13.outputPin[y] > 0) && 
         (configPage13.outputPin[y] < BOARD_MAX_DIGITAL_PINS) ) {   // Note: no need for former check on '(configPage13.outputPin[y] <= 128)' in checkProgrammableIO()
      if ( !pinIsUsed(configPage13.outputPin[y]) )  {                                                             
        pinMode(configPage13.outputPin[y], OUTPUT);
        BIT_SET(pinIsValid, y);
        procesOutputValueConditionally(y, false );                  // Assume conditions are not met at powering up. 
        if (configPage13.outputDelay[y] >= 255){                    // Note: In 202103 there was a check on outputDelay[y] being smaller than 255. Just making it smaller than 254 is more robust
          configPage13.outputDelay[y] = 254; 
          } 
      }
    }
  }
}

/*
 * This procedure is called every 0.1 sec. It checks the conditions for each active Programmable Output Rule.
 * If the conditions are met and there is no delay needed, an output will be set to reflect the new situation.
 * If the conditions are met and there is a delay needed, a counter will be incremented each next call of this procedure  
 *    until we waited long enough. Then the output will be set to reflect the new situation.
 * If the conditions are no longer met, an output will be set immidiately (!) to reflect the new situation.
 */
void checkProgrammableIO()
{
  bool isEveryConditionFulfilled;

  for (uint8_t y = 0; y < sizeof(configPage13.outputPin); y++)  {
    if ( BIT_CHECK(pinIsValid, y) ) {                                                                    // is Rule enabled (is enabled when outputPin==1)?
      bool isFirstConditionFulfilled = compare (ProgrammableIOGetData(configPage13.firstDataIn[y]),      // changing data
                                                configPage13.firstTarget[y],                             // target data
                                                configPage13.operation[y].firstCompType);                // comparator
      if (configPage13.operation[y].bitwise == BITWISE_DISABLED)   {                                     // are we dealing with one or two enabled conditions?
        isEveryConditionFulfilled = isFirstConditionFulfilled;
      } else {
        bool isSecondConditionFulfilled = compare (ProgrammableIOGetData(configPage13.secondDataIn[y]),  // changing data
                                                   configPage13.secondTarget[y],                         // target data
                                                   configPage13.operation[y].secondCompType);            // comparator
        isEveryConditionFulfilled = combineConditions ( isFirstConditionFulfilled,                       // result first condition
                                                        isSecondConditionFulfilled,                      // result second condition
                                                        configPage13.operation[y].bitwise);              // AND, OR or XOR the conditions, depending on the user wishes
      }
      if ( (isEveryConditionFulfilled ) && (configPage13.outputDelay[y] > 0)) {                          // does this rule qualitfy for delayed response when conditons are met?
        if ( (ioDelay[y] >= configPage13.outputDelay[y]) ) {                                             // qualified! and did we already wait long enough?
          procesOutputValueConditionally(y, true);                                                       // waited long enough!! update io-port en status variables
        } else {
          ioDelay[y]++;                                                                                  // update counter. maybe next iteration.....
        }
      } else {
          if (isEveryConditionFulfilled) {
            procesOutputValueConditionally(y, true);
          } else { 
            procesOutputValueConditionally(y, false);
            ioDelay[y] = 0;
          }
      }
    }
    else {                                                                                             
      BIT_CLEAR(currentStatus.outputsStatus, y);
    }
  }
}

bool compare(int16_t data1, int16_t data2, byte comp) {
  if ( (comp == COMPARATOR_EQUAL)          && (data1 == data2) ) { return true; }
  if ( (comp == COMPARATOR_NOT_EQUAL)      && (data1 != data2) ) { return true; }
  if ( (comp == COMPARATOR_GREATER)        && (data1 >  data2) ) { return true; }
  if ( (comp == COMPARATOR_GREATER_EQUAL)  && (data1 >= data2) ) { return true; }
  if ( (comp == COMPARATOR_LESS)           && (data1 < data2)  ) { return true; }
  if ( (comp == COMPARATOR_LESS_EQUAL)     && (data1 <= data2) ) { return true; }
  return false;   // shouldn't happen
}

bool combineConditions(bool bool1, bool bool2, byte operation) {
  if (operation == BITWISE_AND) { return (bool1 & bool2); }
  if (operation == BITWISE_OR)  { return (bool1 | bool2); }
  if (operation == BITWISE_XOR) { return (bool1 ^ bool2); }
  return false;  // shouldn't happen
}

void procesOutputValueConditionally(uint8_t y, bool isFulfilled) {
  uint8_t valueToSet = (configPage13.outputInverted & (1U << y)) ^ isFulfilled;
  digitalWrite(configPage13.outputPin[y], valueToSet);
  if ( valueToSet == 0  )  {
    BIT_CLEAR(currentStatus.outputsStatus, y);
  }  else  {
    BIT_SET(currentStatus.outputsStatus, y);
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
