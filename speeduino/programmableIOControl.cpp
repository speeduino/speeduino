/** @file
 * Custom Programmable I/O.
 * The config related to Programmable I/O is found on page13 (of type @ref config13).
 */
#include "programmableIOControl.h"
#include "logger.h"
#include "units.h"
#include "unit_testing.h"

TESTABLE_STATIC uint8_t ioDelay[_countof(config13::outputPin)];
TESTABLE_STATIC uint8_t ioOutDelay[_countof(config13::outputPin)];
TESTABLE_STATIC uint8_t pinIsValid = 0;
TESTABLE_STATIC uint8_t currentRuleStatus = 0;

//*********************************************************************************************************************************************************************************
void __attribute__((optimize("Os"))) initialiseProgrammableIO(statuses& current, const config13& page13)
{
  uint8_t outputPin;
  for (uint8_t y = 0; y < _countof(ioDelay); y++)
  {
    ioDelay[y] = 0;
    ioOutDelay[y] = 0;
    outputPin = page13.outputPin[y];
    if (outputPin > 0)
    {
      if ( outputPin >= 128 ) //Cascate rule usage
      {
        BIT_WRITE(current.outputsStatus, y, BIT_CHECK(page13.outputInverted, y));
        BIT_SET(pinIsValid, y);
      }
      else if ( !pinIsUsed(outputPin) )
      {
        pinMode(outputPin, OUTPUT);
        digitalWrite(outputPin, BIT_CHECK(page13.outputInverted, y));
        BIT_WRITE(current.outputsStatus, y, BIT_CHECK(page13.outputInverted, y));
        BIT_SET(pinIsValid, y);
      }
      else { BIT_CLEAR(pinIsValid, y); }
    }
  }
}

/** Check all (8) programmable I/O:s and carry out action on output pin as needed.
 * Compare 2 (16 bit) vars in a way configured by @ref cmpOperation (see also @ref config13.operation).
 * Use ProgrammableIOGetData() to get 2 vars to compare.
 * Skip all programmable I/O:s where output pin is set 0 (meaning: not programmed).
 */
