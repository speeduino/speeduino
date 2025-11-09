#pragma once

#include "globals.h"

// Gather all inputs to the calculation into one place
struct ComputePulseWidthsContext {
  config2 page2 = {};
  config6 page6 = {};
  config10 page10 = {};
  statuses current = {};
};

static inline ComputePulseWidthsContext getBasicPwContext(void) {
  ComputePulseWidthsContext context = {};
  context.current.nitrous_status = NITROUS_OFF;
  context.page10.stagingEnabled = false;
  context.page2.multiplyMAP = MULTIPLY_MAP_MODE_OFF;
  context.page2.includeAFR = false;
  context.page2.incorporateAFR = false; 
  context.page2.dutyLim = 100;
  context.page2.strokes = TWO_STROKE;
  context.current.revolutionTime = UINT16_MAX;
  context.current.nSquirts = 1;
  return context;
}
