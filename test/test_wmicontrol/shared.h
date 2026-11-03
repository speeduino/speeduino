#pragma once

#include "src/controllers/vvt/vvtController.h"

struct test_context_t
{
    statuses current;
    pinNumbers_t pins;
    config4 page4;
    config6 page6;
    config10 page10;

    void initialise(void)
    {
        initialiseVvtWmi(current, pins, page4, page6, page10);
    }

    void wmiControl(void)
    {
        ::wmiControl(current, page10);
    }
};

test_context_t setup_wmi_tune(uint8_t mode);