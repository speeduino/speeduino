#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "units.h"
#include "shared.h"

extern table2D_u8_u8_4 fanPWMTable;

static void setup_default_tune(void)
{
  pinFan = TEST_FAN_PIN;

  configPage6.fanInv = 0U;
  configPage6.fanSP = temperatureAddOffset(80);   // ON above 80C
  configPage6.fanHyster = 5U;                      // OFF below 75C
  configPage2.fanEnable = 0U;
  configPage2.fanWhenOff = 0U;
  configPage2.fanWhenCranking = 0U;
  configPage15.airConTurnsFanOn = 0U;
}

void setup_nopwm_tune(void)
{
    setup_default_tune();
    configPage2.fanEnable = 1U;
}

void setup_pwm_tune(void)
{
    setup_default_tune();
    const uint8_t bins[] = { 0U,
         (uint8_t)((configPage6.fanSP - configPage6.fanHyster) - 1U), 
         configPage6.fanSP, 
         (uint8_t)((configPage6.fanSP + configPage6.fanHyster) + 1U)};
    const uint8_t values[] = {0, 0, 200, 200};
    populate_2dtable(&fanPWMTable, values, bins);
    configPage2.fanEnable = 2U;
}
