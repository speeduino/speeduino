#pragma once

#include "globals.h"

// Idle test pins; values are arbitrary free pins under ArduinoFake.
static constexpr uint8_t TEST_IDLE1_PIN = 90U;
static constexpr uint8_t TEST_IDLE2_PIN = 91U;

static void prepare_idle(uint8_t algorithm)
{
  pinNumbers.pinIdle1 = TEST_IDLE1_PIN;
  pinNumbers.pinIdle2 = TEST_IDLE2_PIN;
  configPage6.iacAlgorithm = algorithm;
  configPage6.iacChannels = 0U;
  configPage6.iacPWMdir = 0U;
  configPage6.iacPWMrun = 0U;
  configPage6.iacFastTemp = 0U;            // Skip ON branch in ON_OFF unless we set coolant low
  configPage6.idleKP = 100U;
  configPage6.idleKI = 50U;
  configPage6.idleKD = 0U;
  configPage6.iacStepTime = 1U;
  configPage9.iacCoolTime = 1U;
  configPage2.iacCLminValue = 0U;
  configPage2.iacCLmaxValue = 100U;
  configPage2.idleUpAdder = 0U;
  currentStatus.coolant = 80;
  currentStatus.idleUpActive = false;
  currentStatus.CLIdleTarget = 80U;
}
