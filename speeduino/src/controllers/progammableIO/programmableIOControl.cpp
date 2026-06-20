/** @file
 * Custom Programmable I/O.
 * The config related to Programmable I/O is found on page13 (of type @ref config13).
 */
#include "programmableIOControl.h"
#include "programmableIOControl_details.h"
#include "../../../logger.h"
#include "../../../units.h"
#include "../../../unit_testing.h"

using namespace programmableIOControl_details;

TESTABLE_STATIC state_t state; // The current state of the programmable I/O system, including the status of each channel and its timers

void __attribute__((optimize("Os"))) initialiseProgrammableIO(const config13& page13)
{
  for (uint8_t y = 0; y < _countof(state_t::channels); ++y)
  {
    state.channels[y].initialize(page13, y);
  }
}

TESTABLE_INLINE_STATIC bool applyOutputTimeLimit(const channel_t& channel, bool ruleActive)
{
  return ruleActive && !(channel.hasMaxLimit() && channel.outputDelayExpired());
}

TESTABLE_INLINE_STATIC bool isRuleActive(const rule_t& rule, const channel_t &channel, getDataFn pGetData) noexcept
{
  return applyOutputTimeLimit(channel, rule.evaluate(state, pGetData));
}

static inline void updateChannelStatus(channel_t& channel, bool ruleActive) noexcept
{
  channel.isOutputActive = channel.isOutputInverted ? !ruleActive : ruleActive;
  if (channel.isPhysicalPin()) { 
    digitalWrite(channel.outputPin, channel.isOutputActive); 
  } else {
    channel.isRuleActive = channel.isOutputActive;
  }
}

static inline void processChannelActive(channel_t &channel)
{
  ++channel.activationDelayCount;
  if (channel.activationDelayExpired())
  {
    if (channel.isOutputActive && !channel.outputDelayExpired()) { ++channel.outputDelayCount; }
    updateChannelStatus(channel, true);
  }
}

TESTABLE_INLINE_STATIC uint8_t nextOutDelay(const channel_t& channel)
{
  if (channel.limitType==LimitingType::Max)
  {
    //Released before Maximum time, set delay to maximum to flip the output next
    if (channel.isOutputActive)
    {
      return channel.outputTimeLimit + 1; 
    }
  
    return 1; //Reset the counter for next time
  }
  return channel.outputDelayCount + 1;
}

static inline void processChannelInactive(channel_t &channel)
{
  channel.outputDelayCount = nextOutDelay(channel);
  if (channel.outputDelayExpired())
  {
    if(channel.limitType==LimitingType::Min) { channel.outputDelayCount = 0; }
    updateChannelStatus(channel, false);
  }

  channel.activationDelayCount = 0;
}

static inline void processChannel(channel_t &channel, const config13& page13, getDataFn pGetData)
{
  if ( channel.isPinValid )
  {
    rule_t rule(page13, channel._index);
    if (isRuleActive(rule, channel, pGetData))
    {
      processChannelActive(channel);
    }
    else
    {
      processChannelInactive(channel);
    }
  }
}

/** Check all (8) programmable I/O:s and carry out action on output pin as needed.
 * Compare 2 (16 bit) vars in a way configured by @ref cmpOperation (see also @ref config13.operation).
 * Use programmableIOGetData() to get 2 vars to compare.
 * Skip all programmable I/O:s where output pin is set 0 (meaning: not programmed).
 */
TESTABLE_INLINE_STATIC void programmableIOControl(const config13& page13, getDataFn pGetData)
{
  for (auto& channel: state.channels)
  {
    processChannel(channel, page13, pGetData);
  }
}

// LCOV_EXCL_START
void programmableIOControl(const config13& page13)
{
  programmableIOControl(page13, programmableIOGetData);
}
// LCOV_EXCL_STOP

/** Get single I/O data var (from current) for comparison.
 * @param index - Field index/number (?)
 * @return 16 bit (int) result
 */
TESTABLE_INLINE_STATIC int16_t programmableIOGetData(uint16_t index, byte (*pGetLogEntry)(uint16_t byteNum))
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
int16_t programmableIOGetData(uint16_t index)
{
  return programmableIOGetData(index, getTSLogEntry);
}

uint8_t getProgrammableIOOutputStatus(void)
{
    return state.compressedOutputStatus();
}

// LCOV_EXCL_STOP

