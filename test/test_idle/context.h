#pragma once

#include "globals.h"
#include "src/controllers/idle/idleController_state.h"

// Idle test pins; values are arbitrary free pins under ArduinoFake.
static constexpr uint8_t TEST_IDLE1_PIN = 18U;
static constexpr uint8_t TEST_IDLE2_PIN = 19U;
static constexpr uint8_t TEST_IDLEUP_INPUT_PIN = 20U;
static constexpr uint8_t TEST_IDLEUP_OUTPUT_PIN = 21U;
struct context_t
{
    statuses &current;
    config2 &page2;
    config6 &page6;
    config9 &page9;
    config15 &page15;
    pinNumbers_t &pins;

    context_t(void)
    : current(currentStatus)
    , page2(configPage2)
    , page6(configPage6)
    , page9(configPage9)
    , page15(configPage15)
    , pins(pinNumbers)
    {
        currentStatus = statuses();
        configPage2 = config2();
        configPage6 = config6();
        configPage9 = config9();
        configPage15 = config15();
        pinNumbers = pinNumbers_t();
    }

    void prepare_idle(uint8_t algorithm);

    void init(void);

    uint32_t idleControl(uint32_t startTime, uint32_t increment, uint8_t stepCount);
};

struct idle_micros_override_t
{
  idleController::detail::fnCurMicros_t original;

  idle_micros_override_t(void);
  ~idle_micros_override_t(void);

  static unsigned long mockIdleMicrosValue;
  static unsigned long mockIdleMicros(void)
  {
    return mockIdleMicrosValue;
  }
};
