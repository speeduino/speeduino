#pragma once

#include "src/controllers/aircon/airconController.h"
#include "src/pins/outputPin.h"
#include "src/pins/inputPin.h"

extern bool acIsEnabled;
extern bool acStandAloneFanIsEnabled;
extern uint8_t acStartDelay;
extern uint8_t acTPSLockoutDelay;
extern uint8_t acRPMLockoutDelay;
extern uint8_t acAfterEngineStartDelay;
extern bool waitedAfterCranking;
extern outputPin_t aircon_comp_pin;
extern outputPin_t aircon_fan_pin;
extern inputPin_t aircon_req_pin;

struct test_context
{
    pinNumbers_t pins;
    statuses current;
    config15 page15;

    void initialise(void)
    {
        ::initialiseAirCon(current, page15, pins);
    }

    void control(void)
    {
        ::airConControl(current, page15);
    }
};

void assert_ac_off(const test_context &context);
test_context setup_ac_tune(void);