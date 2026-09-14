#pragma once

#include "src/controllers/fan/fanController.h"

struct test_context_t
{
    statuses current = {};
    config2 page2 = {};
    config6 page6 = {};
    config15 page15 = {};
    pinNumbers_t pins = {};

    void initialise(void)
    {
        initialiseFan(current, page2, page6, pins);
    }

    void fanControl(void)
    {
        extern void fanControlCore(statuses &current, const config2 &page2, const config6 &page6, const config15 &page15);
        fanControlCore(current, page2, page6, page15);
    }
};

test_context_t setup_nopwm_tune(void);
test_context_t setup_pwm_tune(void);