#pragma once

#include <stdint.h>

/** @brief This constant represents no trigger edge */
static constexpr uint8_t TRIGGER_EDGE_NONE = 99;

/** @brief This structure represents a trigger interrupt */
struct interrupt_t
{
  using callback_t = void(*)(void);

  /** @brief The callback function to be called on interrupt */
  callback_t callback = nullptr;
  /** @brief The edge type for the interrupt. E.g. RISING, FALLING, CHANGE */
  uint8_t edge = TRIGGER_EDGE_NONE;

  /** @brief Attach the interrupt to a pin */
  void attach(uint8_t pin) const;

  /** @brief Detach the interrupt from a pin */
  void detach(uint8_t pin) const;

  bool isValid(void) const
  {
    return edge!=TRIGGER_EDGE_NONE && callback!=nullptr;
  }
};

/** @brief This structure represents a decoder configuration 
 * 
 * Create using decoder_builder_t
*/
struct decoder_t
{
  /** @brief The primary interrupt configuration - usually the crank trigger */
  interrupt_t primary;
  /** @brief The secondary interrupt configuration - usually the cam trigger */
  interrupt_t secondary;
  /** @brief The tertiary interrupt configuration - for decoders that use a 3rd input. E.g. VVT */
  interrupt_t tertiary;

  /// @{
  /** @brief The function to get the RPM */
  using getRPM_t = uint16_t(*)(void);
  getRPM_t getRPM;
  /// @}

  /// @{
  /** @brief The function to get the crank angle */
  using getCrankAngle_t = int16_t(*)(void);
  getCrankAngle_t getCrankAngle;
  /// @}

  /// @{
  /** @brief The function to set the end teeth for ignition calculations */
  using setEndTeeth_t = void(*)(void); 
  setEndTeeth_t setEndTeeth;
  /// @}

  /// @{
  /** @brief The function to reset the decoder. Called when the engine is stopped, or when the engine is started */
  using reset_t = void(*)(void);
  reset_t reset;
  /// @}

  /// @{
  /**
   * @brief The function to test if the engine is running
   * 
   * This is based on whether or not the decoder has detected a tooth recently
   * 
   * @param curTime The time in µS to use for the liveness check. Typically the result of a recent call to micros() 
   * @return true If the engine is turning
   * @return false If the engine is not turning
   */  
  using engine_running_t = bool(*)(uint32_t);
  engine_running_t isEngineRunning;
  /// @}
};
