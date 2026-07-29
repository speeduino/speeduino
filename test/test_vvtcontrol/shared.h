#pragma once

#include "table3d_typedefs.h"

static constexpr uint8_t TEST_VVT1_PIN = 19U;
static constexpr uint8_t TEST_VVT2_PIN = 20U;

void setup_vvt_openloop_tune(uint8_t loadSource, bool vvt2Enabled, bool wmiEnabled);
void setup_vvt_onoff_tune(uint8_t loadSource, bool vvt2Enabled, bool wmiEnabled);
void setup_vvt_closedloop_tune(uint8_t loadSource, bool vvt2Enabled, bool wmiEnabled);
void populate_vvt_tables(table3d_value_t vvt1Value, table3d_value_t vvt2Value);