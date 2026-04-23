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

static bool evaluateComparisonOp(const compOperation& operation, int16_t (*getData)(uint16_t index))
{
  return evaluateComparisonOp(operation.opType, getComparisonData(operation.dataIndex, getData), operation.target);
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

static bool isRuleActive(const programmableOutputRule& rule, int16_t (*getData)(uint16_t index))
{
  bool firstCheck = evaluateComparisonOp(rule.firstOp, getData);

  if ((rule.opCompare != BITWISE_DISABLED) && (rule.secondOp.dataIndex <= (REUSE_RULES + _countof(channels))) ) //Failsafe check
  {
    bool secondCheck = evaluateComparisonOp(rule.secondOp, getData);
    firstCheck = evaluateBitwiseOp(rule.opCompare, firstCheck, secondCheck);
  }

  return firstCheck;
}

static bool outputDelayExpired(const programmableOutputRule& rule, const programmableio_channel_t& channel)
{
  return (rule.outputTimeLimit==0) || (channel.ioOutDelay >= rule.outputTimeLimit);
}

TESTABLE_STATIC bool applyOutputTimeLimit(const programmableOutputRule& rule, const programmableio_channel_t& channel, bool ruleActive)
{
  return ruleActive && !(rule.kindOfLimiting && (rule.outputTimeLimit != 0) && outputDelayExpired(rule, channel));
}

static bool updateChannelStatus(programmableio_channel_t& channel, const programmableOutputRule& rule, bool ruleActive)
{
  bool outputStatus = rule.outputInverted ^ ruleActive;
  if (rule.isPhysicalPin()) { 
    digitalWrite(rule.outputPin, outputStatus); 
  } else {
    channel.currentRuleStatus = outputStatus;
  }
  return outputStatus;
}

TESTABLE_STATIC uint8_t nextOutDelay(const statuses& current, uint8_t y, const programmableio_channel_t& channel, const programmableOutputRule& rule)
{
  if (rule.kindOfLimiting)
  {
    //Released before Maximum time, set delay to maximum to flip the output next
    if (BIT_CHECK(current.outputsStatus, y))
    {
      return rule.outputTimeLimit; 
    }
  
    return 0; //Reset the counter for next time
  }
  return channel.ioOutDelay;
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
    auto& channel = channels[y];
    if ( channel.pinIsValid )
    {
      programmableOutputRule rule(page13, y);
      if (applyOutputTimeLimit(rule, channel, isRuleActive(rule, getData)))
      {
        if (channels[y].ioDelay >= page13.outputDelay[y])
        {
          if (BIT_CHECK(current.outputsStatus, y) && !outputDelayExpired(rule, channel)) { channels[y].ioOutDelay++; }
          BIT_WRITE(current.outputsStatus, y, updateChannelStatus(channel, rule, true));
        }
        else { channels[y].ioDelay++; }
      }
      else
      {
        channel.ioOutDelay = nextOutDelay(current, y, channel, rule);
        if (outputDelayExpired(rule, channel))
        {
          if(!BIT_CHECK(page13.kindOfLimiting, y)) { channels[y].ioOutDelay = 0; }
          BIT_WRITE(current.outputsStatus, y, updateChannelStatus(channel, rule, false));
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