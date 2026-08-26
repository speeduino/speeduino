#pragma once

#include "src/controllers/aircon/airconController.h"
#include "src/controllers/aircon/airconController_details.h"
#include "src/pins/outputPin.h"
#include "src/pins/inputPin.h"

extern airConController::details::state_t airConState;

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
void assert_ac_off_fan_on(const test_context &context);
test_context setup_ac_tune(void);