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

    void initialise(void)
    {
        ::initialiseAirCon(pins);
    }

    void control(void)
    {
        ::airConControl();
    }
};

void assert_ac_off(void);
test_context setup_ac_tune(void);