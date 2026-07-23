#pragma once

#include "timers.h"

void setup_oneMsInterval(void);

static inline void run_n_intervals(unsigned n)
{
  for (unsigned i = 0U; i < n; ++i) { oneMSInterval(); }
}
