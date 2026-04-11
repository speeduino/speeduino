#pragma once

#include <stdint.h>
#include "PidTuningParameters.h"

class integerPID
{
public:
  
  /** @brief Default construction */
  integerPID(void); 

  /** @name Configuration methods */
  ///@{

  /** @brief Set the output limits */
  void setOutputLimits(int32_t min, int32_t max); 

  /** @brief Set the controller set point */
  void setSetPoint(int32_t setpoint) { _setpoint = setpoint; } 
  
  /**
   * @brief Set the PID tuning parameters
   * 
   * @param pidParams P, I & D terms
   * @param nowMs Current time in milliseconds
   * @param minComputeInterval Minimum time that should elapse between computations (in milliseconds)
   */
  void setTunings(const PidTuningParameters &pidParams, uint32_t nowMs, uint16_t minComputeInterval);

  /** @brief (Optional) Set the feed forward term (predictive bias) */
  void setFeedForwardTerm(int32_t feedForwardTerm);

  ///@}
  
  /** @brief Activates the controller. Must be called before compute() will have any effect. */
  void activate(int32_t input);

  /**
   * @brief Compute the new output.
   *  
   * @param nowMs Current time in milliseconds
   * @param input The input value
   * @param pOutput A correction to be applied to the input; only valid when true is returned.
   * @return true if a calculation occurred, false otherwise 
   */
  bool compute(uint32_t nowMs, int32_t input, int32_t* pOutput);

  /** @brief (Optional) Reset the controller */
  void reset(int32_t input);

  /** @brief (Optional) Reset the integral term */
  void resetIntegeral(void);

private:

  PidTuningParameters _pidParams;

  int32_t _setpoint = 0;
  int32_t _feedForwardTerm = 0;

	uint32_t _lastTime = 0;
	int32_t _integralTerm = 0;
  int32_t _lastInput = 0;
  
	uint16_t _sampleTime = 0;
	int32_t _outMin = 0;
  int32_t _outMax = 0;
	bool _isActive = false;
};
