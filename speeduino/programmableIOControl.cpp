/** @file
 * Custom Programmable I/O.
 * The config related to Programmable I/O is found on page13 (of type @ref config13).
 */
#include "programmableIOControl.h"
#include "programmableIOControl_internals.h"
#include "logger.h"
#include "units.h"
#include "unit_testing.h"

using namespace programmableIOControl_details;

TESTABLE_STATIC programmableio_channel_t channels[_countof(config13::outputPin)];

static programmableio_channel_t toChannel(const programmableOutputRule& rule) {
  programmableio_channel_t channel;
  channel.pinIsValid = rule.outputPin>0 && (rule.isCascadeRule() || !pinIsUsed(rule.outputPin));
  return channel;
}

void __attribute__((optimize("Os"))) initialiseProgrammableIO(statuses& current, const config13& page13)
{
  for (uint8_t y = 0; y < _countof(channels); y++)
  {
    programmableOutputRule rule(page13, y);
    channels[y] = toChannel(rule);
    BIT_WRITE(current.outputsStatus, y, channels[y].pinIsValid && rule.outputInverted); 
    if (channels[y].pinIsValid && rule.isPhysicalPin()) 
    {
      pinMode(rule.outputPin, OUTPUT);
      digitalWrite(rule.outputPin, rule.outputInverted);
    }
  }
}

TESTABLE_STATIC int16_t getComparisonData(uint8_t request, int16_t (*getData)(uint16_t index))
{
  int16_t data = 0;
  if ( request >= REUSE_RULES )
  {
    request -= REUSE_RULES;
    if ( request <= _countof(channels) ) 
    { 
      data = channels[request].currentRuleStatus; 
    }
  }
  else 
  { 
    data = getData(request); 
  }

  return data;
}

TESTABLE_STATIC bool evaluateComparisonOp(uint8_t compType, int16_t lhs, int16_t rhs)
{
  switch (compType) {
    case COMPARATOR_EQUAL: return lhs == rhs;
    case COMPARATOR_NOT_EQUAL: return lhs != rhs;
    case COMPARATOR_GREATER: return lhs > rhs;
    case COMPARATOR_GREATER_EQUAL: return lhs >= rhs;
    case COMPARATOR_LESS: return lhs < rhs;
    case COMPARATOR_LESS_EQUAL: return lhs <= rhs;
    case COMPARATOR_AND: return (lhs & rhs) != 0;
    case COMPARATOR_XOR: return (lhs ^ rhs) != 0;
    default: return false; // Invalid comparator type
  }
}

TESTABLE_STATIC bool evaluateBitwiseOp(uint8_t compType, bool lhs, bool rhs)
{
  switch (compType) {
    case BITWISE_AND: return (lhs & rhs) != 0;
    case BITWISE_OR: return (lhs | rhs) != 0;
    case BITWISE_XOR: return (lhs ^ rhs) != 0;
    default: return false; // Invalid bitwise operator type
  }
}

/** Check all (8) programmable I/O:s and carry out action on output pin as needed.
 * Compare 2 (16 bit) vars in a way configured by @ref cmpOperation (see also @ref config13.operation).
 * Use ProgrammableIOGetData() to get 2 vars to compare.
 * Skip all programmable I/O:s where output pin is set 0 (meaning: not programmed).
 */
TESTABLE_STATIC void checkProgrammableIO(statuses& current, const config13& page13, int16_t (*getData)(uint16_t index))
{
  for (uint8_t y = 0; y < _countof(channels); y++)
  {
    if ( channels[y].pinIsValid )
    {
      programmableOutputRule rule(page13, y);
      bool firstCheck = evaluateComparisonOp(rule.operation.firstCompType, getComparisonData(rule.firstDataIn, getData), rule.firstTarget);

      if ((rule.operation.bitwise != BITWISE_DISABLED) && (rule.secondDataIn <= (REUSE_RULES + _countof(channels))) ) //Failsafe check
      {
        bool secondCheck = evaluateComparisonOp(rule.operation.secondCompType, getComparisonData(rule.secondDataIn, getData), rule.secondTarget);
        firstCheck = evaluateBitwiseOp(rule.operation.bitwise, firstCheck, secondCheck);
      }

      //If the limiting time is active(>0) and using maximum time
      if (BIT_CHECK(page13.kindOfLimiting, y))
      {
        if(firstCheck)
        {
          if ((page13.outputTimeLimit[y] != 0) && (channels[y].ioOutDelay >= page13.outputTimeLimit[y])) { firstCheck = false; } //Time has counted, disable the output
        }
        else
        {
          //Released before Maximum time, set delay to maximum to flip the output next
          if(BIT_CHECK(current.outputsStatus, y)) { channels[y].ioOutDelay = page13.outputTimeLimit[y]; }
          else { channels[y].ioOutDelay = 0; } //Reset the counter for next time
        }
      }

      if (firstCheck == true)
      {
        if (channels[y].ioDelay >= page13.outputDelay[y])
        {
          bool bitStatus = BIT_CHECK(page13.outputInverted, y) ^ firstCheck;
          if (BIT_CHECK(current.outputsStatus, y) && (channels[y].ioOutDelay < page13.outputTimeLimit[y])) { channels[y].ioOutDelay++; }
          if (page13.outputPin[y] < 128) { digitalWrite(page13.outputPin[y], bitStatus); }
          else { channels[y].currentRuleStatus = bitStatus; }
          BIT_WRITE(current.outputsStatus, y, bitStatus);
        }
        else { channels[y].ioDelay++; }
      }
      else
      {
        if (channels[y].ioOutDelay >= page13.outputTimeLimit[y])
        {
          bool bitStatus = BIT_CHECK(page13.outputInverted, y) ^ firstCheck;
          if (page13.outputPin[y] < 128) { digitalWrite(page13.outputPin[y], bitStatus); }
          else { channels[y].currentRuleStatus = bitStatus; }
          BIT_WRITE(current.outputsStatus, y, bitStatus);
          if(!BIT_CHECK(page13.kindOfLimiting, y)) { channels[y].ioOutDelay = 0; }
        }
        else { channels[y].ioOutDelay++; }

        channels[y].ioDelay = 0;
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