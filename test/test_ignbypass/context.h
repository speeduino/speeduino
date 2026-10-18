#pragma once

#include "src/pins/pinNumbers_t.h"
#include "src/controllers/ignBypass/ignBypassControl.h"

struct test_context_t
{
    statuses cur = {};
    config4 p4 = {};
    pinNumbers_t pins = {};

    void initialise(void)
    {
        initialiseIgnBypass(cur, p4, pins);
    }

    void setupValidIgnBypass(void)
    {
        p4.ignBypassEnabled = true;
        pins.pinIgnBypass = 5;
    }
};
