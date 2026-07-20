#pragma once

#include "src/pins/boardOutputPin.h"
#include "src/pins/boardInputPin.h"

extern bool acIsEnabled;
extern bool acStandAloneFanIsEnabled;
extern uint8_t acStartDelay;
extern uint8_t acTPSLockoutDelay;
extern uint8_t acRPMLockoutDelay;
extern uint8_t acAfterEngineStartDelay;
extern bool waitedAfterCranking;
extern boardOutputPin_t aircon_comp_pin;
extern boardOutputPin_t aircon_fan_pin;
extern fastInputPin_t aircon_req_pin;

constexpr uint8_t TEST_ACREQUEST_PIN = 11;
constexpr uint8_t TEST_ACCOMP_PIN = 12;

void assert_ac_off(void);
void setup_ac_tune(void);