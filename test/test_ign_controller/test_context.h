#pragma once
#include "scheduler_ignition_controller.h"
#include "src/pins/pinNumbers_t.h"

struct test_context_t
{
    statuses current;
    config2 page2;
    config4 page4;
    config10 page10;
    config13 page13;
    pinNumbers_t pins;
};