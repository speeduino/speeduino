#pragma once

#include <stdint.h>
#include "PidBase.h"

class integerPID_ideal : public PidBase
{
public:
  
  /** @brief Default construction */
  integerPID_ideal(void);

  /** @name Configuration methods */
  ///@{

  /** @brief Set the output limits */
  void setOutputLimits(uint8_t min, uint8_t max);

  /**
   * @brief Set the minimum time interval between computations
   * 
   * @param now Current time
   * @param minComputeInterval Interval between valid calls to compute(). Must be in same units as @p now
   */
  void setSampleTime(uint32_t now, uint16_t minComputeInterval) { 
    _sampleTime = minComputeInterval; 
    _lastTime = now-minComputeInterval;
  }
  
  /** @brief Set the PID parameters */
  using PidBase::setTunings;

  /** @brief Set the controller set point */
  void setSetPoint(uint16_t setpoint) { _setpoint = setpoint; }
  
  /** @brief (Optional) Set the sensitivity */
  void setSensitivity(uint16_t sensitivity) { _sensitivity = sensitivity>5000U ? 5000 : sensitivity; }
  
  /** @brief (Optional) Set the forward bias */
  void setFeedForwardTerm(uint16_t feedForwardTerm) { _feedForwardTerm = feedForwardTerm; }
	///@}

  /**
   * @brief Initialize the controller
   * 
   * @param input The first input value (same as the compute() input parameter)
   */
	void initialize(uint16_t input);

  /**
   * @brief Compute the new output.
   * 
   * @note The output is not a correction to be applied to the input.
   * It is the actual output to be applied to the physical system.
   *  
   * @param now Current time (same units as used for setSampleTime)
   * @param input The input value
   * @param pOutput The new output: only valid when true is returned.
   * @return true if a calculation occurred, false otherwise 
   */
  bool compute(uint32_t now, uint16_t input, uint16_t* pOutput);
  
private:

  uint16_t _setpoint = 0;
  uint16_t _sensitivity = 0;
  uint16_t _sampleTime = 0; 
  uint16_t _feedForwardTerm = 0;

	uint32_t _lastTime = 0;
  uint16_t _lastInput = 0;
};