TESTABLE_STATIC void checkProgrammableIO(statuses& current, const config13& page13, int16_t (*getData)(uint16_t index))
{
  int16_t data, data2;
  uint8_t dataRequested;
  bool firstCheck, secondCheck;

  for (uint8_t y = 0; y < _countof(ioDelay); y++)
  {
    firstCheck = false;
    secondCheck = false;
    if ( BIT_CHECK(pinIsValid, y) ) //if outputPin == 0 it is disabled
    {
      dataRequested = page13.firstDataIn[y];
      if ( dataRequested > 239U ) //Somehow using 239 uses 9 bytes of RAM, why??
      {
        dataRequested -= REUSE_RULES;
        if ( dataRequested <= sizeof(page13.outputPin) ) { data = BIT_CHECK(currentRuleStatus, dataRequested); }
        else { data = 0; }
      }
      else { data = getData(dataRequested); }
      data2 = page13.firstTarget[y];

      if ( (page13.operation[y].firstCompType == COMPARATOR_EQUAL) && (data == data2) ) { firstCheck = true; }
      else if ( (page13.operation[y].firstCompType == COMPARATOR_NOT_EQUAL) && (data != data2) ) { firstCheck = true; }
      else if ( (page13.operation[y].firstCompType == COMPARATOR_GREATER) && (data > data2) ) { firstCheck = true; }
      else if ( (page13.operation[y].firstCompType == COMPARATOR_GREATER_EQUAL) && (data >= data2) ) { firstCheck = true; }
      else if ( (page13.operation[y].firstCompType == COMPARATOR_LESS) && (data < data2) ) { firstCheck = true; }
      else if ( (page13.operation[y].firstCompType == COMPARATOR_LESS_EQUAL) && (data <= data2) ) { firstCheck = true; }
      else if ( (page13.operation[y].firstCompType == COMPARATOR_AND) && ((data & data2) != 0) ) { firstCheck = true; }
      else if ( (page13.operation[y].firstCompType == COMPARATOR_XOR) && ((data ^ data2) != 0) ) { firstCheck = true; }

      if (page13.operation[y].bitwise != BITWISE_DISABLED)
      {
        dataRequested = page13.secondDataIn[y];
        if ( dataRequested <= (REUSE_RULES + sizeof(page13.outputPin)) ) //Failsafe check
        {
          if ( dataRequested > 239U ) //Somehow using 239 uses 9 bytes of RAM, why??
          {
            dataRequested -= REUSE_RULES;
            data = BIT_CHECK(currentRuleStatus, dataRequested);
          }
          else { data = getData(dataRequested); }
          data2 = page13.secondTarget[y];
          
          if ( (page13.operation[y].secondCompType == COMPARATOR_EQUAL) && (data == data2) ) { secondCheck = true; }
          else if ( (page13.operation[y].secondCompType == COMPARATOR_NOT_EQUAL) && (data != data2) ) { secondCheck = true; }
          else if ( (page13.operation[y].secondCompType == COMPARATOR_GREATER) && (data > data2) ) { secondCheck = true; }
          else if ( (page13.operation[y].secondCompType == COMPARATOR_GREATER_EQUAL) && (data >= data2) ) { secondCheck = true; }
          else if ( (page13.operation[y].secondCompType == COMPARATOR_LESS) && (data < data2) ) { secondCheck = true; }
          else if ( (page13.operation[y].secondCompType == COMPARATOR_LESS_EQUAL) && (data <= data2) ) { secondCheck = true; }
          else if ( (page13.operation[y].secondCompType == COMPARATOR_AND) && ((data & data2) != 0) ) { secondCheck = true; }
          else if ( (page13.operation[y].secondCompType == COMPARATOR_XOR) && ((data ^ data2) != 0) ) { secondCheck = true; }

          if (page13.operation[y].bitwise == BITWISE_AND) { firstCheck &= secondCheck; }
          if (page13.operation[y].bitwise == BITWISE_OR) { firstCheck |= secondCheck; }
          if (page13.operation[y].bitwise == BITWISE_XOR) { firstCheck ^= secondCheck; }
        }
      }

      //If the limiting time is active(>0) and using maximum time
      if (BIT_CHECK(page13.kindOfLimiting, y))
      {
        if(firstCheck)
        {
          if ((page13.outputTimeLimit[y] != 0) && (ioOutDelay[y] >= page13.outputTimeLimit[y])) { firstCheck = false; } //Time has counted, disable the output
        }
        else
        {
          //Released before Maximum time, set delay to maximum to flip the output next
          if(BIT_CHECK(current.outputsStatus, y)) { ioOutDelay[y] = page13.outputTimeLimit[y]; }
          else { ioOutDelay[y] = 0; } //Reset the counter for next time
        }
      }

      if (firstCheck == true)
      {
        if (ioDelay[y] >= page13.outputDelay[y])
        {
          bool bitStatus = BIT_CHECK(page13.outputInverted, y) ^ firstCheck;
          if (BIT_CHECK(current.outputsStatus, y) && (ioOutDelay[y] < page13.outputTimeLimit[y])) { ioOutDelay[y]++; }
          if (page13.outputPin[y] < 128) { digitalWrite(page13.outputPin[y], bitStatus); }
          else { BIT_WRITE(currentRuleStatus, y, bitStatus); }
          BIT_WRITE(current.outputsStatus, y, bitStatus);
        }
        else { ioDelay[y]++; }
      }
      else
      {
        if (ioOutDelay[y] >= page13.outputTimeLimit[y])
        {
          bool bitStatus = BIT_CHECK(page13.outputInverted, y) ^ firstCheck;
          if (page13.outputPin[y] < 128) { digitalWrite(page13.outputPin[y], bitStatus); }
          else { BIT_WRITE(currentRuleStatus, y, bitStatus); }
          BIT_WRITE(current.outputsStatus, y, bitStatus);
          if(!BIT_CHECK(page13.kindOfLimiting, y)) { ioOutDelay[y] = 0; }
        }
        else { ioOutDelay[y]++; }

        ioDelay[y] = 0;
      }
    }
  }
}

// LCOV_EXCL_START
void checkProgrammableIO(statuses& current, const config13& page13)
{
  checkProgrammableIO(current, page13, ProgrammableIOGetData);
}
// LCOV_EXCL_STOP

/** Get single I/O data var (from current) for comparison.
 * @param index - Field index/number (?)
 * @return 16 bit (int) result
 */
TESTABLE_STATIC int16_t ProgrammableIOGetData(uint16_t index, byte (*pGetLogEntry)(uint16_t byteNum))
{
  int16_t result;
  if ( index < LOG_ENTRY_SIZE )
  {
    if(is2ByteEntry(index)) { result = word(pGetLogEntry(index+1), pGetLogEntry(index)); }
    else { result = pGetLogEntry(index); }
    
    //Special cases for temperatures
    if( (index == 6) || (index == 7) ) { result = temperatureRemoveOffset(result); }
  }
  else if ( index == 239U ) { result = (int16_t)max((uint32_t)runSecsX10, (uint32_t)32768); } //STM32 used std lib
  else { result = -1; } //Index is bigger than fullStatus array
  return result;
}

// LCOV_EXCL_START
int16_t ProgrammableIOGetData(uint16_t index)
{
  return ProgrammableIOGetData(index, getTSLogEntry);
}
// LCOV_EXCL_STOP