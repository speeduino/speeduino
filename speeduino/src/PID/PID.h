#pragma once

#include <stdint.h>
#include "PidCore.h"

class PID
{
public:

  /** @brief Default construction */
  PID(void);

  /** @name Configuration methods */
  ///@{

  /** @brief Set the output limits */
  void setOutputLimits(int32_t min, int32_t max);

  /** @brief Set the PID parameters */
  void setTunings(PidTuningParameters params);

  /** @brief Set the controller set point */
  void setSetPoint(uint16_t setpoint) { _setpoint = setpoint; }
  ///@}

  /**
   * @brief Initialize/reset the controller
   * 
   * @param input Current input value (same value as passed into compute())
   */
	void resetIntegral(int32_t input);

  /**
   * @brief Compute the next correction.
   * 
   * @param input The input value
   * @return A correction to be applied to the input.
   */
  int32_t compute(int32_t input);

private:
  int16_t _setpoint = 0;
  int32_t _integralTerm;
  int32_t _lastInput;
  PidCore _pidCore;
};

