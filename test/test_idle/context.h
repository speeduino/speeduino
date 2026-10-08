#pragma once

#include "globals.h"

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

    void prepare_idle(uint8_t algorithm)
    {
        pins.pinIdle1 = TEST_IDLE1_PIN;
        pins.pinIdle2 = TEST_IDLE2_PIN;
        pins.pinIdleUp = TEST_IDLEUP_INPUT_PIN;
        pins.pinIdleUpOutput = TEST_IDLEUP_OUTPUT_PIN;
        page6.iacAlgorithm = algorithm;
        page6.iacChannels = 0U;
        page6.iacPWMdir = 0U;
        page6.iacPWMrun = 0U;
        page6.iacFastTemp = 0U;            // Skip ON branch in ON_OFF unless we set coolant low
        page6.idleKP = 100U;
        page6.idleKI = 50U;
        page6.idleKD = 0U;
        page6.iacStepTime = 1U;
        page9.iacCoolTime = 1U;
        page2.iacCLminValue = 0U;
        page2.iacCLmaxValue = 100U;
        page2.idleUpAdder = 0U;
        current.coolant = 80;
        current.idleUpActive = false;
        current.CLIdleTarget = 80U;
    }
};
