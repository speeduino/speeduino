#pragma once

#include "src/controllers/nitrous/nitrousController.h"

struct test_context_t
{
    statuses current;
    config10 page10;

    void init(void)
    {
        ::initialiseNitrous(current, page10);
    }

    void control(void)
    {
        extern void nitrousControlCore(statuses &current, const config10 &page10);
        nitrousControlCore(current, page10);
    }
};

test_context_t setup_n20_tune(uint8_t enableMode);
test_context_t setup_rpm_overlap_tune(uint8_t enableMode);