#include "../test_utils.h"
#include "units.h"
#include "shared.h"

extern table2D_u8_u8_4 fanPWMTable;

constexpr uint8_t TEST_FAN_PIN  = 19U;

static test_context_t setup_default_tune(void)
{
  test_context_t context;

  context.pins.pinFan = TEST_FAN_PIN;

  context.page6.fanInv = 0U;
  context.page6.fanSP = temperatureAddOffset(80);   // ON above 80C
  context.page6.fanHyster = 5U;                      // OFF below 75C
  context.page2.fanEnable = FANMODE_OFF;
  context.page2.fanWhenOff = 0U;
  context.page2.fanWhenCranking = 0U;
  context.page15.airConTurnsFanOn = 0U;

  return context;
}

test_context_t setup_nopwm_tune(void)
{
    auto context = setup_default_tune();
    context.page2.fanEnable = FANMODE_ONOFF;
    return context;
}

test_context_t setup_pwm_tune(void)
{
    auto context = setup_default_tune();
    const uint8_t bins[] = { 0U,
         (uint8_t)((context.page6.fanSP - context.page6.fanHyster) - 1U), 
         context.page6.fanSP, 
         (uint8_t)((context.page6.fanSP + context.page6.fanHyster) + 1U)};
    const uint8_t values[] = {0, 75, 150, 200};
    populate_2dtable(&fanPWMTable, values, bins);
    context.page2.fanEnable = FANMODE_PWM;
    context.page6.fanFreq = 55;
    return context;
}
