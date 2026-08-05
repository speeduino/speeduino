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

    void vvtControl(void)
    {
        ::vvtControl(current, page4, page6, page10);
    }
};

test_context_t setup_vvt_openloop_tune(uint8_t loadSource, bool vvt2Enabled, bool wmiEnabled);
test_context_t setup_vvt_onoff_tune(uint8_t loadSource, bool vvt2Enabled, bool wmiEnabled);
test_context_t setup_vvt_closedloop_tune(uint8_t loadSource, bool vvt2Enabled, bool wmiEnabled);
void populate_vvt_tables(table3d_value_t vvt1Value, table3d_value_t vvt2Value);
